PHONY += _all
all: sntpd

ifeq ("$(origin V)", "command line")
  VERBOSE = $(V)
endif

Q = @

ifneq ($(findstring 1, $(VERBOSE)),)
  Q =
endif

CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
LD		= $(CROSS_COMPILE)ld
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
READELF		= $(CROSS_COMPILE)readelf
STRIP		= $(CROSS_COMPILE)strip

RM      = /usr/bin/rm

CFLAGS =

sntpd: sntpd.c
	$(Q)$(CC) -o $@ $(CFLAGS) $(filter %.c, $^)

PHONY += clean
clean:
	$(RM) -f sntpd

.PHONY: $(PHONY)
