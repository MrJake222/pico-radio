#pragma once

class DataInterface {
public:
    // reads exactly one character (return code 0)
    // or fails (with return code -1)
    virtual int read_char(char* chr) = 0;

    // returns whether there is more content to read
    virtual bool more_content() = 0;

    // write all bytes from the buffer
    virtual int write_all(const char* buf, int buflen) = 0;
};

enum ReadLineError {
    RL_OVERRUN = -2,
    RL_ERROR = -1,
};

// reads whole line, supports both \n or \r\n endings
// returns length of line without termination (or -1 on failure)
int read_line(DataInterface* ds, char* buf, int bufsize);

int write_string(DataInterface* ds, char* str);