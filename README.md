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

## Setup UI
<svg xmlns="http://www.w3.org/2000/svg" width="714" height="714" viewBox="0 0 714 714" font-family="Inter, system-ui, -apple-system, Segoe UI, sans-serif">
<rect width="714" height="714" fill="#0c0e13"/>
<rect x="10" y="10" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff26"/>
<text x="42.0" y="46.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">Setup</text>
<rect x="80" y="10" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="112.0" y="46.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">0°</text>
<rect x="150" y="10" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="182.0" y="46.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">0°</text>
<rect x="220" y="10" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="252.0" y="46.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">0°</text>
<rect x="290" y="10" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="322.0" y="46.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">0°</text>
<rect x="360" y="10" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="392.0" y="46.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">0°</text>
<rect x="430" y="10" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="462.0" y="46.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">0°</text>
<rect x="500" y="10" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="532.0" y="46.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">0°</text>
<rect x="570" y="10" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="602.0" y="46.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">0°</text>
<rect x="640" y="10" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="10" y="80" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="42.0" y="116.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">90°</text>
<rect x="80" y="80" width="64" height="64" rx="7" fill="#820000" stroke="#ffffff26"/>
<text x="112.0" y="116.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">Per</text>
<rect x="150" y="80" width="64" height="64" rx="7" fill="#040404" stroke="#ffffff26"/>
<text x="182.0" y="116.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">app</text>
<rect x="220" y="80" width="64" height="64" rx="7" fill="#040404" stroke="#ffffff26"/>
<text x="252.0" y="116.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">app</text>
<rect x="290" y="80" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="360" y="80" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="430" y="80" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="500" y="80" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="570" y="80" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="640" y="80" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="672.0" y="116.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">270°</text>
<rect x="10" y="150" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="42.0" y="186.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">90°</text>
<rect x="80" y="150" width="64" height="64" rx="7" fill="#040404" stroke="#ffffff26"/>
<text x="112.0" y="186.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">app</text>
<rect x="150" y="150" width="64" height="64" rx="7" fill="#040404" stroke="#ffffff26"/>
<text x="182.0" y="186.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">app</text>
<rect x="220" y="150" width="64" height="64" rx="7" fill="#040404" stroke="#ffffff26"/>
<text x="252.0" y="186.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">app</text>
<rect x="290" y="150" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="360" y="150" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="430" y="150" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="500" y="150" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="570" y="150" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="640" y="150" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="672.0" y="186.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">270°</text>
<rect x="10" y="220" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="42.0" y="256.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">90°</text>
<rect x="80" y="220" width="64" height="64" rx="7" fill="#040404" stroke="#ffffff26"/>
<text x="112.0" y="256.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">app</text>
<rect x="150" y="220" width="64" height="64" rx="7" fill="#040404" stroke="#ffffff26"/>
<text x="182.0" y="256.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">app</text>
<rect x="220" y="220" width="64" height="64" rx="7" fill="#040404" stroke="#ffffff26"/>
<text x="252.0" y="256.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">app</text>
<rect x="290" y="220" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="360" y="220" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="430" y="220" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="500" y="220" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="570" y="220" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="640" y="220" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="672.0" y="256.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">270°</text>
<rect x="10" y="290" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="42.0" y="326.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">90°</text>
<rect x="80" y="290" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="150" y="290" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="220" y="290" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="290" y="290" width="64" height="64" rx="7" fill="#969696" stroke="#ffffff26"/>
<text x="322.0" y="326.5" fill="#0b0e13" font-size="13" font-weight="700" text-anchor="middle">Bright</text>
<rect x="360" y="290" width="64" height="64" rx="7" fill="#969696" stroke="#ffffff26"/>
<text x="392.0" y="326.5" fill="#0b0e13" font-size="13" font-weight="700" text-anchor="middle">Bright</text>
<rect x="430" y="290" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="500" y="290" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="570" y="290" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="640" y="290" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="672.0" y="326.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">270°</text>
<rect x="10" y="360" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="42.0" y="396.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">90°</text>
<rect x="80" y="360" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="150" y="360" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="220" y="360" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="290" y="360" width="64" height="64" rx="7" fill="#969696" stroke="#ffffff26"/>
<text x="322.0" y="396.5" fill="#0b0e13" font-size="13" font-weight="700" text-anchor="middle">Bright</text>
<rect x="360" y="360" width="64" height="64" rx="7" fill="#969696" stroke="#ffffff26"/>
<text x="392.0" y="396.5" fill="#0b0e13" font-size="13" font-weight="700" text-anchor="middle">Bright</text>
<rect x="430" y="360" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="500" y="360" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="570" y="360" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="640" y="360" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="672.0" y="396.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">270°</text>
<rect x="10" y="430" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="42.0" y="466.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">90°</text>
<rect x="80" y="430" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="150" y="430" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="220" y="430" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="290" y="430" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="360" y="430" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="430" y="430" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="500" y="430" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="570" y="430" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="640" y="430" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="672.0" y="466.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">270°</text>
<rect x="10" y="500" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="42.0" y="536.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">90°</text>
<rect x="80" y="500" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="150" y="500" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="220" y="500" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="290" y="500" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="360" y="500" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="430" y="500" width="64" height="64" rx="7" fill="#969696" stroke="#ffffff26"/>
<text x="462.0" y="536.5" fill="#0b0e13" font-size="13" font-weight="700" text-anchor="middle">LP RGB</text>
<rect x="500" y="500" width="64" height="64" rx="7" fill="#101010" stroke="#ffffff26"/>
<text x="532.0" y="536.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">LP RG</text>
<rect x="570" y="500" width="64" height="64" rx="7" fill="#101010" stroke="#ffffff26"/>
<text x="602.0" y="536.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">Mat's</text>
<rect x="640" y="500" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="672.0" y="536.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">270°</text>
<rect x="10" y="570" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="42.0" y="606.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">90°</text>
<rect x="80" y="570" width="64" height="64" rx="7" fill="#006400" stroke="#ffffff26"/>
<text x="112.0" y="606.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">Drum</text>
<rect x="150" y="570" width="64" height="64" rx="7" fill="#100600" stroke="#ffffff26"/>
<text x="182.0" y="606.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">Prog</text>
<rect x="220" y="570" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="290" y="570" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="360" y="570" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="430" y="570" width="64" height="64" rx="7" fill="#2c2812" stroke="#ffffff26"/>
<text x="462.0" y="606.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">Custom 1</text>
<rect x="500" y="570" width="64" height="64" rx="7" fill="#2c2812" stroke="#ffffff26"/>
<text x="532.0" y="606.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">Custom 2</text>
<rect x="570" y="570" width="64" height="64" rx="7" fill="#2c2812" stroke="#ffffff26"/>
<text x="602.0" y="606.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">Custom 3</text>
<rect x="640" y="570" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="672.0" y="606.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">270°</text>
<rect x="10" y="640" width="64" height="64" rx="7" fill="#000000" stroke="#ffffff0f"/>
<rect x="80" y="640" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="112.0" y="676.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">180°</text>
<rect x="150" y="640" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="182.0" y="676.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">180°</text>
<rect x="220" y="640" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="252.0" y="676.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">180°</text>
<rect x="290" y="640" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="322.0" y="676.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">180°</text>
<rect x="360" y="640" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="392.0" y="676.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">180°</text>
<rect x="430" y="640" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="462.0" y="676.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">180°</text>
<rect x="500" y="640" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="532.0" y="676.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">180°</text>
<rect x="570" y="640" width="64" height="64" rx="7" fill="#005a00" stroke="#ffffff26"/>
<text x="602.0" y="676.5" fill="#f4f7fb" font-size="13" font-weight="700" text-anchor="middle">180°</text>
<rect x="640" y="640" width="64" height="64" rx="7" fill="#000078" stroke="#ffffff26"/>
<text x="672.0" y="670.0" fill="#f4f7fb" font-size="11" font-weight="700" text-anchor="middle">
  <tspan x="672.0" dy="0">Jump to</tspan>
  <tspan x="672.0" dy="13">Boot</tspan>
</text>
</svg>
<img width="714" height="714" alt="layout_L_removed" src="https://github.com/user-attachments/assets/a7359cf0-e50e-40c6-99db-1302bda52f05" />

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

## Credits

* [peeldok](https://github.com/peeldok)
* [TinyUSB](https://github.com/hathach/tinyusb)
* [FreeRTOS](https://github.com/FreeRTOS/FreeRTOS-Kernel)
* [Apollo Studio](https://github.com/mat1jaczyyy/apollo-studio)

## License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPL-3.0)**.

See the `LICENSE` file for details.
