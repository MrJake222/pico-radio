#pragma once

#include <config.hpp>

enum le_type {
    le_type_radio,  // radio stream
    le_type_local,  // local file/dir
    le_type_const,  // const entry
    le_type_wifi    // wifi entry
};

// loader-local specific
struct le_local {
    // entry url was already prepended with directory info
    bool has_full_path;
    // is directory
    bool is_dir;
};

// loader-const specific
struct le_const {
    int idx;
};

// loader-wifi specific
struct le_wifi {
    int quality;
};

class ListEntry {

    // only these fields are saved
    char name[ENT_NAME_LEN + 1];
    char url[ENT_URL_LEN + 1];

public:
    le_type type;

    union {
        struct le_local llocal;
        struct le_const lconst;
        struct le_wifi  lwifi;
    };

    void set_name(const char* name_);
    void set_url(const char* url_);

    bool no_name() const;
    const char* get_name() const;
    const char* get_url() const;
};