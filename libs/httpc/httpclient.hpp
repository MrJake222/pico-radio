#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include <circularbuffer.hpp>
#include <config.hpp>
#include <util.hpp>

class HttpClient : public DataSource {

    // these are platform-specific
    // send/receive as much as possible, return how many was sent/received (at least one byte)
    // returns -1 on failure
    virtual int send(const char* buf, int buflen, bool more) = 0;
    virtual int recv(char* buf, int buflen) = 0;
    virtual int connect_to(const char* host, unsigned short port) = 0;
    virtual int disconnect() = 0;
    virtual b_type already_read() = 0;

    // returns number of bytes transferred
    // fails only on socket error
    // returns -1 on failure
    int send_all(const char* buf, int buflen, bool more);

    // helper functions
    int send_string(const char* buf, bool more);

    char host[HTTP_HOST_MAX_LEN];
    char path[HTTP_PATH_MAX_LEN];
    unsigned short port;

    int split_host_path_port(const char* url);

    // if not http, these are returned higher
    char buf_http[5] = { 0 };
    int test_for_http();

    // Query/Response buffer
    char qrbuf[HTTP_QUERY_RESP_BUF_SIZE];

    // request headers
    bool send_icy_metadata;

    // response headers
    // how many bytes the headers occupied
    // set after get() finishes
    int headers_length;

    int h_content_length;
    char h_content_type[HTTP_CONTENT_TYPE_HDR_SIZE];
    char h_location[HTTP_LOCATION_HDR_SIZE];
    int h_icy_metaint;
    int parse_headers();
    int parse_http();

    int connect_url(const char* url);

protected:
    virtual void header_parsing_done() { headers_length = (int) already_read(); }

    // always called before starting a connection
    virtual void reset_state() { h_icy_metaint = -1; }
    virtual void reset_state_with_cb() { reset_state(); send_icy_metadata = false; }

public:

    enum Error {
        ERROR = -1,
        OVERRUN = -2
    };

    void begin() { reset_state_with_cb(); }

    // request headers
    void enable_icy_metadata() { send_icy_metadata = true; }

    // one concurrent connection supported
    // this will start a connection
    int get(const char* url);

    // this will close the connection
    int close();

    // header access methods
    // raw headers length in bytes
    int get_headers_length() { return headers_length; }

    int get_content_length() { return h_content_length; }
    bool more_content() { return (already_read() - headers_length) < get_content_length(); }

    const char* get_content_type() { return h_content_type; }
    bool has_icy_metaint() { return h_icy_metaint != -1; }
    int get_icy_metaint() { return h_icy_metaint; }

    // data access methods

    // receive given number of bytes
    // receives <buflen> bytes and returns 0
    // or fails with return value -1
    int recv_all(char* buf, int buflen);

    // read exactly one character or fail with return code -1
    int read_char(char *chr) override { return recv(chr, 1); }
};