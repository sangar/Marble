#include "DigitalIn.h"

DigitalIn::DigitalIn(uint8_t p) : PortBase(p) {
	setPinDirection(p, INPUT);
	_inputPort = getInputRegisterForPin(p);
	setMode(PULL_DOWN);
}

uint8_t DigitalIn::state() {
	
	// Turn off PWM, if on timer
	turnOffPWMForPin(_pin);
	
	switch (_mode) {
		case PULL_UP:
			if ((*_inputPort & (1 << _pinOnPort)) == 0)
				return HIGH;
		case PULL_DOWN:
			if ((*_inputPort & (1 << _pinOnPort)) > 0)
				return HIGH;
	}

	return LOW;
}

void DigitalIn::setMode(PinMode mode) {
	switch (mode) {
		case PULL_UP:
			*_outputPort |= (1 << _pinOnPort);
			_mode = PULL_UP;
			break;
		case PULL_DOWN:
			*_outputPort &= ~(1 << _pinOnPort);
			_mode = PULL_DOWN;
			break;
	}
}
