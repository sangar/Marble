#include "node_io.h"

#include <avr/io.h>
#include <util/delay.h>

enum {
    LIGHT = 1 << PD6, // OC0A, Timer0's PWM output
    BUTTON = 1 << PD2,
};

static uint16_t temperature_sample(void) {
    ADCSRA |= 1 << ADSC;
    while (ADCSRA & 1 << ADSC) {}
    return ADC;
}

void node_io_start(void) {
    DDRD |= LIGHT;
    PORTD |= BUTTON; // pull-up: the pin reads low while the button is pressed
    TCCR0A = 1 << WGM01 | 1 << WGM00; // fast PWM, 976 Hz
    TCCR0B = 1 << CS01 | 1 << CS00;

    ADMUX = 1 << REFS1 | 1 << REFS0 | 1 << MUX3; // sensor on channel 8, against the internal 1.1 V
    ADCSRA = 1 << ADEN | 1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0;
    _delay_ms(5);
    temperature_sample(); // the first conversion after switching reference is wrong
}

void light_set(uint8_t level) {
    if (level == 0) {
        // Fast PWM still pulses at a compare value of 0, so disconnect it.
        TCCR0A &= (uint8_t)~(1 << COM0A1);
        PORTD &= (uint8_t)~LIGHT;
        return;
    }
    OCR0A = level;
    TCCR0A |= 1 << COM0A1;
}

bool button_is_pressed(void) {
    return !(PIND & BUTTON);
}

int16_t temperature_read(void) {
    int32_t sum = 0;
    for (uint8_t i = 0; i < 8; i++) sum += temperature_sample();

    // Celsius = (ADC - 324.31) / 1.22, fitted by the Arduino community to typical chips.
    // Uncalibrated, a chip can be off by up to 10 °C (datasheet: Temperature Measurement).
    return (int16_t)((sum * 1250 - 3243100) / 122);
}
