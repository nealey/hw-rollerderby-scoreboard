#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>
#include <util/delay.h>

#include "avr.h"
#include "config.h"

// 
// Timing stuff
// 

// 
// 2**32 deciseconds = 13.610221 years
// 
// As long as you unplug your scoreboard once every 10 years or so,
// you're good.
// 
volatile uint32_t jiffies = 0;	// Elapsed time in deciseconds (tenths of a second)
volatile bool tick = false;	// Set high when jiffy clock ticks

// Clocks are in deciseconds
int16_t score_a = 0;
int16_t score_b = 0;
int16_t period_clock = PERIOD_DEFAULT;
int16_t jam_duration = JAM_DEFAULT;
int16_t lineup_duration = LINEUP_DEFAULT;
int16_t jam_clock = JAM_DEFAULT;
enum {
	TIMEOUT = 0,
	JAM,
	LINEUP,
	KONAMI
} state = TIMEOUT;
bool setup = true;


// NES Controller buttons

#define BTN_A _BV(7)
#define BTN_B _BV(6)
#define BTN_SELECT _BV(5)
#define BTN_START _BV(4)
#define BTN_UP _BV(3)
#define BTN_DOWN _BV(2)
#define BTN_LEFT _BV(1)
#define BTN_RIGHT _BV(0)

const uint8_t konami_code[] = {
	BTN_UP, BTN_UP, BTN_DOWN, BTN_DOWN,
	BTN_LEFT, BTN_RIGHT, BTN_LEFT, BTN_RIGHT,
	BTN_B, BTN_A,
	0
};
int konami_pos = 0;
const uint8_t test_pattern[] = {
	_BV(2), _BV(3), _BV(4), _BV(5), _BV(6), _BV(1), _BV(0), _BV(7)
};

const uint8_t seven_segment_digits[] = {
	// 0 1 2 3 4 5 6 7 8 9
	0x7b, 0x60, 0x37, 0x76, 0x6c, 0x5e, 0x5f, 0x70, 0x7f, 0x7e
};


// keyed by state
const uint8_t indicator[] = {
	// t, J, L, -
	0x0f, 0x63, 0x0b, 0x04
};

#define max(a, b) ((a)>(b)?(a):(b))
#define min(a, b) ((a)<(b)?(a):(b))


void
latch()
{
	sltch(true);
	sltch(false);
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
		pulse();
	}
}

void
write_num(uint16_t number, int digits)
{
	int i;

	for (i = 0; i < digits; i += 1) {
		uint8_t out = seven_segment_digits[number % 10];

		// Overflow indicator
		if ((i == digits - 1) && (number > 9)) {
			// Blink to indicate double-rollover
			if ((number > 19) && (jiffies % 3 == 0)) {
				// leave it blank
			} else {
				out ^= 0x80;
			}
		}

		write(out);
		number /= 10;
	}
}

uint16_t
clock_of_jiffies(int16_t jiffies)
{
	uint16_t seconds;
	uint16_t ret;

	// People read "0:00" as the time being out.
	// Add 0.9 seconds to make the ALU's truncation be a "round up"
	seconds = (abs(jiffies) + 9) / 10;

	ret = (seconds / 60) * 100;	// Minutes
	ret += seconds % 60;	// Seconds
	
	return ret;
}

inline uint16_t
write_pclock()
{
	uint16_t pclk = clock_of_jiffies(period_clock);
	bool blank = ((state == TIMEOUT) && (jiffies % 8 == 0));
	
	// Period clock
	if (blank) {
		write(0);
		write(0);
		write(0);
		write(0);
	} else {
		write_num(pclk, 4);
	}
}


/*
 * Update all the digits
 */
void
draw()
{
	uint16_t jclk = clock_of_jiffies(jam_clock);

	// Segments test mode
	if (KONAMI == state) {
		int i;

		for (i = 0; i < 12; i += 1) {
			write(test_pattern[jiffies % (sizeof test_pattern)]);
		}

		latch();
		pulse();
		return;
	}

	write_num(score_b, SCORE_DIGITS);

	write_num(jclk, 2);
#ifdef JAM_SPLIT
	write_pclock();
#endif
	write_num(jclk / 100, JAM_DIGITS - 2);
#ifdef JAM_INDICATOR
	write(indicator[state]);
#endif

#ifndef JAM_SPLIT
	write_pclock();
#endif

	write_num(score_a, SCORE_DIGITS);

	// Tell chips to start displaying new values 
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
	static uint8_t last_held = 0;
	static uint32_t last_change = 0;
	static uint32_t last_typematic = 0;
	uint8_t held;
	uint8_t pressed;
	int typematic = 0;
	int inc = 1;

	held = nesprobe();
	pressed = (last_held ^ held) & held;

	// Set up typematic acceleration
	if (last_held != held) {
		// Debounce
		if (last_change == jiffies) {
			return;
		}
		last_change = jiffies;
		last_typematic = jiffies;
		last_held = held;
		typematic = 1;
	} else if (jiffies > last_typematic) {
		last_typematic = jiffies;
		if (jiffies - last_change < 6) {
			typematic = 0;
		} else if (jiffies - last_change < 40) {
			typematic = 1;
		} else if (jiffies - last_change < 80) {
			typematic = 10;
		} else {
			typematic = 20;
		}
	}
	
	if (pressed == konami_code[konami_pos]) {
		konami_pos += 1;

		if (konami_code[konami_pos] == 0) {
			state = KONAMI;
			konami_pos = 0;
			return;
		} else if (konami_pos > 3) {
			return;
		}
	} else if (pressed) {
		konami_pos = 0;
	}
	// Select means subtract
	if (held & BTN_SELECT) {
		inc = -1;
	}
	
	if (setup && (held & BTN_START) && (pressed & BTN_SELECT)) {
		jam_duration += 30 * 10;
		if (jam_duration > -60 * 10) {
			jam_duration = -120 * 10;
		}
		jam_clock = jam_duration;
	}

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

	if ((held & BTN_START) && (state == TIMEOUT)) {
		if (held & BTN_UP) {
			period_clock -= typematic * 10;
		}
		if (held & BTN_DOWN) {
			period_clock += typematic * 10;
		}
		
		period_clock = min(period_clock, 0);
		period_clock = max(period_clock, -90 * 30 * 10);
	} else {
		// Score adjustment and clock adjustment are mutually exclusive
		if (held & BTN_LEFT) {
			score_a += typematic * inc;
			score_a = max(score_a, 0);
		}
	
		if (held & BTN_RIGHT) {
			score_b += typematic * inc;
			score_b = max(score_b, 0);
		}
	}
	
	if (state != TIMEOUT) {
		setup = false;
	}
}

/*
 *
 * Main program
 *
 */


void
loop()
{
	update_controller();

	if (tick) {
		tick = false;

		if (jiffies % 5 == 0) {
			PORTB ^= 0xff;
		}

		switch (state) {
		case JAM:
		case LINEUP:
			if (period_clock) {
				period_clock += 1;
			}
			// fall through
		case TIMEOUT:
			if (jam_clock && !setup) {
				jam_clock += 1;
			}
		}
	}

	draw();
}

int
main(void)
{
	avr_init();
	for (;;) {
		loop();
	}
	return 0;
}
