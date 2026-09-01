/*
 * Third-party provenance:
 * CFY 0x5F/0x6F LED protocol behavior is adapted from
 * mat1jaczyyy/lpp-performance-cfw, which is based on the Novation
 * Launchpad Pro open-source firmware from Focusrite Audio Engineering Ltd.
 * Those upstream materials are BSD-3-Clause licensed.
 * See THIRD_PARTY_LICENSES.md and licenses/FOCUSRITE-LAUNCHPAD-BSD-3-CLAUSE.txt.
 */
#include "sysex.h"
#include "midi.h"
#include "leds.h"
#include "mapping.h"
#include "palette.h"
#include "app_state.h"      /* ui_owns_panel() */
#include "board.h"          /* board_enter_rom_dfu */
#include <string.h>

/* ---- identity ----
 * Universal Device Inquiry reply for "Launchpad Pro + CFY custom firmware".
 * Byte layout Apollo Studio parses (mat1jaczyyy/apollo-studio Launchpad.cs):
 *   F0 7E <id> 06 02  00 20 29        Novation
 *                     51              Launchpad Pro
 *                     00 00 00 00
 *                     63 66 79        "cfy"  -> treated as CFW
 *   F7
 */
static const uint8_t k_inquiry_reply[] = {
  0xF0, 0x7E, 0x00, 0x06, 0x02,
  0x00, 0x20, 0x29,
  0x51,
  0x00, 0x00, 0x00, 0x00,
  0x63, 0x66, 0x79,
  0xF7,
};

/* 6-bit Launchpad colour (0..63) -> 8-bit, full-range. */
static inline uint8_t c6_to_c8(uint8_t v)
{
  v &= 0x3F;
  return (uint8_t)((v << 2) | (v >> 4));
}

static void put(uint8_t pos, uint8_t r6, uint8_t g6, uint8_t b6)
{
  pad_id_t pad = lppro_pos_to_pad(pos);
  if (pad == PAD_NONE) return;          /* unused corners / out of range: never driven */
  leds_set_pad(pad, c6_to_c8(r6), c6_to_c8(g6), c6_to_c8(b6));
}

/* CFY position semantics (shared by 0x5F):
 *   0        -> whole addressable panel (1..98); unused corners already filtered
 *   1..99    -> single position (99 = the LP Pro "mode light" = our TR corner)
 *   100..109 -> a full row of 8
 *   110..119 -> a full column of 8
 */
static void put_range(uint8_t x, uint8_t r6, uint8_t g6, uint8_t b6)
{
  if (x == 0) {
    for (uint8_t k = 1; k < 99; k++) put(k, r6, g6, b6);
  } else if (x <= 99) {
    put(x, r6, g6, b6);
  } else if (x <= 109) {
    uint8_t base = (uint8_t)((x - 100) * 10 + 1);
    for (uint8_t k = base; k < (uint8_t)(base + 8); k++) put(k, r6, g6, b6);
  } else if (x <= 119) {
    uint8_t base = (uint8_t)(x - 100);
    for (uint8_t k = base; k < 90; k = (uint8_t)(k + 10)) put(k, r6, g6, b6);
  }
}

/* ---- C100 web-editor protocol ----
 * Wire form:  F0 7D 43 31 30 30 <cmd> <payload..> F7          ("C100")
 *   0x01 DISCOVER       {token}
 *        -> 0x02 REPLY  {token, proto, fw_maj, fw_min, fw_patch, caps}
 *   0x10 PAL_UPLOAD     {slot(0..2), comp(0..2), v0..v127 (6-bit)}
 *        -> 0x11 ACK    {slot, comp, status}
 *   0x12 PAL_DOWNLOAD   {slot, comp}
 *        -> 0x13 DATA   {slot, comp, v0..v127}
 *   0x14 PAL_COMMIT     {}                       (write custom palettes to flash)
 *        -> 0x11 ACK    {0x7F, 0x7F, status}
 *   0x20 ENTER_BOOTLOADER {}                     (noreturn)
 */
#define WEB_PROTO_VER  1
#define WEB_CAP        0x03   /* bit0 = 6-bit palette, bit1 = 3 custom slots */

static void web_send(uint8_t cmd, const uint8_t *payload, uint16_t plen)
{
  static uint8_t msg[8 + 2 + 128];             /* largest payload: PAL_DATA {slot,comp,128} */
  msg[0] = 0xF0; msg[1] = 0x7D; msg[2] = 0x43; msg[3] = 0x31; msg[4] = 0x30; msg[5] = 0x30;
  msg[6] = cmd;
  if (plen > sizeof(msg) - 8) plen = sizeof(msg) - 8;
  if (payload && plen) memcpy(&msg[7], payload, plen);
  msg[7 + plen] = 0xF7;
  midi_send_sysex(msg, (uint16_t)(8 + plen));
}

static void web_ack(uint8_t a, uint8_t b, uint8_t status)
{
  uint8_t p[3] = { a, b, status };
  web_send(0x11, p, 3);
}

static void sysex_c100_web(const uint8_t *p, uint16_t plen)
{
  if (plen < 1) return;
  uint8_t cmd = p[0];

  switch (cmd) {
    case 0x01: {                                     /* DISCOVER */
      uint8_t token = (plen >= 2) ? p[1] : 0;
      uint8_t r[6] = { token, WEB_PROTO_VER, 1, 0, 0, WEB_CAP };
      web_send(0x02, r, 6);
      break;
    }
    case 0x10: {                                     /* PALETTE_UPLOAD */
      if (plen < 3 + 128) { web_ack(0x7F, 0x7F, 1); break; }
      uint8_t slot = p[1], comp = p[2];
      if (slot > 2 || comp > 2) { web_ack(slot, comp, 1); break; }
      palette_set_custom_component(slot, comp, &p[3]);
      web_ack(slot, comp, 0);
      break;
    }
    case 0x12: {                                     /* PALETTE_DOWNLOAD */
      if (plen < 3) break;
      uint8_t slot = p[1], comp = p[2];
      if (slot > 2 || comp > 2) break;
      uint8_t r[2 + 128];
      r[0] = slot; r[1] = comp;
      palette_get_custom_component(slot, comp, &r[2]);
      web_send(0x13, r, 2 + 128);
      break;
    }
    case 0x14:                                       /* PALETTE_COMMIT */
      palette_commit_to_flash();
      web_ack(0x7F, 0x7F, 0);
      break;
    case 0x20:                                       /* ENTER_BOOTLOADER */
      board_enter_rom_dfu();                          /* noreturn */
      break;
    default:
      break;
  }
}

void sysex_dispatch(const uint8_t *m, uint16_t n)
{
  if (n < 4 || m[0] != 0xF0) return;

  const uint8_t *d   = m + 1;                 /* first byte after F0 */
  const uint8_t *end = m + n;                 /* one past the last byte */
  if (end[-1] == 0xF7) end--;                 /* stop before the terminator */

  /* Universal Device Inquiry:  F0 7E <id> 06 01 F7   (always answered) */
  if (n >= 6 && d[0] == 0x7E && d[2] == 0x06 && d[3] == 0x01) {
    midi_send_sysex(k_inquiry_reply, (uint16_t)sizeof k_inquiry_reply);
    return;
  }

  /* C100 web-editor protocol:  F0 7D 43 31 30 30 <cmd> <payload..> F7  ("C100") */
  if (n >= 8 && d[0] == 0x7D && d[1] == 0x43 && d[2] == 0x31 && d[3] == 0x30 && d[4] == 0x30) {
    sysex_c100_web(d + 5, (uint16_t)(end - (d + 5)));
    return;
  }

  /* Everything below drives the LEDs — ignore while a UI screen owns the panel. */
  if (ui_owns_panel()) return;

  /* CFY compressed LED update:  F0 5F {RR GG BB [NN] XX..}.. F7
   *   count 1..7 packed into bit 6 of RR/GG/BB; 0 -> a following NN byte. */
  if (d[0] == 0x5F) {
    for (const uint8_t *i = d + 1; i + 3 <= end;) {
      uint8_t r = *i++, g = *i++, b = *i++;
      uint8_t cnt = (uint8_t)(((r & 0x40) >> 4) | ((g & 0x40) >> 5) | ((b & 0x40) >> 6));
      if (cnt == 0) {
        if (i >= end) break;
        cnt = *i++;
      }
      for (uint8_t j = 0; j < cnt && i < end; j++)
        put_range(*i++, r, g, b);
    }
    return;
  }

  /* Plain fast RGB (Apollo's fallback when compression can't help):
   *   F0 6F {pos RR GG BB}.. F7 */
  if (d[0] == 0x6F) {
    for (const uint8_t *i = d + 1; i + 4 <= end; i += 4)
      put(i[0], i[1], i[2], i[3]);
    return;
  }

  /* Novation "clear all LEDs":  F0 00 20 29 02 10 0E <val> F7 */
  if (n >= 9 && d[0] == 0x00 && d[1] == 0x20 && d[2] == 0x29 &&
      d[3] == 0x02 && d[4] == 0x10 && d[5] == 0x0E) {
    leds_clear();
    return;
  }
}
