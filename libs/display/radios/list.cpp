#include "list.hpp"

#include <cstring>
#include <random>

ListError List::consume(DataSource* ds) {
    int len;

    len = read_line(ds, line, LIST_MAX_LINE_LENGTH);
    if (len == RL_OVERRUN) {
        // line buffer overrun, ignore
        return ListError::OK;
    }

    if (len == RL_ERROR) {
        return ListError::ERROR;
    }

    if (len == 0) {
        // empty line
        return ListError::OK;
    }

    if (stations_found == stations_len) {
        return ListError::ABORT;
    }

    // not empty line
    // not maxed out stations

    return consume_format(line);
}

int List::consume_all(DataSource* ds, volatile bool& abort, volatile bool& error) {
    while (ds->more_content()) {
        // loop until all content data has been read or aborted
        if (abort) {
            puts("rs: abort");
            break;
        }

        ListError lr = consume(ds);

        if (lr == ListError::ERROR) {
            puts("ds: error");
            return -1;
        }

        else if (lr == ListError::ABORT) {
            // buffer maxed out, don't waste more time
            puts("ds: maxed out stations");
            break;
        }

        if (error) {
            puts("ds: error");
            return -1;
        }
    }

    return 0;
}

void List::set_current_uuid(const char* p) {
    // printf("uuid: '%s'\n", p);
    strncpy(stations[stations_found].uuid, p, ST_UUID_LEN);
    stations[stations_found].uuid[ST_UUID_LEN] = '\0';
}

void List::set_current_name(const char* p) {
    // skip spaces
    while (*p == ' ') p++;

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

    // can't copy name because listpls doesn't parse names
    strcpy(ts->url, ss->url);
}
