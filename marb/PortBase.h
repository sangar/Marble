#ifndef PORTBASE_H
#define PORTBASE_H

#include "libAVR.h"

enum PinState : uint8_t {
	LOW = 0,
	HIGH = 255
};

enum PinDirection : uint8_t {
	INPUT,
	OUTPUT
};

enum AnalogPin : uint8_t {
	A1 = 0x00,
	A2 = 0x01,
	A3 = 0x02,
	A4 = 0x03,
	A5 = 0x04,
	A6 = 0x05,
	TEMP = 0x08
};

class PortBase {

public:
	PortBase(uint8_t);
	PortBase(AnalogPin);
	
	void write(uint8_t);
	
protected:
	uint8_t _pin;
	uint8_t _pinOnPort;
	volatile uint8_t *_dirReg;
	volatile uint8_t *_outputPort;
	volatile uint8_t *_inputPort;
	
	uint8_t getPinOnPort(uint8_t);
	void setPinDirection(uint8_t, PinDirection);
	void turnOffPWMForPin(uint8_t);
	volatile uint8_t *getDirectionRegisterForPin(uint8_t);
	volatile uint8_t *getOutputRegisterForPin(uint8_t);
	volatile uint8_t *getInputRegisterForPin(uint8_t);

};

#endif
