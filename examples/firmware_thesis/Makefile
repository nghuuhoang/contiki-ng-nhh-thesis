CONTIKI_PROJECT = lwm2m_demo

# CONTIKI_SOURCEFILES += serial-protocol.c 
CONTIKI_TARGET_SOURCEFILES += si7021.c tsl2561.c bmpx8x.c relay.c

MODULES += arch/platform/cc2538dk/dev



all: $(CONTIKI_PROJECT)

MODULES += os/net/app-layer/coap
MODULES += os/services/lwm2m
MODULES += os/services/ipso-objects
MODULES += os/services/rpl-border-router

CONTIKI=../..
include $(CONTIKI)/Makefile.identify-target
MODULES_REL += $(TARGET)

include $(CONTIKI)/Makefile.include
