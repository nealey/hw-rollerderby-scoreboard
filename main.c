#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

// Number of shift registers in your scoreboard
// If you want scores to go over 199, you need 8
const int nsr = 6;

volatile bool tick = false; // Set high when clock ticks
uint16_t time = 0; // Tenths of a second elapsed since boot

// Clocks are in deciseconds
uint16_t score_a = 0;
uint16_t score_b = 0;
uint16_t period_clock = 600 * 30;
uint16_t jam_clock = 600 * 2;
enum {
	SETUP,
	JAM,
	LINEUP,
	TIMEOUT
} state = JAM;

uint8_t controller = 0;


#define MODE BIT0
#define SIN BIT1
#define SCLK BIT2
#define XLAT BIT3
// Connect GSCLK to SCLK
// Connect BLANK to XLAT
// TRUST ME, THIS TOTALLY WORKS

#define NESCLK BIT4
#define NESLTCH BIT5
#define NESSOUT BIT7

#define bit(pin, bit, on) pin = (on ? (pin | bit) : (pin & ~bit))

const uint8_t seven_segment_digits[] = {
#if defined(WIKIPEDIA)
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71
#elseif defined(TOPDOWN)
	0x7e, 0x48, 0x3d, 0x6d, 0x4b, 0x67, 0x77, 0x4c, 0x7f, 0x6f,
#else
	0x7b, 0x09, 0xb3, 0x9b, 0xc9, 0xda, 0xfa, 0x0b, 0xfb, 0xdb
#endif
};

#define mode(on) bit(P1OUT, MODE, on)
#define sin(on) bit(P1OUT, SIN, on)
#define sclk(on) bit(P1OUT, SCLK, on)
#define xlat(on) bit(P1OUT, XLAT, on)

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
	uint16_t divisor = 1;
	int i;
	
	for (i = 1; i < digits; i += 1) {
		divisor *= 10;
	}
	
	for (i = 0; i < digits; i += 1) {
		uint16_t n = (number / divisor) % 10;
		
		write(seven_segment_digits[n]);
		divisor /= 10;
	}
}

void
blip()
{
	int i;
	
	for (i = 0; i < 12; i += 1) {
		__delay_cycles(1);
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

/*
 * Update all the digits
 */
void
draw()
{
#if 1
	uint16_t clk;

	write_num(score_a, 3);
	
	clk = ((period_clock / 10) / 60) * 100;
	clk = clk + ((period_clock / 10) % 60);
	write_num(clk, 4);
	
	clk = ((jam_clock / 600) % 10) * 1000;
	clk = clk + (jam_clock % 600);
	write_num(clk, 4);
	
	write_num(score_b, 2);
#else
	int i;
	
	for (i = 0; i < 12; i += 1) {
		write_num(sizeof(seven_segment_digits) - 1, 1);
	}
#endif

	latch();
	pulse();
}

/*
 * Probe the NES controller
 */
uint8_t
nesprobe()
{
	int i;
	uint8_t state = 0;

	P1OUT |= NESLTCH;
	P1OUT &= ~NESLTCH;
	
	for (i = 0; i < 8; i += 1) {
		state <<= 1;
		if (P1IN & NESSOUT) {
			// Button not pressed
		} else {
			state |= 1;
		}
		P1OUT |= NESCLK;
		P1OUT &= ~NESCLK;
	}
	
	return state;
}

void
update_controller()
{
	uint8_t val = nesprobe();

	if (val & 0x80) {
		switch (state) {
		case JAM:
			jam_clock = 300;
			state = LINEUP;
			break;
		default:
			jam_clock = 600 * 2;
			state = JAM;
			break;
		}
	}
}

/*
 * Run logic for this decisecond
 */
void
loop()
{
	if (jam_clock) {
		jam_clock -= 1;
	}
	
	if (period_clock) {
		period_clock -= 1;
	}
	
	draw();
}

int
main(void)
{
	uint16_t jiffies = 0;

	WDTCTL = WDTPW + WDTHOLD;	// Disable Watchdog Timer
	P1DIR |= MODE + SIN + SCLK + XLAT + NESCLK + NESLTCH + BIT6;		// P1 output bits
	P1DIR &= ~(NESSOUT);

	P1OUT = 0;
		
	setup_gs();
	setup_dc();

	// Enable interrupts
	CCTL0 |= CCIE;		// Trigger interrupt on   A checkpoint
	TACTL = TASSEL_2 + MC_1;		// Set timer A to SMCLCK, up mode
	TACCR0 = 0x4444;	// Interrupt 60 times per second
	
	__enable_interrupt();

	// Now actually run
	for (;;) {
		update_controller();		

		if (tick) {
			tick = false;
			jiffies += 1;
			
			if (jiffies == 6) {
				jiffies = 0;
				time += 1;
			
				loop();

				P1OUT ^= BIT6;
			}
		}
	}
}

// Timer A0 interrupt service routine
__attribute__((interrupt(TIMER0_A0_VECTOR)))
void timer_a(void)
{
	tick = true;
}
