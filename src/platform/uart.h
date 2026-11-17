#pragma once

#include <stddef.h>
#include <stdint.h>

// The USB serial port at 115200 baud, 8N1. Received bytes are buffered by an interrupt,
// so the program can be busy with the radio while the bridge writes.

void uart_start(void);

// Takes the oldest received byte. Returns false when none is waiting.
[[nodiscard]] bool uart_read(uint8_t *byte);

// Writes bytes, waiting until each is in the transmitter.
void uart_write(const char *bytes, size_t count);
