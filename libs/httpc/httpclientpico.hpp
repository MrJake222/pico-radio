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
    CircularBuffer& http_buf;

    HttpClientPico(CircularBuffer& http_buf_)
        : HttpClient()
        , http_buf(http_buf_)
        { }
};

using argptr = volatile HttpClientPico*;