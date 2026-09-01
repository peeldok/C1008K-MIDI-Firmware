/* USB-MIDI (channel 1) for the C100 grid. */
#ifndef C100_MIDI_H
#define C100_MIDI_H

#include <stdint.h>
#include <stdbool.h>

/* host -> device: note on/off (drives the LEDs in the drum layout). */
typedef void (*midi_note_cb)(uint8_t note, uint8_t velocity, bool on);

void midi_init(midi_note_cb cb);

void midi_send_note_on(uint8_t note, uint8_t velocity);
void midi_send_note_off(uint8_t note);

/* send a raw SysEx message (must include leading 0xF0 and trailing 0xF7). */
void midi_send_sysex(const uint8_t *data, uint16_t len);

/* drain the USB-MIDI RX FIFO: dispatch note on/off and reassemble + handle
 * SysEx (see sysex.c). Call from a task. */
void midi_poll(void);

#endif
