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

ListError ListM3U::produce_preamble(DataInterface* di, char* scratch) {
    int r;

    r = write_string(di, "#EXTM3U\n");
    if (r < 0)
        return ListError::ERROR;

    return ListError::OK;
}

ListError ListM3U::produce_format(DataInterface* di, char* scratch) {
    set_prev_station(); // predecrement (akin to how stack works)
    int r;

    sprintf(scratch, "#EXTINF:-1,%s\n", get_current_name());
    r = write_string(di, scratch);
    if (r < 0)
        return ListError::ERROR;

    sprintf(scratch, "%s\n", get_current_url());
    r = write_string(di, scratch);
    if (r < 0)
        return ListError::ERROR;

    return ListError::OK;
}
