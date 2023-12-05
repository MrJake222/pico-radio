#include "list.hpp"

#include <cstring>
#include <random>

ListError List::consume_line(DataSource* ds) {
    int r, len;

    r = read_line(ds, line, LIST_MAX_LINE_LENGTH, &len);
    if (r == RL_OVERRUN) {
        // line buffer overrun, ignore
        return ListError::OK;
    }

    if (r == RL_ERROR) {
        return ListError::ERROR;
    }

    if (len == 0) {
        // empty line
        return ListError::OK;
    }

    if (entries_found == entries_len) {
        return ListError::ABORT;
    }

    // not empty line
    // not maxed out entries

    return consume_line_format(line);
}

int List::consume_all(DataSource* ds, volatile const bool& abort, volatile const bool& error) {
    while (ds->more_content()) {
        // loop until all content data has been read or aborted
        if (abort) {
            puts("rs: abort");
            break;
        }

        ListError lr = consume_line(ds);

        if (lr == ListError::ERROR) {
            puts("ds: error");
            return -1;
        }

        else if (lr == ListError::ABORT) {
            // buffer maxed out, don't waste more time
            puts("ds: maxed out entries");
            break;
        }

        if (error) {
            puts("ds: error");
            return -1;
        }
    }

    return 0;
}

void List::set_current_name(const char* p) {
    // skip spaces
    while (*p == ' ') p++;

    //printf("name: '%s'\n", p);
    entries[entries_found].set_name(p);
}

void List::set_current_url(const char* p) {
    // printf("url: '%s'\n", p);
    entries[entries_found].set_url(p);
}

void List::set_next_entry() {
    // type is being set in Loader
    entries_found++;
}

void List::select_random(ListEntry* ts) {
    const ListEntry* ss = &entries[random() % entries_found];

    // can't copy name because listpls doesn't parse names
    ts->set_url(ss->get_url());
}
