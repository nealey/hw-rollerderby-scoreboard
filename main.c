#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ti.h"

// Number of shift registers in your scoreboard
// If you want scores to go over 199, you need 8
const int nsr = 6;

//
// Timing stuff
//

volatile uint32_t jiffies = 0;
volatile bool tick = false; // Set high when jiffy clock ticks

// Clocks are in deciseconds
uint16_t score_a = 0;
uint16_t score_b = 0;
int16_t period_clock = -(30 * 60 * 10);
int16_t jam_duration = -(2 * 60 * 10);
int16_t lineup_duration = (-30 * 10);
int16_t jam_clock = 0;
enum {
	SETUP,
	JAM,
	LINEUP,
	TIMEOUT
} state = SETUP;


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
	0x7b, 0x60, 0x37, 0x76, 0x6c, 0x5e, 0x5f, 0x70, 0x7f, 0x7e
};

const uint8_t setup_digits[] = {
	0x1b, 0x12, 0x72
};

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

	for (i = 0; i < digits; i += 1) {
		uint16_t n = (number / divisor) % 10;
		
		write(seven_segment_digits[n]);
		divisor *= 10;
	}
}

/*
 * Update all the digits
 */
void
draw()
{
	uint16_t clk;

	//XXX testing

	if ((state == TIMEOUT) && (jam_clock % 8 == 0)) {
		for (clk = 0; clk < 4; clk += 1) {
			write(0);
		}
	} else {
		clk = (abs(period_clock / 10) / 60) * 100;
		clk += abs(period_clock / 10) % 60;
		write_num(clk, 4);
	}
	
	write_num(score_b, 2);
	
	if (state == SETUP) {
		write(setup_digits[2]);
		write(setup_digits[1]);
		write(setup_digits[1]);
		write(setup_digits[0]);
	} else {
		clk = (abs(jam_clock / 600) % 10) * 1000;
		clk +=  abs(jam_clock) % 600;
		write_num(clk, 4);
	}
	
	write_num(score_a, 2);

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

	nesltch(true);
	nesltch(false);
	
	for (i = 0; i < 8; i += 1) {
		state <<= 1;
		if (nesout()) {
			// Button not pressed
		} else {
			state |= 1;
		}
		nesclk(true);
		nesclk(false);
	}
	
	// Only report button down events.
	return state;
}

void
update_controller()
{
	static uint8_t last_val = 0;
	uint8_t  cur;
	uint8_t pressed;
	int inc = 1;
	
	cur = nesprobe();
	pressed = (last_val ^ cur) & cur;
	last_val = cur;

	if ((pressed & BTN_A) && ((state != JAM) || (jam_clock == 0))) {
		state = JAM;
		jam_clock = jam_duration;
	}
	
	if ((pressed & BTN_B) && ((state != LINEUP) || (jam_clock == 0))) {
		state = LINEUP;	
		jam_clock = lineup_duration;
	}

	if ((pressed & BTN_START) && (state != TIMEOUT)) {
		state = TIMEOUT;
		jam_clock = 1;
	}
	
	if (cur & BTN_SELECT) {
		inc = -1;

		// XXX: if in timeout, select digit to adjust
	}

	if (pressed & BTN_LEFT) {
		score_a += inc;
	}
	
	if (pressed & BTN_RIGHT) {
		score_b += inc;
	}
}

/*
 *
 * Main program
 *
 */


void
setup()
{
	int i;

	// Datasheet says you have to do this before DC initialization.
	// In practice it doesn't seem to matter, but what the hey.
	draw();

	// Initialize dot correction logic
	mode(true);
	sin(true);
	for (i = 0; i < nsr * 96; i += 1) {
		pulse();
	}
	latch();
	mode(false);
}

void
loop()
{
	uint32_t i;
	

	if (tick) {
		tick = false;
		
		update_controller();

		if (jiffies % (JIFFIES_PER_SECOND / 10) == 0) {
			switch (state) {
			case SETUP:
				break;
			case TIMEOUT:
				if (period_clock) {
					period_clock += 1;
				}
				// fall through
			case JAM:
			case LINEUP:
				if (jam_clock) {
					jam_clock += 1;
				}
			}
			
			draw();
		}
	}
}

int
main(void)
{
	init();
	setup();
	for (;;) {
		loop();
	}
	return 0;
}



