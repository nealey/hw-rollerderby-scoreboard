#include <msp430.h>
#include <stdint.h>

const uint8_t digits[] = {
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f,
	0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71
};

// seconds/60
volatile uint16_t jiffies = 0;
unsigned int i = 0;

int
main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Disable Watchdog Timer
	P1DIR |= BIT0 + BIT6;		// P1.0 output
	P1OUT = (P1OUT & ~BIT6) | BIT0;
	
#if 1
	CCTL0 |= CCIE;		// Trigger interrup on Timer A checkpoint
	TACTL = TASSEL_2 + MC_1;		// Set timer A to SMCLCK, up mode
	TACCR0 = 0x4444;
	
	__enable_interrupt();
#endif
	
	for (;;) {
		i = (i + 1) % 1000;
		if ((jiffies / 60) % 2) {
			P1OUT &= ~BIT0;
		} else {
			P1OUT |= BIT0;
		}
	}
}

// Timer A0 interrupt service routine
__attribute__((interrupt(TIMER0_A0_VECTOR)))
void timer_a(void)
{
	jiffies += 1;
	if ((jiffies / 60) % 2) {
		P1OUT |= BIT6;
	} else {
		P1OUT &= ~BIT6;
	}
}
