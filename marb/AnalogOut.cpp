#include "AnalogOut.h"

AnalogOut::AnalogOut(uint8_t p) : PortBase(p) {
	setPinDirection(p, OUTPUT);
}

uint8_t AnalogOut::value() {
	return _value;
}

void AnalogOut::setValue(uint8_t val) {
	if (val == LOW) {
		write(LOW);
	} else if (val == HIGH) {
		write(HIGH);
	} else
	if (_pin == 6) {
		TCCR0A |= (1 << COM0B1);
		OCR0B = val;
	} else
	if (_pin == 7) {
		TCCR0A |= (1 << COM0A1);
		OCR0A = val;
	} else
	if (_pin == 10) {
		TCCR1A |= (1 << COM1A1);
		OCR1A = val;
	} else
	if (_pin == 11) {
		TCCR1A |= (1 << COM1B1);
		OCR1B = val;
	} else
	if (_pin == 12) {
		TCCR2A |= (1 << COM2A1);
		OCR2A = val;
	} else
	if (_pin == 4) {
		TCCR2A |= (1 << COM2B1);
		OCR2B = val;
	} else 
	if (val < 127) {
		write(LOW);
	} else {
		write(HIGH);
	}
	_value = val;
}
