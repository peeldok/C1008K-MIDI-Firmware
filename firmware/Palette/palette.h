/* Velocity/value -> RGB palette selection for the C100.
 *
 * Six slots: 0 Original, 1 LPS, 2 Mat's (built-in, Palette/palettes.h),
 * 3..5 custom C1/C2/C3 (uploaded later over web SysEx, held in flash by nvs.c).
 * All entries are 6-bit per channel; this module upscales to 8-bit on read.
 * An un-uploaded custom slot reads back as solid white (255,255,255).
 */
#ifndef C100_PALETTE_H
#define C100_PALETTE_H

#include <stdint.h>

#define PALETTE_COUNT   6
#define PALETTE_CUSTOM0 3            /* first custom slot */
#define PALETTE_CUSTOM_BYTES (128 * 3)   /* 6-bit R,G,B per index, per slot */

/* 8-bit colour for `idx` (0..127) from a specific / the active palette. */
void palette_rgb8_of(uint8_t pal, uint8_t idx, uint8_t rgb[3]);
void palette_rgb8(uint8_t idx, uint8_t rgb[3]);   /* uses g_palette */

/* Load the custom-slot data (3 x PALETTE_CUSTOM_BYTES, 6-bit) plus the
 * "uploaded" bitmask (bit i => slot PALETTE_CUSTOM0+i is real), from nvs.c. */
void palette_load_custom(const uint8_t *data6, uint8_t valid);

/* Web-SysEx path: one custom slot (0..2), one channel comp (0=R 1=G 2=B),
 * 128 6-bit values. Setting any component marks the slot uploaded. */
void palette_set_custom_component(uint8_t slot, uint8_t comp, const uint8_t v6[128]);
/* Read a component back (0x3F white for an un-uploaded slot). */
void palette_get_custom_component(uint8_t slot, uint8_t comp, uint8_t v6[128]);

/* Persist all three custom slots + valid mask to flash (nvs.c). Blocking. */
void palette_commit_to_flash(void);

#endif
