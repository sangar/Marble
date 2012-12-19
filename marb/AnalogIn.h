#ifndef ANALOG_IN_H
#define ANALOG_IN_H

#include "PortBase.h"

class AnalogIn : public PortBase {
private:
	AnalogPin _pin;
	
public:
	AnalogIn(AnalogPin);
	
	uint16_t read(void);
};

#endif
