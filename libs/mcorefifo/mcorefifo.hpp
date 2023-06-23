#pragma once

#include <cstdint>

#define MSG_TYPE_BITS 8
#define MSG_DATA_MASK ((1 << (32 - MSG_TYPE_BITS)) - 1)
#define MSG_TYPE(val) ((FifoMsgType) (val >> (32 - MSG_TYPE_BITS)))
#define MSG_DATA(val) (val & MSG_DATA_MASK)
#define MSG_MAKE(type, data) ((type << (32 - MSG_TYPE_BITS)) | (data & MSG_DATA_MASK))

enum FifoMsgType {
    // raw buffer needs filling
    // see decodefile.cpp
    RAW_BUF_TOP_UP,

    // used to wake up the player thread
    // when ending playback
    PLAYER_WAKE_END
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