#pragma once

#include <config.hpp>
#include <httpclient.hpp>
#include <lwip/tcp.h>

#include <FreeRTOS.h>
#include <task.h>

#define HTTP_TIMEOUT_US         (HTTP_TIMEOUT_MS * 1000)
#define NOT_TIMEOUT_US(start)    ((((int) time_us_32()) - start) < HTTP_TIMEOUT_US)

typedef void(*h_cb)(void* arg, int err);

enum HttpNotificationBitEnum {
    BIT_CONNECT,
    BIT_ERROR,
    BIT_DNS,
    BIT_RECV,
    BIT_SENT
};

#define MAKE_NOTIF(bit)          (1 << (bit))
#define HAS_NOTIF(val, bit)      ((val) & MAKE_NOTIF(bit))

class HttpClientPico : public HttpClient {

    int send(const char* buf, int buflen, bool more) override;
    int recv(char* buf, int buflen) override;
    int connect_to(const char* host, unsigned short port) override;
    int disconnect() override;

    // lwip structure
    struct tcp_pcb* pcb;

    // content buffer
    volatile CircularBuffer& cbuf;

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
    TaskHandle_t task;
    // waits for notification
    // returns -1 on error or timeout
    int wait(int bit);
    void notify(int bit) volatile;

    // polling
    // when connection is broken (no RST, but no data received/transmitted)
    // polling callback will report an error (abort the connection, error callback gets ABRT)
    // this variable holds number of bytes communicated since last poll
    unsigned int bytes_rx_tx_since_poll;

protected:
    void reset_state() override;
    void reset_state_with_cb() override;

public:

    HttpClientPico(volatile CircularBuffer& cbuf_)
        : HttpClient()
        , cbuf(cbuf_)
        { }

    void rx_ack(uint16_t bytes);

    void set_err_cb(h_cb cb_, void* arg_);
    bool is_err() volatile { return err; }

    int already_read() override { return cbuf.read_bytes_total(); }

    // callbacks
    friend void gethost_callback(const char* name, const ip_addr_t* ipaddr, void* arg);
    friend void error_callback(void *arg, err_t err);
    friend err_t connected_callback(void* arg, struct tcp_pcb* tpcb, err_t err);
    friend err_t recv_callback(void* arg, struct tcp_pcb* tpcb, struct pbuf* p_head, err_t err);
    friend err_t poll_callback(void* arg, struct tcp_pcb* tpcb);
    friend err_t sent_callback(void* arg, struct tcp_pcb* tpcb, u16_t len);
};

using argptr = volatile HttpClientPico*;