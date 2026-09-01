#include "palette.h"
#include "app_state.h"
#include "nvs.h"
#include "palettes.h"
#include <string.h>

static inline uint8_t up6(uint8_t v6) { v6 &= 0x3F; return (uint8_t)((v6 << 2) | (v6 >> 4)); }

static const uint8_t *const builtin[3][3] = {
  { PAL_ORIGINAL_R, PAL_ORIGINAL_G, PAL_ORIGINAL_B },
  { PAL_LPS_R,      PAL_LPS_G,      PAL_LPS_B      },
  { PAL_MATS_R,     PAL_MATS_G,     PAL_MATS_B     },
};

/* custom slots, 6-bit interleaved R,G,B per index */
static uint8_t custom6[3][PALETTE_CUSTOM_BYTES];
static uint8_t custom_valid;   /* bit i => slot i uploaded */

void palette_rgb8_of(uint8_t pal, uint8_t idx, uint8_t rgb[3])
{
  idx &= 0x7F;
  if (pal < 3) {
    rgb[0] = up6(builtin[pal][0][idx]);
    rgb[1] = up6(builtin[pal][1][idx]);
    rgb[2] = up6(builtin[pal][2][idx]);
    return;
  }
  uint8_t s = (uint8_t)(pal - PALETTE_CUSTOM0);
  if (s > 2 || !(custom_valid & (1u << s))) {   /* not uploaded -> white */
    rgb[0] = rgb[1] = rgb[2] = 255;
    return;
  }
  const uint8_t *p = &custom6[s][idx * 3];
  rgb[0] = up6(p[0]);
  rgb[1] = up6(p[1]);
  rgb[2] = up6(p[2]);
}

void palette_rgb8(uint8_t idx, uint8_t rgb[3])
{
  palette_rgb8_of((uint8_t)g_palette, idx, rgb);
}

void palette_load_custom(const uint8_t *data6, uint8_t valid)
{
  memcpy(custom6, data6, sizeof custom6);
  custom_valid = valid & 0x07u;
}

void palette_set_custom_component(uint8_t slot, uint8_t comp, const uint8_t v6[128])
{
  if (slot > 2 || comp > 2) return;
  for (int i = 0; i < 128; i++)
    custom6[slot][i * 3 + comp] = v6[i] & 0x3F;
  custom_valid |= (uint8_t)(1u << slot);
}

void palette_get_custom_component(uint8_t slot, uint8_t comp, uint8_t v6[128])
{
  if (slot > 2 || comp > 2) return;
  bool up = (custom_valid & (1u << slot)) != 0;
  for (int i = 0; i < 128; i++)
    v6[i] = up ? custom6[slot][i * 3 + comp] : 0x3F;   /* 0x3F -> white */
}

void palette_commit_to_flash(void)
{
  static nvs_palettes_t blob;
  memcpy(&blob.custom6[0][0], custom6, sizeof custom6);
  blob.valid = custom_valid;
  nvs_palettes_save(&blob);                 /* fills magic/version/crc */
}
