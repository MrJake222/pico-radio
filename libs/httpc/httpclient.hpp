#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include <circularbuffer.hpp>
#include <config.hpp>

class HttpClient {

    // these are platform-specific
    // send/receive as much as possible, return how many was sent/received (at least one byte)
    // returns -1 on failure
    virtual int send(const char* buf, int buflen, bool more) = 0;
    virtual int recv(char* buf, int buflen) = 0;
    virtual int connect_to(const char* host, unsigned short port) = 0;
    virtual int disconnect() = 0;
    virtual int already_read() = 0;

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

    // how many bytes the headers occupied
    int headers_length;

    int h_content_length;
    char h_content_type[HTTP_CONTENT_TYPE_HDR_SIZE];
    char h_location[HTTP_LOCATION_HDR_SIZE];
    int parse_headers();
    int parse_http();

    int connect_url(const char* url);

protected:
    virtual void header_parsing_done() { headers_length = already_read(); }

    // always call before starting a connection
    virtual void reset_state() { }
    virtual void reset_state_with_cb() { reset_state(); }

public:

    enum Error {
        ERROR = -1,
        OVERRUN = -2
    };

    void begin() { reset_state_with_cb(); }

    // one concurrent connection supported
    // this will start a connection
    int get(const char* url);

    // this will close the connection
    int close();

    // header access methods
    int get_content_length() { return h_content_length; }
    bool more_content() { return (already_read() - headers_length) < get_content_length(); }

    const char* get_content_type() { return h_content_type; }

    // data access methods

    // receive given number of bytes
    // receives <buflen> bytes and returns 0
    // or fails with return value -1
    int recv_all(char* buf, int buflen);

    // receives whole line
    // searches for \r\n, returns line+length without \r\n (or -1 on failure)
    int recv_line(char* buf, int bufsize);
};