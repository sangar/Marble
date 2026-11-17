#pragma once

#include <stdint.h>

// ATmega328P at 16 MHz on an Arduino Uno or Nano, with an nRF24L01+ on the SPI pins:
// SCK D13, MISO D12, MOSI D11, CSN D10, CE D9.

// Sets up the radio pins, SPI and the clock.
void board_start(void);

uint8_t spi_transfer(uint8_t byte);

// Selects the radio on the SPI bus (CSN low).
void radio_select(bool selected);

// Drives the radio's CE pin: listening or transmitting while high.
void radio_enable(bool enabled);

void delay_us(uint16_t microseconds);
void delay_ms(uint16_t milliseconds);

// Time since start in ticks of 64 µs. Read it at least every 4 seconds: it counts
// the timer's progress since the previous read, and the timer wraps every 4.19 s.
enum { CLOCK_TICKS_PER_SECOND = 15625 };

typedef struct Clock {
    uint32_t ticks;
    uint16_t last_count;
} Clock;

Clock clock_start(void);
uint32_t clock_now(Clock *clock);
