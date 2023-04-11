#pragma once

#include <cstdint>
#include <pico/platform.h>

#define WAVE_19KHZ_LEN 36
const uint8_t wave_19kHz[WAVE_19KHZ_LEN] = {86, 100, 115, 128, 140, 151, 160, 166, 170, 171, 170, 166, 160, 151, 140, 128, 115, 100, 85, 71, 56, 43, 31, 20, 11, 5, 1, 0, 1, 5, 11, 20, 31, 43, 56, 71};

#define WAVE_38KHZ_LEN 18
const uint8_t wave_38kHz[WAVE_38KHZ_LEN] = {86, 115, 140, 160, 170, 170, 160, 140, 115, 85, 56, 31, 11, 1, 1, 11, 31, 56};

#define CCS_LEN MAX(WAVE_19KHZ_LEN, WAVE_38KHZ_LEN)

extern uint32_t ccs[CCS_LEN];

// get counter compares for both waves
void init_ccs();