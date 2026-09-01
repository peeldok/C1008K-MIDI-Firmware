/* Coordinate & note mapping for the C100 10x10 -> 8x8 grid + 32 side keys.
 *
 * Physical matrix: 10 rows x 10 cols (prow, pcol in 0..9).
 * Grid    = centre 8x8  -> prow,pcol in 1..8  -> gx = pcol-1, gy = prow-1 (0..7)
 * Side    = perimeter minus corners (prow or pcol in {0,9}, not both) -> 0..31
 * Corners = unused.
 *
 * C100 grid pad number (bank-interleaved: two 8x4 banks; used by the boot
 * animation and the drum layout):
 *   pad(gx,gy) = (gy<<2) | (gx & 3) | (gx >= 4 ? 32 : 0)
 *
 * All note tables live in mapping.c and are trivially editable.
 */
#ifndef C100_MAPPING_H
#define C100_MAPPING_H

#include <stdint.h>
#include <stdbool.h>

#define MATRIX_ROWS      10
#define MATRIX_COLS      10
#define GRID_W           8
#define GRID_H           8
#define GRID_COUNT       64
#define SIDE_COUNT       32
#define CORNER_COUNT     4
#define PAD_COUNT        (GRID_COUNT + SIDE_COUNT + CORNER_COUNT)   /* 100 physical */

#define MIDI_CHANNEL     0                            /* 0 == channel 1 */

/* Logical pad id:
 *   0..63  grid (C100 pad numbering, bank-interleaved)
 *   64..95 side keys  (Top 64-71, Right 72-79, Left 80-87, Bottom 88-95)
 *   96..99 corners    (TL 96, TR 97, BL 98, BR 99)
 */
typedef uint8_t pad_id_t;
#define PAD_NONE   0xFF
#define PAD_CORNER_TL 96
#define PAD_CORNER_TR 97
#define PAD_CORNER_BL 98
#define PAD_CORNER_BR 99

/* Corner MIDI:
 *   TL  -> no MIDI; firmware-local "enter setup UI" button (handled in main.c)
 *   TR  -> dedicated control note = D#0 in the C1=36 / "C-2 = 0" naming
 *          (Ableton/Cubase): raw MIDI note number 27.
 *   BL/BR -> no MIDI
 */
#define CORNER_TR_NOTE   27

/* physical (prow,pcol) -> logical pad id, or PAD_NONE if out of range */
pad_id_t map_phys_to_pad(uint8_t prow, uint8_t pcol);

/* logical pad -> physical (prow,pcol) 0..9; false if pad invalid */
bool map_pad_to_phys(pad_id_t pad, uint8_t *prow, uint8_t *pcol);

/* logical pad id <-> MIDI note, honouring the active layout (g_layout):
 *   drum       - grid note = 36 + pad, side_note table, TR corner = CORNER_TR_NOTE
 *   programmer - the pad's Launchpad-Pro position (grid 11..88, edges, TR = 99)
 * 0xFF / PAD_NONE means "no MIDI for this pad" / "no pad for this note". */
uint8_t  map_pad_to_note(pad_id_t pad);
pad_id_t map_note_to_pad(uint8_t note);

/* pad id -> Launchpad-Pro position (0..99), the inverse of lppro_pos_to_pad().
 * 0xFF for TL/BL/BR corners. */
uint8_t  pad_to_lppro_pos(pad_id_t pad);

/* Panel rotation (g_rotation, 0..3 quarter-turns clockwise) applied to the whole
 * 10x10 (prow,pcol 0..9). Used at the hardware boundaries only: matrix.c maps a
 * raw scan coordinate back to logical with phys_to_logical(); leds.c maps a
 * logical coordinate out to the physical LED with logical_to_phys(). */
void phys_to_logical(uint8_t praw, uint8_t craw, uint8_t *lrow, uint8_t *lcol);
void logical_to_phys(uint8_t lrow, uint8_t lcol, uint8_t *praw, uint8_t *craw);

/* logical grid pad id (0..63) -> physical LED buffer index (0..99), or 0xFF.
 * Uses the stock-firmware LED map; see leds.c. */
uint8_t  map_pad_to_led_index(pad_id_t pad);

/* For a border pad (side 64..95 or corner 96..99): the grid pad physically just
 * inside it. Used to bleed the boot animation from the 8x8 grid onto the border. */
pad_id_t map_border_to_adjacent_grid(pad_id_t border_pad);

/* C100 grid pad-id helpers (bank-interleaved numbering, grid only) */
static inline uint8_t grid_xy_to_pad(uint8_t gx, uint8_t gy) {
  return (uint8_t)((gy << 2) | (gx & 3) | (gx >= 4 ? 32 : 0));
}
void     pad_to_grid_xy(uint8_t pad, uint8_t *gx, uint8_t *gy);

/* Launchpad Pro "position" (pos = row*10 + col, 0..99) -> our pad id, or
 * PAD_NONE. Used by the CFY SysEx LED path (see sysex.c).
 *   grid   row,col 1..8  -> 0..63
 *   edges  row or col 0/9 -> side keys 64..95
 *   99     LP Pro mode light -> TR corner
 *   0 / 9 / 90 (unused corners) and anything else -> PAD_NONE (not driven)
 */
pad_id_t lppro_pos_to_pad(uint8_t pos);

#endif
