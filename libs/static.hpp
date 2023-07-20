#pragma once

/*
 * This file holds singleton instances of objects reused across the project
 */

#include <config.hpp>
#include <cstdint>
#include <circularbuffer.hpp>
#include <httpclientpico.hpp>
#include <list.hpp>
#include <lfs.h>

volatile CircularBuffer& get_cbuf();
HttpClientPico& get_http_client();

struct station* get_stations();
struct station* get_stations_pls();

lfs_t* get_lfs();