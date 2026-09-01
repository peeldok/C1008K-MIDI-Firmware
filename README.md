[![MCU](https://img.shields.io/badge/MCU-AT32F405RCT7-orange?style=flat-square)](#)
[![License](https://img.shields.io/badge/license-AGPLv3-blue?style=flat-square)](LICENSE)

# 🎹 C100 MIDI

**Custom MIDI firmware for the Keychron C100**

This firmware turns the C100 into an **RGB MIDI controller**.

## Features

* **Apollo Studio** support
* **Drum / Programmer** pad layouts
* **0° / 90° / 180° / 270° panel rotation**
* Velocity-based RGB palettes
* 3 built-in palettes + 3 custom palettes
* Adjustable LED brightness
* Boot animation

## Hardware

* **Device:** Keychron C100
* **MCU:** AT32F405RCT7-7
* **CPU:** ARM Cortex-M4F @ 216 MHz
* **Flash:** 256 KB
* **SRAM:** 96 KB
* **LED Driver:** 2× SNLED2735
* **USB:** USB MIDI
* **Bootloader:** AT32 ROM DFU

The stock ROM bootloader is left untouched, so the original Keychron firmware can be restored at any time.

## Libraries

This firmware uses:

* FreeRTOS
* TinyUSB
* Artery AT32F402/405 Firmware Library
* ARM GNU Toolchain

The firmware size is approximately **40 KB**.

## Building

Required libraries:

```text
Library/
├─ AT32F402_405_Firmware_Library/
├─ tinyusb/
└─ FreeRTOS-Kernel/
```

Build with:

```bash
./build.sh
```

or:

```bash
make
```

Output files:

```text
build/c100-midigrid.elf
build/c100-midigrid.bin
build/c100-midigrid.hex
```

## Flashing

### Enter DFU Mode

Hold the **top-left corner key** while connecting the USB cable.

When DFU mode is entered successfully, the following device should appear:

```text
DFU in FS Mode
VID:PID = 2E3C:DF11
```

If C100 MIDI is already running, DFU mode can also be entered from the setup menu.

### Web Flasher

You can use the web flasher below:

https://fw.peeldok.dev

On Windows, the DFU device may need to be assigned the **WinUSB driver** using Zadig.

### dfu-util

```bash
dfu-util -d 2e3c:df11 -a 0 -s 0x08000000:leave -D c100.bin
```

## Restoring the Stock Firmware

The original Keychron firmware can be restored using the web flasher or `dfu-util`.

### dfu-util

```bash
dfu-util -d 2e3c:df11 -a 0 -s 0x08000000:leave -D original.bin
```

The AT32 ROM bootloader is stored separately from the application flash and is not overwritten when installing C100 MIDI.

## Project Structure

```text
firmware/
├─ Core/
├─ Performance/
├─ Setup/
├─ BootAnimation/
├─ Palette/
├─ Sysex/
├─ Drivers/
└─ USB/

Platform/
└─ AT32F405/
```

## Credits

* [peeldok](https://github.com/peeldok)
* [TinyUSB](https://github.com/hathach/tinyusb)
* [FreeRTOS](https://github.com/FreeRTOS/FreeRTOS-Kernel)
* [Apollo Studio](https://github.com/mat1jaczyyy/apollo-studio)

## License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPL-3.0)**.

See the `LICENSE` file for details.
