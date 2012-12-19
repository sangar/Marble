#include "DigitalOut.h"

DigitalOut::DigitalOut(uint8_t p) : PortBase(p) {
	setPinDirection(p, OUTPUT);
}

uint8_t DigitalOut::state() {
	if ((*_outputPort & (1 << _pinOnPort)) > 0)
		return HIGH;
	
	return LOW;
}

void DigitalOut::flip() {
	if (state() == HIGH)
		write(LOW);
	else
		write(HIGH);
}
