#include "buffers.hpp"

static uint8_t raw_buf_arr[BUF_MP3_SIZE_BYTES + BUF_HIDDEN_MP3_SIZE_BYTES];
static volatile CircularBuffer raw_buf(BUF_MP3_SIZE_BYTES, BUF_HIDDEN_MP3_SIZE_BYTES, raw_buf_arr);

static uint8_t http_buf_arr[HTTP_DATA_BUF_SIZE_BYTES];
static volatile CircularBuffer http_buf(HTTP_DATA_BUF_SIZE_BYTES, 0, http_buf_arr);

volatile CircularBuffer& get_raw_buf() {
    return raw_buf;
}

volatile CircularBuffer& get_http_buf() {
    return http_buf;
}
