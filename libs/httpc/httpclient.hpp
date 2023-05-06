#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include <../circularbuffer.hpp>
#include <../../config.hpp>

static char buf[HTTP_TMP_BUF_SIZE_BYTES];

class HttpClient {

    static const int HTTP_HOST_MAX_LEN = 128;
    static const int HTTP_PATH_MAX_LEN = 256;

    // these are platform-specific
    // send as much as possible, return how many was sent
    virtual int send(const char* buf, int buflen) = 0;
    virtual int recv(char* buf, int buflen) = 0;
    virtual int connect_to(const char* host, unsigned short port) = 0;
    virtual int disconnect() = 0;

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
    int buf_http_content = 0;
    int test_for_http();

    std::map<std::string, std::string> headers;
    int parse_headers();

    int parse_http();

    // marks when data will be flowing into content_buffer
    // instead of http_buf
    volatile bool content;

public:
    // to be used by callback functions
    volatile CircularBuffer& content_buffer;
    bool is_content() volatile const { return content; }

    HttpClient(volatile CircularBuffer& content_buffer_)
        : content_buffer(content_buffer_)
        {

        }

    // one concurrent connection supported
    // these start a connection
    int get(const char* url);

    // this will close the connection
    int close();

};