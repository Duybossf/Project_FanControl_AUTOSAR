# ============================================
#  STM32F103C8T6 + SPL + Renode support
#  Sample Project Makefile
# ============================================

# ===========================
# Project & toolchain
# ===========================
BUILDDIR      := build
TARGET_NAME   := FIRMWARE

TARGET        := $(BUILDDIR)/$(TARGET_NAME)

CROSS         ?= arm-none-eabi-
CC            := $(CROSS)gcc
AS            := $(CROSS)gcc
OBJCOPY       := $(CROSS)objcopy
OBJDUMP       := $(CROSS)objdump
SIZE          := $(CROSS)size

# ===========================
# MCU / CMSIS / SPL
# ===========================
# STM32F103 (Cortex-M3)
CPUFLAGS      := -mcpu=cortex-m3 -mthumb -mfloat-abi=soft

# Default Defines
DEFINES_BASE  := -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 \
                 -DRTE_DEVICE_STDPERIPH_RCC -DRTE_DEVICE_STDPERIPH_GPIO

# Detect Build Mode (Thêm define RUN_ON_RENODE nếu chạy Renode)
ifeq ($(TARGET_ENV),renode)
  DEFINES_MODE := -DRUN_ON_RENODE
else
  DEFINES_MODE :=
endif

DEFINES       := $(DEFINES_BASE) $(DEFINES_MODE)

# Include Directories
INC_DIRS := \
    Config \
    IoHwAb \
    MCAL/Adc \
    MCAL/Dio \
    MCAL/Platform \
    MCAL/Platform/bsp/cmsis \
    MCAL/Platform/debug \
    MCAL/Platform/spl/inc \
    MCAL/Port \
    MCAL/Pwm \
    MCAL/Types \
    .

INCLUDES := $(addprefix -I, $(INC_DIRS))

# ===========================
# C/ASM/LD flags
# ===========================
CFLAGS_COMMON := -O0 -g3 -Wall -Wextra -Wno-unused-parameter \
                 -ffreestanding -fno-builtin \
                 -ffunction-sections -fdata-sections \
                 -MMD -MP

CFLAGS        := $(CPUFLAGS) $(DEFINES) $(INCLUDES) $(CFLAGS_COMMON)
ASFLAGS       := $(CPUFLAGS) $(DEFINES) $(INCLUDES) -x assembler-with-cpp

# Linker Script
LDSCRIPT      := MCAL/Platform/bsp/linker/stm32f103.ld

LDFLAGS       := -T$(LDSCRIPT) -nostartfiles -nostdlib -static \
                 -Wl,--gc-sections -Wl,-Map=$(TARGET).map
LDFLAGS      += -specs=nano.specs -specs=nosys.specs

LDLIBS        := -Wl,--start-group -lc -lm -lgcc -Wl,--end-group

# ===========================
# Sources Configuration
# ===========================
SRCS_C := \
  $(wildcard Config/*.c) \
  $(wildcard IoHwAb/*.c) \
  $(wildcard MCAL/Adc/*.c) \
  $(wildcard MCAL/Dio/*.c) \
  $(wildcard MCAL/Platform/debug/*.c) \
  $(wildcard MCAL/Platform/bsp/cmsis/*.c) \
  $(wildcard MCAL/Platform/spl/src/*.c) \
  $(wildcard MCAL/Port/*.c) \
  $(wildcard MCAL/Pwm/*.c) \
  $(wildcard *.c)

# Startup Code
SRCS_S := \
  MCAL/Platform/bsp/startup_stm32f10x_md.s

# ===========================
# Objects / Deps
# ===========================
OBJS_C := $(patsubst %.c,$(BUILDDIR)/%.o,$(SRCS_C))
OBJS_S := $(patsubst %.s,$(BUILDDIR)/%.o,$(filter %.s,$(SRCS_S))) \
          $(patsubst %.S,$(BUILDDIR)/%.o,$(filter %.S,$(SRCS_S)))

OBJS   := $(OBJS_C) $(OBJS_S)
DEPS   := $(OBJS_C:.o=.d)

# ===========================
# Default goal
# ===========================
.PHONY: all
all: $(TARGET).bin size

# ===========================
# Compile rules
# ===========================
$(BUILDDIR)/%.o: %.c
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.s
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(AS) $(CPUFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.S
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(AS) $(ASFLAGS) -c $< -o $@

# ===========================
# Link
# ===========================
$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@if not exist build mkdir build
	$(CC) $(CPUFLAGS) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# ===========================
# BIN/HEX/SIZE/Listing
# ===========================
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

.PHONY: size
size: $(TARGET).elf
	$(SIZE) --format=berkeley $<

.PHONY: list
list: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $(TARGET).list

# ===============================
# Nạp firmware (OpencOCD)
# ===============================
.PHONY: flash
flash: $(TARGET).bin
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
	        -c "program $(TARGET).bin 0x08000000 verify reset exit" \
		-c "adapter speed 2000" \
		-c "reset_config none separate" \
		-c "init; halt" \
		-c "stm32f1x options_read 0" \
		-c "program {$(abspath $<)} verify" \
		-c "reset run; shutdown"

# ===============================
# Clean
# ===============================
.PHONY: clean
clean:
	@echo "Cleaning target: $(TARGET_NAME)..."
	@if exist "$(subst /,\,$(TARGET)).elf" del /f "$(subst /,\,$(TARGET)).elf"
	@if exist "$(subst /,\,$(TARGET)).bin" del /f "$(subst /,\,$(TARGET)).bin"
	@if exist "$(subst /,\,$(TARGET)).hex" del /f "$(subst /,\,$(TARGET)).hex"
	@if exist "$(subst /,\,$(TARGET)).map" del /f "$(subst /,\,$(TARGET)).map"
	@if exist "$(subst /,\,$(TARGET)).list" del /f "$(subst /,\,$(TARGET)).list"
	@echo "Done."

.PHONY: clean-all
clean-all:
	@echo "Cleaning entire build directory..."
	@if exist "$(subst /,\,$(BUILDDIR))" rmdir /s /q "$(subst /,\,$(BUILDDIR))"

# ===============================
# Help
# ===============================
.PHONY: help
help:
	@echo "STM32F103 Sample Build System"
	@echo ""
	@echo "  make                          Build FIRMWARE"
	@echo "  make flash                    Flash firmware to hardware"
	@echo "  make clean                    Clean artifacts"
	@echo ""
-include $(DEPS)
