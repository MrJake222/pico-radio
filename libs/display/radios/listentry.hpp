#pragma once

#include <config.hpp>

class ListEntry {
    char name[ENT_NAME_LEN + 1];
    char url[ENT_URL_LEN + 1];

public:
    enum le_type {
        le_type_radio,  // radio stream
        le_type_local,  // local file
        le_type_dir     // local dir (not playable, but present on the list)
    };

    le_type type;

    // local-playback specific
    // entry url was already prepended with directory info
    bool dir_added;

    void set_name(const char* name_);
    void set_url(const char* url_);

    bool no_name() const;
    const char* get_name() const;
    const char* get_url() const;
};