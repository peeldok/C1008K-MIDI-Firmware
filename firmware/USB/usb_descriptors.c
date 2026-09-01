/*
 * Descriptor structure and callback pattern adapted from TinyUSB MIDI device
 * examples. TinyUSB is licensed under the MIT License.
 * See THIRD_PARTY_LICENSES.md and licenses/TINYUSB-MIT.txt.
 */
/* USB descriptors — C100 grid, MIDI-only, High-Speed (OTGHS / rhport 1). */
#include "tusb.h"
#include "board.h"

/* Same USB identity as the stock Keychron C100 8K, so the host sees it as the
 * same device (VID 0x3434 Keychron, PID 0x042C). Apollo Studio recognises the
 * grid from the Launchpad/CFY SysEx identity, not the USB ids. */
#define USB_VID   0x3434
#define USB_PID   0x042C
#define USB_BCD   0x0200

/* ---------------- Device descriptor ---------------- */
static const tusb_desc_device_t desc_device = {
  .bLength            = sizeof(tusb_desc_device_t),
  .bDescriptorType    = TUSB_DESC_DEVICE,
  .bcdUSB             = USB_BCD,
  .bDeviceClass       = 0x00,
  .bDeviceSubClass    = 0x00,
  .bDeviceProtocol    = 0x00,
  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
  .idVendor           = USB_VID,
  .idProduct          = USB_PID,
  .bcdDevice          = 0x0200,   /* firmware revision — bump forces the host to re-read
                                     descriptors and drop any stale cached device name */
  .iManufacturer      = 0x01,
  .iProduct           = 0x02,
  .iSerialNumber      = 0x03,
  .bNumConfigurations = 0x01,
};

const uint8_t *tud_descriptor_device_cb(void) { return (const uint8_t *)&desc_device; }

/* ---------------- Configuration ---------------- */
enum { ITF_NUM_MIDI = 0, ITF_NUM_MIDI_STREAMING, ITF_NUM_TOTAL };

#define EPNUM_MIDI_OUT  0x01
#define EPNUM_MIDI_IN   0x81

#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN)

static const uint8_t desc_fs_configuration[] = {
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
  TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 4, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64),
};

/* USB-MIDI streaming endpoints stay 64-byte bulk even at High-Speed, matching
 * the stock firmware. A 512-byte HS bulk endpoint here overruns the AT32 OTGHS
 * DFIFO allocation and breaks MIDI silently. */
static const uint8_t desc_hs_configuration[] = {
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
  TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 4, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64),
};

const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
  (void)index;
  return (tud_speed_get() == TUSB_SPEED_HIGH) ? desc_hs_configuration : desc_fs_configuration;
}

/* Other-speed config: describe the *opposite* speed's operation, and (per USB
 * spec sec 10.2) change bDescriptorType CONFIGURATION -> OTHER_SPEED.
 * TinyUSB returns this verbatim, so we patch the type byte ourselves. */
static uint8_t desc_other_speed[sizeof desc_hs_configuration];

const uint8_t *tud_descriptor_other_speed_configuration_cb(uint8_t index)
{
  (void)index;
  const uint8_t *src = (tud_speed_get() == TUSB_SPEED_HIGH)
                         ? desc_fs_configuration : desc_hs_configuration;
  size_t len = (src == desc_fs_configuration) ? sizeof desc_fs_configuration
                                              : sizeof desc_hs_configuration;
  memcpy(desc_other_speed, src, len);
  desc_other_speed[1] = TUSB_DESC_OTHER_SPEED_CONFIG;   /* 0x02 -> 0x07 */
  return desc_other_speed;
}

static const tusb_desc_device_qualifier_t desc_device_qualifier = {
  .bLength            = sizeof(tusb_desc_device_qualifier_t),
  .bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
  .bcdUSB             = USB_BCD,
  .bDeviceClass       = 0x00,
  .bDeviceSubClass    = 0x00,
  .bDeviceProtocol    = 0x00,
  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
  .bNumConfigurations = 0x01,
  .bReserved          = 0x00,
};

const uint8_t *tud_descriptor_device_qualifier_cb(void)
{
  return (const uint8_t *)&desc_device_qualifier;
}

/* ---------------- Strings ---------------- */
static char serial_str[25];

static const char *string_desc_arr[] = {
  (const char[]){ 0x09, 0x04 },  /* 0: en-US */
  "Keychron",                    /* 1: manufacturer */
  "C100 MIDI",                   /* 2: product (host MIDI port name) */
  serial_str,                    /* 3: serial (filled from MCU UID) */
  "C100 MIDI",                   /* 4: MIDI streaming interface / port name */
};

static uint16_t desc_str[32];

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void)langid;
  size_t chr_count;

  if (index == 0) {
    memcpy(&desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  } else {
    if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) return NULL;

    if (index == 3 && serial_str[0] == 0) {
      uint8_t uid[12];
      board_unique_id(uid);
      static const char hex[] = "0123456789ABCDEF";
      for (int i = 0; i < 12; i++) {
        serial_str[i * 2]     = hex[uid[i] >> 4];
        serial_str[i * 2 + 1] = hex[uid[i] & 0x0F];
      }
      serial_str[24] = 0;
    }

    const char *str = string_desc_arr[index];
    chr_count = strlen(str);
    if (chr_count > 31) chr_count = 31;
    for (size_t i = 0; i < chr_count; i++) desc_str[1 + i] = str[i];
  }

  desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
  return desc_str;
}
