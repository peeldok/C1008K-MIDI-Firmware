# Convenience wrapper. Real build logic is in build.sh / CMakeLists.txt.
# The vendored ARM toolchain is used automatically (toolchain/arm-none-eabi.cmake
# and build.sh both point at toolchain/arm-none-eabi/bin).

BUILD    ?= cmake-build
ELF      := $(BUILD)/c100-midigrid.elf
BIN      := $(BUILD)/c100-midigrid.bin

# AT32F405 system-memory ROM bootloader enumerates as this DFU device.
DFU_ID   ?= 2e3c:df11
FLASH_ADDR ?= 0x08000000

.PHONY: all cmake sh clean flash dfu size

all: cmake

cmake:
	cmake -S . -B $(BUILD) -G Ninja
	ninja -C $(BUILD)

sh:
	bash build.sh

size: cmake
	toolchain/arm-none-eabi/bin/arm-none-eabi-size $(ELF)

clean:
	rm -rf build cmake-build

# --- flashing (run manually; needs the board in ROM-DFU mode) ---
# Enter ROM DFU: hold the top-left corner key (row0 PC12 / col0 PC7) while plugging USB,
# or from running firmware trigger board_enter_rom_dfu().
flash dfu: cmake
	dfu-util -d $(DFU_ID) -a 0 -s $(FLASH_ADDR):leave -D $(BIN)
