#include "screenmng.hpp"

#include <config.hpp>
#include <st7735s.hpp>
#include <screens/scfavourites.hpp>
#include <screens/scsearch.hpp>
#include <screens/scsearchres.hpp>

static ST7735S display(
        160, 128,
        1, 2,
        LCD_SPI ? spi1 : spi0,
        LCD_SCK, LCD_TX, LCD_CS,
        LCD_RST, LCD_DC, LCD_BL);

// static ScFavourites sc_favourites(display);
ScSearch sc_search(display);
ScSearchRes sc_search_res(display);

void screenmng_init() {
    display.init();

    sc_search.begin("RMF", true);
    sc_search_res.begin("");
}

Screen* screenmng_get_default() {
    return &sc_search;
}
