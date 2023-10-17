#include "sd.hpp"

#include <cstdio>
#include <hardware/gpio.h>
#include <config.hpp>

#include <FreeRTOS.h>
#include <task.h>

#include <hw_config.h>
#include <ff.h>
#include <f_util.h>
#include <diskio.h>

#include <pico/time.h>

namespace sd {

static TaskHandle_t sd_task_h;
static bool card_present;
static bool card_mounted;
static volatile int last_interrupt_time_us;

void sd_cd_callback(uint gpio, uint32_t events) {
    if (gpio != SD_CD)
        return;

    last_interrupt_time_us = (int) time_us_32();
    vTaskNotifyGiveFromISR(sd_task_h, nullptr);
}

static inline uint32_t time_since_last_interrupt_ms() {
    return ((int)time_us_32() - last_interrupt_time_us) / 1000u;
}

static inline void fs_err(FRESULT fr, const char* tag) {
    printf("%s: %s (id=%d)\n", tag, FRESULT_str(fr), fr);
}

static int mount(sd_card_t* pSD) {
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (fr != FR_OK) {
        fs_err(fr, "sd mount");
        return -1;
    }

    puts("sd: mount ok");
    return 0;
}

static int unmount(sd_card_t* pSD) {
    FRESULT fr = f_unmount(pSD->pcName);
    if (fr != FR_OK) {
        fs_err(fr, "sd unmount");
        return -1;
    }

    // manually de-init the card (it was removed)
    pSD->m_Status |= STA_NOINIT;

    puts("sd: unmount ok");
    return 0;
}

[[noreturn]] static void sd_task(void* arg) {
    const int all[] = {SD_CD};
    for (int gpio : all) {
        gpio_init(gpio);
        gpio_set_dir(gpio, false);
        gpio_pull_up(gpio);
        gpio_set_input_hysteresis_enabled(gpio, true); // schmitt trigger
    }

    card_mounted = false;
    last_interrupt_time_us = 0;

    // enable interrupts
    for (int gpio : all) {
        gpio_set_irq_enabled(
                gpio,
                GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
                true);
    }

    printf("sd: detection init ok\n");

    sd_card_t* pSD = sd_get_by_num(0);

    while (true) {

        // printf("\tsd: int %7lu ms ago\n", time_since_last_interrupt_ms());

        while (time_since_last_interrupt_ms() < SD_CD_DEBOUNCE_MS)
            // bouncing
            // wait a bit for card to securely be inserted / gpio pull-up to settle
            vTaskDelay((SD_CD_DEBOUNCE_MS + 100) / portTICK_PERIOD_MS);

        printf("\tsd: int %4lu ms ago ", time_since_last_interrupt_ms());

        // clear any pending interrupts (0 wait time)
        ulTaskNotifyTake(true, 0);

        // check SD status (present if shorted to ground)
        card_present = gpio_get(SD_CD) == 0;
        printf("present=%d mounted=%d\n", card_present, card_mounted);

        if (card_present && !card_mounted) {
            puts("mounting sd card");
            int r = mount(pSD);
            if (r == 0)
                card_mounted = true;
        }

        if (!card_present && card_mounted) {
            puts("unmounting sd card");

            // as early as possible because card was already removed
            card_mounted = false;

            unmount(pSD);
        }

        uint32_t min_free_stack = uxTaskGetStackHighWaterMark(nullptr);
        printf("sd unused stack: %ld\n", min_free_stack);

        // wait for changes
        // (check at least once before waiting)
        ulTaskNotifyTake(true, portMAX_DELAY);
    }
}

void init() {
    xTaskCreate(
            sd_task,
            "sd",
            STACK_SD,
            nullptr,
            PRI_SD,
            &sd_task_h);
}

bool is_card_mounted() {
    return card_mounted;
}

} // namespace sd