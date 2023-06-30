#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include <circularbuffer.hpp>
#include <config.hpp>

class HttpClient {

    // these are platform-specific
    // send as much as possible, return how many was sent
    virtual int send(const char* buf, int buflen) = 0;
    virtual int recv(char* buf, int buflen) = 0;
    virtual int connect_to(const char* host, unsigned short port) = 0;
    virtual int disconnect() = 0;
    // true if the connection was closed by the remote host
    virtual bool is_connection_closed() = 0;

    // returns number of bytes transferred
    // fails only on socket error
    // return value < buflen -> error
    int send_all(const char* buf, int buflen);
    int recv_all(char* buf, int buflen);

    // helper functions
    int send_string(const char* buf);
    // searches for \r\n, returns line+length without \r\n
    int recv_line(char* buf, int maxlen);

    char host[HTTP_HOST_MAX_LEN];
    char path[HTTP_PATH_MAX_LEN];
    unsigned short port;

    int split_host_path_port(const char* url);

    // if not http, these are returned higher
    char buf_http[5] = { 0 };
    int test_for_http();

    // Query/Response buffer
    char qrbuf[HTTP_QUERY_RESP_BUF_SIZE];

    int h_content_length;
    char h_content_type[HTTP_CONTENT_TYPE_HDR_SIZE];
    char h_location[HTTP_LOCATION_HDR_SIZE];
    int parse_headers();

    int parse_http();

    // prevents double-closing
    bool closed;

    // marks when data will be flowing into content_buffer
    // instead of http_buf
    volatile bool content;

public:
    // to be used by callback functions
    // for writing content data (not headers)
    volatile CircularBuffer* content_buffer;
    bool is_content() volatile const { return content; }

    HttpClient() { }

    void begin(volatile CircularBuffer* content_buffer_) {
        content_buffer = content_buffer_;
    }

    // one concurrent connection supported
    // this will start a connection
    int get(const char* url);

    // this will close the connection
    int close();

    // header access methods
    int get_content_length() { return h_content_length; }
    const char* get_content_type() { return h_content_type; }
};