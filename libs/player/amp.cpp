#include "amp.hpp"
#include <hardware/gpio.h>
#include <config.hpp>

void amp_init() {
#if AMP_MUTE >= 0
    gpio_init(AMP_MUTE);
    gpio_set_dir(AMP_MUTE, true);
#endif
}

void amp_mute() {
#if AMP_MUTE >= 0
    gpio_put(AMP_MUTE, false);
#endif
}

void amp_unmute() {
#if AMP_MUTE >= 0
        gpio_put(AMP_MUTE, true);
#endif
}
