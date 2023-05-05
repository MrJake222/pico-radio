#pragma once

#include <httpclient.hpp>

struct pico_tcp_state {
    bool connected = false;
    bool err = false;
};

using state_ptr = struct pico_tcp_state*;

class HttpClientPico : public HttpClient {


    volatile struct pico_tcp_state state;
    struct tcp_pcb* pcb;

    int send(const char* buf, int buflen) override;
    int recv(char* buf, int buflen) override;
    int connect_to(const char* host) override;
    int disconnect() override;
};