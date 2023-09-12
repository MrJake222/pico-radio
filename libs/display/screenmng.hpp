#pragma once

#include <screen.hpp>
#include <screens/scsearch.hpp>
#include <screens/scsearchres.hpp>
#include <screens/scplay.hpp>
#include <screens/scfavourites.hpp>
#include <screens/scbattery.hpp>
#include <screens/sclocal.hpp>

extern ScFavourites sc_fav;
extern ScSearch sc_search;
extern ScSearchRes sc_search_res;
extern ScPlay sc_play;
extern ScBattery sc_bat;
extern ScLocal sc_local;

void screenmng_init();
void screenmng_open(Screen* new_screen);
void screenmng_input(ButtonEnum input);
void screenmng_backlight(bool on);
void screenmng_show_error(const char* err);