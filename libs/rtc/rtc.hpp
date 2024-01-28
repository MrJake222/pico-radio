#pragma once

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif
void rtc_set_time(time_t sec);
#ifdef __cplusplus
}

namespace rtc {

void start_sntp();
int get_hm(char* buf);

}
#endif

