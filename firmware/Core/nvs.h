/* Persistent storage for the C100. Two 2 KB flash sectors at the top of the
 * 256 KB image (the linker caps the image at 252 KB):
 *   0x0803F000  settings  — small, rewritten whenever a setting changes
 *   0x0803F800  palettes  — 3 custom palettes, rewritten only on web upload
 */
#ifndef C100_NVS_H
#define C100_NVS_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint32_t magic;
  uint16_t version;
  uint8_t  layout;           /* layout_t   */
  uint8_t  brightness_step;  /* 0..7       */
  uint8_t  rotation;         /* 0..3       */
  uint8_t  app;              /* app id     */
  uint8_t  palette;          /* active palette 0..5 */
  uint8_t  _reserved[3];
  uint32_t crc;
} nvs_settings_t;             /* 20 bytes */

typedef struct {
  uint32_t magic;
  uint16_t version;
  uint8_t  valid;            /* bit i => custom slot i uploaded */
  uint8_t  _reserved;
  uint8_t  custom6[3][128 * 3];   /* 6-bit interleaved R,G,B */
  uint32_t crc;
} nvs_palettes_t;            /* 1160 bytes */

bool nvs_settings_load(nvs_settings_t *out);
void nvs_settings_save(nvs_settings_t *in);

bool nvs_palettes_load(nvs_palettes_t *out);
void nvs_palettes_save(nvs_palettes_t *in);

#endif
