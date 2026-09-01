/* LED framebuffer + render for the C100 (96 logical pads -> 100 physical LEDs). */
#ifndef C100_LEDS_H
#define C100_LEDS_H

#include <stdint.h>
#include "mapping.h"
#include "snled2735.h"

void leds_init(void);                          /* snled_init + clear */
void leds_set_pad(pad_id_t pad, uint8_t r, uint8_t g, uint8_t b);
void leds_set_pad_velocity(pad_id_t pad, uint8_t velocity);  /* palette lookup */
void leds_clear(void);
void leds_set_brightness(uint8_t b);           /* 0..255, default 255 */

/* Direct physical-slot access (used by the boot animation & LED-walk test). */
void leds_set_phys(uint8_t phys_index /*0..99*/, uint8_t r, uint8_t g, uint8_t b);

/* Push current framebuffer to hardware (called from the render task). */
void leds_render(void);

#endif
