#include "static.hpp"

static uint8_t raw_buf_arr[BUF_MP3_SIZE_BYTES + BUF_HIDDEN_MP3_SIZE_BYTES];
static volatile CircularBuffer cbuf(BUF_MP3_SIZE_BYTES, BUF_HIDDEN_MP3_SIZE_BYTES, raw_buf_arr);

static HttpClientPico http_client(cbuf);

static struct station stations[MAX_STATIONS];
static struct station stations_pls[MAX_STATIONS_PLS];

static lfs_t lfs;

volatile CircularBuffer& get_cbuf() {
    return cbuf;
}

HttpClientPico& get_http_client() {
    return http_client;
}

struct station* get_stations() {
    return stations;
}

struct station* get_stations_pls() {
    return stations_pls;
}

lfs_t* get_lfs() {
    return &lfs;
}