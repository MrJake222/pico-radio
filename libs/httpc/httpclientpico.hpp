#pragma once

#include <httpclient.hpp>

class HttpClientPico : public HttpClient {

    int send(const char* buf, int buflen) override;
    int recv(char* buf, int buflen) override;
    int connect_to(const char* host, unsigned short port) override;
    int disconnect() override;

    using HttpClient::HttpClient;

public:
    // to be used by callback functions
    struct tcp_pcb* pcb;
    volatile bool err;
    volatile bool connected;
};

using argptr = volatile HttpClientPico*;