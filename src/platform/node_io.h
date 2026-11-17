#pragma once

#include <stdint.h>

// What a node controls and senses: an LED on D6, a button from D2 to ground,
// and the ATmega328P's built-in temperature sensor.

void node_io_start(void);

// Sets the LED's brightness: 0 is off, 254 is full.
void light_set(uint8_t level);

bool button_is_pressed(void);

// The chip's temperature in hundredths of a degree Celsius.
int16_t temperature_read(void);
