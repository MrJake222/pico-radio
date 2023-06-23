#include <decodestream.hpp>

void DecodeStream::begin(const char* path_, Format* format_) {
    DecodeBase::begin(path_, format_);

    client.begin(&format_->raw_buf);
    client.get(path);
}

void DecodeStream::end() {
    DecodeBase::end();

    client.close();
}

void DecodeStream::raw_buf_read_cb(unsigned int bytes) {
    DecodeBase::raw_buf_read_cb(bytes);
    client.rx_ack(bytes);
}
