#include "mcorefifo.hpp"

#include <config.hpp>
#include <pico/multicore.h>
#include <hardware/irq.h>
#include <cstdio>
#include <map>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

struct cb_data {
    FifoCallback cb;
    void* arg;
    bool use_task;
};

// TODO use zero-alloc
static std::map<FifoMsgType, struct cb_data> cbs;

static bool entry_check(uint32_t val) {
    auto type = (FifoMsgType) MSG_TYPE(FIFO_MSG_BITS, FIFO_MSG_TYPE_BITS, val);

    if (cbs.count(type)) {
        return true;
    } else {
        printf("no callback for message type: %d in value %lu\n", type, val);
        return false;
    }
}

static void entry_handle(uint32_t val) {
    auto type = (FifoMsgType) MSG_TYPE(FIFO_MSG_BITS, FIFO_MSG_TYPE_BITS, val);
    auto cb = cbs[type];
    cb.cb(cb.arg, MSG_DATA(FIFO_MSG_BITS, FIFO_MSG_TYPE_BITS, val));
}

// only valid entries
static QueueHandle_t fifo_queue;

[[noreturn]] static void fifo_task(void* arg) {
    while (true) {
        uint32_t val;
        xQueueReceive(fifo_queue,
                      &val,
                      portMAX_DELAY);

        entry_handle(val);
    }
}

void fifo_init() {
    if (get_core_num() != 0) {
        panic("fifo supported only on core0");
    }

    fifo_queue = xQueueCreate(16, sizeof(uint32_t));

    xTaskCreate(
            fifo_task,
            "fifo",
            STACK_FIFO_QUEUE,
            nullptr,
            PRI_FIFO_QUEUE,
            nullptr);

    multicore_fifo_drain();
    irq_set_exclusive_handler(SIO_IRQ_PROC0, fifo_rx_irq);
    irq_set_enabled(SIO_IRQ_PROC0, true);
}

void fifo_register(FifoMsgType type, FifoCallback cb, void* arg, bool use_task) {
    cbs[type] = {
            .cb = cb,
            .arg = arg,
            .use_task = use_task
    };
}

void fifo_send_with_data(FifoMsgType type, uint32_t data) {
    multicore_fifo_push_blocking(MSG_MAKE(FIFO_MSG_BITS, FIFO_MSG_TYPE_BITS, type, data));
}

void fifo_rx_irq() {
    multicore_fifo_clear_irq();
    uint32_t val = multicore_fifo_pop_blocking();

    if (entry_check(val)) {
        auto type = (FifoMsgType) MSG_TYPE(FIFO_MSG_BITS, FIFO_MSG_TYPE_BITS, val);
        auto cb = cbs[type];
        if (cb.use_task)
            xQueueSendFromISR(fifo_queue, &val, nullptr);
        else
            entry_handle(val);
    }
}

void fifo_send(FifoMsgType type) { fifo_send_with_data(type, 0); }
