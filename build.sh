#!/usr/bin/env bash
# Self-contained build for the C100 MIDI-grid firmware using the vendored
# arm-none-eabi toolchain. No cmake/make required.
set -euo pipefail
cd "$(dirname "$0")"

TC=toolchain/arm-none-eabi/bin/arm-none-eabi
CC=$TC-gcc
OBJCOPY=$TC-objcopy
SIZE=$TC-size

SDK=Library/AT32F402_405_Firmware_Library/libraries
TUSB=Library/tinyusb/src
FRT=Library/FreeRTOS-Kernel
PLAT=Platform/AT32F405
FW=firmware
OUT=build
mkdir -p $OUT

MCU="-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard"
DEFS="-DAT32F405RCT7_7 -DHEXT_VALUE=12000000 -DCFG_TUSB_MCU=OPT_MCU_AT32F402_405 -DCFG_TUSB_OS=OPT_OS_FREERTOS"
# every firmware feature folder is on the include path, so sources keep using
# plain #include "foo.h" regardless of which folder foo.h lives in.
FW_INC="-I$FW/Core -I$FW/Performance -I$FW/Setup -I$FW/BootAnimation -I$FW/Palette -I$FW/Sysex -I$FW/Drivers -I$FW/USB"
INC="$FW_INC -I$PLAT \
 -I$SDK/cmsis/cm4/core_support -I$SDK/cmsis/cm4/device_support -I$SDK/drivers/inc \
 -I$FRT/include -I$FRT/portable/GCC/ARM_CM4F -I$TUSB"
CFLAGS="$MCU $DEFS $INC -Og -g3 -Wall -Wextra -Wno-unused-parameter \
 -ffile-prefix-map=$(pwd)=c100 -ffile-prefix-map=$(pwd)/=c100/ \
 -ffunction-sections -fdata-sections -fno-common -std=gnu11"
LDFLAGS="$MCU -T$PLAT/AT32F405RCT7_FLASH.ld --specs=nano.specs --specs=nosys.specs \
 -Wl,--gc-sections -Wl,-Map=$OUT/c100-midigrid.map -Wl,--print-memory-usage -Wl,--no-warn-rwx-segments"

SRCS_C="
 $FW/Core/main.c $FW/Core/mapping.c $FW/Core/matrix.c $FW/Core/leds.c $FW/Core/midi.c $FW/Core/nvs.c
 $FW/Performance/performance.c $FW/Setup/setup.c
 $FW/Drivers/snled2735.c $FW/Sysex/sysex.c $FW/Palette/palette.c $FW/BootAnimation/boot_anim.c
 $FW/USB/usb_descriptors.c
 $PLAT/board.c $PLAT/dfu.c $PLAT/syscalls.c
 $SDK/cmsis/cm4/device_support/system_at32f402_405.c
 $SDK/drivers/src/at32f402_405_crm.c $SDK/drivers/src/at32f402_405_gpio.c
 $SDK/drivers/src/at32f402_405_spi.c $SDK/drivers/src/at32f402_405_misc.c
 $SDK/drivers/src/at32f402_405_dma.c $SDK/drivers/src/at32f402_405_flash.c
 $SDK/drivers/src/at32f402_405_usart.c $SDK/drivers/src/at32f402_405_pwc.c
 $SDK/drivers/src/at32f402_405_acc.c $SDK/drivers/src/at32f402_405_scfg.c
 $SDK/drivers/src/at32f402_405_tmr.c $SDK/drivers/src/at32f402_405_wdt.c
 $TUSB/tusb.c $TUSB/common/tusb_fifo.c $TUSB/device/usbd.c
 $TUSB/class/midi/midi_device.c
 $TUSB/portable/synopsys/dwc2/dcd_dwc2.c $TUSB/portable/synopsys/dwc2/dwc2_common.c
 $FRT/list.c $FRT/queue.c $FRT/tasks.c $FRT/timers.c
 $FRT/portable/GCC/ARM_CM4F/port.c $FRT/portable/MemMang/heap_4.c
"
SRC_S="$SDK/cmsis/cm4/device_support/startup/gcc/startup_at32f402_405.s"

OBJS=""
for s in $SRCS_C; do
  o="$OUT/$(echo "$s" | tr '/.' '__').o"
  echo "CC  $s"
  $CC $CFLAGS -c "$s" -o "$o"
  OBJS="$OBJS $o"
done
echo "AS  $SRC_S"
o="$OUT/startup.o"; $CC $MCU -c "$SRC_S" -o "$o"; OBJS="$OBJS $o"

echo "LD  c100-midigrid.elf"
$CC $OBJS $LDFLAGS -o $OUT/c100-midigrid.elf
$OBJCOPY -O binary $OUT/c100-midigrid.elf $OUT/c100-midigrid.bin
$OBJCOPY -O ihex   $OUT/c100-midigrid.elf $OUT/c100-midigrid.hex
$SIZE $OUT/c100-midigrid.elf
echo "OK -> $OUT/c100-midigrid.{elf,bin,hex}"
