#include "leds.h"
#include "app_state.h"
#include "palette.h"
#include <string.h>

/* Physical LED index convention (first guess, verify by LED-walk on hardware):
 *   phys_index = prow * 10 + pcol       (prow,pcol 0..9)
 *   dev 0 drives phys 0..49  (rows 0..4),  dev 1 drives phys 50..99 (rows 5..9)
 * The stock PWM-register map (extracted) is applied inside snled2735.c, which
 * expects frame[] already ordered as STOCK_LED_MAP index == phys_index.
 */

static rgb_t   fb[SNLED_PHYS_LEDS];
static uint8_t brightness = 255;

uint8_t map_pad_to_led_index(pad_id_t pad)
{
  uint8_t lr, lc;
  if (!map_pad_to_phys(pad, &lr, &lc)) return 0xFF;
  uint8_t pr = lr, pc = lc;
  /* grid always rotates; side keys only in play mode; corners never (see matrix.c). */
  bool rotate = (pad < GRID_COUNT) ||
                (pad < GRID_COUNT + SIDE_COUNT && g_ui == UI_NORMAL);
  if (rotate) logical_to_phys(lr, lc, &pr, &pc);
  return (uint8_t)(pr * 10 + pc);      /* first guess; LED-walk confirms on HW */
}

void leds_init(void)
{
  memset(fb, 0, sizeof fb);
  snled_init();
}

void leds_clear(void)              { memset(fb, 0, sizeof fb); }
void leds_set_brightness(uint8_t b){ brightness = b; }

void leds_set_phys(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
  if (idx >= SNLED_PHYS_LEDS) return;
  fb[idx].r = r; fb[idx].g = g; fb[idx].b = b;
}

void leds_set_pad(pad_id_t pad, uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t idx = map_pad_to_led_index(pad);
  if (idx != 0xFF) leds_set_phys(idx, r, g, b);
}

void leds_set_pad_velocity(pad_id_t pad, uint8_t velocity)
{
  uint8_t rgb[3];
  palette_rgb8(velocity & 0x7F, rgb);      /* active palette (g_palette) */
  leds_set_pad(pad, rgb[0], rgb[1], rgb[2]);
}

void leds_render(void)
{
  snled_render(fb, brightness);
}
