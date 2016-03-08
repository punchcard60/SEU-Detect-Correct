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
DBG:=

FREERTOS:=$(CURDIR)/FreeRTOS
STARTUP:=$(CURDIR)/hardware
REED_SOLOMON:=$(abspath $(CURDIR)/../Reed-Solomon-Packed)

NUM_TEXT_SECTIONS = 4 #Number of .textX sections declared in linker script

INCLUDE=-I$(CURDIR)/hardware
INCLUDE+=-I$(FREERTOS)/include
INCLUDE+=-I$(FREERTOS)/portable/GCC/ARM_CM4F
INCLUDE+=-I$(CURDIR)/lib/CMSIS/Device/ST/STM32F4xx/Include
INCLUDE+=-I$(CURDIR)/lib/CMSIS/Include
INCLUDE+=-I$(CURDIR)/lib/STM32F4xx_StdPeriph_Driver/inc
INCLUDE+=-I$(CURDIR)/config
INCLUDE+=-Iseu/include
INCLUDE+=-I$(REED_SOLOMON)/include

BUILD_DIR = $(CURDIR)/build
BIN_DIR = $(CURDIR)/binary
SEU_DIR = seu
SEU_SRC_DIR = $(SEU_DIR)/src
SEU_GEN_DIR = $(BUILD_DIR)/gen

ASRC=startup_stm32f40_41xxx.s

#vpath %.c $(CURDIR)/lib/STM32F4xx_StdPeriph_Driver/src \
#          $(CURDIR)/lib/syscall \
#          $(CURDIR)/hardware \
#          $(FREERTOS) \
#          $(FREERTOS)/portable/MemMang \
#          $(FREERTOS)/portable/GCC/ARM_CM4F

UNCHECKED_SRC := hardware/stm32f4xx_it.c #compiled without finstrument-function
SRC := main.c \
       hardware/system_stm32f4xx.c \
       hardware/uart.c \
       lib/syscall/syscall.c

# FreeRTOS sources
SRC += $(FREERTOS)/event_groups.c \
       $(FREERTOS)/list.c \
       $(FREERTOS)/queue.c \
       $(FREERTOS)/tasks.c \
       $(FREERTOS)/timers.c \
       $(FREERTOS)/portable/GCC/ARM_CM4F/port.c \
       $(FREERTOS)/portable/MemMang/heap_4.c

# StdPeriph driver sources
STDPERIPH_SRC := $(CURDIR)/lib/STM32F4xx_StdPeriph_Driver/src
SRC += $(STDPERIPH_SRC)/stm32f4xx_syscfg.c \
       $(STDPERIPH_SRC)/misc.c \
       $(STDPERIPH_SRC)/stm32f4xx_adc.c \
       $(STDPERIPH_SRC)/stm32f4xx_dac.c \
       $(STDPERIPH_SRC)/stm32f4xx_dma.c \
       $(STDPERIPH_SRC)/stm32f4xx_exti.c \
       $(STDPERIPH_SRC)/stm32f4xx_flash.c \
       $(STDPERIPH_SRC)/stm32f4xx_gpio.c \
       $(STDPERIPH_SRC)/stm32f4xx_i2c.c \
       $(STDPERIPH_SRC)/stm32f4xx_pwr.c \
       $(STDPERIPH_SRC)/stm32f4xx_rcc.c \
       $(STDPERIPH_SRC)/stm32f4xx_spi.c \
       $(STDPERIPH_SRC)/stm32f4xx_tim.c \
       $(STDPERIPH_SRC)/stm32f4xx_usart.c \
       $(STDPERIPH_SRC)/stm32f4xx_rng.c

# Reed Solomon sources
RS_SRC := $(REED_SOLOMON)/src
SRC += $(RS_SRC)/alpha_to.c \
       $(RS_SRC)/genpoly.c \
       $(RS_SRC)/index_of.c

# CRC32 sources
CRC_SRCS = lib/CRC_Generator/crcmodel.c \
           lib/CRC_Generator/main.c

CDEFS=-DUSE_STDPERIPH_DRIVER
CDEFS+=-DSTM32F4XX -DSTM32F40_41xxx
#CDEFS+=-DSTM32F40_41xxx
CDEFS+= -D'HSE_VALUE=((uint32_t)24000000)'
#CDEFS+=-D__FPU_PRESENT=1
#CDEFS+=-D__FPU_USED=1
#CDEFS+=-DARM_MATH_CM4

MCU_FLAGS:=-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
COMMONFLAGS=-O$(OPTLVL) $(DBG) -Wall -ffunction-sections -fdata-sections
CFLAGS=$(COMMONFLAGS) $(MCU_FLAGS) $(INCLUDE) $(CDEFS)
#LDLIBS=$(TOOLCHAIN_ROOT)/arm-none-eabi/lib/armv7e-m/fpu/libc.a $(TOOLCHAIN_ROOT)/arm-none-eabi/lib/armv7e-m/fpu/libm.a
#LDFLAGS=$(COMMONFLAGS) -fno-exceptions -nostartfiles $(MCU_FLAGS)
LDLIBS=-lm
LDFLAGS=$(COMMONFLAGS) -fno-exceptions $(MCU_FLAGS)
SEUFLAG=-finstrument-functions

#INITIAL_LINKERSCRIPT=-Tseu/initial_seu_link.ld
INITIAL_LINKERSCRIPT=$(REED_SOLOMON)/STM32F4xx_FLASH.ld
SECONDARY_LINKERSCRIPT=-Tbuild/gen/secondary_seu_link.ld

TRACE_FILES:=$(SEU_SRC_DIR)/trace_functions.c
TRACE_OBJ = build/trace_functions.o

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

OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)
RS_SRCS := $(RS_SRC)/alpha_to.c $(RS_SRC)/index_of.c $(RS_SRC)/genpoly.c
#RS_OBJS := $(RS_SRCS:%.c=$(BUILD_DIR)/%.o)

all: utils SECONDARY_PROFILER

.PHONY: utils

utils:
	@echo [CC] crcGenerator.c
	@test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR)
	$(GCC) $(INCLUDE) -g $(CRC_SRCS) $(RS_SRCS) -o $(BUILD_DIR)/crcGenerator

$(BUILD_DIR)/alpha_to.o: $(RS_SRC)/alpha_to.c $(REED_SOLOMON)/include/*
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(BUILD_DIR)/index_of.o: $(RS_SRC)/index_of.c $(REED_SOLOMON)/include/*
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(BUILD_DIR)/genpoly.o: $(RS_SRC)/genpoly.c $(REED_SOLOMON)/include/*
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

$(BUILD_DIR)/%.o: %.c
	@echo [CC] $<
	@test -d $(dir $@) || mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(SEUFLAG) $< -c -o $@

UNCHECKED_OBJS: $(OBJ)
	@$(CC) $(CFLAGS) hardware/stm32f4xx_it.c -c -o build/stm32f4xx_it.o
	@$(CC) $(CFLAGS) $(TRACE_FILES) -c -o $(TRACE_OBJ)

INITIAL_COMPILATION: UNCHECKED_OBJS $(RS_OBJS)
	@echo "[AS] $(ASRC)"
	@$(AS) -o $(ASRC:%.s=$(BUILD_DIR)/%.o) $(STARTUP)/$(ASRC)
	@echo [LD] $(TARGET).elf
	@test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/initial$(TARGET).elf -T$(INITIAL_LINKERSCRIPT) $(LDFLAGS) $(OBJ) $(RS_OBJS) $(TRACE_OBJ) $(ASRC:%.s=$(BUILD_DIR)/%.o) $(LDLIBS)

INITIAL_PROFILER: INITIAL_COMPILATION
	@echo "Starting Initial Profiler"
	@test -d $(SEU_GEN_DIR) || mkdir -p $(SEU_GEN_DIR)
	@$(READELF) --wide -s binary/initialFreeRTOS.elf | grep " FUNC    " | awk '{print $$3 " " $$8 }' | sort -k 2 | uniq -u  > $(SEU_GEN_DIR)/fullMap.data
	@awk '/\*-{6}\*/{x++}{print >"$(SEU_GEN_DIR)/template_half_" x ".ld" }' x=0 $(INITIAL_LINKERSCRIPT) #Split Linker script in half
	@$(PYTHON) $(SEU_DIR)/initial_profiler.py
	@echo "initial Profiler Completed"

SECONDARY_COMPILATION: INITIAL_PROFILER
	@echo "Starting Secondary Complilation"
	@$(CC) -Wl,-Map,$(TARGET).map -o $(BIN_DIR)/final$(TARGET).elf $(SECONDARY_LINKERSCRIPT) $(LDFLAGS) $(OBJ) $(RS_OBJS) $(TRACE_OBJ) $(HEADER_OBJ) $(ASRC:%.s=$(BUILD_DIR)/%.o) $(LDLIBS)
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
