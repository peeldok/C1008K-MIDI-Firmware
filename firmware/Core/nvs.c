#include "nvs.h"
#include "at32f402_405_conf.h"
#include <string.h>

/* Reserved sectors (see the linker script: FLASH is capped at 252 KB). */
#define NVS_SETTINGS_ADDR  0x0803F000u
#define NVS_PALETTES_ADDR  0x0803F800u
#define NVS_MAGIC          0x30303143u   /* "C100" */
#define NVS_SETTINGS_VER   2
#define NVS_PALETTES_VER   1

static uint32_t crc32(const void *data, uint32_t len)
{
  const uint8_t *p = (const uint8_t *)data;
  uint32_t crc = 0xFFFFFFFFu;
  while (len--) {
    crc ^= *p++;
    for (int i = 0; i < 8; i++)
      crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1u)));
  }
  return ~crc;
}

static void flash_write_blob(uint32_t addr, const void *blob, uint32_t bytes)
{
  const uint32_t *w = (const uint32_t *)blob;
  flash_unlock();
  flash_sector_erase(addr);
  for (uint32_t i = 0; i < bytes / 4u; i++)
    flash_word_program(addr + i * 4u, w[i]);
  flash_lock();
}

/* ---- settings ---- */

bool nvs_settings_load(nvs_settings_t *out)
{
  const nvs_settings_t *f = (const nvs_settings_t *)NVS_SETTINGS_ADDR;
  if (f->magic != NVS_MAGIC || f->version != NVS_SETTINGS_VER) return false;
  if (crc32(f, sizeof(*f) - sizeof(uint32_t)) != f->crc) return false;
  memcpy(out, f, sizeof(*out));
  return true;
}

void nvs_settings_save(nvs_settings_t *in)
{
  nvs_settings_t b;
  memset(&b, 0, sizeof b);           /* deterministic padding for the CRC */
  b.magic = NVS_MAGIC;
  b.version = NVS_SETTINGS_VER;
  b.layout = in->layout;
  b.brightness_step = in->brightness_step;
  b.rotation = in->rotation;
  b.app = in->app;
  b.palette = in->palette;
  b.crc = crc32(&b, sizeof b - sizeof(uint32_t));
  flash_write_blob(NVS_SETTINGS_ADDR, &b, sizeof b);
}

/* ---- palettes ---- */

bool nvs_palettes_load(nvs_palettes_t *out)
{
  const nvs_palettes_t *f = (const nvs_palettes_t *)NVS_PALETTES_ADDR;
  if (f->magic != NVS_MAGIC || f->version != NVS_PALETTES_VER) return false;
  if (crc32(f, sizeof(*f) - sizeof(uint32_t)) != f->crc) return false;
  memcpy(out, f, sizeof(*out));
  return true;
}

void nvs_palettes_save(nvs_palettes_t *in)
{
  in->magic = NVS_MAGIC;
  in->version = NVS_PALETTES_VER;
  in->_reserved = 0;
  in->crc = crc32(in, sizeof(*in) - sizeof(uint32_t));
  flash_write_blob(NVS_PALETTES_ADDR, in, sizeof(*in));
}
