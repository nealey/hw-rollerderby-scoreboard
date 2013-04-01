#ifndef TI_H
#define TI_H

#include <stdint.h>
#include <msp430.h>

#define _BV(x) (1 << x)

#define JIFFIES_PER_SECOND 60

#define mode(on) bit(P1OUT, _BV(0), on)
#define sin(on) bit(P1OUT, _BV(1), on)
#define sclk(on) bit(P1OUT, _BV(2), on)
#define xlat(on) bit(P1OUT, _BV(3), on)
// Connect GSCLK to SCLK
// Connect BLANK to XLAT
// TRUST ME, THIS TOTALLY WORKS

#define NESOUT 6
#define nesclk(on) bit(P1OUT, _BV(4), on)
#define nesltch(on) bit(P1OUT, _BV(5), on)
#define nesout() ((P1IN & _BV(NESOUT)) << NESOUT)

void init();

#endif
