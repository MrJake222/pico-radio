#include "mp3radio.hpp"

const int BUF_MP3RADIO_RELOAD_FRAMES = 1;
const int BUF_MP3RADIO_RELOAD_BYTES = BUF_MP3RADIO_RELOAD_FRAMES * BUF_MP3RADIO_RELOAD_FRAMES;

void MP3Radio::open() {
    puts("open radio");
    client.get(filepath);
}

void MP3Radio::close() {
    client.close();
}

bool MP3Radio::low_on_data() {
    return false;
}

void MP3Radio::load_buffer(int bytes) {
    /*unsigned int read = client.more_data((char*)mp3_buf.write_ptr(), bytes);
    mp3_buf.write_ack(read);

    printf("loaded %d bytes  avail %ld\n", read, mp3_buf.data_left());*/
}
