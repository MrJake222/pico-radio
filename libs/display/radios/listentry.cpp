#include "listentry.hpp"

#include <cstring>

void ListEntry::set_name(const char* name_) {
    strncpy(name, name_, ENT_NAME_LEN);
}

void ListEntry::set_url(const char* url_) {
    strncpy(url, url_, ENT_URL_LEN);
}

const char* ListEntry::get_name() const {
    // if name is empty string, return url
    return name[0] ? name : url;
}

const char* ListEntry::get_url() const {
    return url;
}
