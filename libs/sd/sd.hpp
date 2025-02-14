#pragma once

#include <pico/platform.h>

namespace sd {

// called from gpio_irq
void sd_cd_callback(uint gpio, uint32_t events);

void init();
bool is_card_mounted();
bool has_card_failed();

} // namespace sd