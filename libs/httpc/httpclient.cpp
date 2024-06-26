#include <cstdlib>
#include <httpclient.hpp>

static void str_to_lower(char* str) {
    for (int i=0; i<strlen(str); i++) {
        if (str[i] <= 'Z' && str[i] >= 'A')
            str[i] -= ('A' - 'a');
    }
}

int HttpClient::send_all(const char *buf, int buflen, bool more) {
    int sent_bytes = 0;

    while (sent_bytes < buflen) {
        int res = send(buf + sent_bytes, buflen - sent_bytes, more);
        if (res < 0) {
            return -1;
        }

        sent_bytes += res;
    }

    return 0;
}

int HttpClient::recv_all(char *buf, int buflen) {
    int recv_bytes = 0;

    while (recv_bytes < buflen) {
        int res = recv(buf + recv_bytes, buflen - recv_bytes);
        if (res < 0) {
            return -1;
        }

        recv_bytes += res;
    }

    return 0;
}

int HttpClient::send_string(const char *buf, bool more) {
    int buflen = strlen(buf);
    int ret = send_all(buf, buflen, more);

    if (ret < 0)
        return -1;

    return 0;
}

int HttpClient::split_host_path_port(const char *url) {

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
        if (host_len >= HTTP_HOST_MAX_LEN) {
            return -1;
        }

        size_t path_len = strlen(sep);
        if (path_len >= HTTP_PATH_MAX_LEN) {
            return -1;
        }

        memcpy(host, url, host_len);
        host[host_len] = 0;

        memcpy(path, sep, path_len + 1);
    }
    else {
        // no sep
        size_t host_len = strlen(url);
        if (host_len >= HTTP_HOST_MAX_LEN) {
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
        port = strtol(sep, &end, 10);
        if (*end) {
            return -1;
        }
    }
    else {
        port = 80;
    }

    printf("host: '%s', port: '%d', path: '%s'\n", host, port, path);

    return 0;
}

int HttpClient::test_for_http() {
    int ret = recv_all(buf_http, 4);
    if (ret < 0)
        return -1;

    if (strcmp(buf_http, "HTTP") != 0) {
        puts("no http response found");
        return -1;
    }

    return 0;
}

int HttpClient::parse_headers() {
    int r, len;
    while (true) {
        r = read_line(this, qrbuf, HTTP_QUERY_RESP_BUF_SIZE, &len);
        if (r < 0)
            return -1;
        if (len == 0)
            break;

        char* val = strchr(qrbuf, ':');
        if (!val)
            continue;

        *val++ = 0; // end name / skip separator
        if (*val == ' ')
            val += 1; // skip space

        char* div = strchr(val, ';');
        if (div)
            *div = 0; // trim any parameters, for ex. charset: "audio/scpls; charset=utf-8"

        str_to_lower(qrbuf);

        // names all lowercase
        if (strcmp(qrbuf, "location") == 0)
            strncpy(h_location, val, HTTP_LOCATION_HDR_SIZE);

        else if (strcmp(qrbuf, "content-type") == 0)
            strncpy(h_content_type, val, HTTP_CONTENT_TYPE_HDR_SIZE);

        else if (strcmp(qrbuf, "content-length") == 0)
            h_content_length = atoi(val);

        else if (strcmp(qrbuf, "icy-metaint") == 0)
            h_icy_metaint = atoi(val);
    }

    return 0;
}

int HttpClient::parse_http() {
    memcpy(qrbuf, buf_http, 4);
    int r, len;
    r = read_line(this, qrbuf + 4, HTTP_QUERY_RESP_BUF_SIZE - 4, &len);
    if (r < 0)
        return -1;

    qrbuf[4 + len] = 0;
    // puts(qrbuf);

    // http 4 + slash 1 + version 3 + space 1
    char* codestr = qrbuf + 9;
    *(codestr + 3) = 0;

    // code 3 + space 1
    char* codedesc = codestr + 4;

    // printf("code '%s' str '%s'\n", codestr, codedesc);

    char* end;
    int code = (int)strtol(codestr, &end, 10);
    if (*end)
        return -1;

    int res = parse_headers();
    if (res)
        return res;

    // TODO remember cookies

    // for (auto entry : headers) {
    //     printf("%20s   %s\n", entry.first.c_str(), entry.second.c_str());
    // }

    printf("code %d: ", code);
    switch (code) {
        case 200:
            // ok
            puts("ok");
            return 0;

        case 301: // moved permanently
        case 308: // permanent redirect
        case 302: // found
        case 307: // temporary redirect
            if (redirect_attempts < HTTP_MAX_REDIRECTS) {
                printf("redirect, attempt %d, location: %s\n", redirect_attempts+1, h_location);

                close();
                redirect_attempts++;
                return get(h_location);
            }
            else {
                puts("redirect, reached max attempts");
                return -1;
            }

        default:
            printf("unsupported response status code ('%s' / %s)\n", codestr, codedesc);
            return -1;
    }
}

int HttpClient::connect_url(const char* url) {
    reset_state();

    int res = split_host_path_port(url);
    if (res) {
        puts("split_host_path_port failed");
        return -1;
    }

    res = connect_to(host, port);
    if (res < 0) {
        puts("connect_to failed");
        return -1;
    }

    return 0;
}

int HttpClient::get(const char* url) {

    int res = connect_url(url);
    if (res < 0) {
        return -1;
    }

    // send GET request
    snprintf(qrbuf, HTTP_QUERY_RESP_BUF_SIZE, "GET %s HTTP/1.0\r\n", path);
    res = send_string(qrbuf, true);
    if (res < 0) {
        puts("send GET failed");
        return -1;
    }

    snprintf(qrbuf, HTTP_QUERY_RESP_BUF_SIZE, "Host: %s\r\n", host);
    res = send_string(qrbuf, true);
    if (res < 0) {
        puts("send header Host failed");
        return -1;
    }

    res = send_string("User-agent: PicoRadio/1.0\r\n", true);
    if (res < 0) {
        puts("send header User-agent failed");
        return -1;
    }

    if (send_icy_metadata) {
        res = send_string("Icy-MetaData: 1\r\n", true);
        if (res < 0) {
            puts("send header Icy-MetaData failed");
            return -1;
        }
    }

    // TODO use cookies
    res = send_string("\r\n", false);
    if (res < 0) {
        puts("send <empty-line> failed");
        return -1;
    }

    puts("connect ok, sent request");

    res = test_for_http();
    if (res < 0) {
        puts("not http response");
        return -1;
    }

    res = parse_http();
    if (res < 0) {
        puts("parse_http failed");
        return -1;
    }

    // parse_http consumed all headers
    // now only receiving content
    header_parsing_done();

    return 0;
}

int HttpClient::close() {
    return disconnect();
}

// bool HttpClient::has_header(const std::string& hdr) {
//     return headers.count(str_to_lower(hdr)) > 0;
// }
//
// const std::string& HttpClient::get_header(const std::string& hdr) {
//     return headers.at(str_to_lower(hdr));
// }
//
// int HttpClient::get_header_int(const std::string& hdr) {
//     return stoi(get_header(hdr));
// }
