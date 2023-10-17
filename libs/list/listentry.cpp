#include "listentry.hpp"

#include <cstring>

void ListEntry::set_name(const char* name_) {
    strncpy(name, name_, ENT_NAME_LEN);
}

void ListEntry::set_url(const char* url_) {
    strncpy(url, url_, ENT_URL_LEN);
}

bool ListEntry::no_name() const {
    // if name is empty string
    return name[0] == '\0';
}

const char* ListEntry::get_name() const {
    return no_name() ? url : name;
}

const char* ListEntry::get_url() const {
    return url;
}