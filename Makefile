.DEFAULT_GOAL := default

GNU_INSTALL_BIN_DIR ?= 
CROSS_COMPILE       ?= arm-none-eabi-
VERBOSE             ?= 0
DEBUG               ?= 0
TARGET              ?= main
TARGET_EXE          := $(TARGET).elf
TARGET_BIN          := $(TARGET).bin
TARGET_HEX          := $(TARGET).hex
LINKER_SCRIPT       ?= src/main.ld

ifeq ($(DEBUG),1)
    BUILD_DIR ?= ./build/debug
else
    BUILD_DIR ?= ./build/release
endif

# echo suspend
ifeq ($(VERBOSE),1)
  NO_ECHO :=
else
  NO_ECHO := @
endif

CC      := $(GNU_INSTALL_BIN_DIR)$(CROSS_COMPILE)gcc
AR      := $(GNU_INSTALL_BIN_DIR)$(CROSS_COMPILE)ar
AS      := $(GNU_INSTALL_BIN_DIR)$(CROSS_COMPILE)as
CXX     := $(GNU_INSTALL_BIN_DIR)$(CROSS_COMPILE)g++
OBJCOPY := $(GNU_INSTALL_BIN_DIR)$(CROSS_COMPILE)objcopy
SIZE    := $(GNU_INSTALL_BIN_DIR)$(CROSS_COMPILE)size
MKDIR_P ?= mkdir -p
NRFJPROG ?= nrfjprog

NRFX_DIR  := externals/nrfx
CMSIS_DIR := externals/CMSIS_5
ULOG_DIR  := externals/ulog

SRCS += \
  src/main.c \
  $(NRFX_DIR)/drivers/src/nrfx_uart.c \
  $(NRFX_DIR)/drivers/src/nrfx_rtc.c \
  $(NRFX_DIR)/drivers/src/nrfx_clock.c \
  $(NRFX_DIR)/mdk/gcc_startup_nrf52840.S \
  $(NRFX_DIR)/mdk/system_nrf52840.c \
  $(ULOG_DIR)/lib/src/gen/ulog_msg.pb.c \
  $(ULOG_DIR)/lib/src/ulog_encoder.c \
  $(ULOG_DIR)/lib/src/ulog.c \
  $(ULOG_DIR)/externals/nanopb/pb_common.c \
  $(ULOG_DIR)/externals/nanopb/pb_encode.c

INC_DIRS := \
  include \
  $(NRFX_DIR) \
  $(NRFX_DIR)/mdk \
  $(NRFX_DIR)/helpers \
  $(NRFX_DIR)/hal \
  $(NRFX_DIR)/drivers \
  $(NRFX_DIR)/drivers/include \
  $(CMSIS_DIR)/CMSIS/Core/Include \
  $(ULOG_DIR)/lib/include \
  $(ULOG_DIR)/lib/src/gen \
  $(ULOG_DIR)/externals/nanopb

DEFINES := NRF52840_XXAA

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CFLAGS += $(INC_FLAGS) -MMD -MP
CFLAGS += $(addprefix -D,$(DEFINES))
CFLAGS += -fconserve-stack
CFLAGS += -fdata-sections
CFLAGS += -ffreestanding
CFLAGS += -ffunction-sections
CFLAGS += -g3
CFLAGS += -Og
CFLAGS += -Wformat -Wformat-signedness
CFLAGS += -march=armv7e-m
CFLAGS += -mfloat-abi=hard
CFLAGS += -mfpu=fpv4-sp-d16
CFLAGS += -mthumb
CFLAGS += -mabi=aapcs
CFLAGS += -std=c11
# CFLAGS += -E

CXXFLAGS += $(CFLAGS)

LDFLAGS += -Wl,--no-warn-rwx-segments
LDFLAGS += -mabi=aapcs
LDFLAGS += -mthumb
LDFLAGS += -mfloat-abi=hard
# LDFLAGS += -nostdlib
LDFLAGS += -L$(NRFX_DIR)/mdk
LDFLAGS += --specs=nano.specs
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -T$(LINKER_SCRIPT)

ASFLAGS += -D__HEAP_SIZE=8192
ASFLAGS += -D__STACK_SIZE=8192

$(TARGET_EXE): $(OBJS)
	$(NO_ECHO)$(CC) $(LDFLAGS) -Wl,-Map=$(addprefix $(BUILD_DIR)/,$(@:.elf=.map)) -o $@ $(addprefix $(BUILD_DIR)/,$(notdir $^))
	$(NO_ECHO)$(SIZE) $@

$(TARGET_HEX): $(TARGET_EXE)
	$(NO_ECHO)$(OBJCOPY) -O ihex $< $@

$(TARGET_BIN): $(TARGET_EXE)
	$(NO_ECHO)$(OBJCOPY) -O binary $< $@

# Create object files from assembly source files
$(BUILD_DIR)/%.S.o: %.S
	$(NO_ECHO)$(MKDIR_P) $(dir $@)
	$(NO_ECHO)$(CC) -x assembler-with-cpp $(ASFLAGS) -c $< -o $(addprefix $(BUILD_DIR)/,$(notdir $@))

# Create object files from C source files
$(BUILD_DIR)/%.c.o: %.c
	$(NO_ECHO)$(MKDIR_P) $(BUILD_DIR)
	$(NO_ECHO)$(CC) $(CFLAGS) -c $< -o $(addprefix $(BUILD_DIR)/,$(notdir $@))

# Create object files from C++ source files
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(NO_ECHO)$(MKDIR_P) $(BUILD_DIR)
	$(NO_ECHO)$(CXX) $(CXXFLAGS) -c $< -o $(addprefix $(BUILD_DIR)/,$(notdir $@))

.PHONY: clean flash erase 

default: $(TARGET_EXE) $(TARGET_BIN) $(TARGET_HEX)

clean:
	$(NO_ECHO)$(RM) $(abspath $(TARGET_EXE))
	$(NO_ECHO)$(RM) $(abspath $(TARGET_HEX))
	$(NO_ECHO)$(RM) $(abspath $(TARGET_BIN))
	$(NO_ECHO)$(RM) -r $(abspath $(BUILD_DIR))

# Flash the program
flash: default
	$(NO_ECHO)echo Flashing: $(abspath $(TARGET_HEX))
	$(NO_ECHO)$(NRFJPROG) -f nrf52 --program $(abspath $(TARGET_HEX)) --verify --sectorerase
	$(NO_ECHO)$(NRFJPROG) -f nrf52 --reset

erase:
	$(NO_ECHO)$(NRFJPROG) -f nrf52 --eraseall

-include $(DEPS)
