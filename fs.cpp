#include "fs.hpp"

#include <config.hpp>
#include <hardware/flash.h>
#include <hardware/sync.h>
#include <util.hpp>

#include <FreeRTOS.h>
#include <semphr.h>

#define FS_BASE_IN_FLASH (PICO_FLASH_SIZE_BYTES - LITTLEFS_SIZE)
#define FS_BASE_ABS      (XIP_NOCACHE_NOALLOC_BASE + FS_BASE_IN_FLASH)

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
static int pico_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(FS_BASE_IN_FLASH + block * FLASH_SECTOR_SIZE + off,
                        (const uint8_t*)buffer,
                        size);
    restore_interrupts(ints);
    return 0;
}

// Erase a block. A block must be erased before being programmed.
// The state of an erased block is undefined. Negative error codes
// are propagated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
static int pico_erase(const struct lfs_config *c, lfs_block_t block) {
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FS_BASE_IN_FLASH + block * FLASH_SECTOR_SIZE,
                      1);

    restore_interrupts(ints);
    return 0;
}

// Sync the state of the underlying block device. Negative error codes
// are propagated to the user.
static int pico_sync(const struct lfs_config *c) { return 0; }

// locking mechanisms for thread-safety
static SemaphoreHandle_t lfs_mutex;

int pico_freertos_lock(const struct lfs_config *c) {
    xSemaphoreTake(lfs_mutex, portMAX_DELAY);
    return 0;
}

int pico_freertos_unlock(const struct lfs_config *c) {
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
    create_mutex_give(lfs_mutex);
}
