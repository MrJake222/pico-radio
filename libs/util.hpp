#pragma once

#include <cstdio>
#include <cstdint>
#include <FreeRTOS.h>
#include <semphr.h>

// hexdump-like printing
void debug_print(uint8_t* buffer, int read_at, int bytes, int reverse);

void create_mutex_give(SemaphoreHandle_t& mutex);

// encodes character ' ' into %20
void url_encode_string(char* dst, const char* src);