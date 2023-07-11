#pragma once

#include <filetype.hpp>

typedef void(*player_cb_fn)(void* arg);

void player_init();
void player_start(const char* path, void* cb_arg_, player_cb_fn fail_cb_, player_cb_fn upd_cb_);
void player_stop();
bool player_is_started();
void player_wait_for_end();