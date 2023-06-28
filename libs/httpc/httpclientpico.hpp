#pragma once

#include <httpclient.hpp>

class HttpClientPico : public HttpClient {

    int send(const char* buf, int buflen) override;
    int recv(char* buf, int buflen) override;
    int connect_to(const char* host, unsigned short port) override;
    int disconnect() override;

public:
    // to be used by callback functions
    struct tcp_pcb* pcb;
    volatile bool err;
    volatile bool connected;
    volatile CircularBuffer& http_buf;

    HttpClientPico(volatile CircularBuffer& http_buf_)
        : HttpClient()
        , http_buf(http_buf_)
        { }

    void rx_ack(unsigned int bytes);
};

using argptr = volatile HttpClientPico*;