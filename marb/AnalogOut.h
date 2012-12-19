#ifndef ANALOG_OUT_H
#define ANALOG_OUT_H

#include "PortBase.h"

class AnalogOut : public PortBase {

private:
	uint8_t _value;
	
public:
	AnalogOut(uint8_t);
	
	uint8_t value(void);
	void setValue(uint8_t);
	
	AnalogOut& operator= (const uint8_t value) {
		setValue(value);
		return *this;
	}

};

#endif
