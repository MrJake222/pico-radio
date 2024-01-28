#pragma once

#include <lfsscan.hpp>
#include <pico/cyw43_arch.h>

class WifiScan : public LfsScan {

    // run a scan of available wifi networks and save them as LFS file
    // each line contains one wifi network name prepended with
    // 3-digit percent signal quality and a space
    // this blocks calling task until scan completes
    int scan_internal(flagref_t should_abort) override;

    // copy of should_abort from superclass
    // (for usage when calling thread exits)
    volatile bool should_abort_cpy;

    friend int wscan_res_cb(void* arg, const cyw43_ev_scan_result_t* res);

public:
    using LfsScan::LfsScan;

    void begin();

    static void format_encode(char* buf, int buf_size, const char* ssid, int q);
    static const char* format_decode_ssid(const char* buf);
    static int format_decode_quality(const char* buf);
};