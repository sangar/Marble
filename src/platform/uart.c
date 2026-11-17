#include "uart.h"

#include <avr/interrupt.h>
#include <avr/io.h>

enum { UART_BAUD = 115200, UART_BUFFER_SIZE = 64 };

typedef struct UartBuffer {
    uint8_t bytes[UART_BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
} UartBuffer;

// An interrupt handler takes no arguments, so the buffer it fills lives at file scope.
static volatile UartBuffer uart_received; // modern-c: allow global-mutable

ISR(USART_RX_vect) {
    uint8_t byte = UDR0;
    uint8_t next = (uint8_t)((uart_received.head + 1) % UART_BUFFER_SIZE);
    if (next == uart_received.tail) return;

    uart_received.bytes[uart_received.head] = byte;
    uart_received.head = next;
}

void uart_start(void) {
    UCSR0A = 1 << U2X0;
    UBRR0 = F_CPU / 8 / UART_BAUD - 1;
    UCSR0C = 1 << UCSZ01 | 1 << UCSZ00;
    UCSR0B = 1 << RXEN0 | 1 << TXEN0 | 1 << RXCIE0;
    sei();
}

bool uart_read(uint8_t *byte) {
    uint8_t tail = uart_received.tail;
    if (tail == uart_received.head) return false;

    *byte = uart_received.bytes[tail];
    uart_received.tail = (uint8_t)((tail + 1) % UART_BUFFER_SIZE);
    return true;
}

void uart_write(const char *bytes, size_t count) {
    for (size_t i = 0; i < count; i++) {
        while (!(UCSR0A & 1 << UDRE0)) {}
        UDR0 = (uint8_t)bytes[i];
    }
}
