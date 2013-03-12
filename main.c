#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

// Number of shift registers in your scoreboard
// If you want scores to go over 199, you need 8
const int nsr = 6;

const uint8_t seven_segment_digits[] = {
#if 0
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71
#else
	0x7e, 0x48, 0x3d, 0x6d, 0x4b, 0x67, 0x77, 0x4c, 0x7f, 0x6f,
#endif
};

// seconds/60
volatile uint16_t jiffies = 0;

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
pulse()
{
	sclk(true);
	sclk(false);
}

void
write(uint8_t number)
{
	int i;
	int j;
	
	// MSB first
	for (i = 7; i >= 0; i -= 1) {
		sin(number & (1 << i));

		for (j = 0; j < 12; j += 1) {
			pulse();
		}
	}
}

void
write_num(uint16_t number, int digits)
{
	int i;
	
	for (i = 0; i < digits; i += 1) {
		write(seven_segment_digits[number % 10]);
		number /= 10;
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

/* Set up grayscale */
void
setup_gs()
{
	int i;
	
	for (i = 0; i < nsr; i += 1) {
		write(0);
	}
	latch();
}

/*
 * Set up dot correction.
 * 
 * We don't use dot correction so this is easy: set everything to full brightness.
 */
void
setup_dc()
{
	int i;

	mode(true);
	sin(true);
	for (i = 0; i < nsr * 96; i += 1) {
		pulse();
	}
	latch();
	mode(false);
}

int
main(void)
{
	int gscount = 0;

	WDTCTL = WDTPW + WDTHOLD;	// Disable Watchdog Timer
	P1DIR |= MODE + SIN + SCLK + XLAT + BLANK + GSCLK + BIT6;		// P1 output bits

	P1OUT = 0;
		
	setup_gs();
	setup_dc();

	// Enable interrupts
	CCTL0 |= CCIE;		// Trigger interrup on   A checkpoint
	TACTL = TASSEL_2 + MC_1;		// Set timer A to SMCLCK, up mode
	TACCR0 = 0x4444;	// Interrupt 60 times per second
	
	__enable_interrupt();

	// Now actually run
	for (;;) {
		if ((jiffies % 6) == 0) {
			write_num(jiffies / 6, 4);
	
			gsclk(false);
			latch();
			gscount = 4096;
		}
		
		if (gscount == 4096) {
			// Pulse BLANK when grayscale clock has cycled 4096 times.
			blank(true);
			blank(false);
			gscount = 0;
		}

		// Pulse the grayscale clock.
		gsclk(true);
		gsclk(false);
		gscount += 1;
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
