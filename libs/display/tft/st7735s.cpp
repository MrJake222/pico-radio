#include <st7735s.hpp>
#include <st7735_init.h>

#include <icons.hpp>
#include <font.hpp>

#include <hardware/gpio.h>
#include <pico/time.h>
#include <cstring>

void ST7735S::gpio_setup() {
    gpio_init(p_rst);
    gpio_set_dir(p_rst, GPIO_OUT);
    gpio_put(p_rst, true);

    gpio_init(p_dc);
    gpio_set_dir(p_dc, GPIO_OUT);
    gpio_put(p_dc, true);

    gpio_init(p_bl);
    gpio_set_dir(p_bl, GPIO_OUT);
    gpio_put(p_bl, true);

    gpio_set_function(p_sck, GPIO_FUNC_SPI);
    gpio_set_function(p_tx, GPIO_FUNC_SPI);
    gpio_set_function(p_cs, GPIO_FUNC_SPI);
}

void ST7735S::bl_set(bool on) {
    gpio_put(p_bl, on);
}

void ST7735S::spi_setup() {
    spi_init(spi, 60 * 1000000);
    spi_set_slave(spi, false);
}

void ST7735S::spi_set_bits(int bits) {
    spi_set_format(spi, bits, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

void ST7735S::reset() {
    gpio_put(p_rst, false);
    sleep_us(100); // minimum pulse of 10us
    gpio_put(p_rst, true);

    // resetting
    sleep_ms(100);
}

void ST7735S::write_command8(uint8_t val) {
    spi_set_bits(8);
    gpio_put(p_dc, DC_CMD);
    spi_write_blocking(spi, &val, 1);
}

void ST7735S::write_data8(uint8_t val) {
    spi_set_bits(8);
    gpio_put(p_dc, DC_DATA);
    spi_write_blocking(spi, &val, 1);
}

void ST7735S::write_data16_prepare() {
    spi_set_bits(16);
    gpio_put(p_dc, DC_DATA);
}

void ST7735S::write_data16(uint16_t val) {
    write_data16_prepare();
    spi_write16_blocking(spi, &val, 1);
}

void ST7735S::write_command_list(const uint8_t *addr) {
    uint8_t  numCommands, numArgs;
    uint16_t ms;

    numCommands = *addr++;		        // Number of commands to follow
    while(numCommands--) {				// For each command...
        write_command8(*addr++);	        //   Read, issue command
        numArgs  = *addr++;	            //   Number of args to follow
        ms       = numArgs & DELAY;		//   If hibit set, delay follows args
        numArgs &= ~DELAY;			    //   Mask out delay bit
        while(numArgs--) {			    //   For each argument...
            write_data8(*addr++);         //   Read, issue argument
        }

        if(ms) {
            ms = *addr++;	            // Read post-command delay time (ms)
            if(ms == 255) ms = 500;		// If 255, delay for 500 ms
            sleep_ms(ms);
        }
    }
}

void ST7735S::module_init() {
    write_command_list(Rcmd1);
    write_command_list(Rcmd2green);
    write_command_list(Rcmd3);
}

void ST7735S::setup_write(uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end) {
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    write_command8(ST7735_CASET); // Column addr set
    write_data16(x_start + x_skip);   // XSTART
    write_data16(x_end   + x_skip - 1);   // XEND (no idea why only here -1 applies)
    write_command8(ST7735_RASET); // Row addr set
    write_data16(y_start + y_skip);   // YSTART
    write_data16(y_end   + y_skip);   // YEND
    write_command8(ST7735_RAMWR); // write to RAM
    write_data16_prepare();
}

void ST7735S::end_write() {
    xSemaphoreGive(mutex_display);
}

void ST7735S::init() {
    gpio_setup();
    spi_setup();

    reset();
    module_init();
}

uint16_t ST7735S::from_rgb(int rgb) {
    int r = (rgb >> (16+(8-R_BITS))) & R_MAX;
    int g = (rgb >> ( 8+(8-G_BITS))) & G_MAX;
    int b = (rgb >> (   (8-B_BITS))) & B_MAX;

    return (r << (G_BITS + B_BITS)) | (g << B_BITS) | b;
}

void ST7735S::fill_rect(int x, int y, int w, int h, int bg) {
    uint16_t color = from_rgb(bg);
    setup_write(x, y, x+w, y+h);
    for (int i=0; i<h*w; i++) {
        spi_write16_blocking(spi, &color, 1);
    }
    end_write();
}

void ST7735S::clear_screen(int bg) {
    fill_rect(0, 0, W, H, bg);
}

static int rgb_blend(unsigned int fg, unsigned int bg, unsigned int fg_alpha) {
    unsigned int bg_alpha = 256 - fg_alpha;
    fg_alpha += 1;

    return int (((((fg & 0xFF0000) * fg_alpha + (bg & 0xFF0000) * bg_alpha) >> 8) & 0xFF0000) +\
                ((((fg & 0x00FF00) * fg_alpha + (bg & 0x00FF00) * bg_alpha) >> 8) & 0x00FF00) +\
                ((((fg & 0x0000FF) * fg_alpha + (bg & 0x0000FF) * bg_alpha) >> 8) & 0x0000FF));
}

int ST7735S::write_char(int text_x, int text_y, const char* str, const struct font* font, int bg, int fg, int clip_left, int clip_right) {

    const uint8_t* chr_ptr = get_font_data_ptr(font, str);

    setup_write(text_x + clip_left,
                text_y,
                text_x + font->W - clip_right,
                text_y + font->H);

    for (int y=0; y<font->H; y++) {
        for (int x=clip_left; x<(font->W - clip_right); x++) {
            unsigned int alpha = *(chr_ptr + y*font->W + x);
            uint16_t color = from_rgb(rgb_blend(fg, bg, alpha));

            spi_write16_blocking(spi, &color, 1);
        }
    }

    end_write();
    return get_char_width(str);
}

void ST7735S::write_text(int text_x, int text_y, const char *str, const struct font* font, int bg, int fg, int min_x, int max_x) {
    while (*str) {
        const int  left_x = text_x;
        const int right_x = text_x + font->W;

        if (right_x < min_x) {
            // out of window on the left -> keep going
            str += get_char_width(str);
        }

        else if (left_x > max_x) {
            // out of window on the right
            // as we're only going forward, it's pointless to go further
            break;

        }

        else {
            // at least one corner in window

            const int clip_left  = MAX(0, min_x - left_x);
            const int clip_right = MAX(0, right_x - max_x);

            str += write_char(text_x, text_y, str, font, bg, fg, clip_left, clip_right);
        }

        text_x += font->W;
    }
}

const char* ST7735S::write_text_maxlen(int text_x, int text_y, const char* str, const struct font* font, int bg, int fg, int maxlen) {
    while (*str && maxlen--) {
        str += write_char(text_x, text_y, str, font, bg, fg, 0, 0);
        text_x += font->W;
    }

    return str;
}

void ST7735S::write_text_wrap(int text_x, int text_y, const char* str, const struct font* font, int bg, int fg) {
    const int chars_fitting = (W - text_x) / font->W;

    while (*str) {
        while (*str == ' ') {
            str++; // skip all beginning spaces
        }

        str = write_text_maxlen(text_x, text_y, str, font, bg, fg, chars_fitting);
        text_y += font->H; // newline
    }
}

void ST7735S::draw_icon(int icon_x, int icon_y, struct icon icon, int bg, int fg) {
    setup_write(icon_x, icon_y,
                icon_x + icon.w,
                icon_y + icon.h);

    for (int y=0; y<icon.h; y++) {
        for (int x=0; x<icon.w; x++) {
            unsigned int alpha = icon.data[y*icon.w + x];
            uint16_t color = from_rgb(rgb_blend(fg, bg, alpha));

            spi_write16_blocking(spi, &color, 1);
        }
    }

    end_write();
}


