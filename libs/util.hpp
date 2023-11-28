#pragma once

#include <cstdio>
#include <cstdint>
#include <FreeRTOS.h>
#include <semphr.h>

// hexdump-like printing
void debug_print(uint8_t* buffer, int read_at, int bytes, int reverse);

int create_mutex_give(SemaphoreHandle_t& mutex);

// encodes character ' ' into %20
void url_encode_string(char* dst, const char* src);

// before is a function which takes two const T* pointers e1, e2
// and returns whether e1 is before e2
template<typename T, typename F>
void insert_in_order(T* arr, int arr_len, T* elem, F before) {
    for (int i=arr_len; i>=0; i--) {
        if (i == 0) {
            // first element after elem
            arr[0] = *elem;
            break;
        }

        if (before(&arr[i-1], elem)) {
            // arr[i-1] before elem
            arr[i] = *elem;
            break;
        }

        // elem somewhere before arr[i-1]
        // push arr[i-1] to right
        arr[i] = arr[i-1];
    }
}