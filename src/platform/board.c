#include "board.h"

#include <avr/io.h>
#include <util/delay.h>

static_assert(F_CPU / 1024 == CLOCK_TICKS_PER_SECOND, "the clock assumes a 16 MHz crystal");

enum {
    RADIO_CE = 1 << PB1,
    RADIO_CSN = 1 << PB2,
    SPI_MOSI = 1 << PB3,
    SPI_SCK = 1 << PB5,
};

void board_start(void) {
    PORTB |= RADIO_CSN;
    DDRB |= RADIO_CE | RADIO_CSN | SPI_MOSI | SPI_SCK;
    SPCR = 1 << SPE | 1 << MSTR; // master, mode 0, 4 MHz

    TIMSK1 = 0;
    TCCR1A = 0;
    TCCR1B = 1 << CS12 | 1 << CS10; // count at F_CPU / 1024
}

uint8_t spi_transfer(uint8_t byte) {
    SPDR = byte;
    while (!(SPSR & 1 << SPIF)) {}
    return SPDR;
}

void radio_select(bool selected) {
    if (selected) {
        PORTB &= (uint8_t)~RADIO_CSN;
    } else {
        PORTB |= RADIO_CSN;
    }
}

void radio_enable(bool enabled) {
    if (enabled) {
        PORTB |= RADIO_CE;
    } else {
        PORTB &= (uint8_t)~RADIO_CE;
    }
}

void delay_us(uint16_t microseconds) {
    while (microseconds--) _delay_us(1);
}

void delay_ms(uint16_t milliseconds) {
    while (milliseconds--) _delay_ms(1);
}

Clock clock_start(void) {
    return (Clock){.last_count = TCNT1};
}

uint32_t clock_now(Clock *clock) {
    uint16_t count = TCNT1;
    clock->ticks += (uint16_t)(count - clock->last_count);
    clock->last_count = count;
    return clock->ticks;
}
