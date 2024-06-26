#include <decodestream.hpp>
#include <pico/cyw43_arch.h>

void lwip_err_cb(void* arg, int err) {
    // called from lwip callback
    ((DecodeStream*) arg)->notify_playback_end(true);
}

void cbuf_write_cb(void* arg, unsigned int bytes) {
    // called from lwip callback
    auto dec = (DecodeStream*) arg;

    // read all ICY metadata chunks
    dec->icy_read_all();
}

int DecodeStream::icy_read() {
    int bytes = metadata_icy.read(cbuf);

    if (bytes > 0)
        // if something was read, ACK it
        ack_bytes(bytes);

    return bytes;
}

void DecodeStream::begin(const char* path_, Format* format_) {
    // client begin() before base class begin()
    // this resets cbuf callbacks
    client.begin();
    client.set_err_cb(lwip_err_cb, this);
    client.enable_icy_metadata();

    metadata_icy.begin();

    // this resets cbuf and sets callbacks
    DecodeBase::begin(path_, format_);
}

int DecodeStream::play() {
    int r = client.get(path);
    if (r)
        return -1;

    if (client.has_icy_metaint()) {

        r = metadata_icy.start(client.get_headers_length(),
                               client.get_icy_metaint());

        if (r < 0)
            return -1;

        // lock lwip to cleanly sanitize already received data and set
        // callback for further checks atomically
        cyw43_arch_lwip_begin();

        // read all ICY metadata chunks that already had been received
        icy_read_all();
        // set callback for further readings
        cbuf.set_write_ack_callback(this, cbuf_write_cb);

        cyw43_arch_lwip_end();
    }

    return DecodeBase::play();
}

void DecodeStream::end() {
    client.close();
    DecodeBase::end();
}

void DecodeStream::notify_abort() {
    client.try_abort();
    DecodeBase::notify_abort();
}

void DecodeStream::ack_bytes(uint16_t bytes) {
    // TODO handle content-length
    client.rx_ack(bytes);
}

int DecodeStream::get_meta_str(char* meta, int meta_len) {
    return metadata_icy.get_stream_title(meta, meta_len);
}