# Bare-metal ARM Cortex-M4F toolchain file for the Keychron C100 (AT32F405RCT7-7)
# Usage: cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchain/arm-none-eabi.cmake ...

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m4)

# Toolchain root (vendored). Override with -DTOOLCHAIN_ROOT=... if using a system install.
if(NOT DEFINED TOOLCHAIN_ROOT)
  set(TOOLCHAIN_ROOT "${CMAKE_CURRENT_LIST_DIR}/arm-none-eabi")
endif()
set(TOOLCHAIN_BIN "${TOOLCHAIN_ROOT}/bin")
set(TOOLCHAIN_PREFIX "${TOOLCHAIN_BIN}/arm-none-eabi-")

set(CMAKE_C_COMPILER   "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}g++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_OBJCOPY      "${TOOLCHAIN_PREFIX}objcopy" CACHE INTERNAL "")
set(CMAKE_OBJDUMP      "${TOOLCHAIN_PREFIX}objdump" CACHE INTERNAL "")
set(CMAKE_SIZE         "${TOOLCHAIN_PREFIX}size"    CACHE INTERNAL "")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_EXECUTABLE_SUFFIX_C   ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Cortex-M4F: hardware FPU, single precision
set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS_INIT   "${MCU_FLAGS} -ffunction-sections -fdata-sections -fno-common -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS_INIT "${MCU_FLAGS} -ffunction-sections -fdata-sections -fno-common -fno-strict-aliasing -fno-exceptions -fno-rtti -fno-threadsafe-statics")
set(CMAKE_ASM_FLAGS_INIT "${MCU_FLAGS} -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${MCU_FLAGS} --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections -Wl,--print-memory-usage")
