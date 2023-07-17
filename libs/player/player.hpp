#pragma once

#include <decodebase.hpp>
#include <filetype.hpp>

typedef void(*player_cb_fn)(void* arg);
typedef void(*player_cb_fn_dec)(void* arg, DecodeBase* dec);

void player_init();
void player_start(const char* path, void* cb_arg_, player_cb_fn fail_cb_, player_cb_fn_dec upd_cb_);
void player_stop();
bool player_is_started();
void player_wait_for_end();