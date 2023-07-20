#include "datasource.hpp"

#include <cstdio>

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
