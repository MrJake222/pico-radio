#pragma once

#include <cstdint>
#include <bitmsg.hpp>
#include <pico/platform.h>
#include <pico/multicore.h>

#define FIFO_MSG_BITS       32
#define FIFO_MSG_TYPE_BITS   8

enum FifoMsgType {
    // used to notify player thread
    PLAYER,

    // notify lock_rtos integration event group
    LOCK_RTOS,

    // enum count, do not use
    FifoMsgType_count
};

typedef void(*FifoCallback)(void* arg, uint32_t data);

void fifo_init();

// there can only be one listener per message type
// use_task should be set to false for short tasks (called directly from ISR)
// true for long, blocking tasks (run from task)
void fifo_register(FifoMsgType type, FifoCallback cb, void* arg, bool use_task);

void fifo_send_with_data(FifoMsgType type, uint32_t data);

__force_inline void fifo_send_with_data_inline(FifoMsgType type, uint32_t data) {
    // copied from multicore.c (as multicore_fifo_push_blocking_inline is static)

    // We wait for the fifo to have some space
    while (!multicore_fifo_wready())
        tight_loop_contents();

    sio_hw->fifo_wr = MSG_MAKE(FIFO_MSG_BITS, FIFO_MSG_TYPE_BITS, type, data);

    // Fire off an event to the other core
    __sev();
}

void fifo_rx_irq();