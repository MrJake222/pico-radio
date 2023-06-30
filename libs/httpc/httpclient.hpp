#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include <circularbuffer.hpp>
#include <config.hpp>

class HttpClient {

    static const int HTTP_HOST_MAX_LEN = 128;
    static const int HTTP_PATH_MAX_LEN = 256;

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

    static int split_host_path_port(const char* url, char* host, int host_maxlen, char* path, int path_maxlen, unsigned short* port);

    // if not http, these are returned higher
    char buf_http[5] = { 0 };
    int test_for_http();

    // Query response buffer
    const static int qrbuf_size = 1024;
    char qrbuf[qrbuf_size];

    std::map<std::string, std::string> headers;
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
    bool has_header(const std::string& hdr);
    const std::string& get_header(const std::string& hdr);
    int get_header_int(const std::string& hdr);
};