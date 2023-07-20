#pragma once

class DataSource {
public:
    // reads exactly one character (return code 0)
    // or fails (with return code -1)
    virtual int read_char(char* chr) = 0;

    // returns whether there is more content to read
    virtual bool more_content() = 0;
};

enum ReadLineError {
    RL_OVERRUN = -2,
    RL_ERROR = -1,
};

// reads whole line, supports both \n or \r\n endings
// returns length of line without termination (or -1 on failure)
int read_line(DataSource* ds, char* buf, int bufsize);
