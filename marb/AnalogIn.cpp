#include "AnalogIn.h"

AnalogIn::AnalogIn(AnalogPin p) : PortBase(p) {}

uint16_t AnalogIn::read() {

	ADMUX = ((1 << REFS0) | _pin);
	if (_pin == TEMP) {
		ADMUX |= (1 << REFS1);
	} else {
		ADMUX &= ~(1 << REFS1);
	}
	ADMUX &= ~(1 << ADLAR);
	
	ADCSRA |= (1 << ADSC); // start convertion
	
	while (bit_is_set(ADCSRA, ADSC));
	
	return (ADCL | (ADCH << 8));
}
