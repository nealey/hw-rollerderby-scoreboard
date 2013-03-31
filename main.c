#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// Number of shift registers in your scoreboard
// If you want scores to go over 199, you need 8
const int nsr = 6;

//
// Timing stuff
//
// Make sure JIFFY_uS is going to be an integer and not a float!
#define JIFFIES_PER_SECOND 50
#define JIFFY_uS (1000000 / JIFFIES_PER_SECOND)
volatile uint32_t jiffies = 0;
volatile bool tick = false; // Set high when jiffy clock ticks

// Clocks are in deciseconds
uint16_t score_a = 0;
uint16_t score_b = 0;
int16_t period_clock = -600 * 30;
int16_t jam_clock = 0;
enum {
	SETUP,
	JAM,
	LINEUP,
	TIMEOUT
} state = SETUP;



#define cbi(byt, bit)   (byt &= ~_BV(bit))
#define sbi(byt, bit)   (byt |= _BV(bit))

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
	0x7b, 0x60, 0x37, 0x76, 0x6c, 0x5e, 0x5f, 0x70, 0x7f, 0x7e
};

const uint8_t setup_digits[] = {
	0x1b, 0x12, 0x72
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

volatile uint32_t micros = 0;

// Interrupt called every 1024 µs
SIGNAL(TIMER0_OVF_vect)
{
	uint32_t m = micros;
	
	m += 1024;
	if (m >= JIFFY_uS) {
		m %= JIFFY_uS;
		tick = true;
		jiffies += 1;
	}
	micros = m;
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
	
	//write_num(score_b, 2);

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
	static uint8_t last_controller = 0;

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
	int inc = 1;

	if ((val & BTN_A) && (state != JAM)) {
		state = JAM;
		jam_clock = -600 * 2;
	}
	
	if ((val & BTN_B) && (state != LINEUP)) {
		state = LINEUP;	
		jam_clock = -300;
	}

	if ((val & BTN_START) && (state != TIMEOUT)) {
		state = TIMEOUT;
		jam_clock = 1;
	}
	
	if (val & BTN_SELECT) {
		inc = -1;

		// XXX: if in timeout, select digit to adjust
	}

	if (val & BTN_LEFT) {
		score_a += inc;
	}
	
	if (val & BTN_RIGHT) {
		score_b += inc;
	}

	if (val) {
		PORTB = 0xff;
	} else {
		PORTB = 0;
	}
}

/*
 *
 * Main program
 *
 */

void
init(void)
{
	// Set timer 0 interrupt clock divider to 64
	TCCR0B = 0x03;

	// enable timer 0 overflow interrupt
	TIMSK0 = 0x01;
	
	// Enable interrupts
	sei();
}

void
setup()
{
	int i;

	DDRD = ~(NESSOUT);
	DDRB = 0xff;

	PORTD = 0;
	
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



