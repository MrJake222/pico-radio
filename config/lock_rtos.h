#ifndef PICO_RADIO_LOCK_RTOS_H
#define PICO_RADIO_LOCK_RTOS_H

// RTOS is only running on one core, defined below
#define FREERTOS_CORE   0

#ifndef __ASSEMBLER__
#include <pico/types.h>

/**
 * file intends to integrate lock_core.h (pico-sdk locking mechanisms) with FreeRTOS
 */

#ifdef __cplusplus
extern "C" {
#endif

// call before RTOS starts
void lock_rtos_init();

/**
 * Squashes task pointers into ints (>=0x20000000)
 * Other values are free to use (for non-rtos core it's core number)
 */
extern int freertos_get_caller_owner_id();

struct lock_core;

extern void freertos_wait(struct lock_core* lock);
// return true if time_reached(until) is true
extern bool freertos_wait_until(struct lock_core* lock, absolute_time_t until);
extern void freertos_notify(struct lock_core* lock);
extern void freertos_delay(absolute_time_t until);


#define lock_owner_id_t int
#define LOCK_INVALID_OWNER_ID ((lock_owner_id_t)-1)
#define lock_get_caller_owner_id() (freertos_get_caller_owner_id())

#define lock_internal_spin_unlock_with_wait(lock, save) spin_unlock((lock)->spin_lock, save), freertos_wait(lock)
#define lock_internal_spin_unlock_with_notify(lock, save) spin_unlock((lock)->spin_lock, save), freertos_notify(lock)
#define lock_internal_spin_unlock_with_best_effort_wait_or_timeout(lock, save, until) ({    \
    spin_unlock((lock)->spin_lock, save);                                                   \
    freertos_wait_until(lock, until);                                                       \
})

#define sync_internal_yield_until_before(until) (freertos_delay(until))

#ifdef __cplusplus
}
#endif

#endif // __ASSEMBLER__
#endif // PICO_RADIO_LOCK_RTOS_H
