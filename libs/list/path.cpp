#include "path.hpp"

#include <cstring>

void Path::begin(const char* path_) {
    strncpy(path, path_, MAX_PATH_LEN);
}

char* Path::last_entered() {
    int slash_seen = 0;
    char* end;

    // iterate from end to begin
    // return pointer to second-last slash
    for (end = path+strlen(path); end != path; end--) {
        if (*end == '/')
            slash_seen++;

        // trailing + second-last
        if (slash_seen == 2)
            break;
    }

    // either because of break (second-last slash)
    // or end == start (top-level)
    return end;
}

int Path::go(const char* dirpath) {
    if (strlen(path) + strlen(dirpath) + 1 >= MAX_PATH_LEN)
        // buffer will overflow
        return -1;

    char* end = strrchr(path, '/');
    if (!end)
        // no separator found
        return -1;

    // skip separator
    end += 1;

    // same as strcpy but returns end pointer
    end = stpcpy(end, dirpath);
    // add trailing slash
    *end++ = '/';
    *end++ = '\0';

    return 0;
}

int Path::up() {
    if (strcmp(path, "/") == 0)
        // top-level directory
        return -1;

    // not top-level, will return second-last slash pointer
    char* slash = last_entered();

    // cut off at last dividing part (not counting trailing)
    slash++;       // skip slash
    *slash = '\0'; // trim right after

    return 0;
}
