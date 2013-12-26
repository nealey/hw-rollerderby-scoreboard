#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "avr.h"

volatile uint32_t jiffies = 0;
volatile bool tick = false;

void
loop(void)
{
	if (tick) {
		tick = false;

		if (jiffies % 10 == 0) {
			PORTB ^= 0xff;
		}
	}
}

int
main(void)
{
	init();

	for (;;) {
		loop();
	}

	return 0;
}
