#pragma once

#include <circularbuffer.hpp>

/*
 * Searches for \n. Returns the offset of the \n. Replaces \n (and possibly \r) with \0.
 * Returns offset of \n (user needs to move the read pointer beyond the \n)
 * If no line ending found, returns -1.
 */

/*
 * Searches for \n. Returns line length (without \r or \n).
 * If no line ending found, returns -1.
 */
int cbt_end_of_line(volatile CircularBuffer& buf);