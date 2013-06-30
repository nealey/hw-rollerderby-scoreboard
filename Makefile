PROG = main

MCU = attiny84

CC = avr-gcc
CFLAGS += -mmcu=$(MCU)
CFLAGS += -Os
CFLAGS += -w

LDFLAGS += -mmcu=$(MCU)

AVDFLAGS += -p $(MCU)
AVDFLAGS += -c usbtiny

CLOCK_HZ = 16000000
FUSES += -U lfuse:w:0xff:m
FUSES += -U hfuse:w:0xd9:m
FUSES += -U efuse:w:0xff:m


upload: .upload

.upload: $(PROG).hex
	avrdude $(AVDFLAGS) -U flash:w:$<
	touch $@

fuses:
	avrdude $(AVDFLAGS) $(FUSES)
	
main: main.o avr.o

$(PROG).hex: $(PROG)
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

clean:
	rm -f $(PROG) *.o *.hex .upload



	
