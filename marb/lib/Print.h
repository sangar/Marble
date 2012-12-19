#ifndef PRINT_H
#define PRINT_H

#include "libAVR.h"

class Print {

public:
	virtual uint8_t write(uint8_t) = 0;
	
	void print(const char *);
	
	void println(void);
	void println(const char *);
	
	void printf(const char *, ...);
	
};

#endif