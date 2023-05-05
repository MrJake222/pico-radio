#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <map>

#define HTTP_BUFSIZE 1024
static char buf[HTTP_BUFSIZE];

class HttpClient {


    // these are platform-specific
    // send as much as possible, return how many was sent
    virtual int send(const char* buf, int buflen) = 0;
    virtual int recv(char* buf, int buflen) = 0;
    virtual int connect_to(const char* host) = 0;
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

    int split_host_path(const char* url, char* host, int host_maxlen, char* path, int path_maxlen);

    // if not http, these are returned higher
    char buf_http[5] = { 0 };
    int buf_http_content = 0;
    int test_for_http();

    std::map<std::string, std::string> headers;
    int parse_headers();

    int parse_http();

public:

    // one concurrent connection supported
    // these start a connection
    int get(const char* url);

    // these will fetch more data from the connection opened
    // by methods above (returns number of bytes read)
    int more_data(char* buf, int buflen);

    // this will close the connection
    int close();
};