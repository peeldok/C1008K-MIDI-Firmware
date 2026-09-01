<div align="center">

  

  <h1>🎹 <code>C100 MIDI</code> 🟩</h1>

  

  <p>

    <strong>A bare-metal firmware for the Keychron C100 that turns it into a

    Launchpad-Pro-compatible RGB grid controller.</strong>

  </p>

  

  <p>

    <a href="#-license"><img src="https://img.shields.io/badge/license-AGPLv3-blue?style=flat-square" alt="License"></a>

    <img src="https://img.shields.io/badge/MCU-AT32F405RCT7-orange?style=flat-square" alt="MCU">

    <img src="https://img.shields.io/badge/flash-40KB%20%2F%20256KB-green?style=flat-square" alt="Flash usage">

  </p>

  

</div>

  

**C100 MIDI** is a custom bare-metal firmware for the

[Keychron C100](https://www.keychron.com) 8×8 pad grid. It re-uses the hardware as a

**USB-MIDI performance grid**: to the host it looks like a *Launchpad Pro running the

"CFY" custom firmware*, so lighting hosts that speak that protocol —

[Apollo Studio](https://github.com/mat1jaczyyy/apollo-studio) in particular — can drive

the LEDs directly.

  

It runs on **FreeRTOS + TinyUSB**, keeps the AT32F405's stock ROM DFU bootloader intact

(so you can flash back to the official Keychron firmware at any time), and ships with a

[web flasher / palette editor](#-flashing).

  

To get started, grab the latest `c100-midigrid.bin` from the

[releases](../../releases/latest) and flash it over DFU (see [Flashing](#-flashing)).

  

---

  

## 🌟 Authors

  

- [@peeldok](https://github.com/peeldok)

  

## 🌠 Features

  

- **Launchpad Pro / CFY emulation**: full Universal Device Inquiry identity plus the

  compressed `0xF0 5F` and plain `0xF0 6F` LED SysEx, so Apollo Studio lightshows work

  out of the box.

- **Two pad layouts**: *Drum* (grid notes 36–99, side-key table, top-right corner = D#0)

  and *Programmer* (Launchpad-Pro grid positions 11–88). Switchable on-device.

- **On-device setup screen**: enter with the top-left corner key to pick the layout, the

  active app, the velocity palette, and the panel rotation, open the brightness screen,

  or jump straight to the bootloader — no host software needed.

- **Global panel rotation**: a single 0 / 90 / 180 / 270° transform applied at the

  hardware boundary (key-scan in, LED index out), so every app is rotation-agnostic.

- **Velocity → RGB palettes**: 3 built-in palettes compiled into flash, plus 3 custom

  slots you upload over Web MIDI SysEx and that persist in flash.

- **Persistent settings**: layout, rotation, app, brightness step, active palette and the

  custom palettes live in two dedicated flash sectors that a firmware update never

  touches.

- **Boot animation** that plays once at power-up and clears the moment it's done.

- **Watchdog + brown-out awareness**: a ~3 s independent watchdog reboots a wedged

  firmware (the boot animation replaying is the visible symptom), and the LED current

  tune is a build-time knob so a fully-lit panel can be kept inside the USB power budget.

  

## 🎛️ Hardware

  

The Keychron C100 is an 8×8 RGB pad grid with a perimeter of keys:

  

- **MCU**: Artery **AT32F405RCT7-7** (ARM Cortex-M4F @ 216 MHz, 256 KB flash, 96 KB SRAM)

- **LEDs**: 2× **SNLED2735**

- **Keys**: 10 × 10 scanned matrix

- **USB**: High-speed USB 2.0 Type-C

## 🧰 Building

  
You need an `arm-none-eabi` toolchain and three libraries checked out under `Library/`:

  

| Path | What | Version |

| ---- | ---- | ------- |

| `toolchain/arm-none-eabi/` | ARM GNU bare-metal toolchain (a system install also works) | 13.x |

| `Library/AT32F402_405_Firmware_Library/` | Artery AT32F402/405 BSP (CMSIS + peripheral drivers) | V2.1.5 |

| `Library/tinyusb/` | [TinyUSB](https://github.com/hathach/tinyusb) device stack | latest |

| `Library/FreeRTOS-Kernel/` | [FreeRTOS](https://github.com/FreeRTOS/FreeRTOS-Kernel) (ARM_CM4F GCC port) | V11.x |

  

Then:

  

```bash

./build.sh          # self-contained, no cmake/make required

# or

make                # CMake + Ninja into cmake-build/

```

  

Output: `build/c100-midigrid.{elf,bin,hex}` — about **40 KB** (16 % of flash).

  

### 🚩 Build options

  

Override on the compiler command line (all optional):

  

| Define | Default | Effect |

| ------ | ------- | ------ |

| `SNLED_CURRENT_TUNE` | `0xFF` | SNLED2735 global current. `0xFF` is full scale; a fully-lit panel can exceed the 500 mA USB budget on a bus-powered port. Lower it (e.g. `0x78`) if you see LED stutter / brown-out. |

| `SNLED_SPI_MCLK_DIV` | `SPI_MCLK_DIV_64` | SNLED SPI bit clock. Faster dividers were seen to disturb the panel on this board. |

| `BOOT_ANIM_TIME_NUM` / `_DEN` | `3` / `1` | Boot-animation playback speed multiplier. |

  

## 📁 Layout

  

```

firmware/

  Core/           main.c (wiring + dispatch), app_state.h, matrix, mapping, leds, midi, nvs

  Performance/    the normal MIDI-grid app (UI_NORMAL)

  Setup/          the setup + brightness screens, flash persistence

  BootAnimation/  startup animation player + frame data

  Palette/        velocity → RGB palette selection + built-in tables

  Sysex/          Launchpad/CFY identity, LED SysEx, web-editor protocol

  Drivers/        SNLED2735 SPI driver + stock LED map

  USB/            device / config / string descriptors, TinyUSB config

Platform/AT32F405/

  board.c         clocks, USB PHY bring-up, watchdog, fault handlers

  dfu.c           ROM-DFU jump + recovery-key scan

  AT32F405RCT7_FLASH.ld, FreeRTOSConfig.h, ...

```

  

Adding a new app is a folder next to `Performance/` implementing `*_key` / `*_midi_note`

/ `*_render` and one line in the `on_key` dispatcher in `Core/main.c`.

  

## 💉 Flashing

  

> [!NOTE]

> This is *community firmware*. It will **not** brick your C100 — the AT32 ROM

> bootloader lives in a separate, unwritable region and always comes back if you hold

> the recovery key on power-up. You can flash Keychron's official firmware back at any

> time. That said, you are flashing low-level firmware: use a decent cable, avoid USB

> hubs, and don't yank the cable mid-write.

  

### 1. Enter the bootloader

  

Hold the **top-left corner key** while plugging in the USB cable. Windows detects a new

device named **DFU in FS Mode** (USB ID `2E3C:DF11`). From a running C100 MIDI firmware you

can also enter it from the setup screen (bottom-right corner) or over Web MIDI.

  

### 2a. Web flasher (easiest)

  

Open the [web editor](https://fw.peeldok.dev) in Chrome or Edge and use the

**Firmware** page. It flashes over WebUSB (macOS / Linux need nothing).

  

> [!IMPORTANT]

> **Windows only:** the ROM bootloader has no in-box driver, so the browser can't reach

> it until you bind **WinUSB** to `2E3C:DF11` once with [Zadig](https://zadig.akeo.ie).

> See the guide linked from the Firmware page.

  

### 2b. `dfu-util`

  

```bash

dfu-util -d 2e3c:df11 -a 0 -s 0x08000000:leave -D c100-midigrid.bin

```

  

## 🔁 Rolling back to the stock firmware

  

C100 MIDI never touches the ROM bootloader, so recovery is always possible:

  

- **Web editor**: the Firmware page has an **Original Firmware** option that downloads

  the latest official Keychron C100 image and flashes it.

- **`dfu-util`**: grab the official `.bin` from

  [launcher.keychron.com](https://launcher.keychron.com) and flash it to `0x08000000`

  the same way as above.

  

## 💛 Acknowledgements

  

- **[Keychron](https://www.keychron.com)** for the C100 hardware.

- **[MatrixOS](https://github.com/203-Systems/MatrixOS)** (203 Systems) — this project

  started from its repository, though the firmware here is an independent bare-metal

  implementation.

- **[TinyUSB](https://github.com/hathach/tinyusb)** and

  **[FreeRTOS](https://github.com/FreeRTOS/FreeRTOS-Kernel)** for the USB stack and RTOS.

- **[Artery](https://www.arterychip.com)** for the AT32F402/405 BSP.

- **[Apollo Studio](https://github.com/mat1jaczyyy/apollo-studio)** (mat1jaczyyy) and the

  Launchpad Pro CFW project, whose SysEx protocol this firmware emulates.

  

## 📝 License

  

This project is licensed under the **GNU Affero General Public License v3.0

(AGPL-3.0)**.

  

**What this means:**

  

- ✅ **You can** use, study and modify this firmware.

- ✅ **You can** build other open-source things on top of it.

- 🛑 **If you distribute** modified versions (including flashing them onto devices you

  sell), you **must** make the corresponding source available under the same license.

  

See [LICENSE](LICENSE) for the full text.
