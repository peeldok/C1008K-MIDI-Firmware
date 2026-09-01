#include "midi.h"
#include "mapping.h"
#include "sysex.h"
#include "tusb.h"

#define STATUS_NOTE_OFF 0x80
#define STATUS_NOTE_ON  0x90

static midi_note_cb note_cb;

void midi_init(midi_note_cb cb) { note_cb = cb; }

void midi_send_note_on(uint8_t note, uint8_t velocity)
{
  uint8_t m[3] = { (uint8_t)(STATUS_NOTE_ON | MIDI_CHANNEL), (uint8_t)(note & 0x7F),
                   (uint8_t)(velocity & 0x7F) };
  tud_midi_stream_write(0, m, 3);
}

void midi_send_note_off(uint8_t note)
{
  uint8_t m[3] = { (uint8_t)(STATUS_NOTE_OFF | MIDI_CHANNEL), (uint8_t)(note & 0x7F), 0 };
  tud_midi_stream_write(0, m, 3);
}

void midi_send_sysex(const uint8_t *data, uint16_t len)
{
  /* stream_write may stop early if the TX FIFO fills; keep feeding it. */
  uint16_t off = 0;
  for (int guard = 0; off < len && guard < 2000; guard++) {
    uint32_t w = tud_midi_stream_write(0, data + off, (uint32_t)(len - off));
    off += (uint16_t)w;
  }
}

/* ---- inbound: byte-stream MIDI parser (note on/off + SysEx reassembly) ---- */

static uint8_t  run_status = 0;   /* current channel/system status */
static uint8_t  data0;            /* first data byte of a 2-byte message */
static bool     have_data0 = false;

static uint8_t  sx_buf[SYSEX_MAX];
static uint16_t sx_len = 0;
static bool     sx_active = false;

static uint8_t data_bytes_for(uint8_t status)
{
  switch (status & 0xF0) {
    case 0x80: case 0x90: case 0xA0: case 0xB0: case 0xE0: return 2;
    case 0xC0: case 0xD0: return 1;
    default:                                   /* 0xF0 system common */
      if (status == 0xF2) return 2;
      if (status == 0xF1 || status == 0xF3) return 1;
      return 0;
  }
}

static void dispatch_channel(uint8_t status, uint8_t a, uint8_t b)
{
  if ((status & 0x0F) != MIDI_CHANNEL) return;
  uint8_t type = status & 0xF0;
  if (type == STATUS_NOTE_ON && b > 0) {
    if (note_cb) note_cb(a, b, true);
  } else if (type == STATUS_NOTE_OFF || (type == STATUS_NOTE_ON && b == 0)) {
    if (note_cb) note_cb(a, 0, false);
  }
}

static void parse_byte(uint8_t b)
{
  if (b >= 0xF8) return;                        /* realtime: ignore */

  if (b == 0xF0) {                              /* SysEx start */
    sx_active = true;
    sx_len = 0;
    sx_buf[sx_len++] = 0xF0;
    return;
  }

  if (b == 0xF7) {                              /* SysEx end */
    if (sx_active) {
      if (sx_len < SYSEX_MAX) sx_buf[sx_len++] = 0xF7;
      sysex_dispatch(sx_buf, sx_len);
      sx_active = false;
    }
    return;
  }

  if (b & 0x80) {                               /* any other status byte */
    sx_active = false;                          /* aborts an in-flight SysEx */
    run_status = (data_bytes_for(b) ? b : 0);
    have_data0 = false;
    return;
  }

  /* data byte */
  if (sx_active) {
    if (sx_len < SYSEX_MAX) sx_buf[sx_len++] = b;
    return;
  }

  if (run_status == 0) return;
  if (data_bytes_for(run_status) == 1) {
    dispatch_channel(run_status, b, 0);
  } else if (!have_data0) {
    data0 = b;
    have_data0 = true;
  } else {
    dispatch_channel(run_status, data0, b);
    have_data0 = false;                         /* running status: next msg reuses run_status */
  }
}

void midi_poll(void)
{
  uint8_t buf[64];
  uint32_t n;
  while ((n = tud_midi_stream_read(buf, sizeof buf)) > 0)
    for (uint32_t i = 0; i < n; i++) parse_byte(buf[i]);
}
