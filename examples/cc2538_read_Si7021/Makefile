CONTIKI_PROJECT = test
all: $(CONTIKI_PROJECT)

#MODULES += os/services/shell
CONTIKI_TARGET_SOURCEFILES += si7021.c

MODULES += arch/platform/cc2538dk/dev

#MAKE_MAC = MAKE_MAC_TSCH
#MODULES += os/services/rpl-border-router

CONTIKI = ../..
include $(CONTIKI)/Makefile.include