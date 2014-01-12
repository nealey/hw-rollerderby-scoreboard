#ifndef __CONFIG_H__
#define __CONFIG_H__

#define CLOCK_MHZ 16
#define CLOCK_HZ (CLOCK_MHZ * 1000000)

#define SCORE_DIGITS 2
#define JAM_INDICATOR 1
#define JAM_DIGITS 3

// Set these to the PORTA pins you use
#define SIN 3
#define SCLK 4
#define SLTCH 5
#define NESCLK 0
#define NESLTCH 1
#define NESOUT 2

#endif