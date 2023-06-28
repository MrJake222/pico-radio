#pragma once

#include <httpclient.hpp>

class HttpClientPico : public HttpClient {

    int send(const char* buf, int buflen) override;
    int recv(char* buf, int buflen) override;
    int connect_to(const char* host, unsigned short port) override;
    int disconnect() override;
    bool is_connection_closed() override { return connection_closed; }

public:
    // to be used by callback functions
    struct tcp_pcb* pcb;
    volatile bool err;
    volatile bool connected;
    volatile bool connection_closed;
    volatile CircularBuffer& http_buf;

    void http_to_content() volatile;

    HttpClientPico(volatile CircularBuffer& http_buf_)
        : HttpClient()
        , http_buf(http_buf_)
        { }

    void rx_ack(unsigned int bytes);
};

using argptr = volatile HttpClientPico*;