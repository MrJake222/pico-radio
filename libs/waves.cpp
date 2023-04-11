#include "waves.hpp"

uint32_t ccs[CCS_LEN];

void init_ccs() {
    uint w19idx = 0;
    uint w38idx = 0;

    for (uint i=0; i<CCS_LEN; i++) {

        ccs[i] = wave_19kHz[w19idx];
        ccs[i] <<= 16;
        ccs[i] |= wave_38kHz[w38idx];

        w19idx++;
        w38idx++;
        if (w19idx == WAVE_19KHZ_LEN) w19idx = 0;
        if (w38idx == WAVE_38KHZ_LEN) w38idx = 0;
    }
}