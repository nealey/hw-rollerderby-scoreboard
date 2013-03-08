CC = msp430-gcc
CFLAGS += -mmcu=msp430g2553
CFLAGS += -Wall
CFLAGS += -Os
CFLAGS += -g

all: main blink
