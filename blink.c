#include <avr/io.h>
#include <avr/interrupt.h>

/* This only works out for a 16MHz or 8MHz clock */
#define CLOCK_HZ 16000000
#define TICKS_PER_SECOND (CLOCK_HZ / 256)
#define TICKS_PER_JIFFY (TICKS_PER_SECOND / 10)

volatile uint32_t jiffies = 0;

ISR(TIM1_COMPA_vect)
{
	jiffies += 1;
}

int
main(void)
{
	unsigned char counter;

	/*
	 * set PORTB for output
	 */
	DDRB = 0xFF;
	PORTB = 0x00;

	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;
	OCR1A = TICKS_PER_JIFFY - 1;
	TCCR1B |= (1 << WGM12);
	TCCR1B |= (1 << CS12);
	TIMSK1 |= (1 << OCIE1A);

	sei();

	while (1) {
		PORTB = 0xFF * ((jiffies/ 10) % 2);
	}

	return 0;
}
