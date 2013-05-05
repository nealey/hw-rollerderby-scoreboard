ARCH = avr

CFLAGS += -DARCH=$(ARCH)

include Makefile.$(ARCH)

