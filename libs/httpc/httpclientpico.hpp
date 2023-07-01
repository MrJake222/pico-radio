#pragma once

#include <httpclient.hpp>
#include <lwip/tcp.h>

typedef void(*h_cb)(void* arg, int err);

class HttpClientPico : public HttpClient {

    int send(const char* buf, int buflen) override;
    int recv(char* buf, int buflen) override;
    int connect_to(const char* host, unsigned short port) override;
    int disconnect() override;

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

    // callbacks (for reporting errors/timeouts during receiving content data)
    void* err_cb_arg;
    h_cb err_cb;

protected:
    void header_parsing_done() override;

    void reset_state() override;
    void reset_state_with_cb() override;

public:

    HttpClientPico(volatile CircularBuffer& http_buf_, volatile CircularBuffer& cbuf_)
        : HttpClient()
        , http_buf(http_buf_)
        , cbuf(cbuf_)
        { }

    void rx_ack(unsigned int bytes);

    void set_err_cb(h_cb cb_, void* arg_);

    // callbacks
    friend void error_callback(void *arg, err_t err);
    friend err_t connected_callback(void* arg, struct tcp_pcb* tpcb, err_t err);
    friend err_t recv_callback(void* arg, struct tcp_pcb* tpcb, struct pbuf* p_head, err_t err);
};

using argptr = volatile HttpClientPico*;