#include "Marble.h"

Marble::Marble() {
	init();
}

void Marble::init() {
	sei();
	
	// Timer 0
	// set Timer 0 fast PWM mode
	TCCR0A |= (1 << WGM01) | (1 << WGM00);
	// set Timer 0 prescale to 64
	TCCR0B |= (1 << CS01) | (1 << CS00);
	
	// Timer 1
	// set Timer 1 PWM mode
	TCCR1A |= (1 << WGM10);
	// set Timer 1 prescale to 64
	TCCR1B |= (1 << CS22);
	
	// Timer 2
	// set Timer 2 PWM mode
	TCCR2A |= (1 << WGM20);
	// set Timer 2 prescale to 64
	TCCR2B |= (1 << CS22);
	
	// ADC
	// ADC prescaling
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	// enable ADC
	ADCSRA |= (1 << ADEN);
}

void Marble::setup() {}
void Marble::loop() {}

void Marble::run() {
	setup();
	
	for (;;) {
		loop();
	}
}
