#pragma once

#include <stddef.h>
#include <stdint.h>

#include "message.h"

// The gateway and the bridge exchange one message per text line: "<node> <kind> <value>\n".
// "255 4 -32768" is the longest message; a longer line is not one.
enum { LINE_CAPACITY = 16 };

// Collects bytes from the serial port into lines.
typedef struct LineReader {
    char text[LINE_CAPACITY];
    uint8_t length;
    bool complete;
    bool overflowed;
} LineReader;

// Adds one byte. Returns true when the byte ends a line that fits; the line, without
// its line ending, is then text[0..length) until the next push. Longer lines are dropped.
[[nodiscard]] bool line_reader_push(LineReader *reader, char byte);

// Parses "<node> <kind> <value>". Returns false for anything that is not a valid message.
[[nodiscard]] bool line_parse(const char *text, size_t length, Message *message);

// Writes "<node> <kind> <value>\n" into out and returns its length.
size_t line_format(Message message, char out[LINE_CAPACITY]);
