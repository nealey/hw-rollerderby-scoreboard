#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

const uint8_t digits[] = {
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f,
	0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71
};

// seconds/60
volatile uint16_t jiffies = 0;
unsigned int i = 0;

#define MODE BIT0
#define SIN BIT1
#define SCLK BIT2
#define XLAT BIT3
#define BLANK BIT4
#define GSCLK BIT5

#define bit(pin, bit, on) pin = (on ? (pin | bit) : (pin & ~bit))

#define mode(on) bit(P1OUT, MODE, on)
#define sin(on) bit(P1OUT, SIN, on)
#define sclk(on) bit(P1OUT, SCLK, on)
#define xlat(on) bit(P1OUT, XLAT, on)
#define blank(on) bit(P1OUT, BLANK, on)
#define gsclk(on) bit(P1OUT, GSCLK, on)

void
latch()
{
	xlat(true);
	xlat(false);
}

void
write(uint16_t number)
{
	int i;
	int j;
	
	// MSB first
	for (i = 15; i >= 0; i -= 1) {
		sin(number & (1 << i));

		for (j = 0; j < 12; j += 1) {
			sclk(true);
			sclk(false);
		}
	}
}

void
blip()
{
	int i;
	
	for (i = 0; i < 1000; i += 1) {
		__delay_cycles(1000);
	}
}

int
main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Disable Watchdog Timer
	P1DIR |= MODE + SIN + SCLK + XLAT + BLANK + GSCLK + BIT6;		// P1 output bits

	P1OUT = 0;
		
	// Set up the stupid DC crappo
	mode(true);
	sin(false);
	for (i = 0; i < 96; i += 1) {
		sclk(true);
		sclk(false);
	}
	latch();
	mode(false);
		
	// Zero out numbers
	sin(false);
	for (i = 0; i < 192; i += 1) {
		sclk(true);
		sclk(false);
	}
	latch();
	

#if 1
	CCTL0 |= CCIE;		// Trigger interrup on   A checkpoint
	TACTL = TASSEL_2 + MC_1;		// Set timer A to SMCLCK, up mode
	TACCR0 = 0x4444;
	
	__enable_interrupt();
#endif

	// Now actually run
	for (;;) {
		if ((jiffies % 60) == 0) {
			write(0xffff);
	
			blank(true);
			latch();
			blank(false);
		} else {
			P1OUT ^= GSCLK;
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
