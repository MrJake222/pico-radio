#include "pwm.hpp"

#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <hardware/dma.h>
#include <cstdio>

#include "waves.hpp"

// DMA
static int dma_channel_a;
static int dma_channel_b;

static void dma_channelA() {
    dma_channel_set_read_addr(dma_channel_a, ccs, false);
}

static void dma_channelB() {
    dma_channel_set_read_addr(dma_channel_b, ccs, false);
}

static void dma_irq0() {
    if (dma_channel_get_irq0_status(dma_channel_a)) {
        dma_channel_acknowledge_irq0(dma_channel_a);
        dma_channelA();
    }

    if (dma_channel_get_irq0_status(dma_channel_b)) {
        dma_channel_acknowledge_irq0(dma_channel_b);
        dma_channelB();
    }
}

static void configure_pwm_dma(int dma_chan, uint pwm_slice, uint32_t* data, uint count, int chain_to) {
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pwm_get_dreq(pwm_slice));
    channel_config_set_chain_to(&c, chain_to);

    dma_channel_set_irq0_enabled(dma_chan, true);

    dma_channel_configure(
            dma_chan,
            &c,
            &pwm_hw->slice[pwm_slice].cc,
            data,
            count,
            false
    );
}

void my_pwm_init(uint slice) {
    // PWM
    // TOP = 171
    // div = 1 + 1/16
    // freq 683994.53Hz

    // waves
    // 19kHz 36 samples = 18999.85Hz
    // 38kHz 18 samples = 37999.70Hz

    // IO
    gpio_set_function((slice << 1),     GPIO_FUNC_PWM);
    gpio_set_function((slice << 1) + 1, GPIO_FUNC_PWM);

    // PWM
    pwm_config cpwm = pwm_get_default_config();
    pwm_config_set_clkdiv_int_frac(&cpwm, 1, 1);
    pwm_config_set_wrap(&cpwm, 171);

    pwm_set_both_levels(slice, 0, 0);
    pwm_init(slice, &cpwm, true);

    // DMA
    irq_add_shared_handler(DMA_IRQ_0, dma_irq0, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_channel_a = dma_claim_unused_channel(true);
    dma_channel_b = dma_claim_unused_channel(true);

    init_ccs();
    configure_pwm_dma(dma_channel_a, slice, ccs, CCS_LEN, dma_channel_b);
    configure_pwm_dma(dma_channel_b, slice, ccs, CCS_LEN, dma_channel_a);

    dma_channel_start(dma_channel_a);
}