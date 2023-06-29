#pragma once

#include <filetype.hpp>


void player_init();
void player_start(const char* path);
void player_stop();
bool player_is_running();