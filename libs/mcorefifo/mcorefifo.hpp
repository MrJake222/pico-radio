#pragma once

#include <cstdint>
#include <bitmsg.hpp>

enum FifoMsgType {
    // raw buffer just got read on core1
    // send to core0 to confirm reception (stream) or read from file (file)
    // data is number of bytes read
    RAW_BUF_READ,

    // used to wake up the player thread
    // when ending playback
    PLAYER_WAKE
};

typedef void(*FifoCallback)(void* arg, uint32_t data);

void fifo_init();

// there can only be one listener per message type
// use_task should be set to false for short tasks (called directly from ISR)
// true for long, blocking tasks (run from task)
void fifo_register(FifoMsgType type, FifoCallback cb, void* arg, bool use_task);

void fifo_send_with_data(FifoMsgType type, uint32_t data);
void fifo_send(FifoMsgType type);

void fifo_rx_irq();