#pragma once

#include <filetype.hpp>

typedef void(*fail_cb_fn)(void* arg);

void player_init();
void player_start(const char* path, fail_cb_fn fail_cb_, void* fail_cb_arg_);
void player_stop();
bool player_is_started();
void player_wait_for_end();