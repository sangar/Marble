#include "SPI.h"

SPI::SPI(SPIPin ss, SPIPin mosi, SPIPin miso, SPIPin sck) {
	
//	uint8_t oldSREG = SREG;
	
	DDRB |= (1 << ss); // SS, OUTPUT
	DDRB |= (1 << mosi); // MOSI, OUTPUT
	DDRB &= ~(1 << miso); // MISO, INPUT
	DDRB |= (1 << sck); // SCK, OUTPUT
	
//	SREG = oldSREG;
	
	link();
}

void SPI::link(void) {
	// Default setup
	mode();
	format();
	frequency();
	
	SPCR |= (1 << MSTR); // Set to master SPI
	SPCR |= (1 << SPE);  // SPI enable
}


void SPI::unlink(void) {
	SPCR &= ~(1 << SPE); // SPI disable
}

void SPI::mode(SPIMode mode) {
	SPCR = (SPCR & ~SPI_MODE_MASK) | mode;
}

void SPI::format(SPIBitOrder bitOrder) {
	if (bitOrder == LSB) {
		SPCR |= (1 << DORD);
	} else {
		SPCR &= ~(1 << DORD);
	}
}

void SPI::frequency(SPIFreq freq) {
	SPCR = (SPCR & ~FREQUENCY_MASK) | freq;
	SPSR = (SPSR & ~FREQ_SPI2X_MASK) | (2 >> freq);
}

uint8_t SPI::write(uint8_t data) {
	SPDR = data;
	
	while (bit_is_clear(SPSR, SPIF)); // SPIF set when transfer is complete
	
	return SPDR;
}