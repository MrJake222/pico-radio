#pragma once

#include <decodebase.hpp>
#include <filetype.hpp>

typedef void(*player_cb_fn_err)(void* arg, bool errored);
typedef void(*player_cb_fn_dec)(void* arg, DecodeBase* dec);

void player_init();
void player_start(const char* path, void* cb_arg_, player_cb_fn_err fail_cb_, player_cb_fn_dec upd_cb_);
void player_stop();
bool player_is_started();