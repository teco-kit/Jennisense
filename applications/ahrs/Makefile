CONTIKI_PROJECT = ahrs_estimate
all: $(CONTIKI_PROJECT)

APPS = telnetd
CONTIKI_SOURCEFILES = imu.c ahrs.c
TARGET_LIBFILES = -lm

UIP_CONF_IPV6=1
DEFINES=WITH_UIP6

CONTIKI = ../../contiki-jn51xx
include $(CONTIKI)/Makefile.include
