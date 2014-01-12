PROG = main

MCU = attiny84

CC = avr-gcc
CFLAGS += -mmcu=$(MCU)
CFLAGS += -Os
CFLAGS += -w

LDFLAGS += -mmcu=$(MCU)

AVDFLAGS += -p $(MCU)
AVDFLAGS += -c usbtiny

FUSES += -U lfuse:w:0x7f:m
FUSES += -U hfuse:w:0xdd:m
FUSES += -U efuse:w:0xff:m

upload: .upload

.upload: $(PROG).hex
	avrdude $(AVDFLAGS) -U flash:w:$<
	touch $@

fuses:
	avrdude $(AVDFLAGS) $(FUSES)
	
main: main.o avr.o

blink: blink.o avr.o

avr.o: avr.c config.h

%.hex: %
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

clean:
	rm -f $(PROG) *.o *.hex .upload



	
