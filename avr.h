#ifndef AVR_H
#define AVR_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define bit(pin, bit, on) pin = (on ? (pin | bit) : (pin & ~bit))

#define sin(on) bit(PORTA, _BV(0), on)
#define sclk(on) bit(PORTA, _BV(1), on)
#define sltch(on) bit(PORTA, _BV(2), on)

#define nesclk(on) bit(PORTA, _BV(3), on)
#define nesltch(on) bit(PORTA, _BV(4), on)
#define NESOUT 5
#define nesout() ((PINA & _BV(NESOUT)) << NESOUT)

void init(void);

#endif
