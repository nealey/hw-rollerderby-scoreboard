#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "avr.h"
#include "config.h"

/* Clock must be a multiple of 2MHz or there will be clock drift */
#define TICKS_PER_SECOND (CLOCK_HZ / 64)
#define TICKS_PER_JIFFY (TICKS_PER_SECOND / 10)

#define cbi(byt, bit)   (byt &= ~_BV(bit))
#define sbi(byt, bit)   (byt |= _BV(bit))

extern volatile bool tick;
extern volatile uint32_t jiffies;

// Interrupt called every jiffy
ISR(TIM1_COMPA_vect)
{
	jiffies += 1;
	tick = true;
}

void
init(void)
{
	int i;

	DDRA = ~(_BV(NESOUT));
	DDRB = 0xff;

	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;
	OCR1A = TICKS_PER_JIFFY - 1;
	TCCR1B |= _BV(WGM12);
	TCCR1B |= _BV(CS11) | _BV(CS10);
	TIMSK1 |= _BV(OCIE1A);
	
	bit(PORTA, _BV(7), true);

	sei();
}
