#ifndef MARBLE_H
#define MARBLE_H

/* Marble modules */
#include "DigitalOut.h"
#include "DigitalIn.h"
#include "AnalogOut.h"
#include "AnalogIn.h"
#include "SerialCom.h"
#include "SPI.h"

/* Marble typedefs */
#ifndef byte
#define byte unsigned char
#endif

/* Marble initialization class */
class Marble {
	
private:
	void init(void);
	
public:
	Marble(void);
	
	void run(void);

protected:
	virtual void setup(void);
	virtual void loop(void);
	
};

#endif
