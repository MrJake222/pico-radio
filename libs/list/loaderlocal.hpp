#pragma once

#include <loader.hpp>

#include <lfsaccess.hpp>
#include <path.hpp>
#include <sdscan.hpp>

class LoaderLocal : public Loader {

    static constexpr const char* lfs_path = ".tmp_local";

    SDScan& scan;
    Path* path;

    friend void llocal_res_cb(void* arg, const char* res);
    void task() override;
    void set_file(const char* path_, bool is_dir);
    void setup_entry(ListEntry *ent) override { ent->type = le_type_local; }

    int get_entry_count_whole() override;

public:
    LoaderLocal(ListEntry* entries_, int entries_max_,
                SDScan& scan_)
            : Loader(entries_, entries_max_)
            , scan(scan_)
    { }

    void begin(Path* path);

    void load_abort() override;

    int check_entry_url(int i) override;
};
