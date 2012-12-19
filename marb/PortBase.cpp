#include "PortBase.h"

PortBase::PortBase(uint8_t p) {
	_pin = p;
	_outputPort = getOutputRegisterForPin(p);
}

PortBase::PortBase(AnalogPin p) {
	_pin = p;
}


/*
 * p is 1-20, return 0-19
 */
uint8_t PortBase::getPinOnPort(uint8_t p) {
	if (p < 9)
		return p-1;
	if (p > 8 && p < 15)
		return p-9;
	if (p > 14 && p < 21)
		return p-15;
		
	return 0;
}

void PortBase::setPinDirection(uint8_t p, PinDirection dir) {
	
	_pinOnPort = getPinOnPort(p);
	_dirReg = getDirectionRegisterForPin(p);
	
	uint8_t oldSREG = SREG;
	if (dir == INPUT) {
		*_dirReg &= ~(1 << _pinOnPort);
	} else {
		*_dirReg |= (1 << _pinOnPort);
	}
	SREG = oldSREG;
}

void PortBase::turnOffPWMForPin(uint8_t p) {
	if (_pin == 6) {
		TCCR0A &= ~(1 << COM0B1);
	} else
	if (_pin == 7) {
		TCCR0A &= ~(1 << COM0A1);
	} else
	if (_pin == 10) {
		TCCR1A &= ~(1 << COM1A1);
	} else
	if (_pin == 11) {
		TCCR1A &= ~(1 << COM1B1);
	} else
	if (_pin == 12) {
		TCCR2A &= ~(1 << COM2A1);
	} else
	if (_pin == 4) {
		TCCR2A &= ~(1 << COM2B1);
	}
}

volatile uint8_t *PortBase::getDirectionRegisterForPin(uint8_t p) {
	if (p < 9)
		return &DDRD;
	if (p > 8 && p < 15)
		return &DDRB;
	if (p > 14 && p < 21)
		return &DDRC;
		
	return 0;
}

volatile uint8_t *PortBase::getOutputRegisterForPin(uint8_t p) {
	if (p < 9)
		return &PORTD;
	if (p > 8 && p < 15)
		return &PORTB;
	if (p > 14 && p < 21)
		return &PORTC;
	
	return 0;
}

volatile uint8_t *PortBase::getInputRegisterForPin(uint8_t p) {
	if (p < 9)
		return &PIND;
	if (p > 8 && p < 15)
		return &PINB;
	if (p > 14 && p < 21)
		return &PINC;
	
	return 0;
}

void PortBase::write(uint8_t val) {
	
	// turn off PWM, if on timer
	turnOffPWMForPin(_pin);
	
	uint8_t oldSREG = SREG;
	cli();
	
	if (val == LOW) {
		*_outputPort &= ~(1 << _pinOnPort);
	} else {
		*_outputPort |= (1 << _pinOnPort);
	}
	SREG = oldSREG;
}
