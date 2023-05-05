#include "mp3radio.hpp"

const int BUF_MP3RADIO_RELOAD_FRAMES = 1;
const int BUF_MP3RADIO_RELOAD_BYTES = BUF_MP3RADIO_RELOAD_FRAMES * BUF_MP3RADIO_RELOAD_FRAMES;

void MP3Radio::open() {

}

void MP3Radio::close() {

}

bool MP3Radio::low_on_data() {
    return buffer_consumed_since_load() > BUF_MP3RADIO_RELOAD_BYTES;
}

void MP3Radio::load_buffer(int bytes) {

}
