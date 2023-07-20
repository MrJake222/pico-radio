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

void create_mutex_give(SemaphoreHandle_t& mutex) {
    mutex = xSemaphoreCreateMutex();
    assert(mutex != nullptr);
    if (mutex)
        puts("mutex creation ok");
    else
        puts("mutex creation failed");

    xSemaphoreGive(mutex);
}