#include "listpls.hpp"

#include <cstdio>
#include <cstring>

ListPLS listpls;

ListError ListPLS::try_consume() {
    int eol;
    ListError r = try_consume_pre(eol);
    if (r != ListError::OK)
        // propagate higher if not ok
        return r;

    // not empty line
    // not maxed out stations

    *buf->read_ptr_of(eol) = '\0';
    auto line = (const char*)buf->read_ptr();

    if (*buf->read_ptr() == '[') {
        // tag name
        // skip
        try_consume_post(eol);
        return ListError::OK;
    }

    if (strncmp(line, "File", 4) == 0) {
        // url
        char* val = strchr(line, '=');
        val++;

        set_current_url(val);
        stations_found++;
    }

    try_consume_post(eol);
    return ListError::OK;
}
