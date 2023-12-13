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
    RL_OK = 0,
};

// reads whole line, supports both \n or \r\n endings
// stores length of line without termination (in pointer parameter <line_length>)
// returns <ReadLineError> on error or 0 on success
// can be used to:
//   skip lines with buf==nullptr and bufsize==0
//   read partial lines with bufsize set to arbitrary number
//  (will return RL_OVERRUN, but the line length will be calculated properly)
int read_line(DataSource* ds, char* buf, int bufsize, int* line_length);
