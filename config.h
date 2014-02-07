#ifndef __CONFIG_H__
#define __CONFIG_H__

#define CLOCK_MHZ 16
#define CLOCK_HZ (CLOCK_MHZ * 1000000)

#define LINEUP_DEFAULT (-30 * 10)
#define JAM_DEFAULT (-2 * 60 * 10)
#define PERIOD_DEFAULT (-30 * 60 * 10)

// TPIC uses weird pin names
#define SRCK SCLK
#define RCK SLTCH

// Set these to the PORTA pins you use
#define NESCLK 0
#define NESLTCH 1
#define NESOUT 2
#define SIN 3
#define SCLK 4
#define SLTCH 5

#define JAM_DIGITS 3

#define std 0
#define neale 1
#define susan1 2
#define susan2 3
#define robbie 3

#if VARIANT == neale
//
// Neale variant
//
// With Jam indicator, split jam clock, and only 2 score digits.
// Also featuring goofy brain wiring.
//

#define SCORE_DIGITS 2
#define JAM_INDICATOR
#define JAM_SPLIT

// I wired mine differently and I'm too lazy to fix it.
#undef NESCLK
#undef NESLTCH
#undef NESOUT
#undef SIN
#undef SCLK
#undef SLTCH

#define SIN 0
#define SCLK 1
#define SLTCH 2
#define NESCLK 3
#define NESLTCH 4
#define NESOUT 5



#elif VARIANT == susan1
//
// Susan 1 variant
//
// Like the neale variant but with correct brain wiring
//


#define SCORE_DIGITS 2
#define JAM_INDICATOR
#define JAM_SPLIT



#elif VARIANT == susan2
//
// Susan 2 variant
//


#define SCORE_DIGITS 3


#else
//
// Default variant
//
// Nobody has built this yet!
// But it's the one described by the instructions.
//


#define SCORE_DIGITS 3
#define JAM_SPLIT


#endif


#endif
