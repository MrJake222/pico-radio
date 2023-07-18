#include <httpclientpico.hpp>

#include <lwip/tcp.h>
#include <lwip/dns.h>
#include <pico/cyw43_arch.h>

void gethost_callback(const char* name, const ip_addr_t* ipaddr, void* arg) {
    cyw43_arch_lwip_check();

    auto httpc = ((argptr)arg);

    if (ipaddr) {
        // printf("resolved callback %s (hex %08x)\n", ipaddr_ntoa(ipaddr), *ipaddr);
        memcpy((void*)&httpc->dns_addr, ipaddr, sizeof(ip_addr_t));
        httpc->dns_found = true;
    } else {
        puts("resolve failed");
        httpc->dns_failed = true;
    }

    httpc->notify(BIT_DNS);
}

err_t HttpClientPico::gethostbyname(const char* host) {
    cyw43_arch_lwip_check(); // only call this with lwip lock

    dns_found = false;
    dns_failed = false;

    int r;
    err_t ret = dns_gethostbyname_addrtype(host, (ip_addr_t*)&dns_addr, gethost_callback, this, LWIP_DNS_ADDRTYPE_IPV4);
    switch (ret) {
        case ERR_OK:
            // printf("resolved cache %s (hex %08x)\n", ipaddr_ntoa(result), *result);
            break;

        case ERR_INPROGRESS:
            // puts("in progress");
            r = wait(BIT_DNS);
            if (r < 0) {
                puts("wait error during dns resolve");
                return -1;
            }

            // puts("done");
            if (dns_failed) {
                puts("dns query failed");
                return -1;
            }

            if (!dns_found) {
                puts("dns query timeout");
                return -1;
            }

            break;

        case ERR_ARG:
            puts("dns query call failed");
            return -1;

        default:
            printf("dns query unknown error code %d\n", ret);
            return -1;
    }

    return 0;
}

void error_callback(void *arg, err_t err) {
    cyw43_arch_lwip_check();

    printf("error callback code %d\n", err);

    auto httpc = ((argptr)arg);
    httpc->err = true;
    httpc->pcb = nullptr;
    httpc->notify(BIT_ERROR);

    httpc->err_cb_call(err);
}

err_t connected_callback(void* arg, struct tcp_pcb* tpcb, err_t err) {
    cyw43_arch_lwip_check();

    // puts("connected callback");

    auto httpc = ((argptr)arg);
    httpc->notify(BIT_CONNECT);

    return err;
}

err_t recv_callback(void* arg, struct tcp_pcb* tpcb, struct pbuf* p_head, err_t err) {

    cyw43_arch_lwip_check();

    auto httpc = ((argptr)arg);

    if (!p_head) {
        // connection closed
        puts("recv callback p null - disconnected");
        return ERR_OK;
    }

    if (err != ERR_OK) {
        // some error occurred
        printf("recv callback error code %d\n", err);
        httpc->err = true;
        httpc->notify(BIT_ERROR);
        pbuf_free(p_head);
        return err;
    }

    struct pbuf* p = p_head;

    while(true) {
        // printf("recv cb received %d bytes (free http %ld mp3 %ld)\n", p->len, httpc->http_buf.space_left(), httpc->cbuf.space_left());

        if (httpc->cbuf.space_left() < p->len) {
            puts("end of buffer");
#if BUF_OVERRUN_PROTECTION
            httpc->err = true;
            return ERR_MEM;
#endif
        }

        httpc->cbuf.write((uint8_t*)p->payload, p->len);
        httpc->notify(BIT_RECV);

        httpc->bytes_rx_tx_since_poll += p->len;

        if (p->len == p->tot_len)
            break;

        p = p->next;
    }

    pbuf_free(p_head);
    return ERR_OK;
}

err_t sent_callback(void* arg, struct tcp_pcb* tpcb, u16_t len) {

    auto httpc = ((argptr)arg);
    httpc->bytes_rx_tx_since_poll += len;
    httpc->notify(BIT_SENT);

    return ERR_OK;
}

err_t poll_callback(void* arg, struct tcp_pcb* tpcb) {

    auto httpc = ((argptr)arg);
    // printf("poll bytes rx/tx %6d\n", httpc->bytes_rx_tx_since_poll);
    if (httpc->bytes_rx_tx_since_poll == 0) {
        puts("poll callback aborting stalling connection");
        tcp_abort(httpc->pcb);

        // this gets propagated as ABRT to error_callback
        return ERR_ABRT;
    }

    httpc->bytes_rx_tx_since_poll = 0;
    return ERR_OK;
}

int HttpClientPico::send(const char *buf, int buflen, bool more) {
    // printf("send %5d bytes: %p\n", buflen, buf);
    cyw43_arch_lwip_begin();

    err_t ret;
    bool senderr = false;
    int f = (more ? TCP_WRITE_FLAG_MORE : 0) | TCP_WRITE_FLAG_COPY;

    if (tcp_sndbuf(pcb) == 0) {
        int r = wait(BIT_SENT);
        if (r < 0) {
            puts("wait error in send");
            senderr = true;
            goto end;
        }
    }
    if (err) {
        puts("error during send");
        senderr = true;
        goto end;
    }

    buflen = MIN(buflen, tcp_sndbuf(pcb));

    ret = tcp_write(pcb, buf, buflen, f);
    if (ret != ERR_OK) {
        printf("tcp_write failed code %d\n", ret);
        senderr = true;
        goto end;
    }

    if (!more) {
        ret = tcp_output(pcb);
        if (ret != ERR_OK) {
            printf("tcp_output failed code %d\n", ret);
            senderr = true;
            goto end;
        }
    }

end:
    cyw43_arch_lwip_end();
    return senderr ? -1 : buflen;
}

int HttpClientPico::recv(char* buf, int buflen) {
    // printf("recv o %d read %d\n", stream_offset, stream_offset_read);
    cyw43_arch_lwip_begin();

    bool recverr = false;
    long len;

    if (cbuf.data_left() == 0) {
        int r = wait(BIT_RECV);
        if (r < 0) {
            puts("wait error in recv");
            recverr = true;
            goto end;
        }
    }
    if (err) {
        puts("error during recv");
        recverr = true;
        goto end;
    }

    if (cbuf.data_left() == 0) {
        // no data received, timeout
        recverr = true;
        goto end;
    }

    len = MIN(cbuf.data_left(), buflen);

    // printf("reading %d bytes from o_read %d\n", len, stream_offset_read);

    memcpy(buf, cbuf.read_ptr(), len);
    cbuf.read_ack(len);

    // caller should confirm received data by cbuf read callback
    // but sometimes the caller doesn't have access to cbuf
    if (!cbuf.is_read_ack_callback_set())
        tcp_recved(pcb, len);

end:
    cyw43_arch_lwip_end();
    return recverr ? -1 : len;
}

void HttpClientPico::reset_state() {
    HttpClient::reset_state();

    err = false;
    bytes_rx_tx_since_poll = 0;

    cbuf.reset_only_data();
}

void HttpClientPico::reset_state_with_cb() {
    HttpClient::reset_state_with_cb(); // calls reset state

    cbuf.reset_with_cb();
    err_cb = nullptr;
}

void HttpClientPico::set_err_cb(h_cb cb_, void* arg_) {
    err_cb = cb_;
    err_cb_arg = arg_;
}

void HttpClientPico::notify(int bit) volatile {
    if (task) {
        // printf("notify bit %d\n", bit);
        xTaskNotifyIndexed(task, HTTP_NOTIFY_INDEX, MAKE_NOTIF(bit), eSetBits);
    }
}

int HttpClientPico::wait(int bit) {
    // check if owner of the lock
    // the caller should've locked the lwip thread for whatever it's doing
    cyw43_arch_lwip_check();
    // check if no task already waiting
    assert(task == nullptr);

    // printf("wait bit %d\n", bit);
    task = xTaskGetCurrentTaskHandle();

    // use signed arithmetic (overflows handled)
    int start = (int) time_us_32();

    int ret;
    uint32_t val;
    do {
        // on both entry & exit clear bit
        cyw43_arch_lwip_end();

        xTaskNotifyWaitIndexed(HTTP_NOTIFY_INDEX,
                               0,
                               ULONG_MAX,
                               &val,
                               HTTP_TIMEOUT_MS / portTICK_PERIOD_MS);

        cyw43_arch_lwip_begin();
    } while (!HAS_NOTIF(val, bit) && NOT_TIMEOUT_US(start) && !err);

    if (err) {
        puts("error occurred while waiting for notification");
        ret = -1;
        goto end;
    }

    if (!HAS_NOTIF(val, bit)) {
        // notification not occurred
        puts("timeout waiting for notification");
        ret = -1;
        goto end;
    }

    // puts("wait end");
    // no error, notification occurred
    ret = 0;

end:
    task = nullptr;
    return ret;
}

int HttpClientPico::connect_to(const char *host, unsigned short port) {
    cyw43_arch_lwip_begin();

    int r;
    err_t ret;
    ret = gethostbyname(host);
    if (ret) {
        err = true;
        puts("gethostbyname failed");
        goto clean_up_failed;
    }

    printf("dns resolved: %s\n", ipaddr_ntoa((ip_addr_t*)&dns_addr));

    pcb = tcp_new();

    if (!pcb) {
        err = true;
        puts("tcp_new failed");
        goto clean_up_failed;
    }

    tcp_arg(pcb, this);
    tcp_err(pcb, error_callback);
    tcp_recv(pcb, recv_callback);
    tcp_sent(pcb, sent_callback);

    ret = tcp_connect(pcb, (ip_addr_t*)&dns_addr, port, connected_callback);
    if (ret != ERR_OK) {
        printf("connect failed code %d\n", ret);
        goto clean_up_failed;
    }

    r = wait(BIT_CONNECT);
    if (r < 0) {
        puts("wait error in connect");
        goto clean_up_failed;
    }

    // puts("connected");

    // start polling
    tcp_poll(pcb, poll_callback, HTTP_POLL_INTERVAL_MS / 500); // interval unit is 0.5s

    cyw43_arch_lwip_end();
    return 0;

clean_up_failed:
    cyw43_arch_lwip_end();
    return -1;
}

int HttpClientPico::disconnect() {
    cyw43_arch_lwip_begin();

    // pcb can be null after tcp_err fires
    if (!err) {
        err_t ret = tcp_close(pcb);

        if (ret != ERR_OK) {
            printf("tcp_close failed code %d\n", ret);
            cyw43_arch_lwip_end();
            return -1;
        }
    }

    puts("disconnected");

    cyw43_arch_lwip_end();
    return 0;
}

void HttpClientPico::rx_ack(uint16_t bytes) {
    // not using sketchy multiply techniques fixes a lot

    cyw43_arch_lwip_begin();
    tcp_recved(pcb, (uint16_t)bytes);
    cyw43_arch_lwip_end();
}
