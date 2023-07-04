#include "buffers.hpp"

static uint8_t raw_buf_arr[BUF_MP3_SIZE_BYTES + BUF_HIDDEN_MP3_SIZE_BYTES];
static volatile CircularBuffer cbuf(BUF_MP3_SIZE_BYTES, BUF_HIDDEN_MP3_SIZE_BYTES, raw_buf_arr);

static HttpClientPico http_client(cbuf);

volatile CircularBuffer& get_cbuf() {
    return cbuf;
}

HttpClientPico& get_http_client() {
    return http_client;
}