#pragma once

#include <screen.hpp>
#include <screens/scsearch.hpp>
#include <screens/scsearchres.hpp>
#include <screens/scplay.hpp>

extern ScSearch sc_search;
extern ScSearchRes sc_search_res;
extern ScPlay sc_play;

void screenmng_init();
Screen* screenmng_get_default();
