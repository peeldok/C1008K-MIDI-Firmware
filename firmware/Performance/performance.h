/* Performance app for the C100 — the normal MIDI-grid behaviour.
 *
 * Active while g_ui == UI_NORMAL. Grid / side / corner keys are mapped to notes
 * through the current layout (mapping.c) and sent on MIDI channel 1; inbound
 * host note-on/off light the matching pad through the active velocity palette.
 */
#ifndef C100_PERFORMANCE_H
#define C100_PERFORMANCE_H

#include <stdint.h>
#include <stdbool.h>
#include "mapping.h"

/* One key event while the Performance app owns the panel. */
void performance_key(pad_id_t pad, bool pressed);

/* One host note-on/off while the Performance app owns the panel (drives an LED). */
void performance_midi_note(uint8_t note, uint8_t velocity, bool on);

#endif
