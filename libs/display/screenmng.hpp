#pragma once

#include <screen.hpp>
#include <screens/scsearch.hpp>
#include <screens/scsearchres.hpp>

extern ScSearch sc_search;
extern ScSearchRes sc_search_res;

void screenmng_init();
Screen* screenmng_get_default();
