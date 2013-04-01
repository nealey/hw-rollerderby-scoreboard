#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "avr.h"

#define cbi(byt, bit)   (byt &= ~_BV(bit))
#define sbi(byt, bit)   (byt |= _BV(bit))

extern volatile bool tick;
extern volatile uint32_t jiffies;

// Count microseconds
volatile uint32_t micros = 0;

// Interrupt called every 1024 µs
SIGNAL(TIMER0_OVF_vect)
{
	uint32_t m = micros;
	
	m += 1024;
	if (m >= JIFFY_uS) {
		m %= JIFFY_uS;
		tick = true;
		jiffies += 1;
	}
	micros = m;
}


#define mode(on) bit(PORTD, _BV(0), on)
#define sin(on) bit(PORTD, _BV(1), on)
#define sclk(on) bit(PORTD, _BV(2), on)
#define xlat(on) bit(PORTD, _BV(3), on)
// Connect GSCLK to SCLK
// Connect BLANK to XLAT
// TRUST ME, THIS TOTALLY WORKS

#define nesclk(on) bit(PORTD, _BV(4), on)
#define nesltch(on) bit(PORTD, _BV(5), on)
#define nesout() ((PIND & _BV(6)) << 6)

void
init(void)
{
	// Set timer 0 interrupt clock divider to 64
	TCCR0B = 0x03;

	// enable timer 0 overflow interrupt
	TIMSK0 = 0x01;
	
	// Enable interrupts
	sei();

	DDRD = ~(_BV(NESOUT));
	DDRB = 0xff;

	PORTD = 0;
	
}
