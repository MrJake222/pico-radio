#include <decodestream.hpp>

void DecodeStream::begin(const char* path_, Format* format_) {
    DecodeBase::begin(path_, format_);

    client.begin(&format_->raw_buf);
}

int DecodeStream::start() {
    int r = client.get(path);
    if (r)
        return r;

    return DecodeBase::start();
}

int DecodeStream::stop() {
    int r = client.close();
    if (r)
        return r;

    return DecodeBase::stop();
}

void DecodeStream::raw_buf_read_msg(unsigned int bytes) {
    // TODO handle content-length
    DecodeBase::raw_buf_read_msg(bytes);
    client.rx_ack(bytes);
}
