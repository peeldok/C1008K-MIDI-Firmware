/* SNLED2735 x2 RGB matrix driver for the Keychron C100.
 *
 * Bus: SPI1 SCK PA5 / MOSI PA7 (AF5), software chip-select CS0 PA8 -> SNLED #0,
 *   CS1 PC9 -> SNLED #1, SDB PB7 shared. Mode 0, MSB first, 8-bit frames,
 *   bit clock PCLK2/64. Transfers are blocking (no DMA); MISO is unused.
 *
 * The physical LED buffer is 100 slots (10x10). Colours are plain 8-bit RGB;
 * per-frame global brightness is applied here on the way out.
 */
#ifndef C100_SNLED2735_H
#define C100_SNLED2735_H

#include <stdint.h>

#define SNLED_PHYS_LEDS 100

typedef struct { uint8_t r, g, b; } rgb_t;

/* SDB wake + both devices' register init (config page, current tune, normal mode). */
void snled_init(void);

/* Push a full 100-slot RGB frame to both SNLED2735 devices. Blocking. */
void snled_render(const rgb_t frame[SNLED_PHYS_LEDS], uint8_t brightness /*0..255*/);

#endif
