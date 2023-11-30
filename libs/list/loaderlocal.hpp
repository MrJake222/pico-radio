#pragma once

#include <loader.hpp>

#include <lfsaccess.hpp>
#include <path.hpp>
#include <ff.h>

#define PATH_LEN   (FATFS_MAX_PATH_LEN)
#define BUF_LEN    (PATH_LEN)

class LoaderLocal : public Loader {

    DIR dir;
    FILINFO fileinfo;

    LfsAccess& acc;
    Path* path;

    // used in <check_entry_url> to update the path of the entry
    // to a concatenation of path and entry path
    char buf[PATH_LEN];

    int fatfs_list_dir();

    friend void llocal_res_cb(void* arg, const char* res);
    void task() override;
    void set_file(const char* path_, bool is_dir);

    // should the file be listed
    // eiter a dir or a supported file by player
    bool is_valid();

    int get_entry_count_whole() override;

public:
    LoaderLocal(ListEntry* entries_, int entries_max_,
                LfsAccess& acc_)
            : Loader(entries_, entries_max_)
            , acc(acc_)
    { }

    void begin(le_type type_, Path* path);

    int check_entry_url(int i) override;
};
