#include "datasource.hpp"

#include <cstdio>

// TODO DS::read_line optimize to use buffer for seekable mediums
int read_line(DataSource* ds, char* buf, int bufsize, int* line_length) {

    // buffers underruns need to be verified before calling this
    // ds.more_content() may not be available here (no content-lenght header)

    bool overrun = false;
    bool error = false;

    if (buf == nullptr || bufsize == 0)
        // special handling, skip all data
        overrun = true;

    int ret;
    char chr;
    *line_length = 0;

    while (true) {
        // read_char will read at least one byte
        ret = ds->read_char(&chr);
        if (ret < 0) {
            puts("read_line: ds error");
            error = true;
        }

        if (chr == '\r') {
            // ignore
        }

        else if (chr == '\n' || error) {
            // found \n: it's either \n or \r\n
            // error: read errored
            //   -> finish now
            // on overrun keep going (no error)

            if (!overrun)
                // not overrun, terminate now
                buf[*line_length] = '\0';

            // only way to exit the loop
            break;
        }

        else {
            // not \r or \n
            if (!overrun)
                buf[*line_length] = chr;

            (*line_length)++;

            // this won't fire when bufsize is 0, which is good
            if (*line_length == bufsize - 1) {
                // no more space except for termination
                // null-terminate
                buf[*line_length] = '\0';
                // mark override
                // puts("read_line: buffer overrun");
                overrun = true;
            }
        }
    }

    return error    ? RL_ERROR
         : overrun  ? RL_OVERRUN
         :            RL_OK;
}