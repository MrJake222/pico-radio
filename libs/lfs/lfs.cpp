#include "lfs.hpp"

#include <config.hpp>
#include <hardware/flash.h>
#include <hardware/sync.h>
#include <pico/multicore.h>
#include <util.hpp>

#include <FreeRTOS.h>
#include <semphr.h>
#include <static.hpp>

#define FS_BASE_IN_FLASH (PICO_FLASH_SIZE_BYTES - LITTLEFS_SIZE)
#define FS_BASE_ABS      (XIP_NOCACHE_NOALLOC_BASE + FS_BASE_IN_FLASH)

static bool core1_running;

// Read a region in a block. Negative error codes are propagated
// to the user.
static int pico_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
    memcpy(buffer,
           (const void*)(FS_BASE_ABS + block * FLASH_SECTOR_SIZE + off),
           size);

    return 0;
}

// Program a region in a block. The block must have previously
// been erased. Negative error codes are propagated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
// First lock out core1, then disable interrupts
static int pico_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    if (core1_running) multicore_lockout_start_blocking();
    uint32_t ints = save_and_disable_interrupts();

    flash_range_program(FS_BASE_IN_FLASH + block * FLASH_SECTOR_SIZE + off,
                        (const uint8_t*)buffer,
                        size);

    restore_interrupts(ints);
    if (core1_running) multicore_lockout_end_blocking();
    return 0;
}

// Erase a block. A block must be erased before being programmed.
// The state of an erased block is undefined. Negative error codes
// are propagated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
// First lock out core1, then disable interrupts
static int pico_erase(const struct lfs_config *c, lfs_block_t block) {
    if (core1_running) multicore_lockout_start_blocking();
    uint32_t ints = save_and_disable_interrupts();

    flash_range_erase(FS_BASE_IN_FLASH + block * FLASH_SECTOR_SIZE,
                      1);

    restore_interrupts(ints);
    if (core1_running) multicore_lockout_end_blocking();
    return 0;
}

// Sync the state of the underlying block device. Negative error codes
// are propagated to the user.
static int pico_sync(const struct lfs_config *c) { return 0; }

// locking mechanisms for thread-safety
static SemaphoreHandle_t lfs_mutex;

static int pico_freertos_lock(const struct lfs_config *c) {
    xSemaphoreTake(lfs_mutex, portMAX_DELAY);
    return 0;
}

static int pico_freertos_unlock(const struct lfs_config *c) {
    xSemaphoreGive(lfs_mutex);
    return 0;
}

static uint8_t buf[3][LITTLEFS_CACHES];

const struct lfs_config pico_lfs_config = {
    .context = nullptr,

    // block device operations
    .read  = pico_read,
    .prog  = pico_prog,
    .erase = pico_erase,
    .sync  = pico_sync,

    // thread-safe
    .lock = pico_freertos_lock,
    .unlock = pico_freertos_unlock,

    // block device configuration
    .read_size = FLASH_PAGE_SIZE,
    .prog_size = FLASH_PAGE_SIZE,
    .block_size = FLASH_SECTOR_SIZE,

    .block_count = LITTLEFS_SIZE / FLASH_SECTOR_SIZE,

    // wear-levelling
    .block_cycles = 500,

    // caches
    .cache_size = LITTLEFS_CACHES,
    .lookahead_size = LITTLEFS_CACHES,
    .read_buffer = buf[0],
    .prog_buffer = buf[1],
    .lookahead_buffer = buf[2],
};

void pico_lfs_init() {
    core1_running = false;
    create_mutex_give(lfs_mutex);
}

void pico_lfs_mount_format() {
    int r;
    for (int i=0; i<2; i++) {
        r = lfs_mount(get_lfs(), &pico_lfs_config);
        if (r == 0)
            break;

        printf("littlefs: failed to mount err=%d\n", r);
        if (r == LFS_ERR_CORRUPT) {
            puts("littlefs: formatting");
            lfs_format(get_lfs(), &pico_lfs_config);
        }
    }

    if (r == 0)
        puts("littlefs: mount ok");
    else
        puts("littlefs: failed to mount, giving up.");
}

void pico_lfs_launch_core1(entry_fn entry) {
    pico_freertos_lock(nullptr);
    core1_running = true;
    multicore_launch_core1(entry);
    pico_freertos_unlock(nullptr);
}

void pico_lfs_reset_core1() {
    pico_freertos_lock(nullptr);
    core1_running = false;
    multicore_reset_core1();
    pico_freertos_unlock(nullptr);
}
