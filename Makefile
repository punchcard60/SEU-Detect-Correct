TARGET := FreeRTOS

ifeq (,$(TOOLCHAIN_ROOT))
TOOLCHAIN_ROOT := /usr/local
endif

TOOLCHAIN_ROOT := $(abspath $(TOOLCHAIN_ROOT))
TOOLCHAIN_BIN := $(TOOLCHAIN_ROOT)/bin
TOOLCHAIN_PREFIX := arm-none-eabi

STM32F4_DISCO ?= 0

ifneq ($(STM32F4_DISCO),0)
  HSE_VALUE ?= 8000000
  CFLAGS += -DSTM32F4_SYSCLK=16000000 -DSTM32F4_PLL_M=8
else
  HSE_VALUE ?= 24000000
endif

HEAP_IMPL ?= heap_3
OPTLVL ?= 0
DBG ?= -g

CMD := $(if $(V),,@)

FREERTOS := FreeRTOS
HW_DIR := hardware
REED_SOLOMON := ../Reed-Solomon-Packed
RS_SRC := $(REED_SOLOMON)/src
SEU_DIR := seu
BUILD_DIR := build
SEU_SRC_DIR := $(SEU_DIR)/src
SEU_GEN_DIR := $(BUILD_DIR)/gen

INCLUDE += -Iconfig \
           -I$(HW_DIR) \
           -I$(SEU_DIR)/include \
           -I$(REED_SOLOMON)/include \
           -Ilib/CMSIS/Device/ST/STM32F4xx/Include \
           -Ilib/CMSIS/Include \
           -Ilib/STM32F4xx_StdPeriph_Driver/inc \
           -Ilib/CRC_Generator \
           -I$(FREERTOS)/include \
           -I$(FREERTOS)/portable/GCC/ARM_CM4F

ASRC := $(HW_DIR)/startup_stm32f40_41xxx.s

SRC += $(wildcard $(SEU_SRC_DIR)/*.c) \
       $(wildcard $(HW_DIR)/*.c) \
       lib/syscall/syscall.c \
       main.c \
       $(FREERTOS)/list.c \
       $(FREERTOS)/queue.c \
       $(FREERTOS)/tasks.c \
       $(FREERTOS)/timers.c \
       $(FREERTOS)/portable/GCC/ARM_CM4F/port.c \
       $(FREERTOS)/portable/MemMang/$(HEAP_IMPL).c

RS_SRCS := $(wildcard $(RS_SRC)/*.c)

# CRC32 sources
CRC_SRCS := $(RS_SRCS) \
            lib/CRC_Generator/main.c

OBJ := $(addprefix $(BUILD_DIR)/,$(SRC:.c=.c.o))
OBJ += $(addprefix $(BUILD_DIR)/RS/,$(notdir $(RS_SRCS:.c=.c.o)))
AOBJ := $(addprefix $(BUILD_DIR)/,$(ASRC:.s=.s.o))

MCU_FLAGS := -mcpu=cortex-m4 -mthumb -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=hard
COMMONFLAGS := -O$(OPTLVL) $(DBG) -Wall -falign-functions=32 -ffunction-sections -fdata-sections
CFLAGS += $(COMMONFLAGS) $(MCU_FLAGS) -DSTM32F40_41xxx -DHSE_VALUE="((uint32_t)$(HSE_VALUE))" $(INCLUDE)
LDLIBS += -lm
LDFLAGS += $(COMMONFLAGS) $(MCU_FLAGS) -fno-exceptions

STEP1_LINKERSCRIPT := $(REED_SOLOMON)/STM32F4xx_FLASH.ld
STEP2_LINKERSCRIPT := $(SEU_GEN_DIR)/secondary_seu_link.ld

STEP1_ELF := $(BUILD_DIR)/step1.elf
STEP2_ELF := $(BUILD_DIR)/step2.elf
FINAL_ELF := $(BUILD_DIR)/$(TARGET).elf

# don't count on having the tools in the PATH...
CC := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-gcc
LD := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-gcc
OBJCOPY := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-objcopy
AS := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-as
AR := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-ar
GDB := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-gdb
READELF := $(TOOLCHAIN_BIN)/$(TOOLCHAIN_PREFIX)-readelf

HOST_CC := gcc
PYTHON := python3

.PHONY: all flash gdbserver gdb clean

all: $(FINAL_ELF)

$(BUILD_DIR)/crcGenerator: $(CRC_SRCS)
	@echo [CC] crcGenerator.c
	@test -d $(dir $@) || mkdir -p $(dir $@)
	$(CMD) $(HOST_CC) $(INCLUDE) -g $(CRC_SRCS) -o $(BUILD_DIR)/crcGenerator

$(BUILD_DIR)/%.s.o: %.s
	@test -d $(dir $@) || mkdir -p $(dir $@)
	@echo [AS] $<
	$(CMD) $(AS) -o $@ $^

$(BUILD_DIR)/RS/%.c.o: $(RS_SRC)/%.c
	@test -d $(dir $@) || mkdir -p $(dir $@)
	@echo [CC] $<
	$(CMD) $(CC) $(CFLAGS) $< -c -o $@

$(BUILD_DIR)/%.c.o: %.c
	@test -d $(dir $@) || mkdir -p $(dir $@)
	@echo [CC] $<
	$(CMD) $(CC) $(CFLAGS) $< -c -o $@

$(STEP1_ELF): $(AOBJ) $(OBJ)
	@echo [LD] $(STEP1_ELF)
	@test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR)
	$(CMD) $(CC) -o $(STEP1_ELF) -T$(STEP1_LINKERSCRIPT) $(LDFLAGS) $(AOBJ) $(OBJ) $(LDLIBS)

$(STEP2_LINKERSCRIPT): $(STEP1_ELF) $(STEP1_LINKERSCRIPT)
	@echo "Starting Initial Profiler"
	@test -d $(SEU_GEN_DIR) || mkdir -p $(SEU_GEN_DIR)
	$(CMD) $(READELF) --wide -s $(STEP1_ELF) | grep " FUNC    " | awk '{print $$3 " " $$8 }' | sort -k 2 | uniq -u  > $(SEU_GEN_DIR)/fullMap.data
	$(CMD) awk '/\*-{6}\*/{x++}{print >"$(SEU_GEN_DIR)/template_half_" x ".ld" }' x=0 $(STEP1_LINKERSCRIPT) #Split Linker script in half
	$(CMD) $(PYTHON) $(SEU_DIR)/initial_profiler.py
	@echo "initial Profiler Completed"

$(STEP2_ELF): $(STEP2_LINKERSCRIPT)
	@echo "Starting Secondary Complilation"
	$(CMD) $(CC) -Wl,-Map,$(BUILD_DIR)/$(TARGET).map -o $(STEP2_ELF) -T$(STEP2_LINKERSCRIPT) $(LDFLAGS) $(AOBJ) $(OBJ) $(LDLIBS)
	@echo "Secondary Complilation Completed"

$(FINAL_ELF): $(STEP2_ELF) $(BUILD_DIR)/crcGenerator
	@echo "Starting Final Profiler"
	$(eval textOffset:=$(shell $(READELF) -S $(STEP2_ELF) | grep ".text  " | awk '{print $$6}'))
	@echo "Starting CRC_Generator/elf modifier"
	$(CMD) $(BUILD_DIR)/crcGenerator $(STEP2_ELF) $(textOffset) $(FINAL_ELF)

flash: $(FINAL_ELF)
	$(CMD) openocd -f config/openocd.cfg -f config/my_stm32f4.cfg -c "myFlash $(FINAL_ELF)"

gdbserver:
	$(CMD) openocd -f config/openocd.cfg

gdb: $(FINAL_ELF)
	$(CMD) $(GDB) $(FINAL_ELF)

clean:
	$(CMD) rm -rf $(BUILD_DIR)
