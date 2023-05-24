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
    CircularBuffer http_buf;

    HttpClientPico()
        : HttpClient()
        , http_buf(HTTP_DATA_BUF_SIZE_BYTES, 0)
        { }
};

using argptr = volatile HttpClientPico*;