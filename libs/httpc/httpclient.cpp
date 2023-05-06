#include <cstdlib>
#include "httpclient.hpp"

int HttpClient::send_all(const char *buf, int buflen) {
    int sent_bytes = 0;

    while (sent_bytes < buflen) {
        int res = send(buf + sent_bytes, buflen - sent_bytes);
        if (res <= 0) {
            return sent_bytes;
        }

        sent_bytes += res;
    }

    return sent_bytes;
}

int HttpClient::recv_all(char *buf, int buflen) {
    int recv_bytes = 0;

    while (recv_bytes < buflen) {
        int res = recv(buf + recv_bytes, buflen - recv_bytes);
        if (res <= 0) {
            return recv_bytes;
        }

        recv_bytes += res;
    }

    return recv_bytes;
}

int HttpClient::send_string(const char *buf) {
    int buflen = strlen(buf);
    int sent = send_all(buf, buflen);

    return sent == buflen;
}

int HttpClient::recv_line(char *buf, int maxlen) {

    int line_length = 0;

    while (line_length+1 < maxlen) {
        recv_all(buf, 2);
        if (buf[0] == '\r' && buf[1] == '\n') {
            buf[0] = 0;
            return line_length;
        }

        if (buf[1] == '\r') {
            buf += 2;
            recv_all(buf, 1);
            if (buf[0] == '\n') {
                buf[-1] = 0;
                return line_length - 1;
            }
        }

        buf += 2;
        line_length += 2;
    }

    return line_length;
}

int HttpClient::split_host_path_port(const char *url, char *host, int host_maxlen, char *path, int path_maxlen, unsigned short* port) {

    if (strncmp(url, "http://", 7) == 0) {
        url += 7;
    }

    if (strncmp(url, "https://", 8) == 0) {
        url += 8;
    }

    char* sep = strchr(url, '/');
    if (sep) {
        // separator found
        size_t host_len = sep - url;
        if (host_len > host_maxlen) {
            return -1;
        }

        size_t path_len = strlen(sep);
        if (path_len > path_maxlen) {
            return -1;
        }

        memcpy(host, url, host_len);
        host[host_len] = 0;

        memcpy(path, sep, path_len + 1);
    }
    else {
        // no sep
        size_t host_len = strlen(url);
        if (host_len > host_maxlen) {
            return -1;
        }

        memcpy(host, url, host_len);
        host[host_len] = 0;

        strcpy(path, "/");
    }

    // separate port
    sep = strchr(host, ':');
    if (sep) {
        *sep = 0;
        // host is now null-terminated on separator

        sep += 1;
        // sep points to port
        char* end;
        *port = strtol(sep, &end, 10);
        if (*end) {
            return -1;
        }
    }
    else {
        *port = 80;
    }

    printf("host: '%s'\n", host);
    printf("port: '%d'\n", *port);
    printf("path: '%s'\n", path);

    return 0;
}

int HttpClient::test_for_http() {
    recv_all(buf_http, 4);
    if (strcmp(buf_http, "HTTP") != 0) {
        puts("no http");
        buf_http_content = 4;
        return 0;
    }

    return 1;
}

int HttpClient::parse_headers() {
    headers.clear();

    while (1) {
        int len = recv_line(buf, HTTP_TMP_BUF_SIZE_BYTES);
        if (len == 0)
            break;

        char* sep = strchr(buf, ':');
        if (!sep)
            break;

        *sep = 0;
        sep += 1; // skip separator
        if (*sep == ' ')
            sep += 1; // skip space

        headers[buf] = sep;
    }

    return 0;
}

int HttpClient::parse_http() {
    memcpy(buf, buf_http, 4);

    int len = recv_line(buf+4, HTTP_TMP_BUF_SIZE_BYTES - 4);
    buf[4+len] = 0;
    puts(buf);

    // http 4 + slash 1 + version 3 + space 1
    char* codestr = buf + 9;
    *(codestr + 3) = 0;

    // code 3 + space 1
    char* codedesc = codestr + 4;

    printf("code '%s' str '%s'\n", codestr, codedesc);

    char* end;
    int code = (int)strtol(codestr, &end, 10);
    if (*end)
        return -1;

    int res = parse_headers();
    if (res)
        return res;

    // TODO remember cookies

    for (auto entry : headers) {
        printf("%20s   %s\n", entry.first.c_str(), entry.second.c_str());
    }

    printf("code %d\n", code);
    switch (code) {
        case 200:
            // ok
            puts("ok");
            // http stream ended
            // time to receive content
            content = true;
            return 0;

        case 301:
        case 302:
        case 307:
        case 308:
            // redirection
            puts("redirect");
            if (!headers.count("Location")) {
                puts("no location");
                return -1;
            }

            close();
            return get(headers["Location"].c_str());

        default:
            puts("unsupported response status code");
            return -1;
    }
}

int HttpClient::get(const char* url) {

    char host[HTTP_HOST_MAX_LEN];
    char path[HTTP_PATH_MAX_LEN];
    unsigned short port;

    int res = split_host_path_port(
            url,
            host, HTTP_HOST_MAX_LEN,
            path, HTTP_PATH_MAX_LEN,
            &port);

    if (res) {
        puts("split_host_path_port failed");
        return -1;
    }

    res = connect_to(host, port);
    if (res < 0) {
        puts("connect_to failed");
        return -1;
    }

    content = false;

    sprintf(buf, "GET %s HTTP/1.0\r\n", path);
    send_string(buf);
    sprintf(buf, "Host: %s\r\n", host);
    send_string(buf);
    // TODO use cookies
    send_string("\r\n");

    if (test_for_http()) {
        puts("http");
        res = parse_http();
        if (res) {
            puts("parse_http failed");
        }
    }

    return 0;
}

int HttpClient::close() {
    return disconnect();
}