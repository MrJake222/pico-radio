#pragma once

#include <screen.hpp>
#include <screens/scsearch.hpp>
#include <screens/scsearchres.hpp>
#include <screens/scplay.hpp>
#include <screens/scfavourites.hpp>

extern ScFavourites sc_fav;
extern ScSearch sc_search;
extern ScSearchRes sc_search_res;
extern ScPlay sc_play;

void screenmng_init();
void screenmng_input(ButtonEnum input);