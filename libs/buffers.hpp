#pragma once

/*
 * This file holds singleton buffers for manipulating HTTP streams
 * <http_buf> serves as a query response buffer and initially as a data buffer,
 * then data stream is redirected to <raw_buf>
 */

#include <config.hpp>
#include <cstdint>
#include <circularbuffer.hpp>

volatile CircularBuffer& get_raw_buf();
volatile CircularBuffer& get_http_buf();