#include "SerialCom.h"

#define _BUFFER_SIZE 256

struct ring_buffer {
	uint8_t buffer[_BUFFER_SIZE];
	uint8_t head;
	uint8_t tail;
};

ring_buffer rx_buffer = {{0}, 0, 0};
ring_buffer tx_buffer = {{0}, 0, 0};

inline void store_byte(uint8_t b, ring_buffer *buffer) {
	uint8_t i = (buffer->head + 1) % _BUFFER_SIZE;
	
	if (i != buffer->tail) {
		buffer->buffer[buffer->head] = b;
		buffer->head = i;
	}
}

SIGNAL(USART_RX_vect) {
	uint8_t recv = UDR0;
	store_byte(recv, &rx_buffer);
}

ISR(USART_UDRE_vect) {
	if (tx_buffer.head == tx_buffer.tail) {
		UCSR0B &= ~(1 << UDRIE0);
	} else {
		unsigned char c = tx_buffer.buffer[tx_buffer.tail];
		tx_buffer.tail = (tx_buffer.tail + 1) % _BUFFER_SIZE;
		
		UDR0 = c;
	}
}

/*
 * Constructor
 */
SerialCom::SerialCom(ring_buffer *rx_buffer, ring_buffer *tx_buffer) {
	_rx_buffer = rx_buffer;
	_tx_buffer = tx_buffer;
}

/*
 * Public methods
 */
#define MYUBRR (F_CPU/16/BAUD -1)

void SerialCom::link(uint16_t baud) {
	uint16_t ubrr = (F_CPU/16/baud -1);
	UBRR0H = (ubrr >> 8);
	UBRR0L = ubrr; // MYUBRR
	
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);      	// Enable receiver and transmitter
	UCSR0B |= (1 << RXCIE0);                    	// Enable reciever interrupt
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);		// Set frame: 8data, 1 stp
	UCSR0B &= ~(1 << UDRIE0);
}

void SerialCom::unlink() {
	UCSR0B &= ~((1 << RXEN0) | (1 << TXEN0));      	// Disable receiver and transmitter
	UCSR0B &= ~(1 << RXCIE0);						// Disable receiver interrupt
	UCSR0B &= ~(1 << UDRIE0);
	
	_rx_buffer->head = _rx_buffer->tail;
}

uint8_t SerialCom::readable() {
	return (_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % _BUFFER_SIZE;
}

uint8_t SerialCom::read() {
	if (_rx_buffer->head == _rx_buffer->tail) {
		return -1;
	} else {
		uint8_t c = _rx_buffer->buffer[_rx_buffer->tail];
		_rx_buffer->tail = (_rx_buffer->tail + 1) % _BUFFER_SIZE;
		return c;
	}
}

void SerialCom::flush() {
	while (_tx_buffer->head != _tx_buffer->tail);
}

uint8_t SerialCom::write(uint8_t b) {
	uint8_t i = (_tx_buffer->head + 1) % _BUFFER_SIZE;
	
	if (i == _tx_buffer->tail)
		return 0;
	
	_tx_buffer->buffer[_tx_buffer->head] = b;
	_tx_buffer->head = i;

	UCSR0B |= (1 << UDRIE0);

	return 1;
}

SerialCom Serial(&rx_buffer, &tx_buffer);
