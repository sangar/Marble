#ifndef SERIAL_H
#define SERIAL_H

#include "Print.h"

struct ring_buffer;

class SerialCom : public Print {
private:
	ring_buffer *_rx_buffer;
	ring_buffer *_tx_buffer;
	
public:
	SerialCom(ring_buffer *rx_buffer, ring_buffer *tx_buffer);
	
	void link(uint16_t baud=BAUD);
	void unlink(void);
	uint8_t readable(void);
	uint8_t read(void);
	void flush(void);
	virtual uint8_t write(uint8_t);
	
	using Print::write;
};

extern SerialCom Serial;

#endif
