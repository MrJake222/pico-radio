#include "radiosearch.hpp"

static void raw_buf_write_cb_static(void* arg, unsigned int bytes) {
    // called directly from lwip callback
    ((RadioSearch*) arg)->raw_buf_write_cb(bytes);
}

void RadioSearch::begin(volatile CircularBuffer* raw_buf_) {
    // TODO refactor into init/begin/start(search)
    raw_buf = raw_buf_;

    raw_buf->reset();
    raw_buf->set_write_ack_callback(this, raw_buf_write_cb_static);

    client.begin(raw_buf);
    client.get("http://de1.api.radio-browser.info/m3u/stations/search?codec=mp3&name=rmf");
}

void RadioSearch::raw_buf_write_cb(unsigned int bytes) {
    raw_buf->debug_read(bytes, 0);
    raw_buf->read_ack(bytes);
}
