#pragma once

#include <hardware/spi.h>
#include <ubuntu_mono.hpp>

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

    void reset();
    void write_command_list(const uint8_t *addr);
    void module_init();

    void setup_write(uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end);
    // void fillScreen(uint16_t color);
    // void fillRect(int x, int y, int w, int h, uint16_t color);

    int bg;
    int fg;

public:
    const int W;
    const int H;

    ST7735S(int w_, int h_,
            int x_skip_, int y_skip_,
            spi_inst_t* spi_, int p_sck_, int p_tx_, int p_cs_,
            int p_rst_, int p_dc_, int p_bl_)
        : W(w_), H(h_)
        , x_skip(x_skip_), y_skip(y_skip_)
        , spi(spi_)
        , p_sck(p_sck_) , p_tx(p_tx_), p_cs(p_cs_)
        , p_rst(p_rst_), p_dc(p_dc_), p_bl(p_bl_)
        { }

    int size() { return sizeof(ST7735S); }

    void init();
    // normal 24-bit RGB format
    void set_bg(int bg_) { bg = bg_; }
    void set_fg(int fg_) { fg = fg_; }
    void set_bg_fg(int bg_, int fg_) {
        set_bg(bg_);
        set_fg(fg_);
    }

    static const int R_BITS = 5;
    static const int G_BITS = 6;
    static const int B_BITS = 5;
    static const int R_MAX = (1<<R_BITS) - 1;
    static const int G_MAX = (1<<G_BITS) - 1;
    static const int B_MAX = (1<<B_BITS) - 1;

    uint16_t from_rgb(int rgb);
    void fill_rect(int x, int y, int w, int h, bool fill_with_bg);
    void clear_screen();

    // may try to access 2 bytes (for unicode)
    // returns number of bytes consumed
    int write_char(int text_x, int text_y, const char* str, const struct font* font);

    // returns the pointer to byte after the text displayed
    const char* write_text(int text_x, int text_y, const char *str, const struct font* font);
    const char* write_text_maxlen(int text_x, int text_y, const char *str, const struct font* font, int maxlen);
    const char* write_text_wrap(int text_x, int text_y, const char *str, const struct font* font);

    void draw_icon(int icon_x, int icon_y, struct icon icon);
};
