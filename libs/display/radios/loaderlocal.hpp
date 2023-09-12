#pragma once

#include <loader.hpp>
#include <ff.h>


class LoaderLocal : public Loader {

    DIR dir;
    FILINFO fileinfo;

    char path[FATFS_MAX_PATH_LEN];

    void task() override;
    void set_file(const char* path_, bool is_dir);

public:
    LoaderLocal(ListEntry* entries_, int entries_count_)
            : Loader(entries_, entries_count_)
    { }

    void begin(const char* path_);

    int get_page_count() override;
};
