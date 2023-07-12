#include <decodestream.hpp>

void lwip_err_cb(void* arg, int err) {
    // called from lwip callback
    ((DecodeStream*) arg)->notify_playback_end(true);
}

void DecodeStream::begin(const char* path_, Format* format_) {
    // client begin() before base class begin()
    // this resets cbuf callbacks
    client.begin();
    client.set_err_cb(lwip_err_cb, this);
    client.enable_icy_metadata();

    // this sets cbuf callbacks
    DecodeBase::begin(path_, format_);
}

int DecodeStream::play() {
    int r = client.get(path);
    if (r)
        return -1;

    return DecodeBase::play();
}

void DecodeStream::end() {
    client.close();
    DecodeBase::end();
}

void DecodeStream::ack_bytes(uint16_t bytes) {
    // TODO handle content-length
    client.rx_ack(bytes);
}
