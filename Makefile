TARGETS += scoreboard-neale.hex
TARGETS += scoreboard-susan1.hex
TARGETS += scoreboard-susan2.hex
TARGETS += scoreboard-std.hex

MCU = attiny84

CC = avr-gcc
CFLAGS += -mmcu=$(MCU)
CFLAGS += -Os
CFLAGS += -w

LDFLAGS += -mmcu=$(MCU)

all: $(TARGETS)

scoreboard-%.elf: main.c avr.c config.h avr.h
	$(CC) $(CFLAGS) -DVARIANT=$* $(LDFLAGS) -o $@ avr.c main.c

%.hex: %.elf
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

clean:
	rm -f *.elf *.o *.hex .upload



##
## Helpful targets for development
##

VARIANT = neale

AVDFLAGS += -p $(MCU)
AVDFLAGS += -c usbtiny

upload: scoreboard-$(VARIANT).hex
	avrdude $(AVDFLAGS) -U flash:w:$<

fuses: FUSES += -U lfuse:w:0x7f:m
fuses: FUSES += -U hfuse:w:0xdd:m
fuses: FUSES += -U efuse:w:0xff:m
fuses:
	avrdude $(AVDFLAGS) $(FUSES)
	
