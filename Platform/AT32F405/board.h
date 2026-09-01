/* Board bring-up API for the Keychron C100 (AT32F405RCT7-7) firmware. */
#ifndef C100_BOARD_H
#define C100_BOARD_H

#include "at32f402_405.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* C100 ROM-DFU request magic word, kept in the reserved top-of-RAM slot. */
#define C100_DFU_MAGIC_ADDR   ((volatile uint32_t*)0x20017FF0u)
#define C100_DFU_MAGIC        0x44465521u   /* "DFU!" */
#define C100_ROM_DFU_BASE     0x1FFFA400u

/* Runs first thing in main(), before FreeRTOS / USB / the app.
 * Checks the SRAM magic and the recovery key (row0 PC12 / col0 PC7); if either
 * asks for DFU it jumps straight into the AT32 system-memory ROM bootloader. */
void board_early_dfu_check(void);

/* Full hardware bring-up: 216 MHz clock, PLLU for USB, NVIC group 4,
 * SysTick left disabled for FreeRTOS to own. */
void board_init(void);

/* Request a reboot into the ROM DFU bootloader (writes magic + SYSRESETREQ). */
void board_enter_rom_dfu(void) __attribute__((noreturn));

/* ~3 s independent watchdog. Must be kicked from the app task; if USB init or any
 * task wedges, the board reboots (boot animation replays = visible symptom). */
void board_watchdog_init(void);
void board_watchdog_kick(void);

/* 96-bit MCU unique id as 12 bytes; returns bytes written. */
uint32_t board_unique_id(uint8_t out[12]);

#ifdef __cplusplus
}
#endif
#endif
