#include "scfavourites.hpp"

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
        if (i == 2)
            display.set_bg(COLOR_BG_SEL);
        else
            display.set_bg(COLOR_BG_DARK);

        display.fill_rect(5, 23 + 23*i, 150, 20, true);
        display.write_text(9, 25 + 23*i, tests[i], 1);
    }
}
