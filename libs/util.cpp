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

int read_line(DataSource* ds, char* buf, int bufsize) {

    int line_length = 0;
    int ret;

    char chr;
    bool r_seen = false;
    bool overrun = false;

    while (true) {
        // recv will receive at least one byte
        ret = ds->read_char(&chr);
        if (ret < 0)
            return RL_ERROR;

        if (chr == '\r') {
            r_seen = true;
            chr = 0;
        }

        else if (chr == '\n') {
            // found \n
            // it's either singular line ending (only \n)
            // or \r\n\ if \r was seen (r_seen = true)

            if (overrun) {
                puts("line buffer overrun");
                return RL_OVERRUN;
            }

            if (r_seen) {
                // string already terminated
                return line_length - 1; // line length without \r
            }

            // \r not seen
            // terminate string
            buf[line_length] = 0;
            return line_length;
        }

        if (line_length+1 < bufsize) {
            buf[line_length++] = chr;
        }
        else {
            overrun = true;
        }
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