#pragma once

#include <screen.hpp>
#include <scsearch.hpp>
#include <scsearchres.hpp>
#include <scplay.hpp>
#include <scfavourites.hpp>
#include <scbattery.hpp>
#include <sclocal.hpp>
#include <scsettings.hpp>

#include <scwifipwd.hpp>

extern ScFavourites sc_fav;
extern ScSearch sc_search;
extern ScSearchRes sc_search_res;
extern ScPlay sc_play;
extern ScBattery sc_bat;
extern ScLocal sc_local;
extern ScSettings sc_settings;
extern ScWifiPwd sc_wifi_pwd;

void screenmng_init();
void screenmng_open(Screen* new_screen);
void screenmng_input(ButtonEnum input);
void screenmng_backlight(bool on);
void screenmng_show_error(const char* err);