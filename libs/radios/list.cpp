#include "list.hpp"

#include <circularbuffertools.hpp>
#include <cstdio>
#include <cstring>
#include <random>

ListError List::try_consume_pre(int& eol) {
    eol = cbt_end_of_line(*buf);
    if (eol == -1)
        return ListError::NO_DATA;

    if (eol == 0) {
        // empty line
        try_consume_post(eol);
        return ListError::OK_IGNORE;
    }

    if (stations_found == stations_len) {
        puts("maxed out stations");
        try_consume_post(eol);
        return ListError::OK_ABORT;
    }

    return ListError::OK;
}

void List::try_consume_post(int eol) {
    buf->read_ack(eol + 1); // ack line + (\r or \n)
    if (*buf->read_ptr() == '\n')
        buf->read_ack(1);   // ack remaining line delimiter
}

void List::set_current_uuid(const char* p) {
    // printf("uuid: '%s'\n", p);
    strncpy(stations[stations_found].uuid, p, ST_UUID_LEN);
    stations[stations_found].uuid[ST_UUID_LEN] = '\0';
}

void List::set_current_name(const char* p) {
    //printf("name: '%s'\n", p);
    strncpy(stations[stations_found].name, p, ST_NAME_LEN);
    stations[stations_found].name[ST_NAME_LEN] = '\0';
}

void List::set_current_url(const char* p) {
    // printf("url: '%s'\n", p);
    strncpy(stations[stations_found].url, p, ST_URL_LEN);
    stations[stations_found].url[ST_URL_LEN] = '\0';
}

void List::select_random(station* ts) {
    const struct station* ss = &stations[random() % stations_found];

    strcpy(ts->url, ss->url);
}
