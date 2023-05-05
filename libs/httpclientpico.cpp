#include "httpclientpico.hpp"

#include <lwip/tcp.h>
#include <lwip/dns.h>

struct dns_query {
    bool found = false;
    bool failed = false;
    ip_addr_t* addr = nullptr;
};

void gethost_callback(const char* name, const ip_addr_t* ipaddr, void* callback_arg) {
    auto query = (struct dns_query*) callback_arg;
    if (ipaddr) {
        printf("resolved callback %s (hex %08x)\n", ipaddr_ntoa(ipaddr), *ipaddr);
        memcpy(query->addr, ipaddr, sizeof(ip_addr_t));
        query->found = true;
    } else {
        query->failed = true;
    }
}

static err_t gethostbyname(const char* host, ip_addr_t* result) {

    volatile struct dns_query query;
    query.addr = result;

    err_t ret = dns_gethostbyname_addrtype(host, result, gethost_callback, (void *) &query, LWIP_DNS_ADDRTYPE_IPV4);
    switch (ret) {
        case ERR_OK:
            printf("resolved cache %s (hex %08x)\n", ipaddr_ntoa(result), *result);
            break;

        case ERR_INPROGRESS:
            puts("in progress");
            while (!query.found && !query.failed);
            puts("done");
            if (query.failed) {
                puts("dns query failed");
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




#define STREAM_BUFFER_SIZE (1024 * 4)
static uint8_t stream_buffer[STREAM_BUFFER_SIZE];
static volatile int stream_offset;
static int stream_offset_read;

void error_callback(void *arg, err_t err) {
    printf("error callback code %d\n", err);
    ((state_ptr)arg)->err = true;
}

err_t connected_callback(void* arg, struct tcp_pcb* tpcb, err_t err) {
    puts("connected callback");
    ((state_ptr) arg)->connected = true;

    return err;
}

err_t recv_callback(void* arg, struct tcp_pcb* tpcb, struct pbuf* p, err_t err) {

    if (!p || err != ERR_OK) {
        // some error occurred
        printf("recv callback error code %d\n", err);
        return err;
    }

    printf("recv callback, received %d bytes (o %d)\n", p->len, stream_offset);

    if (STREAM_BUFFER_SIZE - stream_offset < p->len) {
        puts("end of buffer");
        return ERR_MEM;
    }
    else {
        memcpy(stream_buffer + stream_offset, p->payload, p->len);
        stream_offset += p->len;
        tcp_recved(tpcb, p->len);

        // printf("recv done o %d\n", stream_offset);
    }

    pbuf_free(p);
    return ERR_OK;
}


int HttpClientPico::send(const char *buf, int buflen) {
    err_t ret;
    ret = tcp_write(pcb, buf, buflen, 0);
    if (ret != ERR_OK) {
        printf("tcp_write failed code %d\n", ret);
        return 0;
    }

    ret = tcp_output(pcb);
    if (ret != ERR_OK) {
        printf("tcp_output failed code %d\n", ret);
        return 0;
    }

    return buflen;
}

int HttpClientPico::recv(char* buf, int buflen) {
    // printf("recv o %d read %d\n", stream_offset, stream_offset_read);

    volatile int len;
    do {
        len = stream_offset - stream_offset_read;
    } while (len == 0);

    len = MIN(len, buflen);

    printf("reading %d bytes from o_read %d\n", len, stream_offset_read);

    memcpy(buf, stream_buffer + stream_offset_read, len);
    stream_offset_read += len;
    return len;
}

int HttpClientPico::connect_to(const char *host) {
    ip_addr_t addr;
    err_t ret;
    ret = gethostbyname(host, &addr);
    if (ret) {
        puts("gethostbyname failed");
        return -1;
    }

    printf("connecting to: %s (hex %08x)\n", ipaddr_ntoa(&addr), addr);

    pcb = tcp_new();

    if (!pcb) {
        puts("tcp_new failed");
        return -1;
    }

    tcp_arg(pcb, (void *) &state);
    tcp_err(pcb, error_callback);
    tcp_recv(pcb, recv_callback);

    state.err = false;
    state.connected = false;
    ret = tcp_connect(pcb, &addr, 80, connected_callback);
    if (ret != ERR_OK) {
        printf("connect failed code %d\n", ret);
        return -1;
    }

    while (!state.err && !state.connected);
    if (state.err) {
        puts("connect failed");
        return -1;
    }

    puts("connected");
    stream_offset = 0;
    stream_offset_read = 0;

    return 0;
}

int HttpClientPico::disconnect() {
    return tcp_close(pcb);
}
