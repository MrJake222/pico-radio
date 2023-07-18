#pragma once

#include <cstdio>
#include <cstdint>

// hexdump-like printing
void debug_print(uint8_t* buffer, int read_at, int bytes, int reverse);

enum ReadLineError {
    RL_OVERRUN = -2,
    RL_ERROR = -1,
};

class DataSource {
public:
    // reads exactly one character (return code 0)
    // or fails (with return code -1)
    virtual int read_char(char* chr) = 0;
};

// reads whole line, supports both \n or \r\n endings
// returns length of line without termination (or -1 on failure)
int read_line(DataSource* ds, char* buf, int bufsize);