#ifndef DIGITAL_IN_H
#define DIGITAL_IN_H

#include "PortBase.h"

enum PinMode : uint8_t {
	PULL_UP,
	PULL_DOWN
};

class DigitalIn : public PortBase {
private:
	PinMode _mode;
	
public:
	DigitalIn(uint8_t);
	
	uint8_t state(void);
	void setMode(PinMode);
	
};

#endif
