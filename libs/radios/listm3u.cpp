#include "listm3u.hpp"

#include <cstdio>
#include <cstring>

ListM3U listm3u;

ListError ListM3U::try_consume() {
    int eol;
    ListError r = try_consume_pre(eol);
    if (r != ListError::OK)
        // propagate higher if not ok
        return r;

    // not empty line
    // not maxed out stations

    *buf->read_ptr_of(eol) = '\0';
    auto line = (const char*)buf->read_ptr();

    if (*buf->read_ptr() == '#') {
        // comment (metadata)
        line++; // skip #

        if (strcmp(line, "EXTM3U") == 0) {
            printf("header: '%s'\n", line);
        }
        else {
            char* val = strchr(line, ':');
            *val++ = '\0';

            if (strcmp(line, "RADIOBROWSERUUID") == 0) {
                set_current_uuid(val);
            }
            else if (strcmp(line, "EXTINF") == 0) {
                char* name = strchr(val, ',');
                *name++ = '\0';
                set_current_name(name);
            }
        }
    }
    else {
        // playback source (url)
        set_current_url(line);

        // url marks the end of station definition
        stations_found++;
    }

    try_consume_post(eol);
    return ListError::OK;
}
