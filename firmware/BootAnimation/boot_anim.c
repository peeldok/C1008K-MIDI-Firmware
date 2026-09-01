#include "boot_anim.h"
#include "boot_animation_data.h"
#include "leds.h"
#include "mapping.h"
#include "palette.h"
#include "FreeRTOS.h"
#include "task.h"

/* The boot animation was authored against the Original palette — keep it fixed
 * so a user-selected play palette can't distort it. */
static void anim_led(pad_id_t pad, uint8_t vel)
{
  uint8_t rgb[3];
  palette_rgb8_of(0, vel, rgb);
  leds_set_pad(pad, rgb[0], rgb[1], rgb[2]);
}

/* The supplied stream is an 8x8 animation (grid only). We render it on the
 * centre 8x8 and *extend it to the full 10x10* by bleeding each grid edge cell
 * outward onto the adjacent border LED, so the diagonal sweeps and the centred
 * diamond pulse flow across the whole panel. */

/* 90-degree pad rotation, ported verbatim from the stock boot_animation_map_key(). */
static uint8_t map_key(uint8_t key)
{
  if (key >= 64) return key;
  uint8_t row = (uint8_t)((key >> 2) & 0x07);
  uint8_t col = (uint8_t)((key & 0x03) + (key >= 32 ? 4 : 0));
  uint8_t mrow = col;
  uint8_t mcol = (uint8_t)(7 - row);
  return (uint8_t)((mrow << 2) + (mcol & 0x03) + (mcol >= 4 ? 32 : 0));
}

static uint8_t grid_vel[GRID_COUNT];   /* current velocity per grid pad (C100 grid numbering) */

static void render_frame(void)
{
  for (uint8_t p = 0; p < GRID_COUNT; p++)
    anim_led(p, grid_vel[p]);

  for (uint8_t b = GRID_COUNT; b < PAD_COUNT; b++) {   /* 32 sides + 4 corners */
    pad_id_t inner = map_border_to_adjacent_grid(b);
    anim_led(b, (inner != PAD_NONE) ? grid_vel[inner] : 0);
  }
  leds_render();
}

/* Playback speed. The supplied frame delays felt too fast on hardware
 * (partly because a per-ms vTaskDelay(1) loop under-waits). We now wait the
 * whole frame delay in one call and stretch it by BOOT_ANIM_TIME_NUM/DEN. */
#ifndef BOOT_ANIM_TIME_NUM
#define BOOT_ANIM_TIME_NUM 3
#endif
#ifndef BOOT_ANIM_TIME_DEN
#define BOOT_ANIM_TIME_DEN 2      /* 1.5x slower than the raw data (tune to taste) */
#endif

static void wait_ms(uint16_t ms, void (*tick)(void))
{
  uint32_t scaled = ((uint32_t)ms * BOOT_ANIM_TIME_NUM) / BOOT_ANIM_TIME_DEN;
  if (scaled == 0) scaled = 1;
  /* break the wait into <=100 ms chunks so `tick` (watchdog kick) runs often */
  while (scaled) {
    uint32_t chunk = scaled > 100 ? 100 : scaled;
    if (tick) tick();
    vTaskDelay(pdMS_TO_TICKS(chunk));
    scaled -= chunk;
  }
}

void boot_anim_play(void (*tick)(void))
{
  uint16_t pos = 0;
  const uint16_t size = BOOT_ANIM_DATA_LEN;

  for (uint8_t i = 0; i < GRID_COUNT; i++) grid_vel[i] = 0;
  leds_clear();
  leds_render();

  while ((uint32_t)pos + 3u <= size) {
    uint16_t delay_ms = (uint16_t)BOOT_ANIM_DATA[pos]
                      | (uint16_t)((uint16_t)BOOT_ANIM_DATA[pos + 1] << 8);
    uint8_t count = BOOT_ANIM_DATA[pos + 2];
    pos += 3;

    wait_ms(delay_ms, tick);

    for (uint8_t i = 0; i < count; i++) {
      if ((uint32_t)pos + 2u > size) break;
      uint8_t key = BOOT_ANIM_DATA[pos++];
      uint8_t vel = BOOT_ANIM_DATA[pos++];
      if (key < GRID_COUNT) grid_vel[map_key(key)] = vel;
    }
    render_frame();
    if (tick) tick();
  }

  leds_clear();
  leds_render();
}
