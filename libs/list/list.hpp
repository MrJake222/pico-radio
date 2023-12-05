#pragma once

#include <config.hpp>
#include <datasource.hpp>
#include <listentry.hpp>

enum class ListError {
    ERROR,
    OK,
    ABORT,
};

class List {

    char line[LIST_MAX_LINE_LENGTH + 1];

    ListEntry* entries;
    int entries_len;
    int entries_found;

    ListError consume_line(DataSource* ds);
    virtual ListError consume_line_format(char* line) = 0;

protected:
    void set_current_name(const char* p);
    void set_current_url(const char* p);
    void set_next_entry();

public:
    virtual void begin(ListEntry* entries_, int entries_len_) {
        entries = entries_;
        entries_len = entries_len_;
        entries_found = 0;
    }

    int consume_all(DataSource* ds, volatile const bool& abort, volatile const bool& error);

    int get_entries_found() { return entries_found; }

    // select random station from this List instance and copy it into <ts>
    void select_random(ListEntry* ts);
};
