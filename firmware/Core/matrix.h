/* 10x10 key matrix scan for the Keychron C100.
 * Rows = push-pull outputs, driven LOW one at a time.
 * Cols = inputs with pull-up; a pressed key pulls its column LOW.
 * (Polarity from the stock-firmware reverse engineering.)
 */
#ifndef C100_MATRIX_H
#define C100_MATRIX_H

#include <stdint.h>
#include <stdbool.h>
#include "mapping.h"

/* Called from the scan task whenever a debounced key state changes. */
typedef void (*matrix_event_cb)(pad_id_t pad, bool pressed);

void matrix_init(void);
void matrix_set_callback(matrix_event_cb cb);

/* One full 10x10 pass with debounce bookkeeping; call at ~1 kHz. */
void matrix_scan(void);

#endif
