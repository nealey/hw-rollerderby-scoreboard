#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

/*
bit 7 = A
bit 6 = B
bit 5 = select
bit 4 = start
bit 3 = up
bit 2 = down
bit 1 = left 
bit 0 = right
*/
uint8_t
read_gamepad()
{
	return 0;
}

const uint8_t digits[] = {
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f,
	0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71
};

// in deciseconds
uint16_t period;
uint16_t jam;

void
update_display()
{
	printf("%06u    %06u\r", period, jam);
	fflush(stdout);
}

int
main(int argc, char *argv[])
{
	period = 30 * 600;
	jam = 2 * 600;
	
	for (;;) {
		period -= 1;
		jam -= 1;
		
		update_display();
		usleep(100000);
	}
	return 0;
}

