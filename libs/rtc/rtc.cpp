#include "rtc.hpp"

#include <lwip/apps/sntp.h>
#include <hardware/rtc.h>
#include <cstdlib>
#include <ctime>
#include <cstdio>

void rtc_set_time(time_t sec) {
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);

    struct tm* tm = localtime((const time_t*) &sec);
    if (!tm)
        return;

    datetime_t t = {
            .year  = (int16_t) (tm->tm_year + 1900),
            .month =  (int8_t) (tm->tm_mon + 1),
            .day   =  (int8_t) tm->tm_mday,
            .dotw  =  (int8_t) tm->tm_wday,
            .hour  =  (int8_t) tm->tm_hour,
            .min   =  (int8_t) tm->tm_min,
            .sec   =  (int8_t) tm->tm_sec,
    };

    printf("setting epoch=%lld time: %04d-%02d-%02d %02d:%02d (wday %d)\n", sec, t.year, t.month, t.day, t.hour, t.min, t.dotw);

    rtc_init();
    rtc_set_datetime(&t);
}

namespace rtc {

void start_sntp() {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "0.pl.pool.ntp.org");
    sntp_init();
}

int get_hm(char* buf) {
    datetime_t t;
    bool valid = rtc_get_datetime(&t);
    if (!valid)
        return -1;

    sprintf(buf, "%02d:%02d", t.hour, t.min);
    return 0;
}

} // namespace