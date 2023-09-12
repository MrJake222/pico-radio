#include <lock_rtos.h>

#include <pico/types.h>
#include <pico/platform.h>
#include <pico/lock_core.h>
#include <hardware/timer.h>
#include <hardware/sync.h>

#include <FreeRTOS.h>
#include <task.h>
#include <event_groups.h>

#include <mcorefifo.hpp>
#include <cstdio>

static EventGroupHandle_t event_group_h = nullptr;
static volatile bool rtos_running = false;

/*
 * These functions may be called from RTOS core, non-RTOS core
 * or RTOS core but before the RTOS is running and even before main() was run (early mallocs/asserts)
 * use below function to determine whether RTOS is running.
 * Can be run in lockout state, use __force_inline to not cause flash reads
 */

#define rtos_core() (get_core_num() == FREERTOS_CORE)

__force_inline static uint get_lock_bit(uint lock_num) {
    // map higher-numbered locks to 0-15
    // make bitfield (16-bit)
    return 1u << (lock_num & 0x0F);
}

__force_inline static TickType_t until_to_ticks(absolute_time_t until) {
    return us_to_ms(absolute_time_diff_us(get_absolute_time(), until)) / portTICK_PERIOD_MS;
}

static void __not_in_flash_func(rtos_fifo_cb)(void* arg, uint32_t data) {
    // run from ISR
    xEventGroupSetBitsFromISR(
            event_group_h,
            get_lock_bit(data),
            nullptr);
}

void lock_rtos_init() {
    event_group_h = xEventGroupCreate();
    assert(event_group_h != nullptr);

    fifo_register(LOCK_RTOS, rtos_fifo_cb, nullptr, false);
    rtos_running = true;
}

static void __not_in_flash_func(freertos_wait_internal)(struct lock_core* lock, TickType_t ticks_to_wait) {
    xEventGroupWaitBits(
            event_group_h,
            get_lock_bit(spin_lock_get_num(lock->spin_lock)),
            pdTRUE,
            pdTRUE,
            ticks_to_wait);

    // every waiting task is woken, so bits can be cleared
    // tasks will re-wait if not able to lock the object
}

int __not_in_flash_func(freertos_get_caller_owner_id)() {
    if (!rtos_core() || !rtos_running)
        return (int)get_core_num(); // 0(rtos not running) or 1(second core)

    // on rtos core, return pointer to task control structure as int (>=0x20000000)
    return (int)xTaskGetCurrentTaskHandle();
}


void __not_in_flash_func(freertos_wait)(struct lock_core* lock) {
    if (!rtos_core() || !rtos_running) {
        __wfe();
        return;
    }

    // rtos core, wait on event group
    freertos_wait_internal(lock, portMAX_DELAY);
}

bool __not_in_flash_func(freertos_wait_until)(struct lock_core* lock, absolute_time_t until) {
    /* Waiting function
     * wait on one of:
     *       RTOS core: event group
     *   non-RTOS core: wfe
     */

    if (!rtos_core() || !rtos_running) {
        return best_effort_wfe_or_timeout(until);
    }

    freertos_wait_internal(lock, until_to_ticks(until));
    return time_reached(until);
}

void __not_in_flash_func(freertos_notify)(struct lock_core* lock) {
    /* Notifying function
     * notify all of:
     *       RTOS core (waits on event group): from RTOS -> event group, from non-RTOS -> fifo
     *   non-RTOS core (waits on wfe):         from RTOS -> sev, from non-RTOS - not possible (wfe locks entire core)
     */

    // always
    // use vanilla notify function
    __sev();

    if (!rtos_running) {
        // nothing else to notify
        return;
    }

    if (!rtos_core()) {
        // non-RTOS core, but RTOS already running on the other core
        // notify other core, this core can't be running any other code right now
        fifo_send_with_data_inline(LOCK_RTOS, spin_lock_get_num(lock->spin_lock)); // TODO make inline
        return;
    }

    // notify self
    xEventGroupSetBits(
            event_group_h,
            get_lock_bit(spin_lock_get_num(lock->spin_lock)));
}

void __not_in_flash_func(freertos_delay)(absolute_time_t until) {
    if (!rtos_running)
        return;

    vTaskDelay(until_to_ticks(until));
}