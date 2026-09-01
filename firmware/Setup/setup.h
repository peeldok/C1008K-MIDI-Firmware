/* Setup + brightness screens for the C100.
 *
 * Entered from normal play with the top-left corner key (handled by the
 * dispatcher in Core/main.c). While UI_SETUP / UI_BRIGHTNESS is active this
 * module owns every key event and the whole panel:
 *
 *   UI_SETUP      bottom-left 2 cells  : layout (green drum / orange programmer)
 *                 centre 4 cells       : white — enter the brightness screen
 *                 32 side keys         : rotation (Top 0 / Left 90 / Bottom 180 / Right 270)
 *                 top-left 3x3         : app selector ((0,7) red = Performance)
 *                 gx5..7 gy1 (O S M)   : built-in palette 0..2
 *                 gx5..7 gy0 (C1..C3)  : custom palette 3..5
 *                 gx4    gy0 (L)       : live palette preview (display only)
 *                 bottom-right corner  : jump to the ROM bootloader
 *   UI_BRIGHTNESS "LED" label + 8-step bar on the bottom row
 *
 * Layout / rotation / app / brightness / palette and the custom palettes are
 * persisted to flash (nvs.c) when the setup screen is left.
 */
#ifndef C100_SETUP_H
#define C100_SETUP_H

#include <stdint.h>
#include <stdbool.h>
#include "mapping.h"

/* Load persisted settings from flash into the g_* state. Call once at startup. */
void setup_load_settings(void);

/* Write settings to flash if anything changed since the last save. Call when
 * leaving the setup screen and before a bootloader jump. */
void setup_save_settings(void);

/* Brightness ladder value (0..255) for the persisted step — used to init the
 * LED driver at boot. */
uint8_t setup_brightness(void);

/* One key event while a setup/brightness screen owns the panel. */
void setup_key(pad_id_t pad, bool pressed);

/* Draw the active screen. The caller clears the panel and calls leds_render();
 * this also advances the palette-preview colour cycle. */
void setup_render(void);

#endif
