CC = avr-gcc
CFLAGS += -mmcu=atmega328p
CFLAGS += -DF_CPU=16000000UL
CFLAGS += -Os
CFLAGS += -w

AVDFLAGS += -p m328p
AVDFLAGS += -c arduino
AVDFLAGS += -b 115200
AVDFLAGS += -P /dev/ttyACM0


upload: .upload

.upload: blink.hex
	avrdude $(AVDFLAGS) -U flash:w:$<
	touch $@

blink.hex: blink
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

clean:
	rm -f blink.hex blink



	
