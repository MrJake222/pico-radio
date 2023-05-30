#include "st7735s.hpp"
#include "st7735_init.h"

#include "ubuntu_mono.hpp"

#include <hardware/gpio.h>
#include <pico/time.h>

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
    write_command8(ST7735_CASET); // Column addr set
    write_data16(x_start + x_skip);   // XSTART
    write_data16(x_end   + x_skip - 1);   // XEND (no idea why only here -1 applies)
    write_command8(ST7735_RASET); // Row addr set
    write_data16(y_start + y_skip);   // YSTART
    write_data16(y_end   + y_skip);   // YEND
    write_command8(ST7735_RAMWR); // write to RAM
    write_data16_prepare();
}

void ST7735S::begin() {
    gpio_setup();
    spi_setup();

    reset();
    module_init();
}

uint16_t ST7735S::from_rgb(int rgb) {
    int r = (rgb >> (16+(8-R_BITS))) & R_MAX;
    int g = (rgb >> ( 8+(8-G_BITS))) & G_MAX;
    int b = (rgb >> (   (8-B_BITS))) & B_MAX;

    return (r << 11) | (g << 5) | b;
}

void ST7735S::fill_rect(int x, int y, int w, int h, uint16_t color) {
    setup_write(x, y, x+w, y+h);
    for (int i=0; i<h*w; i++) {
        spi_write16_blocking(spi, &color, 1);
    }
}

void ST7735S::fill_screen(uint16_t color) {
    fill_rect(0, 0, W, H, color);
}

static int rgb_blend(unsigned int fg, unsigned int bg, unsigned int fg_alpha) {
    unsigned int bg_alpha = 256 - fg_alpha;
    fg_alpha += 1;

    return int (((((fg & 0xFF0000) * fg_alpha + (bg & 0xFF0000) * bg_alpha) >> 8) & 0xFF0000) +\
                ((((fg & 0x00FF00) * fg_alpha + (bg & 0x00FF00) * bg_alpha) >> 8) & 0x00FF00) +\
                ((((fg & 0x0000FF) * fg_alpha + (bg & 0x0000FF) * bg_alpha) >> 8) & 0x0000FF));
}

void ST7735S::write_text(int text_x, int text_y, const char *str, int scale) {
    while (*str) {
        const uint8_t* chr_ptr;

        int index = try_map_utf8(str);
        if (index != -1) {
            // utf character
            chr_ptr = get_font_data_ptr(utf8_data, index);
            str += 2;
        }
        else {
            // ascii character (probably)
            index = try_map_ascii(str);
            chr_ptr = get_font_data_ptr(ascii_data, index);
            str += 1;
        }

        const unsigned int bg = 0xCCCCCC;
        const unsigned int fg = 0xFF0000;

        setup_write(text_x, text_y,
                    text_x + FONT_W*scale,
                    text_y + FONT_H*scale);

        for (int y=0; y<FONT_H; y++) {
            for (int i=0; i<scale; i++) {
                for (int x=0; x<FONT_W; x++) {
                    unsigned int alpha = *(chr_ptr + y*FONT_W + x);
                    uint16_t color = from_rgb(rgb_blend(fg, bg, alpha));

                    for (int j=0; j<scale; j++) {
                        spi_write16_blocking(spi, &color, 1);
                    }
                }
            }
        }

        text_x += FONT_W * scale;
    }
}


