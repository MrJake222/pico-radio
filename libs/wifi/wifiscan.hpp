#pragma once

#include <lfsaccess.hpp>
#include <lfsorter.hpp>
#include <util.hpp>

namespace wifi::lfs {

void format_encode(char* buf, int buf_size, const char* ssid, int q);
const char* format_decode_ssid(const char* buf);
int format_decode_quality(const char* buf);

// run a scan of available wifi networks and save them as LFS file
// each line contains one wifi network name prepended with
// 3-digit percent signal quality and a space
// this blocks calling task until scan completes
int scan(LfsAccess& acc, flagref_t should_abort);

// skips k first stations and returns n-highest-quality in a callback
// returns strings in format, use decoding functions
// this function is not async, after it returns all entries had been loaded and passed to callback
int read(LfsAccess& acc, flagref_t should_abort, int n, int k, void* cb_arg, lfsorter::res_cb_fn cb);

// count the entries
int count(LfsAccess& acc);

void abort_wait(LfsAccess& acc);

} // namespace