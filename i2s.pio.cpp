#include <i2s.pio.h>
#include <hardware/clocks.h>

const uint INSTRUCTION_PER_CLOCK = 2;

// uses 3 pins
// base   : CLK         side-set LSB
// base+1 : CHANNEL     side-set MSB
// data   : DATA        out
void i2s_program_init(PIO pio, uint sm, uint offset, uint pin_clk_channel_base, uint pin_data) {

    // setup GPIO
    // driven by pio
    pio_gpio_init(pio, pin_clk_channel_base);
    pio_gpio_init(pio, pin_clk_channel_base + 1);
    pio_gpio_init(pio, pin_data);
    // output (from <pin>, len <3>, output <true>)
    pio_sm_set_consecutive_pindirs(pio, sm, pin_clk_channel_base, 2, true);
    pio_sm_set_consecutive_pindirs(pio, sm, pin_data, 1, true);


    pio_sm_config c = i2s_program_get_default_config(offset);

    // set where <out pins> outputs (shifts), start <pin>, count <1>
    sm_config_set_out_pins(&c, pin_data, 1);

    // set where <side> outputs (base pin, count in .pio file)
    sm_config_set_sideset_pins(&c, pin_clk_channel_base); // CLK, CHANNEL

    // set divisors using <i2s_program_set_bit_freq>

    // setup datapath
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
    sm_config_set_out_shift(&c, false, true, 32);

    // set config to <c>
    pio_sm_init(pio, sm, offset + i2s_offset_entry, &c);

    // start state machine
    pio_sm_set_enabled(pio, sm, true);
}

void i2s_program_set_bit_freq(PIO pio, uint sm, uint bit_freq_hz) {
    uint sysclk = clock_get_hz(clk_sys);
    float div = (float)sysclk / ((float)bit_freq_hz * INSTRUCTION_PER_CLOCK);
    pio_sm_set_clkdiv(pio, sm, div);
}