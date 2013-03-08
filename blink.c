#include <msp430.h>

unsigned int i = 0;

int
main(void)
{
	WDTCTL = WDTPW + WDTHOLD;
	P1DIR |= BIT0 + BIT6;
	P1OUT = (P1OUT & ~BIT6) | BIT0;

	for (;;) {
		i = (i + 1) % 1000;
		
		if (i == 0) {
			P1OUT ^= BIT0 + BIT6;
		}
	}		

	return 0;
}