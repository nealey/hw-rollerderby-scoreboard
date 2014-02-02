#ifndef AVR_H
#define AVR_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define bit(pin, bit, on) pin = (on ? (pin | bit) : (pin & ~bit))

#define sin(on) bit(PORTA, _BV(SIN), on)
#define sclk(on) bit(PORTA, _BV(SCLK), on)
#define sltch(on) bit(PORTA, _BV(SLTCH), on)

#define nesclk(on) bit(PORTA, _BV(NESCLK), on)
#define nesltch(on) bit(PORTA, _BV(NESLTCH), on)
#define nesout() ((PINA & _BV(NESOUT)) << NESOUT)

void avr_init(void);

#endif
