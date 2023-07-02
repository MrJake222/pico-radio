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

    httpc->notify();
}

err_t HttpClientPico::gethostbyname(const char* host) {
    cyw43_arch_lwip_check(); // only call this with lwip lock
    save_current_task_handle();

    err_t ret = dns_gethostbyname_addrtype(host, (ip_addr_t*)&dns_addr, gethost_callback, this, LWIP_DNS_ADDRTYPE_IPV4);
    switch (ret) {
        case ERR_OK:
            // printf("resolved cache %s (hex %08x)\n", ipaddr_ntoa(result), *result);
            break;

        case ERR_INPROGRESS:
            // puts("in progress");
            wait();

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
    httpc->notify();

    httpc->err_cb_call(err);
}

err_t connected_callback(void* arg, struct tcp_pcb* tpcb, err_t err) {
    cyw43_arch_lwip_check();

    // puts("connected callback");

    auto httpc = ((argptr)arg);
    httpc->connected = true;
    httpc->notify();

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
        pbuf_free(p_head);
        return err;
    }

    struct pbuf* p = p_head;

    while(true) {
        // printf("recv cb received %d bytes (free http %ld mp3 %ld)\n", p->len, httpc->http_buf.space_left(), httpc->cbuf.space_left());

        if (httpc->get_buffer().space_left() < p->len) {
            puts("end of buffer");
#if BUF_OVERRUN_PROTECTION
            httpc->err = true;
            return ERR_MEM;
#endif
        }

        httpc->get_buffer().write((uint8_t*)p->payload, p->len);
        if (!httpc->content)
            httpc->notify();

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

void HttpClientPico::header_parsing_done() {
    HttpClient::header_parsing_done();

    // blocks lwip thread to enter callbacks
    cyw43_arch_lwip_begin();

    content = true;
    http_to_content();

    cyw43_arch_lwip_end();
}

void HttpClientPico::http_to_content() {

    // needs to be locked not to mess up the recv_callback
    cyw43_arch_lwip_check();

    if (http_buf.data_left() > 0) {
        if (cbuf.space_left() < http_buf.data_left()) {
            puts("end of content buffer");
#if BUF_OVERRUN_PROTECTION
            err = true;
                return ERR_MEM;
#endif
        }

        printf("moving buffer: data %ld to -> free %ld\n", http_buf.data_left(), cbuf.space_left());
        http_buf.move_to(cbuf);
    }
}

int HttpClientPico::send(const char *buf, int buflen, bool more) {
    // printf("send %5d bytes: %p\n", buflen, buf);
    cyw43_arch_lwip_begin();

    int f = more ? TCP_WRITE_FLAG_MORE : 0;

    buflen = MIN(buflen, tcp_sndbuf(pcb));

    err_t ret;
    ret = tcp_write(pcb, buf, buflen, f);
    if (ret == ERR_MEM) {
        cyw43_arch_lwip_end();
        return 0;
    }
    if (ret != ERR_OK) {
        printf("tcp_write failed code %d\n", ret);
        cyw43_arch_lwip_end();
        return -1;
    }

    if (!more) {
        ret = tcp_output(pcb);
        if (ret != ERR_OK) {
            printf("tcp_output failed code %d\n", ret);
            cyw43_arch_lwip_end();
            return -1;
        }
    }

    cyw43_arch_lwip_end();
    return buflen;
}

int HttpClientPico::recv(char* buf, int buflen) {
    // printf("recv o %d read %d\n", stream_offset, stream_offset_read);
    save_current_task_handle();

    if (http_buf.data_left() == 0)
        wait(false);

    if (http_buf.data_left() == 0)
        // no data received, timeout
        return -1;

    long len = MIN(http_buf.data_left(), buflen);

    // printf("reading %d bytes from o_read %d\n", len, stream_offset_read);

    memcpy(buf, http_buf.read_ptr(), len);
    http_buf.read_ack(len);

    cyw43_arch_lwip_begin();
    tcp_recved(pcb, len);
    cyw43_arch_lwip_end();

    return len;
}

void HttpClientPico::reset_state() {
    HttpClient::reset_state();

    content = false;
    err = false;
    connected = false;
    bytes_rx_tx_since_poll = 0;
}

void HttpClientPico::reset_state_with_cb() {
    HttpClient::reset_state_with_cb(); // calls reset state

    http_buf.reset_with_cb();
    err_cb = nullptr;
}

void HttpClientPico::set_err_cb(h_cb cb_, void* arg_) {
    err_cb = cb_;
    err_cb_arg = arg_;
}

void HttpClientPico::wait(bool unlock_lwip) {
    save_current_task_handle();

    if (unlock_lwip) {
        // check if owner of the lock if unlocking requested
        cyw43_arch_lwip_check();
        cyw43_arch_lwip_end();
    }

    ulTaskNotifyTakeIndexed(HTTP_NOTIFY_INDEX, pdTRUE, HTTP_CONNECT_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (unlock_lwip) cyw43_arch_lwip_begin();

    erase_task_handle();
}

int HttpClientPico::connect_to(const char *host, unsigned short port) {
    cyw43_arch_lwip_begin();

    err_t ret;
    ret = gethostbyname(host);
    if (ret) {
        puts("gethostbyname failed");
        goto clean_up_failed;
    }

    printf("dns resolved: %s\n", ipaddr_ntoa((ip_addr_t*)&dns_addr));

    pcb = tcp_new();

    if (!pcb) {
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

    wait();

    if (err) {
        puts("connect failed");
        goto clean_up_failed;
    }

    if (!connected) {
        puts("not connected (timeout)");
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

void HttpClientPico::rx_ack(unsigned int bytes) {
    // int d = HTTP_CONTENT_BUFFER_TARGET - cbuf.health();
    // int b_new = ((100 + d) * (int)bytes) / 100;

    // printf("[%ld us]try ack bytes %d\n", time_us_32(), bytes);

    // not using sketchy multiply techniques fixes a lot

    cyw43_arch_lwip_begin();
    tcp_recved(pcb, (uint16_t)bytes);
    cyw43_arch_lwip_end();

    // printf("[%ld us]acked b %d\n", time_us_32(), b_new);
}
