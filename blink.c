#include <msp430.h>

unsigned int i = 0;

unsigned char bits1 = BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7;
unsigned char bits2 = BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5;

int
main(void)
{
	WDTCTL = WDTPW + WDTHOLD;
	P1DIR |= bits1;
	P2DIR |= bits2;

	P1OUT |= bits1;
	P2OUT &= ~bits2;

	for (;;) {
		i = (i + 1) % 40000;
		
		if (i == 0) {
			P1OUT ^= bits1;
			P2OUT ^= bits2;
		}
	}		

	return 0;
}