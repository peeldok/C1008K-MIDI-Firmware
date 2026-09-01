#include "setup.h"
#include "app_state.h"
#include "leds.h"
#include "palette.h"
#include "nvs.h"
#include "board.h"          /* board_enter_rom_dfu */

/* ---- persisted state owned here ---- */
static uint8_t s_bright_step = 7;    /* 8-step ladder index; default = full */
static bool    s_dirty       = false; /* a setting changed since the last save */
static uint8_t s_preview_idx = 1;    /* L-cell palette preview cursor (0..127) */

/* 8-step brightness ladder (spec: min 25, max 255, +33/step). */
static const uint8_t BRIGHT_STEP[8] = { 25, 58, 91, 124, 157, 190, 223, 255 };

uint8_t setup_brightness(void) { return BRIGHT_STEP[s_bright_step]; }

/* ---- setup-screen cell map (logical grid pad ids) ----
 * The grid still rotates in the setup UI (these cells move as a live preview of
 * g_rotation); only the side rotation selector and the corners are pinned. */
#define SETUP_PAD_DRUM        0     /* grid (0,0) */
#define SETUP_PAD_PROGRAMMER  1     /* grid (1,0) */
#define APP_PAD_PERFORMANCE   28    /* grid (0,7) */
#define PAL_PREVIEW_PAD       32    /* grid (4,0) — the "L" cell */

static bool is_centre4(pad_id_t p)   /* grid (3,3)(4,3)(3,4)(4,4) */
{
  return p == grid_xy_to_pad(3, 3) || p == grid_xy_to_pad(4, 3)
      || p == grid_xy_to_pad(3, 4) || p == grid_xy_to_pad(4, 4);
}

static bool is_app_cell(pad_id_t p)  /* top-left 3x3: gx 0..2, gy 5..7 */
{
  if (p >= GRID_COUNT) return false;
  uint8_t gx, gy;
  pad_to_grid_xy(p, &gx, &gy);
  return gx < 3 && gy >= 5;
}

/* Palette selector cells: O/S/M on gy1 (gx5..7), C1/C2/C3 on gy0 (gx5..7).
 * Returns palette id 0..5, or -1 for none / the L preview cell. */
static int palette_for_cell(pad_id_t p)
{
  if (p >= GRID_COUNT) return -1;
  uint8_t gx, gy;
  pad_to_grid_xy(p, &gx, &gy);
  if (gx >= 5 && gx <= 7 && gy == 1) return gx - 5;         /* O S M    -> 0 1 2 */
  if (gx >= 5 && gx <= 7 && gy == 0) return gx - 5 + 3;     /* C1 C2 C3 -> 3 4 5 */
  return -1;
}

/* Side pad (64..95) -> rotation: Top=0, Right=270(3), Left=90(1), Bottom=180(2). */
static uint8_t side_rotation(pad_id_t p)
{
  static const uint8_t by_edge[4] = { 0, 3, 1, 2 };   /* Top, Right, Left, Bottom */
  return by_edge[(p - GRID_COUNT) >> 3];
}

/* Advance the preview cursor to the next non-(0,0,0) entry of the active palette. */
static void preview_advance(void)
{
  for (int n = 0; n < 128; n++) {
    s_preview_idx = (uint8_t)((s_preview_idx + 1) & 0x7F);
    uint8_t rgb[3];
    palette_rgb8(s_preview_idx, rgb);
    if (rgb[0] | rgb[1] | rgb[2]) return;
  }
}

/* ---- persistence ---- */
void setup_load_settings(void)
{
  nvs_settings_t s;
  if (nvs_settings_load(&s)) {                 /* blank/stale -> keep defaults */
    g_layout      = (s.layout == LAYOUT_PROGRAMMER) ? LAYOUT_PROGRAMMER : LAYOUT_DRUM;
    s_bright_step = (s.brightness_step < 8) ? s.brightness_step : 7;
    g_rotation    = s.rotation & 3u;
    g_app         = s.app;
    g_palette     = (s.palette < PALETTE_COUNT) ? s.palette : 0;
  }

  static nvs_palettes_t p;              /* ~1.1 KB — keep it off the task stack */
  if (nvs_palettes_load(&p))
    palette_load_custom(&p.custom6[0][0], p.valid);
}

void setup_save_settings(void)
{
  if (!s_dirty) return;
  nvs_settings_t s = { .layout = (uint8_t)g_layout, .brightness_step = s_bright_step,
                       .rotation = g_rotation, .app = g_app, .palette = g_palette };
  nvs_settings_save(&s);
  s_dirty = false;
}

/* ---- key handling ---- */
void setup_key(pad_id_t pad, bool pressed)
{
  if (!pressed) return;

  if (g_ui == UI_BRIGHTNESS) {
    if (pad < GRID_COUNT) {
      uint8_t gx, gy;
      pad_to_grid_xy(pad, &gx, &gy);
      if (gy == 0) {                            /* bottom row = the 8 steps */
        s_bright_step = gx;
        leds_set_brightness(BRIGHT_STEP[s_bright_step]);
        s_dirty = true;
      }
    }
    return;
  }

  /* UI_SETUP */
  if (pad == PAD_CORNER_BR) {                   /* jump to the ROM bootloader */
    leds_clear();                                /* blank the panel — the ROM */
    leds_render();                               /* bootloader never drives it */
    setup_save_settings();
    board_enter_rom_dfu();                        /* noreturn: magic + system reset */
  } else if (pad == SETUP_PAD_DRUM || pad == SETUP_PAD_PROGRAMMER) {
    g_layout = (pad == SETUP_PAD_PROGRAMMER) ? LAYOUT_PROGRAMMER : LAYOUT_DRUM;
    s_dirty = true;
  } else if (is_centre4(pad)) {
    g_ui = UI_BRIGHTNESS;
    leds_clear();
  } else if (pad >= GRID_COUNT && pad < GRID_COUNT + SIDE_COUNT) {
    g_rotation = side_rotation(pad);
    s_dirty = true;
  } else if (is_app_cell(pad)) {
    if (pad == APP_PAD_PERFORMANCE) { g_app = APP_PERFORMANCE; s_dirty = true; }
    /* the other 8 cells are empty app slots for now */
  } else {
    int pal = palette_for_cell(pad);
    if (pal >= 0) {
      g_palette = (uint8_t)pal;
      s_dirty = true;
    }
    /* PAL_PREVIEW_PAD (the "L" cell) is display-only — no action */
  }
}

/* ---- screen rendering (panel is cleared by the caller before these run) ---- */
static void draw_setup_screen(void)
{
  bool prog = (g_layout == LAYOUT_PROGRAMMER);

  /* bottom-left: layout selector — chosen one bright, other dim */
  leds_set_pad(SETUP_PAD_DRUM,       0, prog ? 12 : 100, 0);               /* green  */
  leds_set_pad(SETUP_PAD_PROGRAMMER, prog ? 110 : 16, prog ? 40 : 6, 0);   /* orange */

  /* centre 4: brightness-screen entry — bright white */
  uint8_t c[4] = { grid_xy_to_pad(3, 3), grid_xy_to_pad(4, 3),
                   grid_xy_to_pad(3, 4), grid_xy_to_pad(4, 4) };
  for (int i = 0; i < 4; i++) leds_set_pad(c[i], 150, 150, 150);

  /* 32 side keys: rotation selector — always all green, fixed physical position
   * (Top=0 / Left=90 / Bottom=180 / Right=270). No selected-state highlight. */
  for (pad_id_t p = GRID_COUNT; p < GRID_COUNT + SIDE_COUNT; p++)
    leds_set_pad(p, 0, 90, 0);

  /* bottom-right corner: jump-to-bootloader (blue) */
  leds_set_pad(PAD_CORNER_BR, 0, 0, 120);

  /* top-left 3x3: app selector. (0,7) = Performance (red); empty slots faint. */
  for (uint8_t gy = 5; gy <= 7; gy++)
    for (uint8_t gx = 0; gx < 3; gx++) {
      pad_id_t p = grid_xy_to_pad(gx, gy);
      if (p == APP_PAD_PERFORMANCE)
        leds_set_pad(p, (g_app == APP_PERFORMANCE) ? 130 : 30, 0, 0);
      else
        leds_set_pad(p, 4, 4, 4);
    }

  /* palette selectors:
   *   gy1 O/S/M (built-in)  -> selected bright white, else dim white
   *   gy0 C1/C2/C3 (custom) -> selected bright yellow, else pale (pastel) yellow */
  for (uint8_t gx = 5; gx <= 7; gx++) {
    bool sel_top = (g_palette == (uint8_t)(gx - 5));
    bool sel_bot = (g_palette == (uint8_t)(gx - 5 + 3));
    leds_set_pad(grid_xy_to_pad(gx, 1), sel_top ? 150 : 16,
                                        sel_top ? 150 : 16,
                                        sel_top ? 150 : 16);
    leds_set_pad(grid_xy_to_pad(gx, 0), sel_bot ? 150 : 44,
                                        sel_bot ? 140 : 40,
                                        sel_bot ? 20  : 18);
  }

  /* L cell (4,0): live one-pad preview of the active palette at s_preview_idx. */
  uint8_t rgb[3];
  palette_rgb8(s_preview_idx, rgb);
  leds_set_pad(PAL_PREVIEW_PAD, rgb[0], rgb[1], rgb[2]);
}

static void draw_brightness_screen(void)
{
  /* "LED" across the top 4 rows (gy7..gy4). L/D green, E white. */
  static const char *const rows[4] = {   /* [0] = gy7 (top) .. [3] = gy4 */
    "L.EEEDD.",
    "L.EE.D.D",
    "L.E..D.D",
    "LLEEEDD.",
  };
  for (int ri = 0; ri < 4; ri++) {
    uint8_t gy = (uint8_t)(7 - ri);
    for (uint8_t gx = 0; gx < 8; gx++) {
      char ch = rows[ri][gx];
      if (ch == 'L' || ch == 'D') leds_set_pad(grid_xy_to_pad(gx, gy), 0, 110, 0);
      else if (ch == 'E')         leds_set_pad(grid_xy_to_pad(gx, gy), 95, 95, 95);
    }
  }
  /* bottom row = 8-step brightness bar: all lit, current step brightest */
  for (uint8_t gx = 0; gx < 8; gx++) {
    uint8_t v = (gx == s_bright_step) ? 140 : 14;
    leds_set_pad(grid_xy_to_pad(gx, 0), v, v, v);
  }
}

void setup_render(void)
{
  if (g_ui == UI_SETUP) {
    static uint8_t pv_div;                  /* ~100 ms per preview colour (render ~3 ms) */
    if (++pv_div >= 33) { pv_div = 0; preview_advance(); }
    draw_setup_screen();
  } else {
    draw_brightness_screen();
  }
}
