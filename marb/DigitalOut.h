#ifndef DIGITAL_OUT_H
#define DIGITAL_OUT_H

#include "PortBase.h"

class DigitalOut : public PortBase {

public:
	DigitalOut(uint8_t);
	
	uint8_t state(void);
	void flip(void);

	// en om gangen?
	DigitalOut& operator= (const PinState value) {
		write(value);
		return *this;
	}
/*	
	DigitalOut& operator= (DigitalOut& pin) {
		write(pin.state());
		return *this;
	}
	
	operator PinState() {
		return state();
	}
*/
};

#endif
