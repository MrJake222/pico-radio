#include <decodestream.hpp>

void lwip_err_cb(void* arg, int err) {
    // called from lwip callback
    ((DecodeStream*) arg)->notify_playback_end(true);
}

void DecodeStream::begin(const char* path_, Format* format_) {
    DecodeBase::begin(path_, format_);

    client.begin();
    client.set_err_cb(lwip_err_cb, this);
}

int DecodeStream::play_() {
    int r = client.get(path);
    if (r)
        return -1;

    return 0;
}

int DecodeStream::stop() {
    int r = client.close();
    if (r)
        return -1;

    return DecodeBase::stop();
}

void DecodeStream::raw_buf_just_read(unsigned int bytes) {
    // TODO handle content-length
    DecodeBase::raw_buf_just_read(bytes);
    client.rx_ack(bytes);
}
