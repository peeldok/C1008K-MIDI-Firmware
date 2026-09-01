# Third-Party Licenses and Notices

C100 MIDI is distributed under the GNU Affero General Public License v3.0 as described in `LICENSE`.
The following third-party projects are used by the build or are the source of specific compatible/derived material. Their original license terms remain applicable to those portions.

## Novation Launchpad Pro Open Source Firmware

Source: https://github.com/dvhdr/launchpad-pro  
Copyright (c) 2015, Focusrite Audio Engineering Ltd.  
License: BSD 3-Clause

C100 MIDI contains Launchpad-compatible behavior and palette/protocol material whose upstream lineage includes this firmware. The full BSD license text is in `licenses/FOCUSRITE-LAUNCHPAD-BSD-3-CLAUSE.txt`.

## Launchpad Pro Performance CFW

Source: https://github.com/mat1jaczyyy/lpp-performance-cfw  
Modification project by mat1jaczyyy; based on the Focusrite/Novation Launchpad Pro open-source firmware.  
License: BSD 3-Clause (repository license retains the Focusrite copyright notice)

C100 MIDI's CFY 0x5F/0x6F LED protocol behavior, Drum Rack compatibility mapping, and built-in Launchpad palette data are derived from or adapted from this project.

## TinyUSB

Source: https://github.com/hathach/tinyusb  
Copyright (c) 2012-2026, Ha Thach (tinyusb.org) and contributors  
License: MIT

TinyUSB provides the USB device stack. `firmware/USB/usb_descriptors.c` also follows the structure of TinyUSB MIDI examples. The full MIT license text is in `licenses/TINYUSB-MIT.txt`.

## FreeRTOS Kernel

Source: https://github.com/FreeRTOS/FreeRTOS-Kernel  
Copyright: Amazon.com, Inc. or its affiliates and contributors  
License: MIT

FreeRTOS provides the RTOS kernel. The full MIT license text is in `licenses/FREERTOS-MIT.txt`.

## Artery AT32F402/405 Firmware Library

Source: https://github.com/ArteryTek/AT32F402_405_Firmware_Library  
Copyright (c) 2022, Artery-MCU  
License: BSD 3-Clause

The AT32 peripheral drivers/CMSIS device support are linked into the firmware at build time. Portions of the C100 clock setup, peripheral configuration and linker script are adapted from Artery reference files. The full BSD license text is in `licenses/ARTERY-BSD-3-CLAUSE.txt`.

## Arm CMSIS

Source: https://github.com/ARM-software/CMSIS_5  
Copyright: Arm Limited and contributors  
License: Apache License 2.0

CMSIS core support is supplied through the Artery SDK. The Apache 2.0 license text is in `licenses/ARM-CMSIS-APACHE-2.0.txt`.

## Apollo Studio

Source: https://github.com/mat1jaczyyy/apollo-studio  
License: BSD 3-Clause

Apollo Studio is a compatibility target/reference. No Apollo Studio source code is included in this repository.
