/* SysEx handling for the C100.
 *
 * The C100 presents to the host as a Launchpad Pro running the "CFY" performance
 * custom firmware (mat1jaczyyy/lpp-performance-cfw). Apollo Studio recognises
 * that identity from the Universal Device Inquiry reply, then drives the LEDs
 * with its compressed 0x5F SysEx (falling back to the plain 0x6F form).
 */
#ifndef C100_SYSEX_H
#define C100_SYSEX_H

#include <stdint.h>

/* Largest inbound SysEx we reassemble (spec: 512, not the LP Pro 320). */
#define SYSEX_MAX 512

/* Handle one complete inbound SysEx message (includes leading 0xF0; the
 * trailing 0xF7 is included when it was received). Called from midi_poll(). */
void sysex_dispatch(const uint8_t *msg, uint16_t len);

#endif
