#pragma once

#include <cstdint>
#include <bitmsg.hpp>

#define FIFO_MSG_BITS       32
#define FIFO_MSG_TYPE_BITS   8

enum FifoMsgType {
    // used to notify player thread
    PLAYER
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