#pragma once

#include <lfsaccess.hpp>
#include <lfsorter.hpp>

namespace wifi {

// run a scan of available wifi networks and save them as LFS file
// each line contains one wifi network name prepended with
// 3-digit percent signal quality and a space
// this blocks calling task until scan completes
int scan_lfs(LfsAccess& acc);

// skips k first stations and returns n-highest-quality in a callback
int read_lfs(LfsAccess& acc, int n, int k, void* cb_arg, lfsorter::res_cb_fn cb);

} // namespace