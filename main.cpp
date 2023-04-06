#include <cstdio>
#include <pico/stdlib.h>
#include <hardware/pwm.h>
#include <hardware/clocks.h>

// const int PWM_FAST_OUT_19 = 0;
// const int PWM_FAST_OUT_38 = 1;
const int PWM_FAST_SLICE = 7;

// const int PWM_AUDIO_OUT_L = 14;
// const int PWM_AUDIO_OUT_R = 15;
const int PWM_AUDIO_SLICE = 0;

#include "audio.h"

const uint8_t sine_1kHz_for_top_171_x4[171] = {86, 89, 92, 95, 98, 101, 104, 107, 110, 113, 116, 119, 122, 125, 128, 130, 133, 136, 138, 140, 143, 145, 147, 149, 151, 153, 155, 157, 159, 160, 162, 163, 164, 166, 167, 168, 168, 169, 170, 170, 171, 171, 171, 171, 171, 171, 170, 170, 169, 169, 168, 167, 166, 165, 164, 162, 161, 160, 158, 156, 154, 152, 150, 148, 146, 144, 142, 139, 137, 134, 132, 129, 126, 123, 121, 118, 115, 112, 109, 106, 103, 100, 96, 93, 90, 87, 84, 81, 78, 75, 71, 68, 65, 62, 59, 56, 53, 50, 48, 45, 42, 39, 37, 34, 32, 29, 27, 25, 23, 21, 19, 17, 15, 13, 11, 10, 9, 7, 6, 5, 4, 3, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 3, 4, 5, 7, 8, 9, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 31, 33, 35, 38, 41, 43, 46, 49, 52, 55, 58, 61, 64, 67, 70, 73, 76, 79, 82};


const int8_t sine_19kHz_for_top_171_zero[36] = {0, 15, 29, 42, 55, 65, 74, 80, 84, 85, 84, 80, 74, 65, 55, 43, 29, 15, 0, -15, -29, -43, -55, -65, -74, -80, -84, -85, -84, -80, -74, -65, -55, -42, -29, -15};

#define BITS 11

uint16_t ls, rs;

void pwm_fast_wrap_irq() {
    // static bool led = false;
    // static uint8_t p = 0;
    // static uint cnt = 0;
    static uint8_t index19 = 0;
    static uint8_t index38 = 0;
    static uint16_t index1k = 0;

    pwm_clear_irq(PWM_FAST_SLICE);

    /*gpio_put(LED_RED, led);
    led ^= true;

    if (cnt == 0) {
        pwm_set_gpio_level(PWM_OUT, p++);
        cnt = 1907;
    }

    cnt--;*/

    const uint8_t C = 85;
    const uint8_t T = 171;

    int8_t pilot = sine_19kHz_for_top_171_zero[index19];
    int8_t modulator = sine_19kHz_for_top_171_zero[index38];
    int8_t sine = sine_1kHz_for_top_171_x4[index1k / 4] - C;

    /*pwm_set_gpio_level(PWM_FAST_OUT_19, );
    pwm_set_gpio_level(PWM_FAST_OUT_38, (sine_19kHz_for_top_171_zero[index38] * sine_1kHz_for_top_171_x4[index1k / 4] / 171) + 85);

    */

    uint16_t ls_fast = ls * T / ((1<<BITS)-1);
    uint16_t rs_fast = rs * T / ((1<<BITS)-1);

    uint8_t sum = (ls_fast + rs_fast) / 2;
    uint8_t diff = (ls_fast - rs_fast) / 2;

    /*uint16_t modulated = (diff * modulator / T) + C;

    pwm_set_both_levels(PWM_FAST_SLICE, (sum + pilot + modulated) / 3, pilot);*/

    int8_t modulated = diff * modulator / T;

    pwm_set_both_levels(PWM_FAST_SLICE, (modulated + pilot + sum) / 3 + C, pilot + C);


    index19++;
    index38+=2;
    index1k++;
    if (index19 == 36) index19 = 0;
    if (index38 == 36) index38 = 0;
    if (index1k >= 171*4) index1k = 0;
}




uint16_t center(uint16_t sample) {
    uint16_t sig;
    if (sample & (1<<(BITS-1))) {
        // negative
        sig = ~sample;
        sig &= (1<<BITS)-1;
        sig = -sig;
    }
    else {
        // positive
        sig = sample;
    }

    return sig + (1<<(BITS-1));
}

void pwm_audio_wrap_irq() {
    static int aidx = 0;

    pwm_clear_irq(PWM_AUDIO_SLICE);

    uint16_t ls_raw = (audio[aidx+1] << 8) + audio[aidx];
    uint16_t rs_raw = (audio[aidx+3] << 8) + audio[aidx+2];

    ls = center(ls_raw >> (16 - BITS));
    rs = center(rs_raw >> (16 - BITS));

    pwm_set_both_levels(PWM_AUDIO_SLICE, ls, rs);

    aidx+=4;
    if (aidx == audio_size) {
        aidx = 0;
    }
}

void pwm_wrap_irq() {
    if (pwm_get_irq_status_mask() & (1<<PWM_AUDIO_SLICE))
        pwm_audio_wrap_irq();

    if (pwm_get_irq_status_mask() & (1<<PWM_FAST_SLICE))
        pwm_fast_wrap_irq();
}

void init_pwm(uint slice, pwm_config* cfg, bool irq_en) {
    uint pinA = (slice << 1);
    uint pinB = (slice << 1) + 1;

    gpio_set_function(pinA, GPIO_FUNC_PWM);
    gpio_set_function(pinB, GPIO_FUNC_PWM);

    pwm_set_irq_enabled(slice, irq_en);

    pwm_init(slice, cfg, true);
}

int main() {

    // UART on USB
    stdio_usb_init();

    // sleep_ms(2000);
    printf("\n\nHello usb pico-radio!\n");
    printf("sys clock: %lu MHz", clock_get_hz(clk_sys)/1000000);

    pwm_config cfg;

    cfg = pwm_get_default_config();
    // 683 994.53Hz
    pwm_config_set_clkdiv_int_frac(&cfg, 1, 1);
    pwm_config_set_wrap(&cfg, 171);
    // init_pwm(PWM_FAST_SLICE, &cfg, true);



    cfg = pwm_get_default_config();
    // exact 8kHz
    // pwm_config_set_clkdiv_int_frac(&cfg, 62, 8);
    // pwm_config_set_wrap(&cfg, 249);
    // off by 0.04%
    // pwm_config_set_clkdiv_int_frac(&cfg, 61, 1);
    //pwm_config_set_wrap(&cfg, 255);
    // 32kHz ~0.06%
    // pwm_config_set_clkdiv_int_frac(&cfg, 15, 4);
    // 44.1kHz ~0.09%
    // pwm_config_set_clkdiv_int_frac(&cfg, 11, 4);

    #if BITS == 8
        // 44.1kHz ~0.09%
        pwm_config_set_wrap(&cfg, 255);
        pwm_config_set_clkdiv_int_frac(&cfg, 11, 1);
    #elif BITS == 9
        // 44.1kHz -0.48%
        pwm_config_set_wrap(&cfg, 511);
        pwm_config_set_clkdiv_int_frac(&cfg, 5, 9);
    #elif BITS == 10
        // 44.1kHz +0.66%
        pwm_config_set_wrap(&cfg, 1023);
        pwm_config_set_clkdiv_int_frac(&cfg, 2, 12);
    #elif BITS == 11
        // 44.1kHz +0.66% (top 2047)
        //         +0.02% (top 2060)
        pwm_config_set_wrap(&cfg, 2060);
        pwm_config_set_clkdiv_int_frac(&cfg, 1, 6);
    #elif BITS == 12
        // 44.1kHz -1.58%
        set_sys_clock_khz(200000, true);
        pwm_config_set_wrap(&cfg, 4095);
        pwm_config_set_clkdiv_int_frac(&cfg, 1, 2);
    #else
        err;
    #endif



    init_pwm(PWM_AUDIO_SLICE, &cfg, true);

    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_wrap_irq);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    // pwm_set_gpio_level(PWM_AUDIO_OUT_L, 249 / 3);
    // pwm_set_gpio_level(PWM_AUDIO_OUT_R, 249 / 4);

    //gpio_init(LED_RED);
    //gpio_set_dir(LED_RED, true);
    //gpio_put(LED_RED, 1);

    while (1) {

    }
}
