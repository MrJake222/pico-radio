#include "util.hpp"

void debug_print(uint8_t* buffer, int read_at, int bytes, int reverse) {

    const int width = 16;

    int len = bytes + reverse;
    int r = -reverse;

    while (len > 0) {

        printf("%5d: ", read_at + r);

        for (int i=0; i<width; i++) {
            if (i < len)
                printf("%02x ", buffer[read_at + r + i]);
            else
                printf("   ");
        }

        printf("   ");

        for (int i=0; i<width; i++) {
            if (i < len) {
                char c = buffer[read_at + r + i];
                printf("%c", (c<32 || c>127) ? '.' : c);
            }
            else
                printf(" ");
        }

        r += width;
        len -= width;
        printf("\n");
    }
}

int create_mutex_give(SemaphoreHandle_t& mutex) {
    mutex = xSemaphoreCreateMutex();

    if (!mutex) {
        puts("mutex creation failed");
        assert(false);
        return -1;
    }

    puts("mutex creation ok");
    xSemaphoreGive(mutex);

    return 0;
}

void url_encode_string(char* dst, const char* src) {
    while (*src) {
        if (*src == ' ') {
            *dst++ = '%';
            *dst++ = '2';
            *dst++ = '0'; // %20
        }

        // add other characters here

        else {
            *dst++ = *src;
        }

        src++;
    }
}
