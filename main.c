#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// Number of shift registers in your scoreboard
// If you want scores to go over 199, you need 8
const int nsr = 6;

volatile bool tick = false; // Set high when clock ticks
uint16_t time = 0; // Tenths of a second elapsed since boot

// Clocks are in deciseconds
uint16_t score_a = 0;
uint16_t score_b = 0;
int16_t period_clock = -600 * 30;
int16_t jam_clock = -600 * 2;
enum {
	SETUP,
	JAM,
	LINEUP,
	TIMEOUT
} state = SETUP;

uint8_t last_controller = 0;


#define MODE _BV(0)
#define SIN _BV(1)
#define SCLK _BV(2)
#define XLAT _BV(3)
// Connect GSCLK to SCLK
// Connect BLANK to XLAT
// TRUST ME, THIS TOTALLY WORKS

#define NESCLK _BV(4)
#define NESLTCH _BV(5)
#define NESSOUT _BV(6)


// NES Controller buttons

#define BTN_A _BV(7)
#define BTN_B _BV(6)
#define BTN_SELECT _BV(5)
#define BTN_START _BV(4)
#define BTN_UP _BV(3)
#define BTN_DOWN _BV(2)
#define BTN_LEFT _BV(1)
#define BTN_RIGHT _BV(0)


#define bit(pin, bit, on) pin = (on ? (pin | bit) : (pin & ~bit))

const uint8_t seven_segment_digits[] = {
	0x7b, 0x09, 0xb3, 0x9b, 0xc9, 0xda, 0xfa, 0x0b, 0xfb, 0xdb,
};

#define mode(on) bit(PORTD, MODE, on)
#define sin(on) bit(PORTD, SIN, on)
#define sclk(on) bit(PORTD, SCLK, on)
#define xlat(on) bit(PORTD, XLAT, on)

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
	uint16_t clk;

	write_num(score_a, 3);
	
	if ((state == TIMEOUT) && (jam_clock % 8 == 0)) {
		for (clk = 0; clk < 4; clk += 1) {
			write(0);
		}
	} else {
		clk = (abs(period_clock / 10) / 60) * 100;
		clk += abs(period_clock / 10) % 60;
		write_num(clk, 4);
	}
	
	clk = (abs(jam_clock / 600) % 10) * 1000;
	clk +=  abs(jam_clock) % 600;
	write_num(clk, 4);
	
	write_num(score_b, 2);

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
	uint8_t ret = 0;

	PORTD |= NESLTCH;
	PORTD &= ~NESLTCH;
	
	for (i = 0; i < 8; i += 1) {
		state <<= 1;
		if (PIND & NESSOUT) {
			// Button not pressed
		} else {
			state |= 1;
		}
		PORTD |= NESCLK;
		PORTD &= ~NESCLK;
	}
	
	// Only report button down events.
	ret = (last_controller ^ state) & state;
	last_controller = state;

	return ret;
}

void
update_controller()
{
	uint8_t val = nesprobe();

	if (val & BTN_A) {
		switch (state) {
		case JAM:
			jam_clock = -300;
			state = LINEUP;
			break;
		default:
			jam_clock = -600 * 2;
			state = JAM;
			break;
		}
	}
	
	if (val & BTN_START) {
		switch (state) {
		case TIMEOUT:
			break;
		default:
			state = TIMEOUT;
			jam_clock = 1;
		}
	}

	if (val & BTN_LEFT) {
		score_a += 1;
	}
	
	if (val & BTN_RIGHT) {
		score_b += 1;
	}
}

/*
 * Run logic for this decisecond
 */
void
loop()
{
	switch (state) {
	case SETUP:
		break;
	default:
		if (jam_clock) {
			jam_clock += 1;
		}
	}
	
	switch (state) {
	case SETUP:
	case TIMEOUT:
		break;
	default:
		if (period_clock) {
			period_clock += 1;
		}
	}
	
	draw();
}

int
main(void)
{
	uint16_t jiffies = 0;

	DDRD = ~(NESSOUT);
	DDRB = 0xff;

	PORTD = 0;
		
	//setup_gs();
	setup_dc();

	// Now actually run
	for (;;) {
		uint32_t i;
		
		update_controller();		

		if (tick) {
			tick = false;
			jiffies += 1;
			
			if (jiffies == 6) {
				jiffies = 0;
				time += 1;
			
				loop();

				PORTB ^= 0xff;
			}
		}
		
		// XXX: fix timer_a
		for (i = 0; i < 20000; i += 1) {
			tick = !tick;
		}
		tick = true;
	}
}

// XXX: reenable this
// Timer A0 interrupt service routine
//__attribute__((interrupt(TIMER0_A0_VECTOR)))
void timer_a(void)
{
	tick = true;
}
