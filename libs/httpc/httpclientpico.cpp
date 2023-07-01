#include <httpclientpico.hpp>
#include <circularbuffer.hpp>

#include <lwip/tcp.h>
#include <lwip/dns.h>
#include <pico/cyw43_arch.h>

struct dns_query {
    bool found = false;
    bool failed = false;
    ip_addr_t* addr = nullptr;
};

void gethost_callback(const char* name, const ip_addr_t* ipaddr, void* callback_arg) {
    cyw43_arch_lwip_check();

    auto query = (struct dns_query*) callback_arg;
    if (ipaddr) {
        // printf("resolved callback %s (hex %08x)\n", ipaddr_ntoa(ipaddr), *ipaddr);
        memcpy(query->addr, ipaddr, sizeof(ip_addr_t));
        query->found = true;
    } else {
        puts("resolve failed");
        query->failed = true;
    }
}

// this is private, only called from connect_to
static err_t gethostbyname(const char* host, ip_addr_t* result) {

    volatile struct dns_query query;
    query.addr = result;

    err_t ret = dns_gethostbyname_addrtype(host, result, gethost_callback, (void *) &query, LWIP_DNS_ADDRTYPE_IPV4);
    switch (ret) {
        case ERR_OK:
            // printf("resolved cache %s (hex %08x)\n", ipaddr_ntoa(result), *result);
            break;

        case ERR_INPROGRESS:
            // puts("in progress");

            cyw43_arch_lwip_end();
            while (!query.found && !query.failed); // TODO refactor notification (+timeout)
            cyw43_arch_lwip_begin();

            // puts("done");
            if (query.failed) {
                puts("dns query failed");
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
    ((argptr)arg)->err = true;
}

err_t connected_callback(void* arg, struct tcp_pcb* tpcb, err_t err) {
    cyw43_arch_lwip_check();

    // puts("connected callback");
    ((argptr)arg)->connected = true;

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
        // printf("recv cb received %d bytes (free http %ld mp3 %ld)\n", p->len, httpc->http_buf.space_left(), httpc->content_buffer.space_left());

        if (httpc->content) {
            if (httpc->cbuf.space_left()<p->len) {
                puts("end of content buffer");
#if BUF_OVERRUN_PROTECTION
                httpc->err = true;
                return ERR_MEM;
#endif
            }

            httpc->cbuf.write((uint8_t*)p->payload, p->len);
        }

        else {
            if (httpc->http_buf.space_left() < p->len) {
                puts("end of http buffer");
#if BUF_OVERRUN_PROTECTION
                httpc->err = true;
                return ERR_MEM;
#endif
            }

            httpc->http_buf.write((uint8_t*)p->payload, p->len);
        }

        if (p->len == p->tot_len)
            break;

        p = p->next;
    }

    pbuf_free(p_head);
    return ERR_OK;
}

void HttpClientPico::connect_ok() {
    // blocks lwip thread to enter callbacks
    cyw43_arch_lwip_begin();

    content = true;
    http_to_content();

    cyw43_arch_lwip_end();
}

void HttpClientPico::http_to_content() volatile {
    if (http_buf.data_left() > 0) {
        if (cbuf.space_left()<http_buf.data_left()) {
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

int HttpClientPico::send(const char *buf, int buflen) {
    cyw43_arch_lwip_begin();

    err_t ret;
    ret = tcp_write(pcb, buf, buflen, 0);
    if (ret != ERR_OK) {
        printf("tcp_write failed code %d\n", ret);
        goto clean_up_failed;
    }

    ret = tcp_output(pcb);
    if (ret != ERR_OK) {
        printf("tcp_output failed code %d\n", ret);
        goto clean_up_failed;
    }

    cyw43_arch_lwip_end();
    return buflen;

clean_up_failed:
    cyw43_arch_lwip_end();
    return 0;
}

int HttpClientPico::recv(char* buf, int buflen) {
    // printf("recv o %d read %d\n", stream_offset, stream_offset_read);

    // probably not worth the refactor right now
    // it only serves for receiving response headers
    // (short wait time)
    while (http_buf.data_left() == 0);

    long len = MIN(http_buf.data_left(), buflen);

    // printf("reading %d bytes from o_read %d\n", len, stream_offset_read);

    memcpy(buf, http_buf.read_ptr(), len);
    http_buf.read_ack(len);

    cyw43_arch_lwip_begin();
    tcp_recved(pcb, len);
    cyw43_arch_lwip_end();

    return len;
}

int HttpClientPico::connect_to(const char *host, unsigned short port) {
    cyw43_arch_lwip_begin();

    ip_addr_t addr;
    err_t ret;
    ret = gethostbyname(host, &addr);
    if (ret) {
        puts("gethostbyname failed");
        goto clean_up_failed;
    }

    // printf("connecting to: %s (hex %08x)\n", ipaddr_ntoa(&addr), addr);

    pcb = tcp_new();

    if (!pcb) {
        puts("tcp_new failed");
        goto clean_up_failed;
    }

    tcp_arg(pcb, this);
    tcp_err(pcb, error_callback);
    tcp_recv(pcb, recv_callback);

    content = false;
    http_buf.reset_with_cb();

    err = false;
    connected = false;

    ret = tcp_connect(pcb, &addr, port, connected_callback);
    if (ret != ERR_OK) {
        printf("connect failed code %d\n", ret);
        goto clean_up_failed;
    }

    cyw43_arch_lwip_end();
    while (!err && !connected); // TODO refactor notification (+timeout)
    cyw43_arch_lwip_begin();

    if (err) {
        puts("connect failed");
        goto clean_up_failed;
    }

    // puts("connected");

    cyw43_arch_lwip_end();
    return 0;

clean_up_failed:
    cyw43_arch_lwip_end();
    return -1;
}

int HttpClientPico::disconnect() {
    cyw43_arch_lwip_begin();

    err_t ret = tcp_close(pcb);

    if (ret != ERR_OK) {
        printf("tcp_close failed code %d\n", ret);
        cyw43_arch_lwip_end();
        return -1;
    }

    cyw43_arch_lwip_end();
    return 0;
}

void HttpClientPico::rx_ack(unsigned int bytes) {
    int d = HTTP_CONTENT_BUFFER_TARGET - cbuf.health();
    int b_new = ((100 + d) * (int)bytes) / 100;

    // printf("[%ld us]try ack bytes %d\n", time_us_32(), bytes);

    cyw43_arch_lwip_begin();
    tcp_recved(pcb, (uint16_t)b_new);
    cyw43_arch_lwip_end();

    // printf("[%ld us]acked b %d\n", time_us_32(), b_new);
}
