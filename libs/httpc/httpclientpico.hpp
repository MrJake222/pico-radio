#pragma once

#include <httpclient.hpp>
#include <lwip/tcp.h>

#include <FreeRTOS.h>
#include <task.h>

typedef void(*h_cb)(void* arg, int err);

class HttpClientPico : public HttpClient {

    int send(const char* buf, int buflen, bool more) override;
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
    // dns
    volatile bool dns_found;
    volatile bool dns_failed;
    volatile ip_addr_t dns_addr;

    err_t gethostbyname(const char* host);

    // callbacks (for reporting errors/timeouts during receiving content data)
    void* err_cb_arg;
    h_cb err_cb;
    void err_cb_call(err_t err_code) volatile { if (err_cb) err_cb(err_cb_arg, err_code); }

    // used to notify the connection task
    // on dns query/connect complete/error
    TaskHandle_t current_task;
    void save_current_task_handle() { current_task = xTaskGetCurrentTaskHandle(); }
    void erase_task_handle() { current_task = nullptr; }
    void wait(bool unlock_lwip=true);
    void notify() volatile { if (current_task) xTaskNotifyGiveIndexed(current_task, HTTP_NOTIFY_INDEX); }

    // polling
    // when connection is broken (no RST, but no data received/transmitted)
    // polling callback will report an error
    // this variable holds number of bytes communicated since last poll
    unsigned int bytes_rx_tx_since_poll;

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
    bool is_err() volatile { return err; }

    // callbacks
    friend void gethost_callback(const char* name, const ip_addr_t* ipaddr, void* arg);
    friend void error_callback(void *arg, err_t err);
    friend err_t connected_callback(void* arg, struct tcp_pcb* tpcb, err_t err);
    friend err_t recv_callback(void* arg, struct tcp_pcb* tpcb, struct pbuf* p_head, err_t err);
    friend err_t poll_callback(void* arg, struct tcp_pcb* tpcb);
    friend err_t sent_callback(void* arg, struct tcp_pcb* tpcb, u16_t len);
};

using argptr = volatile HttpClientPico*;