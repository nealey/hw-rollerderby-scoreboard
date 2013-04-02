#ifndef AVR_H
#define AVR_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Make sure JIFFY_uS is going to be an integer and not a float!
#define JIFFIES_PER_SECOND 50
#define JIFFY_uS (1000000 / JIFFIES_PER_SECOND)

#define bit(pin, bit, on) pin = (on ? (pin | bit) : (pin & ~bit))

#define mode(on) bit(PORTD, _BV(0), on)
#define sin(on) bit(PORTD, _BV(1), on)
#define sclk(on) bit(PORTD, _BV(2), on)
#define xlat(on) bit(PORTD, _BV(3), on)
// Connect GSCLK to SCLK
// Connect BLANK to XLAT
// TRUST ME, THIS TOTALLY WORKS

#define NESOUT 6
#define nesclk(on) bit(PORTD, _BV(4), on)
#define nesltch(on) bit(PORTD, _BV(5), on)
#define nesout() ((PIND & _BV(NESOUT)) << NESOUT)

void init(void);

#endif