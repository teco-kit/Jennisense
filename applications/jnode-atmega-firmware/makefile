#
#             LUFA Library
#     Copyright (C) Dean Camera, 2012.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
#
# --------------------------------------
#         LUFA Project Makefile.
# --------------------------------------

# Run "make help" for target help.

MCU          = atmega32u4
ARCH         = AVR8
BOARD        = USER
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = USBtoSerial
SRC          = $(TARGET).c Descriptors.c $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS) clock.c
LUFA_PATH    = lufa-lib-svn/LUFA
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -IConfig/
LD_FLAGS     =

# Default target
all:

# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
include $(LUFA_PATH)/Build/lufa_doxygen.mk
#include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk

## copied from lufa_dfu-mk
MSG_COPY_CMD   := ' [CP]      :'
MSG_REMOVE_CMD := ' [RM]      :'
MSG_DFU_CMD    := ' [DFU]     :'

# Programs in the target FLASH memory using DFU-PROGRAMMER
dfu: $(TARGET).hex $(MAKEFILE_LIST)
	@echo $(MSG_DFU_CMD) Programming FLASH with dfu-programmer using \"$<\"
	sudo dfu-programmer $(MCU) erase
	sudo dfu-programmer $(MCU) flash $<
	sudo dfu-programmer $(MCU) reset

# Programs in the target EEPROM memory using DFU-PROGRAMMER
dfu-ee: $(TARGET).eep $(MAKEFILE_LIST)
	@echo $(MSG_DFU_CMD) Programming EEPROM with dfu-programmer using \"$<\"
	dfu-programmer $(MCU) eeprom-flash $<
	dfu-programmer $(MCU) reset
