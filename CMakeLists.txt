cmake_minimum_required(VERSION 3.16)

# Toolchain (vendored arm-none-eabi) — allow override
if(NOT CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain/arm-none-eabi.cmake")
endif()

project(c100-midigrid C ASM)

set(CMAKE_C_STANDARD 11)

set(SDK   ${CMAKE_SOURCE_DIR}/Library/AT32F402_405_Firmware_Library/libraries)
set(TUSB  ${CMAKE_SOURCE_DIR}/Library/tinyusb/src)
set(FRT   ${CMAKE_SOURCE_DIR}/Library/FreeRTOS-Kernel)
set(PLAT  ${CMAKE_SOURCE_DIR}/Platform/AT32F405)
set(FW    ${CMAKE_SOURCE_DIR}/firmware)

set(LINKER_SCRIPT ${PLAT}/AT32F405RCT7_FLASH.ld)

add_executable(${PROJECT_NAME}
  # firmware — feature folders
  ${FW}/Core/main.c ${FW}/Core/mapping.c ${FW}/Core/matrix.c ${FW}/Core/leds.c
  ${FW}/Core/midi.c ${FW}/Core/nvs.c
  ${FW}/Performance/performance.c
  ${FW}/Setup/setup.c
  ${FW}/Drivers/snled2735.c
  ${FW}/Sysex/sysex.c
  ${FW}/Palette/palette.c
  ${FW}/BootAnimation/boot_anim.c
  ${FW}/USB/usb_descriptors.c
  # platform
  ${PLAT}/board.c ${PLAT}/dfu.c ${PLAT}/syscalls.c
  # AT32 SDK
  ${SDK}/cmsis/cm4/device_support/system_at32f402_405.c
  ${SDK}/cmsis/cm4/device_support/startup/gcc/startup_at32f402_405.s
  ${SDK}/drivers/src/at32f402_405_crm.c
  ${SDK}/drivers/src/at32f402_405_gpio.c
  ${SDK}/drivers/src/at32f402_405_spi.c
  ${SDK}/drivers/src/at32f402_405_misc.c
  ${SDK}/drivers/src/at32f402_405_dma.c
  ${SDK}/drivers/src/at32f402_405_flash.c
  ${SDK}/drivers/src/at32f402_405_usart.c
  ${SDK}/drivers/src/at32f402_405_pwc.c
  ${SDK}/drivers/src/at32f402_405_acc.c
  ${SDK}/drivers/src/at32f402_405_scfg.c
  ${SDK}/drivers/src/at32f402_405_tmr.c
  ${SDK}/drivers/src/at32f402_405_wdt.c
  # TinyUSB
  ${TUSB}/tusb.c
  ${TUSB}/common/tusb_fifo.c
  ${TUSB}/device/usbd.c
  ${TUSB}/class/midi/midi_device.c
  ${TUSB}/portable/synopsys/dwc2/dcd_dwc2.c
  ${TUSB}/portable/synopsys/dwc2/dwc2_common.c
  # FreeRTOS
  ${FRT}/list.c ${FRT}/queue.c ${FRT}/tasks.c ${FRT}/timers.c
  ${FRT}/portable/GCC/ARM_CM4F/port.c
  ${FRT}/portable/MemMang/heap_4.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
  # every firmware feature folder — sources use plain #include "foo.h"
  ${FW}/Core ${FW}/Performance ${FW}/Setup ${FW}/BootAnimation
  ${FW}/Palette ${FW}/Sysex ${FW}/Drivers ${FW}/USB
  ${PLAT}
  ${SDK}/cmsis/cm4/core_support
  ${SDK}/cmsis/cm4/device_support
  ${SDK}/drivers/inc
  ${FRT}/include
  ${FRT}/portable/GCC/ARM_CM4F
  ${TUSB}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  AT32F405RCT7_7
  HEXT_VALUE=12000000
  CFG_TUSB_MCU=OPT_MCU_AT32F402_405
  CFG_TUSB_OS=OPT_OS_FREERTOS
)

target_compile_options(${PROJECT_NAME} PRIVATE
  -Og -g3 -Wall -Wextra -Wno-unused-parameter
  -ffile-prefix-map=${CMAKE_SOURCE_DIR}=c100
)

target_link_options(${PROJECT_NAME} PRIVATE
  -T${LINKER_SCRIPT}
  -Wl,-Map=${PROJECT_NAME}.map
  -Wl,--gc-sections
  -Wl,--print-memory-usage
  -Wl,--no-warn-rwx-segments
)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
  VERBATIM
)
