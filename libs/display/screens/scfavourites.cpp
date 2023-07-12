#include "scfavourites.hpp"

#include <ubuntu_mono.hpp>

void ScFavourites::show() {
    Screen::show();

    const char* tests[] = {
            "RMF FM",
            "Złote przeboje",
            "Radio 357",
            "Radio 2223",
            "Radio Eska",
    };

    for (int i=0; i<5; i++) {
        int bg;

        if (i == 2)
            bg = COLOR_BG_SEL;
        else
            bg = COLOR_BG_DARK;

        display.fill_rect(5, 23 + 23*i, 150, 20, bg);
        add_scrolled_text_or_normal(9, 25 + 23*i, tests[i],
                                    ubuntu_font_get_size(UbuntuFontSize::FONT_16),
                                    bg, COLOR_FG,
                                    145 - 4*2);
    }
}
