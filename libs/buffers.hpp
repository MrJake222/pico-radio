#pragma once

/*
 * This file holds singleton instances of objects reused across the project
 */

#include <config.hpp>
#include <cstdint>
#include <circularbuffer.hpp>
#include <httpclientpico.hpp>

volatile CircularBuffer& get_cbuf();
HttpClientPico& get_http_client();