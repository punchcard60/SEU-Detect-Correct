TARGET:=FreeRTOS
# old root for reference TOOLCHAIN_ROOT ?= ~/stm/gcc-arm-none-eabi-4_9-2015q3
ifeq (,$(TOOLCHAIN_ROOT))
TOOLCHAIN_ROOT := /usr/local
endif
TOOLCHAIN_ROOT := $(abspath $(TOOLCHAIN_ROOT))
TOOLCHAIN_BIN := $(TOOLCHAIN_ROOT)/bin
TOOLCHAIN_PREFIX := arm-none-eabi

OPTLVL:=0
#DBG:=-g
DBG:=-g

FREERTOS:=$(CURDIR)/FreeRTOS
STARTUP:=$(CURDIR)/hardware
REED_SOLOMON:=$(abspath $(CURDIR)/../Reed-Solomon-Packed)
RS_SRC := $(REED_SOLOMON)/src

INCLUDE+=-I$(CURDIR)/config
INCLUDE+=-I$(CURDIR)/hardware
INCLUDE+=-Iseu/include
INCLUDE+=-I$(REED_SOLOMON)/include
INCLUDE+=-I$(CURDIR)/lib/CMSIS/Device/ST/STM32F4xx/Include
INCLUDE+=-I$(CURDIR)/lib/CMSIS/Include
INCLUDE+=-I$(CURDIR)/lib/STM32F4xx_StdPeriph_Driver/inc
INCLUDE+=-I$(CURDIR)/lib/CRC_Generator
INCLUDE+=-I$(FREERTOS)/include
INCLUDE+=-I$(FREERTOS)/portable/GCC/ARM_CM4F

BUILD_DIR = $(CURDIR)/build
BIN_DIR = $(CURDIR)/binary
SEU_DIR = $(CURDIR)/seu
SEU_SRC_DIR = $(SEU_DIR)/src
SEU_GEN_DIR = $(BUILD_DIR)/gen

ASRC:=$(STARTUP)/startup_stm32f40_41xxx.s

#compiled without finstrument-function
SRC += $(SEU_SRC_DIR)/reboot.c \
	   $(STARTUP)/handlers.c \
	   $(STARTUP)/system_stm32f4xx.c \
	   $(CURDIR)/hardware/uart.c \
	   $(CURDIR)/lib/syscall/syscall.c \
	   $(RS_SRC)/alpha_to.c \
       $(RS_SRC)/genpoly.c \
       $(RS_SRC)/index_of.c \
	   $(SEU_SRC_DIR)/seu.c

SRC += main.c

# FreeRTOS sources
SRC += $(FREERTOS)/event_groups.c \
       $(FREERTOS)/list.c \
       $(FREERTOS)/queue.c \
       $(FREERTOS)/tasks.c \
       $(FREERTOS)/timers.c \
       $(FREERTOS)/portable/GCC/ARM_CM4F/port.c \
       $(FREERTOS)/portable/MemMang/heap_4.c

# CRC32 sources
CRC_SRCS := $(RS_SRC)/alpha_to.c \
       	    $(RS_SRC)/genpoly.c \
       	    $(RS_SRC)/index_of.c \
		    lib/CRC_Generator/main.c

AOBJ:=$(addsuffix .o, $(addprefix $(BUILD_DIR)/, $(basename $(notdir $(ASRC)))))
OBJ:=$(addsuffix .o, $(addprefix $(BUILD_DIR)/, $(basename $(notdir $(SRC)))))

MCU_FLAGS:=-mcpu=cortex-m4 -mthumb -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb-interwork
COMMONFLAGS=-O$(OPTLVL) $(DBG) -Wall -ffunction-sections -fdata-sections
CFLAGS=$(COMMONFLAGS) $(MCU_FLAGS) $(INCLUDE) -DSTM32F40_41xxx
LDLIBS=-lm
LDFLAGS=$(COMMONFLAGS) $(MCU_FLAGS) -fno-exceptions

INITIAL_LINKERSCRIPT=$(REED_SOLOMON)/STM32F4xx_FLASH.ld
SECONDARY_LINKERSCRIPT=-Tbuild/gen/secondary_seu_link.ld

# don't count on having the tools in the PATH...
CC := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-gcc
LD := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-gcc
OBJCOPY := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-objcopy
AS := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-as
AR := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-ar
GDB := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-gdb
READELF := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-readelf

GCC=gcc #Standard Desktop GCC

PYTHON = python3

TO_OBJ=$(addsuffix .o, $(addprefix $(BUILD_DIR)/, $(basename $(notdir $(1)))))

.PHONY: all utils clean

all: utils SECONDARY_PROFILER

utils:
	@echo [CC] crcGenerator.c
	@$(GCC) $(INCLUDE) -g $(CRC_SRCS) -o $(BUILD_DIR)/crcGenerator

$(call TO_OBJ,$(ASRC)): $(ASRC)
	@echo [AS] $(notdir $<)
	@$(AS) -o $@ $^

$(call TO_OBJ,main.c): main.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(STARTUP)/handlers.c): $(STARTUP)/handlers.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(SEU_SRC_DIR)/reboot.c): $(SEU_SRC_DIR)/reboot.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(STARTUP)/system_stm32f4xx.c): $(STARTUP)/system_stm32f4xx.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(CURDIR)/hardware/uart.c): $(CURDIR)/hardware/uart.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(CURDIR)/lib/syscall/syscall.c): $(CURDIR)/lib/syscall/syscall.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(RS_SRC)/alpha_to.c): $(RS_SRC)/alpha_to.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(RS_SRC)/genpoly.c): $(RS_SRC)/genpoly.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(RS_SRC)/index_of.c): $(RS_SRC)/index_of.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(SEU_SRC_DIR)/seu.c): $(SEU_SRC_DIR)/seu.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(FREERTOS)/event_groups.c): $(FREERTOS)/event_groups.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(FREERTOS)/list.c): $(FREERTOS)/list.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(FREERTOS)/queue.c): $(FREERTOS)/queue.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(FREERTOS)/tasks.c): $(FREERTOS)/tasks.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(FREERTOS)/timers.c): $(FREERTOS)/timers.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(FREERTOS)/portable/GCC/ARM_CM4F/port.c): $(FREERTOS)/portable/GCC/ARM_CM4F/port.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(call TO_OBJ,$(FREERTOS)/portable/MemMang/heap_4.c): $(FREERTOS)/portable/MemMang/heap_4.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@


INITIAL_COMPILATION: $(AOBJ) $(OBJ)
	@echo [LD] $(TARGET).elf
	@test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	@$(CC) -o $(BIN_DIR)/initial$(TARGET).elf -T$(INITIAL_LINKERSCRIPT) $(LDFLAGS) $(AOBJ) $(OBJ) $(LDLIBS)

INITIAL_PROFILER: INITIAL_COMPILATION
	@echo "Starting Initial Profiler"
	@test -d $(SEU_GEN_DIR) || mkdir -p $(SEU_GEN_DIR)
	@$(READELF) --wide -s binary/initialFreeRTOS.elf | grep " FUNC    " | awk '{print $$3 " " $$8 }' | sort -k 2 | uniq -u  > $(SEU_GEN_DIR)/fullMap.data
	@awk '/\*-{6}\*/{x++}{print >"$(SEU_GEN_DIR)/template_half_" x ".ld" }' x=0 $(INITIAL_LINKERSCRIPT) #Split Linker script in half
	@$(PYTHON) $(SEU_DIR)/initial_profiler.py
	@echo "initial Profiler Completed"

SECONDARY_COMPILATION: INITIAL_PROFILER
	@echo "Starting Secondary Complilation"
	@$(CC) -Wl,-Map,$(TARGET).map -o $(BIN_DIR)/final$(TARGET).elf $(SECONDARY_LINKERSCRIPT) $(LDFLAGS) $(AOBJ) $(OBJ) $(LDLIBS)
	@echo "Secondary Complilation Completed"

SECONDARY_PROFILER: SECONDARY_COMPILATION
	@echo "Starting Final Profiler"
	$(eval textOffset:=$(shell $(READELF) -S $(BIN_DIR)/final$(TARGET).elf | grep ".text  " | awk '{print $$6}'))
	@echo "Starting CRC_Generator/elf modifier"
	./build/crcGenerator $(BIN_DIR)/final$(TARGET).elf $(textOffset) $(BIN_DIR)/encoded$(TARGET).elf

.PHONY: clean

clean:
	@echo [RM] BUILD DIR
	@rm -rf $(BUILD_DIR)/*
	@echo [RM] BIN DIR
	@rm -rf $(BIN_DIR)/*
