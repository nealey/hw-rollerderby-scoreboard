#include <stdbool.h>
#include <stdint.h>
#include "ti.h"

extern volatile bool tick;
extern volatile uint32_t jiffies;

// Count microseconds
volatile uint32_t micros = 0;

// Interrupt called every 1024 µs
__attribute__((interrupt(TIMER0_A0_VECTOR)))
void
timer_a(void)
{
	tick = true;
	jiffies += 1;
}



void
init()
{
	WDTCTL = WDTPW + WDTHOLD;	// Disable Watchdog Timer
	P1DIR = ~(_BV(NESOUT));
	P1OUT = 0;

	CCTL0 |= CCIE;		// Trigger interrupt on A checkpoint
	TACTL = TASSEL_2 + MC_1;	// Set timer A to SMCLCK, up mode
	TACCR0 = 0x4444;	// Interrupt 60 times per second

	__enable_interrupt();
}
