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

// Interrupt called every 256 * 64 microseconds
ISR(TIM0_OVF_vect)
{
	uint32_t m = micros;
	
	m += (256 * 64);
	if (m >= JIFFY_uS) {
		m -= JIFFY_uS;
		tick = true;
		jiffies += 1;
	}
	micros = m;
}


void
init(void)
{
	// Set prescaler to 16, so we get clock ticks at exactly 1MHz
	CLKPR = _BV(CLKPCE);
	CLKPR = 0x04;

	// Set timer 0 interrupt clock divider to 64
	TCCR0B = _BV(CS01) + _BV(CS00);

	// enable timer 0 overflow interrupt
	TIMSK0 |= _BV(TOIE0);    //enable compare match interrupt;
	
	// Enable interrupts
	sei();

	DDRA = ~(_BV(NESOUT));
	DDRB = 0xff;

	PORTA = 0;
	PORTB = 0xff;
}
