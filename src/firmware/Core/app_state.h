/* Shared C100 app state: current pad layout + UI state.
 * Defined in main.c; read by mapping.c (layout-aware note mapping) and
 * sysex.c (suppress host LED writes while a UI screen owns the panel). */
#ifndef C100_APP_STATE_H
#define C100_APP_STATE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  LAYOUT_DRUM       = 0,   /* grid notes 36..99, side_note table, TR corner = note 27 */
  LAYOUT_PROGRAMMER = 1,   /* Launchpad-Pro positions: grid 11..88, edges, TR = 99    */
} layout_t;

typedef enum {
  UI_NORMAL     = 0,       /* play mode — host + local drive the LEDs */
  UI_SETUP,                /* setup screen: layout / rotation / app / brightness entry */
  UI_BRIGHTNESS,           /* brightness screen: "LED" label + 8-step bar */
} ui_state_t;

#define APP_PERFORMANCE 0  /* the only app for now — current normal behaviour */

extern volatile layout_t   g_layout;   /* default LAYOUT_DRUM */
extern volatile ui_state_t g_ui;       /* default UI_NORMAL   */
extern volatile uint8_t    g_rotation; /* 0..3 quarter-turns clockwise (whole 10x10) */
extern volatile uint8_t    g_app;      /* app id; default APP_PERFORMANCE */
extern volatile uint8_t    g_palette;  /* active velocity->RGB palette 0..5; default 0 (Original) */

/* true while a setup/brightness screen owns the panel (host LED writes ignored) */
static inline bool ui_owns_panel(void) { return g_ui != UI_NORMAL; }

#endif
