#ifndef LIBAVR_H
#define LIBAVR_H

/* AVR modules */
#include <alloca.h> 			// Allocate space in the stack
#include <assert.h> 			// Diagnostics
#include <ctype.h> 				// Character Operations
#include <errno.h> 				// System Errors
#include <inttypes.h> 			// Integer Type conversions
#include <math.h> 				// Mathematics
#include <setjmp.h> 			// Non-local goto
#include <stdint.h> 			// Standard Integer Types
#include <stdio.h> 				// Standard IO facilities
#include <stdlib.h> 			// General utilities
#include <string.h> 			// Strings
#include <avr/boot.h> 			// Bootloader Support Utilities
#include <avr/cpufunc.h> 		// Special AVR CPU functions
#include <avr/eeprom.h> 		// EEPROM handling
#include <avr/fuse.h> 			// Fuse Support
#include <avr/interrupt.h> 		// Interrupts
#include <avr/io.h> 			// AVR device-specific IO definitions
#include <avr/lock.h> 			// Lockbit Support
#include <avr/pgmspace.h> 		// Program Space Utilities
#include <avr/power.h> 			// Power Reduction Management
#include <avr/sfr_defs.h> 		// Special function registers
// Additional notes from <avr/sfr_defs.h>
#include <avr/signature.h> 		// Signature Support
#include <avr/sleep.h> 			// Power Management and Sleep Modes
#include <avr/version.h> 		// avr-libc version macros
#include <avr/wdt.h> 			// Watchdog timer handling
#include <util/atomic.h> 		// Atomically and Non-Atomically Executed Code Blocks
#include <util/crc16.h> 		// CRC Computations
#include <util/delay.h> 		// Convenience functions for busy-wait delay loops
#include <util/delay_basic.h> 	// Basic busy-wait delay loops
#include <util/parity.h> 		// Parity bit generation
#include <util/setbaud.h> 		// Helper macros for baud rate calculations
#include <util/twi.h> 			// TWI bit mask definitions
#include <compat/deprecated.h> 	// Deprecated items
#include <compat/ina90.h> 		// Compatibility with IAR EWB 3.x

#endif