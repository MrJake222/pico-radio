#pragma once

#include <lfs.h>
extern const struct lfs_config pico_lfs_config;

void pico_lfs_init();
void pico_lfs_mount_format();

typedef void(*entry_fn)();

// used to safely launch core1
// and ensure correct flash access locking
void pico_lfs_launch_core1(entry_fn entry);
void pico_lfs_reset_core1();