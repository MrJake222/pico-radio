#pragma once

#include <decodebase.hpp>
#include <filetype.hpp>

// player finished callback
// called by player task on playback end
// argument indicates error (hence the name)
// can restart player by returning true
typedef bool(*player_cb_fn_fin)(void* arg, bool errored);
// player update callback
// should be used to update player statistics in console/screen
typedef void(*player_cb_fn_upd)(void* arg, DecodeBase* dec);

void player_init();
int player_start(const char* path, void* cb_arg_, player_cb_fn_fin fin_cb_, player_cb_fn_upd upd_cb_);
void player_stop();
bool player_is_started();