#pragma once

#include <httpclient.hpp>
#include <lwip/tcp.h>

class HttpClientPico : public HttpClient {

    int send(const char* buf, int buflen) override;
    int recv(char* buf, int buflen) override;
    int connect_to(const char* host, unsigned short port) override;
    int disconnect() override;

    void header_parsing_done() override;

    // lwip structure
    struct tcp_pcb* pcb;

    // buffers
    // <content> decides where incoming data goes. By default, it is written to <http_buf> (<content> = false)
    // then after <connect_ok> remaining data is moved to <cbuf> and <content> is set to true
    volatile bool content;
    volatile CircularBuffer& http_buf;
    volatile CircularBuffer& cbuf;
    volatile CircularBuffer& get_buffer() volatile { return content ? cbuf : http_buf; }

    // moves all data from <http_buf> to <cbuf>
    void http_to_content();

    // status variables
    volatile bool err;
    volatile bool connected;

public:

    HttpClientPico(volatile CircularBuffer& http_buf_, volatile CircularBuffer& cbuf_)
        : HttpClient()
        , http_buf(http_buf_)
        , cbuf(cbuf_)
        { }

    void rx_ack(unsigned int bytes);

    // callbacks
    friend void error_callback(void *arg, err_t err);
    friend err_t connected_callback(void* arg, struct tcp_pcb* tpcb, err_t err);
    friend err_t recv_callback(void* arg, struct tcp_pcb* tpcb, struct pbuf* p_head, err_t err);
};

using argptr = volatile HttpClientPico*;