##
## This file is derived from the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
## Copyright (C) 2010 Piotr Esden-Tempski <piotr@esden.net>
## Copyright (C) 2013 Frantisek Burian <BuFran@seznam.cz>
## Copyright (C) 2016 Devan Lai
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q              := @
NULL           := 2>/dev/null
endif

####################################################################
# Target Architecture flags
ifeq ($(ARCH),STM32F1)
    LIBNAME      = opencm3_stm32f1
    DEFS        += -DSTM32F1
    FP_FLAGS    ?= -msoft-float
    ARCH_FLAGS   = -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd
    OOCD_TARGET ?= target/stm32f1x.cfg
    FLASH_BASE  ?= 0x08000000
else
    $(error Target architecture $(ARCH) not supported)
endif

####################################################################
# OpenOCD specific variables

OOCD           ?= openocd
OOCD_INTERFACE ?= interface/stlink.cfg

####################################################################
# Executables

PREFIX         ?= arm-none-eabi

CC             := $(PREFIX)-gcc
LD             := $(PREFIX)-gcc
OBJCOPY        := $(PREFIX)-objcopy
OBJDUMP        := $(PREFIX)-objdump

####################################################################
# libopencm3 files

INCLUDE_DIR = $(OPENCM3_DIR)/include
LIB_DIR     = $(OPENCM3_DIR)/lib

####################################################################
# C flags

CFLAGS      += -Os -g -std=c2x
CFLAGS      += -Wextra -Wshadow -Wimplicit-function-declaration
CFLAGS      += -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
CFLAGS      += -fno-common -ffunction-sections -fdata-sections

####################################################################
# C preprocessor flags

CPPFLAGS    += -MD
CPPFLAGS    += -Wall -Wundef
CPPFLAGS    += -I$(INCLUDE_DIR) $(DEFS)

####################################################################
# Linker flags

LDFLAGS    += --static -nostartfiles
LDFLAGS    += --specs=nano.specs
LDFLAGS    += -L$(LIB_DIR)
LDFLAGS    += -L$(LIB_DIR)/stm32/f1
LDFLAGS    += -T$(LDSCRIPT)
LDFLAGS    += -Wl,-Map=$(*).map
LDFLAGS    += -Wl,--gc-sections
LDFLAGS    += -Wl,--print-memory-usage

####################################################################
# Used libraries

LDLIBS     += -l$(LIBNAME)
LDLIBS     += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

####################################################################
####################################################################
####################################################################
.SECONDEXPANSION:
.SECONDARY:

elf: $(BUILD)/$(BINARY).elf
bin: $(BUILD)/$(BINARY).bin
hex: $(BUILD)/$(BINARY).hex
list: $(BUILD)/$(BINARY).list

images: $(BUILD)/$(BINARY).images

$(LDSCRIPT):
    ifeq (,$(wildcard $(LDSCRIPT)))
        $(error Unable to find specified linker script: $(LDSCRIPT))
    endif

$(LIB_DIR)/lib$(LIBNAME).a:
	$(Q)$(MAKE) -C $(OPENCM3_DIR)

locm3: $(LIB_DIR)/lib$(LIBNAME).a

%.images: %.bin %.hex %.list %.map
	@printf "*** $* images generated ***\n"

%.bin: %.elf
	@printf "  OBJCOPY $(*).bin\n"
	$(Q)$(OBJCOPY) -Obinary --gap-fill 0xff --pad-to=$$(($(FLASH_BASE) + $(BOOTLOADER_SIZE))) $(*).elf $(*).bin

%.hex: %.elf
	@printf "  OBJCOPY $(*).hex\n"
	$(Q)$(OBJCOPY) -Oihex $(*).elf $(*).hex

%.list: %.elf
	@printf "  OBJDUMP $(*).list\n"
	$(Q)$(OBJDUMP) -S $(*).elf > $(*).list

%.elf %.map: $(OBJS) $(LDSCRIPT) $(LIB_DIR)/lib$(LIBNAME).a
	@printf "  LD      $(*).elf\n"
	$(Q)$(LD) $(LDFLAGS) $(ARCH_FLAGS) $(OBJS) $(LDLIBS) -o $(*).elf

$(BUILD)/%.o: %.c $(LIB_DIR)/lib$(LIBNAME).a
	@printf "  CC      $(*).c\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) $(ARCH_FLAGS) $(VER_FLAGS) -o $@ -c $(*).c

.PHONY: images clean elf bin hex list locm3

-include $(OBJS:.o=.d)
