#ifndef AVR_H
#define AVR_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define bit(pin, bit, on) pin = (on ? (pin | bit) : (pin & ~bit))

#define mode(on) bit(PORTA, _BV(0), on)
#define sin(on) bit(PORTA, _BV(1), on)
#define sclk(on) bit(PORTA, _BV(2), on)
#define xlat(on) bit(PORTA, _BV(3), on)
// Connect GSCLK to SCLK
// Connect BLANK to XLAT
// TRUST ME, THIS TOTALLY WORKS

#define NESOUT 6
#define nesclk(on) bit(PORTA, _BV(4), on)
#define nesltch(on) bit(PORTA, _BV(5), on)
#define nesout() ((PINA & _BV(NESOUT)) << NESOUT)

void init(void);

#endif