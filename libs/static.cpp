#include "static.hpp"

static uint8_t raw_buf_arr[BUF_MP3_SIZE_BYTES + BUF_HIDDEN_MP3_SIZE_BYTES];
static volatile CircularBuffer cbuf(BUF_MP3_SIZE_BYTES, BUF_HIDDEN_MP3_SIZE_BYTES, raw_buf_arr);

static HttpClientPico http_client(cbuf);

static ListEntry entries[MAX_ENTRIES];
static ListEntry entries_pls[MAX_ENTRIES_PLS];

static lfs_t lfs;

volatile CircularBuffer& get_cbuf() {
    return cbuf;
}

HttpClientPico& get_http_client() {
    return http_client;
}

ListEntry* get_entries() {
    return entries;
}

ListEntry* get_entries_pls() {
    return entries_pls;
}

lfs_t* get_lfs() {
    return &lfs;
}