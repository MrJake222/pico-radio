#include <cstdio>
#include <cstdlib>
#include <pico/stdlib.h>
#include <hardware/pwm.h>
#include <hardware/clocks.h>

#include "f_util.h"
#include "ff.h"
#include "hw_config.h"

#include "libs/waveheader.hpp"

const uint I2C_CLK = 18;
const uint I2C_CHAN = 19;
const uint I2C_DATA = 20;
const uint PWM_FAST_SLICE = pwm_gpio_to_slice_num(I2C_CLK);

volatile int aread = 0; // non-wrapping index
volatile int aidx = 0;  // wrapping index
volatile int abit = 0;  // bit index
volatile int asam = 0;  // bit index

bool last_bit;

WaveHeader hdr;

#define BUF_SIZE 20000 // 1/8 second
uint8_t audio[BUF_SIZE];
uint32_t* audio32 = (uint32_t*)audio;
void pwm_fast_wrap_irq() {

    // set channel (right=1)
    gpio_put(I2C_CHAN, abit & 0x10);

    // send data
    gpio_put(I2C_DATA, last_bit);

    
    pwm_clear_irq(PWM_FAST_SLICE);

    // ll rr
    uint32_t raw = audio32[asam];
    //uint32_t raw = 0xF0000000;
    last_bit = (raw << abit) & 0x80000000;

    abit++;
    if (abit == 32) {
        abit = 0;
        //aread += 4;
        asam += 1;
        aidx += 4;
        if (aidx == BUF_SIZE) {
            asam = 0;
            aidx = 0;
        }

        /*if (aread == hdr.get_data_size()) {
            // stop pwm
            pwm_set_enabled(PWM_FAST_SLICE, false);
        }*/
    }
}

void pwm_wrap_irq() {
    if (pwm_get_irq_status_mask() & (1<<PWM_FAST_SLICE))
        pwm_fast_wrap_irq();
}

void fs_err(FRESULT fr, const char* tag) {
    panic("%s: %s (id=%d)\n", tag, FRESULT_str(fr), fr);
}

int main() {

    // UART on USB
    stdio_usb_init();

    // sleep_ms(2000);
    printf("\n\nHello usb pico-radio!\n");
    printf("sys clock: %lu MHz", clock_get_hz(clk_sys)/1000000);


    pwm_config cfg;
    cfg = pwm_get_default_config();

    // 1 412 429.38Hz
    cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_int_frac(&cfg, 1, 8);
    pwm_config_set_wrap(&cfg, 58);

    pwm_set_irq_enabled(PWM_FAST_SLICE, true);
    pwm_init(PWM_FAST_SLICE, &cfg, false);

    gpio_set_function(I2C_CLK, GPIO_FUNC_PWM);
    pwm_set_gpio_level(I2C_CLK, 58/2);

    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_wrap_irq);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    gpio_init(I2C_DATA); gpio_set_dir(I2C_DATA, GPIO_OUT);
    gpio_init(I2C_CHAN); gpio_set_dir(I2C_CHAN, GPIO_OUT);

    sleep_ms(2000);

    FRESULT fr;
    FIL fp;

    sd_card_t *pSD = sd_get_by_num(0);

    fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (fr != FR_OK) {
        fs_err(fr, "f_mount");
    }

    puts("\n");

//    const char* path = "silence.wav";
    const char* path = "Ed44.wav";
//    const char* path = "The Memory Remains.wav";
//    const char* path = "Rock or Bust/01. Rock Or Bust.wav";
//    const char* path = "Rock or Bust/08. Baptism By Fire.wav";
//    const char* path = "Rock or Bust/08. Baptism By Fire.wav";
//    const char* path = "latwa.wav";
//    const char* path = "4mmc.wav";
//    const char* path = "too_much_short.wav";

    printf("opening: %s\n\n", path);

    fr = f_open(&fp, path, FA_READ);
    if (fr != FR_OK) {
        fs_err(fr, "f_open");
    }

    uint read;

    fr = hdr.read(&fp, &read);
    if (fr != FR_OK) {
        fs_err(fr, "f_read header");
    }

    printf("header size: %u, read %u\n", WAVE_HEADER_SIZE, read);

    if (hdr.check()) {
        panic("wrong header\n");
    }

    hdr.print();

    // preload buffer
    fr = f_read(&fp, (uint8_t*)audio, BUF_SIZE, &read);
    if (fr != FR_OK) {
        fs_err(fr, "f_read preload");
    }

    // start pwm
    pwm_set_enabled(PWM_FAST_SLICE, true);

    int start = 0;
    uint last_seconds = 0;
    bool eof = false;

    while (!eof) {
        uint left = aidx > start
                ? BUF_SIZE - (aidx - start)
                : start - aidx;

        if (left < BUF_SIZE / 2) {
            fr = f_read(&fp, ((uint8_t*)audio) + start, BUF_SIZE / 2, &read);
            if (fr != FR_OK) {
                fs_err(fr, "f_read loop");
            }
            if (read < BUF_SIZE/2) {
                // EOF reached
                eof = true;
            }

            start += BUF_SIZE/2;
            if (start == BUF_SIZE) start = 0;
        }

        uint seconds = hdr.byte_to_sec(aread);

        if (seconds != last_seconds) {
            printf("%02d:%02d / %02d:%02d\r",
                   seconds/60, seconds%60,
                   hdr.get_duration()/60, hdr.get_duration()%60);

            last_seconds = seconds;
        }
    }

    while (aread < hdr.get_data_size());

    printf("\nfinished.\n");
    printf("\nplayed %u / %u bytes.\n", aread, hdr.get_data_size());

    while (1);
}
