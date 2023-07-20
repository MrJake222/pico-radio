#include "listm3u.hpp"

#include <cstdio>
#include <cstring>

ListM3U listm3u;

ListError ListM3U::consume_format(char* line) {
    if (line[0] == '#') {
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
        set_next_station();
    }

    return ListError::OK;
}
