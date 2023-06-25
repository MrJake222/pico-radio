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

void DecodeStream::raw_buf_read_msg(unsigned int bytes) {
    // TODO handle content-length
    DecodeBase::raw_buf_read_msg(bytes);
    client.rx_ack(bytes);
}
