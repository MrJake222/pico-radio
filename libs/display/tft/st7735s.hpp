#pragma once

#include <hardware/spi.h>
#include <config.hpp>

#include <FreeRTOS.h>
#include <semphr.h>

class ST7735S {

    const int x_skip;
    const int y_skip;

    // pins
    // hardware spi
    const int p_sck; // clock
    const int p_tx;  // master out
    const int p_cs;  // chip select
    // software control pins
    const int p_rst; // reset
    const int p_dc;  // data/command
    const int p_bl;  // backlight
    void gpio_setup();

    spi_inst_t* const spi;
    void spi_setup();
    void spi_set_bits(int bits);

    const int DC_CMD = 0;
    const int DC_DATA = 1;
    void write_command8(uint8_t val);
    void write_data8(uint8_t val);
    // to be called before loops
    // with spi_write16_blocking inside
    void write_data16_prepare();
    // only to be called on singular value writes
    void write_data16(uint16_t val);

    // source
    // https://github.com/PaulStoffregen/Adafruit_ST7735
    void reset();
    void write_command_list(const uint8_t *addr);
    void module_init();
    // this also handles mutex lock/unlock
    void setup_write(uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end);
    void end_write();

    SemaphoreHandle_t& mutex_display;

public:
    const int W;
    const int H;

    ST7735S(int w_, int h_,
            int x_skip_, int y_skip_,
            spi_inst_t* spi_, int p_sck_, int p_tx_, int p_cs_,
            int p_rst_, int p_dc_, int p_bl_,
            SemaphoreHandle_t& mutex_display_)
        : W(w_), H(h_)
        , x_skip(x_skip_), y_skip(y_skip_)
        , spi(spi_)
        , p_sck(p_sck_) , p_tx(p_tx_), p_cs(p_cs_)
        , p_rst(p_rst_), p_dc(p_dc_), p_bl(p_bl_)
        , mutex_display(mutex_display_)
        { }

    int size() { return sizeof(ST7735S); }

    void init();

    static const int R_BITS = 5;
    static const int G_BITS = 6;
    static const int B_BITS = 5;
    static const int R_MAX = (1<<R_BITS) - 1;
    static const int G_MAX = (1<<G_BITS) - 1;
    static const int B_MAX = (1<<B_BITS) - 1;

    // all functions take normal
    // 24-bit RGB format as input
    uint16_t from_rgb(int rgb);
    void fill_rect(int x, int y, int w, int h, int bg);
    void fill_rect(int x, int y, int w, int h, bool fill_with_bg) = delete;
    void clear_screen(int bg);

    // may try to access 2 bytes (for unicode), returns number of bytes consumed
    // clip limits the width of the character by its value (cuts from the left or right)
    int write_char(int text_x, int text_y, const char* str, const struct font* font, int bg, int fg, int clip_left, int clip_right);

    // limits shown character to <min_x, max_x> window
    // text_x can be outside this window, the text will be clipped
    void write_text(int text_x, int text_y, const char *str, const struct font* font, int bg, int fg, int min_x, int max_x);
    // returns pointer to character after the text displayed
    const char* write_text_maxlen(int text_x, int text_y, const char* str, const struct font* font, int bg, int fg, int maxlen);
    void write_text_wrap(int text_x, int text_y, const char *str, const struct font* font, int bg, int fg);

    void draw_icon(int icon_x, int icon_y, struct icon icon, int bg, int fg);
};
