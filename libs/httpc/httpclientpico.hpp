#pragma once

#include <httpclient.hpp>

class HttpClientPico : public HttpClient {

    int send(const char* buf, int buflen) override;
    int recv(char* buf, int buflen) override;
    int connect_to(const char* host, unsigned short port) override;
    int disconnect() override;

    void connect_ok() override;

public:
    // to be used by callback functions
    struct tcp_pcb* pcb;

    volatile bool content;
    volatile CircularBuffer& http_buf;
    volatile CircularBuffer& cbuf;

    // status variables
    volatile bool err;
    volatile bool connected;

    void http_to_content() volatile;

    HttpClientPico(volatile CircularBuffer& http_buf_, volatile CircularBuffer& cbuf_)
        : HttpClient()
        , http_buf(http_buf_)
        , cbuf(cbuf_)
        { }

    void rx_ack(unsigned int bytes);
};

using argptr = volatile HttpClientPico*;