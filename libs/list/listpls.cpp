#include "listpls.hpp"

#include <cstring>

ListError ListPLS::consume_line_format(char* line) {

    if (line[0] == '[') {
        // tag name
        // skip
        return ListError::OK;
    }

    // TODO interpret names
    if (strncmp(line, "File", 4) == 0) {
        // url
        char* val = strchr(line, '=');
        val++;

        set_current_url(val);
        set_next_entry();
    }

    return ListError::OK;
}
