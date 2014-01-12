#ifndef __CONFIG_H__
#define __CONFIG_H__

#define CLOCK_MHZ 16
#define CLOCK_HZ (CLOCK_MHZ * 1000000)

#define SCORE_DIGITS 2
#define JAM_DIGITS 3

// Show an indicator (J/L/t) to the left of the jam clock
#define JAM_INDICATOR

// The jam clock is split across the period clock (easier folding, harder wiring)
#define JAM_SPLIT

// Set these to the PORTA pins you use
#define NESCLK 3
#define NESLTCH 4
#define NESOUT 5
#define SIN 0
#define SCLK 1
#define SLTCH 2

#endif
