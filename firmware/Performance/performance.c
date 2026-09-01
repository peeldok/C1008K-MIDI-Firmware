#include "performance.h"
#include "leds.h"
#include "midi.h"

void performance_key(pad_id_t pad, bool pressed)
{
  uint8_t note = map_pad_to_note(pad);
  if (note > 127) return;                   /* corners with no note, etc. */
  if (pressed) midi_send_note_on(note, 127);
  else         midi_send_note_off(note);
}

void performance_midi_note(uint8_t note, uint8_t velocity, bool on)
{
  pad_id_t pad = map_note_to_pad(note);
  if (pad == PAD_NONE) return;
  if (on) leds_set_pad_velocity(pad, velocity);
  else    leds_set_pad(pad, 0, 0, 0);
}
