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

namespace sd {

static TaskHandle_t sd_task_h;
static volatile bool card_present;
static volatile bool card_mounted;

void sd_cd_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        // card inserted
        card_present = true;
    }
    else if (events & GPIO_IRQ_EDGE_RISE) {
        // card removed
        card_present = false;
    }

    vTaskNotifyGiveFromISR(sd_task_h, nullptr);
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
    // wait a bit for gpio to settle after boot
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // check if card is present on boot
    // card present if gpio low
    card_present = gpio_get(SD_CD) == 0;
    card_mounted = false;

    sd_card_t* pSD = sd_get_by_num(0);

    while (true) {
        // wait a bit for card to securely be inserted
        vTaskDelay(500 / portTICK_PERIOD_MS);

        if (card_present) {
            puts("mounting sd card");
            int r = mount(pSD);
            if (r == 0)
                card_mounted = true;
        }
        else if (card_mounted) { // avoid unmounting if never present (on boot)
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
    const int all[] = {SD_CD};
    for (int gpio : all) {
        gpio_init(gpio);
        gpio_set_dir(gpio, false);
        gpio_pull_up(gpio);
        gpio_set_input_hysteresis_enabled(gpio, true); // schmitt trigger
    }

    gpio_set_irq_enabled(
            SD_CD,
            GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
            true);

    puts("sd: detection init ok");

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