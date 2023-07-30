#include "analog.hpp"

#include <config.hpp>
#include <hardware/adc.h>

#define ADC_BASE_PIN       26

namespace analog {

const float conversion_ratio = 3.3f / (1<<12);

void init() {
    adc_init();
#if ADC_VCC >= ADC_BASE_PIN
    adc_gpio_init(ADC_VCC);
#endif
#if ADC_VBATT >= ADC_BASE_PIN
    adc_gpio_init(ADC_VBATT);
#endif
}

float vcc_voltage() {
#if ADC_VCC >= ADC_BASE_PIN
    adc_select_input(ADC_VCC - ADC_BASE_PIN);
    return (float)adc_read() * conversion_ratio * ADC_VCC_MUL;
#endif
    return -1;
}

float battery_voltage() {
#if ADC_VBATT >= ADC_BASE_PIN
    adc_select_input(ADC_VBATT - ADC_BASE_PIN);
    return (float)adc_read() * conversion_ratio * ADC_VBATT_MUL;
#endif
    return -1;
}

// 6th degree polynomial approximation of Li-ion battery discharge curve
// data source: https://www.richtek.com/Design%20Support/Technical%20Document/AN024
// from the highest power to the lowest, input 3.0 - 4.2, output 0 - 1
// voltage >3.675V (almost nominal voltage)
const double poly_high[] = {
        -648.3050941489363,
        15683.35414488613,
        -158018.994634744,
        848783.248894731,
        -2563433.1126427907,
        4127225.955897427,
        -2767535.4085456543,
};
// voltage below
const double poly_low[] = {
        76.54079395955621,
        -1517.7707491201104,
        12529.852043810739,
        -55121.14598520336,
        136283.5582523768,
        -179553.85891177552,
        98482.5318346571,
};

int battery_percentage() {
    const float v = battery_voltage();
    const double* poly;

    if (v > 3.675f)
        poly = poly_high;
    else
        poly = poly_low;

    double value = 0;
    for (int i=0; i<7; i++) {
        value *= v;
        value += poly[i];
    }

    if (value > 1) value = 1;
    if (value < 0) value = 0;

    return (int)(value * 100.0);
}

}