/*
 * The Drum Rack note layout follows the mapping used by
 * mat1jaczyyy/lpp-performance-cfw for Launchpad compatibility.
 * The C100 physical mapping implementation in this file is device-specific.
 * See THIRD_PARTY_LICENSES.md.
 */
#include "mapping.h"
#include "app_state.h"

/* ------------------------------------------------------------------ *
 *  Physical orientation.
 *  The C100 matrix row/col indices vs. the player's view of the grid
 *  can only be pinned down on hardware (Phase 1 LED-walk). Flip here.
 *  Default assumption:
 *    - prow 0 = TOP edge row,  prow 9 = BOTTOM edge row
 *    - pcol 0 = LEFT edge col,  pcol 9 = RIGHT edge col
 *    - grid logical (gx=0,gy=0) = bottom-left  (Ableton origin, note 36)
 * ------------------------------------------------------------------ */
#ifndef GRID_FLIP_X
#define GRID_FLIP_X 0
#endif
#ifndef GRID_FLIP_Y
#define GRID_FLIP_Y 1   /* prow grows downward, gy grows upward -> flip */
#endif

/* ---- MIDI note tables (editable) ---- */

/* Grid drum layout: bank-interleaved, two 8x4 banks, row-major, gy 0 = bottom row.
 *     gy7  64 65 66 67 | 96 97 98 99
 *     ...
 *     gy0  36 37 38 39 | 68 69 70 71
 *   left  bank (gx 0..3): notes 36..67
 *   right bank (gx 4..7): notes 68..99
 * This is exactly 36 + (grid pad id), so grid pad id == note - 36. */
static inline uint8_t grid_note(uint8_t gx, uint8_t gy) {
  return (uint8_t)(36u + grid_xy_to_pad(gx, gy));
}

/* Side keys, 32 entries, order = Top[0..7], Right[0..7], Left[0..7], Bottom[0..7]. */
static const uint8_t side_note[SIDE_COUNT] = {
  /* Top    */ 28, 29, 30, 31, 32, 33, 34, 35,
  /* Right  */ 100,101,102,103,104,105,106,107,
  /* Left   */ 108,109,110,111,112,113,114,115,
  /* Bottom */ 116,117,118,119,120,121,122,123,
};

/* Corners: TR sends CORNER_TR_NOTE, the rest send no MIDI (see mapping.h). */

enum { SIDE_TOP = 64, SIDE_RIGHT = 72, SIDE_LEFT = 80, SIDE_BOTTOM = 88 };

/* ------------------------------------------------------------------ */

void pad_to_grid_xy(uint8_t pad, uint8_t *gx, uint8_t *gy)
{
  *gy = (uint8_t)((pad >> 2) & 0x07);
  *gx = (uint8_t)((pad & 0x03) + ((pad & 0x20) ? 4 : 0));
}

/* logical grid (gx,gy) 0..7  ->  physical (prow,pcol) 1..8, applying flips */
static void grid_xy_to_phys(uint8_t gx, uint8_t gy, uint8_t *prow, uint8_t *pcol)
{
#if GRID_FLIP_X
  gx = (uint8_t)(GRID_W - 1 - gx);
#endif
#if GRID_FLIP_Y
  gy = (uint8_t)(GRID_H - 1 - gy);
#endif
  *pcol = (uint8_t)(gx + 1);
  *prow = (uint8_t)(gy + 1);
}

pad_id_t map_phys_to_pad(uint8_t prow, uint8_t pcol)
{
  if (prow >= MATRIX_ROWS || pcol >= MATRIX_COLS) return PAD_NONE;

  const bool row_edge = (prow == 0 || prow == 9);
  const bool col_edge = (pcol == 0 || pcol == 9);

  if (row_edge && col_edge) {                         /* corner */
    bool is_top  = GRID_FLIP_Y ? (prow == 0) : (prow == 9);
    bool is_left = GRID_FLIP_X ? (pcol == 9) : (pcol == 0);
    if (is_top)  return is_left ? PAD_CORNER_TL : PAD_CORNER_TR;
    return           is_left ? PAD_CORNER_BL : PAD_CORNER_BR;
  }

  if (!row_edge && !col_edge) {                       /* centre 8x8 */
    uint8_t gx = (uint8_t)(pcol - 1);
    uint8_t gy = (uint8_t)(prow - 1);
#if GRID_FLIP_X
    gx = (uint8_t)(GRID_W - 1 - gx);
#endif
#if GRID_FLIP_Y
    gy = (uint8_t)(GRID_H - 1 - gy);
#endif
    return grid_xy_to_pad(gx, gy);
  }

  /* edge (side key) */
  uint8_t i = (uint8_t)(row_edge ? (pcol - 1) : (prow - 1));   /* 0..7 */
  if (row_edge) return (uint8_t)((prow == 0 ? SIDE_TOP : SIDE_BOTTOM) + i);
  return (uint8_t)((pcol == 9 ? SIDE_RIGHT : SIDE_LEFT) + i);
}

/* logical pad -> physical (prow,pcol) 0..9. Returns false if pad is invalid. */
bool map_pad_to_phys(pad_id_t pad, uint8_t *prow, uint8_t *pcol)
{
  if (pad < GRID_COUNT) {
    uint8_t gx, gy;
    pad_to_grid_xy(pad, &gx, &gy);
    grid_xy_to_phys(gx, gy, prow, pcol);
    return true;
  }
  if (pad < GRID_COUNT + SIDE_COUNT) {
    uint8_t s = pad - GRID_COUNT, edge = s >> 3, i = s & 7;   /* 0..7 */
    switch (edge) {
      case 0: *prow = 0; *pcol = i + 1; break;   /* Top    */
      case 1: *pcol = 9; *prow = i + 1; break;   /* Right  */
      case 2: *pcol = 0; *prow = i + 1; break;   /* Left   */
      default:*prow = 9; *pcol = i + 1; break;   /* Bottom */
    }
    return true;
  }
  if (pad < PAD_COUNT) {                                        /* corner */
    /* place corner diagonally outside its logical-neighbour grid cell */
    uint8_t gx = (pad == PAD_CORNER_TL || pad == PAD_CORNER_BL) ? 0 : 7;
    uint8_t gy = (pad == PAD_CORNER_TL || pad == PAD_CORNER_TR) ? 7 : 0;
    uint8_t r, c;
    grid_xy_to_phys(gx, gy, &r, &c);
    *prow = (r == 1) ? 0 : 9;
    *pcol = (c == 1) ? 0 : 9;
    return true;
  }
  return false;
}

/* ---- drum layout ---- */
static uint8_t pad_note_drum(pad_id_t pad)
{
  if (pad < GRID_COUNT) {
    uint8_t gx, gy;
    pad_to_grid_xy(pad, &gx, &gy);
    return grid_note(gx, gy);
  }
  if (pad < GRID_COUNT + SIDE_COUNT) return side_note[pad - GRID_COUNT];
  if (pad == PAD_CORNER_TR) return CORNER_TR_NOTE;   /* only TR corner sends MIDI */
  return 0xFF;                                       /* TL / BL / BR corners: no MIDI */
}

static pad_id_t note_pad_drum(uint8_t note)
{
  if (note >= 36 && note <= 99) return (pad_id_t)(note - 36);   /* grid pad == note - 36 */
  if (note == CORNER_TR_NOTE) return PAD_CORNER_TR;
  for (uint8_t i = 0; i < SIDE_COUNT; i++)
    if (side_note[i] == note) return (uint8_t)(GRID_COUNT + i);
  return PAD_NONE;
}

/* ---- programmer layout: a pad's "note" is its Launchpad-Pro position ---- */
uint8_t pad_to_lppro_pos(pad_id_t pad)
{
  if (pad < GRID_COUNT) {
    uint8_t gx, gy;
    pad_to_grid_xy(pad, &gx, &gy);
    return (uint8_t)((gy + 1) * 10 + (gx + 1));           /* 11..88 */
  }
  if (pad < GRID_COUNT + SIDE_COUNT) {
    uint8_t s = pad - GRID_COUNT, edge = s >> 3, i = s & 7;
    switch (edge) {
      case 0:  return (uint8_t)(91 + i);                  /* Top    -> 91..98 */
      case 1:  return (uint8_t)((8 - i) * 10 + 9);        /* Right  -> 89..19 */
      case 2:  return (uint8_t)((8 - i) * 10);            /* Left   -> 80..10 */
      default: return (uint8_t)(1 + i);                   /* Bottom -> 1..8   */
    }
  }
  if (pad == PAD_CORNER_TR) return 99;                    /* mode light */
  return 0xFF;                                            /* TL / BL / BR: no MIDI */
}

uint8_t map_pad_to_note(pad_id_t pad)
{
  return (g_layout == LAYOUT_PROGRAMMER) ? pad_to_lppro_pos(pad) : pad_note_drum(pad);
}

pad_id_t map_note_to_pad(uint8_t note)
{
  return (g_layout == LAYOUT_PROGRAMMER) ? lppro_pos_to_pad(note) : note_pad_drum(note);
}

pad_id_t lppro_pos_to_pad(uint8_t pos)
{
  if (pos == 99) return PAD_CORNER_TR;          /* LP Pro mode light -> TR corner */

  uint8_t r = (uint8_t)(pos / 10);
  uint8_t c = (uint8_t)(pos % 10);

  if (r >= 1 && r <= 8 && c >= 1 && c <= 8)     /* 8x8 grid */
    return grid_xy_to_pad((uint8_t)(c - 1), (uint8_t)(r - 1));

  /* LP Pro rows count upward (r=1 bottom .. r=8 top). Our side-key indices for
   * the vertical edges count top-down (index 0 = prow 1 = top), so invert r
   * for the left/right edges — without this the left/right sides come out
   * vertically mirrored under SysEx LED control. Top/bottom edges are
   * horizontal, no inversion. */
  if (r == 0 && c >= 1 && c <= 8) return (uint8_t)(SIDE_BOTTOM + (c - 1));
  if (r == 9 && c >= 1 && c <= 8) return (uint8_t)(SIDE_TOP    + (c - 1));
  if (c == 0 && r >= 1 && r <= 8) return (uint8_t)(SIDE_LEFT   + (8 - r));
  if (c == 9 && r >= 1 && r <= 8) return (uint8_t)(SIDE_RIGHT  + (8 - r));

  return PAD_NONE;   /* 0, 9, 90 (unused corners) and anything out of range */
}

/* ---- panel rotation (whole 10x10, clockwise quarter-turns) ---- */

/* Quarter-turns are clockwise from the player's view. The grid coordinate system
 * has gy pointing up while prow points down, so a clockwise turn for the player
 * is a counter-clockwise turn in (prow,pcol) space — hence the 90/270 cases look
 * "backwards" here. */
void logical_to_phys(uint8_t lr, uint8_t lc, uint8_t *pr, uint8_t *pc)
{
  switch (g_rotation & 3u) {
    default: *pr = lr;                *pc = lc;                break;  /*   0 */
    case 1:  *pr = (uint8_t)(9 - lc); *pc = lr;                break;  /*  90 CW */
    case 2:  *pr = (uint8_t)(9 - lr); *pc = (uint8_t)(9 - lc); break;  /* 180 */
    case 3:  *pr = lc;                *pc = (uint8_t)(9 - lr); break;  /* 270 CW */
  }
}

void phys_to_logical(uint8_t pr, uint8_t pc, uint8_t *lr, uint8_t *lc)
{
  switch (g_rotation & 3u) {
    default: *lr = pr;                *lc = pc;                break;
    case 1:  *lr = pc;                *lc = (uint8_t)(9 - pr); break;
    case 2:  *lr = (uint8_t)(9 - pr); *lc = (uint8_t)(9 - pc); break;
    case 3:  *lr = (uint8_t)(9 - pc); *lc = pr;                break;
  }
}

pad_id_t map_border_to_adjacent_grid(pad_id_t border_pad)
{
  if (border_pad < GRID_COUNT || border_pad >= PAD_COUNT) return PAD_NONE;

  if (border_pad < GRID_COUNT + SIDE_COUNT) {          /* side */
    uint8_t s = border_pad - GRID_COUNT, edge = s >> 3, i = s & 7;
    uint8_t prow, pcol;
    switch (edge) {
      case 0: prow = 1; pcol = i + 1; break;
      case 1: prow = i + 1; pcol = 8; break;
      case 2: prow = i + 1; pcol = 1; break;
      default:prow = 8; pcol = i + 1; break;
    }
    return map_phys_to_pad(prow, pcol);
  }
  /* corner -> its diagonal grid cell, in logical grid space */
  uint8_t gx = (border_pad == PAD_CORNER_TL || border_pad == PAD_CORNER_BL) ? 0 : 7;
  uint8_t gy = (border_pad == PAD_CORNER_TL || border_pad == PAD_CORNER_TR) ? 7 : 0;
  return grid_xy_to_pad(gx, gy);
}
