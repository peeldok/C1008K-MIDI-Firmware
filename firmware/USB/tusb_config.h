/*
 * TinyUSB configuration for C100 MIDI. TinyUSB is licensed under the MIT License.
 * See THIRD_PARTY_LICENSES.md and licenses/TINYUSB-MIT.txt.
 */
/* TinyUSB config — Keychron C100 MIDI-grid firmware.
 * OTGHS (rhport 1), USB 2.0 High-Speed, FreeRTOS, MIDI class only. */
#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU              OPT_MCU_AT32F402_405
#endif
#define CFG_TUSB_OS               OPT_OS_FREERTOS

/* AT32F405 OTGHS has ONLY the embedded UTMI HS PHY (no internal FS transceiver).
 * TinyUSB's dcd_dwc2 FS path sets DCFG.DSPD = 3 ("FS on FS PHY"), which this core
 * cannot do -> the D+ line is never driven -> RESET_FAILURE. So run High Speed:
 * DCFG.DSPD = 0, exactly what the stock firmware uses. */
#define CFG_TUSB_RHPORT1_MODE     (OPT_MODE_DEVICE | OPT_MODE_HIGH_SPEED)
#define CFG_TUD_MAX_SPEED         OPT_MODE_HIGH_SPEED

#define BOARD_DEVICE_RHPORT_NUM    1
#define BOARD_DEVICE_RHPORT_SPEED  OPT_MODE_HIGH_SPEED
#define BOARD_TUD_RHPORT           1
#define BOARD_TUD_MAX_SPEED        OPT_MODE_HIGH_SPEED

/* TinyUSB internal logging OFF (also: >= 2 runs a vprintf from the USB ISR
 * during enumeration, which at High-Speed can stretch EP0 handling enough to
 * lose SETUP packets -> "device descriptor request failed"). */
#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG           0
#endif

#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN       __attribute__((aligned(4)))

#define CFG_TUD_ENABLED          1
#define CFG_TUD_ENDPOINT0_SIZE   64

#define CFG_TUD_CDC              0
#define CFG_TUD_MSC              0
#define CFG_TUD_HID              0
#define CFG_TUD_MIDI             1
#define CFG_TUD_VENDOR          0

/* Match the stock firmware USB config: the USB-MIDI streaming
 * endpoints are 64-byte bulk at both speeds (a 512-byte HS bulk EP overruns the
 * AT32 OTGHS DFIFO budget and makes MIDI transfers fail silently). The FIFO
 * ring buffers behind them stay large. */
#define CFG_TUD_MIDI_RX_EPSIZE   64
#define CFG_TUD_MIDI_TX_EPSIZE   64
#define CFG_TUD_MIDI_RX_BUFSIZE  512
#define CFG_TUD_MIDI_TX_BUFSIZE  512

#ifdef __cplusplus
}
#endif
#endif
