#ifndef SPI_H
#define SPI_H

#include "PortBase.h"

enum SPIPin : uint8_t {
	SS = DDB2,
	MOSI = DDB3,
	MISO = DDB4,
	SCK = DDB5
};

#define SPI_MODE_MASK 	0x0C

enum SPIMode : uint8_t {
	MODE0 = 0x00,
	MODE1 = 0x04,
	MODE2 = 0x08,
	MODE3 = 0x0C
};

enum SPIBitOrder : uint8_t {
	MSB = 0x00,
	LSB = 0x01
};

#define FREQUENCY_MASK 	0x03
#define FREQ_SPI2X_MASK	0x01

enum SPIFreq : uint8_t {
	FREQ2 = 0x04,
	FREQ4 = 0x00,
	FREQ8 = 0x05,
	FREQ16 = 0x01,
	FREQ32 = 0x06,
	FREQ64 = 0x02,
	FREQ128 = 0x03
};

class SPI {
private:
	
public:
	SPI(SPIPin ss = SS, SPIPin mosi = MOSI, SPIPin miso = MISO, SPIPin sck = SCK);
	void link(void);
	void unlink(void);
	
	void mode(SPIMode mode = MODE0);
	void format(SPIBitOrder bitOrder = MSB); // Data order
	void frequency(SPIFreq freq = FREQ16);
	
	uint8_t write(uint8_t);
};

#endif