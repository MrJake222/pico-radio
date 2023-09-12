#pragma once

#include <config.hpp>

class ListEntry {
    char name[ENT_NAME_LEN + 1];
    char url[ENT_URL_LEN + 1];

public:
    void set_name(const char* name_);
    void set_url(const char* url_);

    const char* get_name() const;
    const char* get_url() const;
};
