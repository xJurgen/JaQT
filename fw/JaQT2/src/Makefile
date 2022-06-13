# Original project makefile made by satoshinm
# Changes made by: Jiří Veverka (xvever12)

VERSION = 1.1 #Added by xvever12
DATETIME := $(shell date --iso-8601=seconds)
ENABLE_DEBUG ?=
JAQT_DEBUG = 0 #Added by xvever12

ifneq ($(V), 1)
MAKEFLAGS += --no-print-dir
Q := @
endif

OPT_FLAGS ?= -O2

CFLAGS += -Wall -Wextra -Werror -Wno-char-subscripts -pedantic\
	$(OPT_FLAGS) -std=gnu99 -g3 -MD \
	-I.
LDFLAGS += $(OPT_FLAGS)
#LDFLAGS += -u _printf_float #Cannot use this because resulting binary is too big

ifeq ($(ENABLE_DEBUG), 1)
CFLAGS += -DENABLE_DEBUG
endif

#Added new modules (by xvever12)
SRC =			\
	main.c		\
	platform.c	\
	print_float.c \
	usb/cdcacm.c	\
	usb/usbuart.c	\
	usb/usartconf.c \
	wiegand/wiegand.c	\
	control_shell/commandbuffer.c \
	control_shell/tokenizer.c \
	control_shell/wiegandcommands.c \
	control_shell/gpiocommands.c	\
	control_shell/usbshell.c	\
	gpio/gpiocontrol.c \
	i2c/i2cgpio.c \
	i2c/i2cbase.c	\
	i2c/i2cina219.c \
	periph/clock/clockconf.c \
	periph/exti/exticonf.c \
	periph/timer/timerconf.c \
	periph/gpio/gpioconf.c \
	serial/rs485/debug/debugrs485.c \
	serial/tester/honeywell/honeywell.c \
	flash/flash_eeprom.c \

#Added new modules (by xvever12)
OBJ =			\
	usb/cdcacm.o	\
	usb/usbuart.o	\
	usb/usartconf.o \
	wiegand/wiegand.o	\
	control_shell/commandbuffer.o \
	control_shell/tokenizer.o \
	control_shell/wiegandcommands.o \
	control_shell/gpiocommands.o	\
	control_shell/usbshell.o	\
	gpio/gpiocontrol.o \
	i2c/i2cgpio.o \
	i2c/i2cbase.o	\
	i2c/i2cina219.o \
	periph/clock/clockconf.o \
	periph/exti/exticonf.o \
	periph/timer/timerconf.o \
	periph/gpio/gpioconf.o \
	serial/rs485/debug/debugrs485.o \
	serial/tester/honeywell/honeywell.o \
	flash/flash_eeprom.o \

#Added new modules (by xvever12)
DEP =			\
	usb/cdcacm.d	\
	usb/usbuart.d	\
	usb/usartconf.d \
	wiegand/wiegand.d	\
	control_shell/commandbuffer.d \
	control_shell/tokenizer.d \
	control_shell/wiegandcommands.d \
	control_shell/gpiocommands.d	\
	control_shell/usbshell.d	\
	gpio/gpiocontrol.d \
	i2c/i2cgpio.d \
	i2c/i2cbase.d	\
	i2c/i2cina219.d \
	periph/clock/clockconf.d \
	periph/exti/exticonf.d \
	periph/timer/timerconf.d \
	periph/gpio/gpioconf.d \
	serial/rs485/debug/debugrs485.d \
	serial/tester/honeywell/honeywell.d \
	flash/flash_eeprom.d \

CROSS_COMPILE ?= arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy

OPT_FLAGS = -Os
CFLAGS += -mcpu=cortex-m3 -mthumb \
	-DSTM32F1 -DDISCOVERY_STLINK -I../libopencm3/include \
	-I .
LDFLAGS_BOOT := $(LDFLAGS) --specs=nano.specs \
	-lopencm3_stm32f1 -Wl,--defsym,_stack=0x20005000 \
	-Wl,-T,bluepill.ld -nostartfiles -lc \
	-Wl,-Map=mapfile -mthumb -mcpu=cortex-m3 -Wl,-gc-sections \
	-L../libopencm3/lib
LDFLAGS = $(LDFLAGS_BOOT)

ifeq ($(ENABLE_DEBUG), 1)
LDFLAGS += --specs=rdimon.specs
else
LDFLAGS += --specs=nosys.specs
endif

#Changes pill_serial to jaqt2 by xvever12
all: jaqt2.bin

#Added by xvever12
help:
	$(Q)echo "Usage example: make flash BOARD_NO=4 PORT=/dev/ttyACM3"

host_clean:
	-$(Q)$(RM) jaqt2.bin

OBJ = $(SRC:.c=.o)

jaqt2.elf: version.h $(OBJ)
	@echo "  LD      $@"
	$(Q)$(CC) -o $@ $(OBJ) $(LDFLAGS)

%.o:	%.c
	@echo "  CC      $<"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.bin:	%.elf
	@echo "  OBJCOPY $@"
	$(Q)$(OBJCOPY) -O binary $^ $@

%.hex:	%
	@echo "  OBJCOPY $@"
	$(Q)$(OBJCOPY) -O ihex $^ $@

.PHONY:	clean host_clean FORCE

clean:	host_clean
	$(Q)echo "  CLEAN"
	-$(Q)$(RM) -f *.o *.d *~ *.elf *.bin $(HOSTFILES)
	-$(Q)$(RM) -f mapfile version.h
	-$(Q)$(RM) -f $(DEP)
	-$(Q)$(RM) -f $(OBJ)

#Changed by xvever12 - added new defines to generate
version.h: FORCE
	$(Q)echo "  CREATE  version.h"
	$(Q)echo "#define FIRMWARE_VERSION \"$(VERSION)\"" > $@
	$(Q)echo "#define BUILD_TIME \"$(DATETIME)\"" >> $@
	$(Q)echo "#define WIEGAND_DEBUG $(JAQT_DEBUG)" >> $@
	$(Q)echo "#define DEBUG_COMMANDS $(JAQT_DEBUG)" >> $@

#Added by xvever12
flash: all
ifndef PORT
	$(error PORT is undefined)
endif
ifdef PORT
	@python3 ../../../sw/scripts/reset_device.py $(PORT)
endif
ifndef BOARD_NO
	$(Q)echo "BOARD_NO is undefined"
	@-sleep .5 && dfu-util -a 2 -D jaqt2.bin
else
	@-sleep .5 && dfu-util --serial "$(BOARD_NO)" -a 2 -D jaqt2.bin
endif
	$(Q)echo "dfu-util may throw an error. This is a known bug on linux systems."
	$(Q)echo "If so, please repeat action."

-include *.d

#Added by xvever12
init_flash: jaqt2.bin
	@-sleep .5 && dfu-util -a 2 -D jaqt2.bin
	$(Q)echo "dfu-util may throw an error. This is a known bug on linux systems."
	$(Q)echo "If so, please repeat action."
