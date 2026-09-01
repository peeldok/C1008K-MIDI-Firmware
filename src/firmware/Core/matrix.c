#include "matrix.h"
#include "app_state.h"
#include "at32f402_405_conf.h"

typedef struct { gpio_type *port; uint16_t pin; } pin_t;

/* Pin map (from stock-firmware reverse engineering) */
static const pin_t row_pin[MATRIX_ROWS] = {
  {GPIOC, GPIO_PINS_12}, {GPIOD, GPIO_PINS_2}, {GPIOB, GPIO_PINS_3}, {GPIOB, GPIO_PINS_4},
  {GPIOB, GPIO_PINS_5},  {GPIOB, GPIO_PINS_6}, {GPIOC, GPIO_PINS_1}, {GPIOC, GPIO_PINS_2},
  {GPIOC, GPIO_PINS_3},  {GPIOA, GPIO_PINS_0},
};
static const pin_t col_pin[MATRIX_COLS] = {
  {GPIOC, GPIO_PINS_7},  {GPIOC, GPIO_PINS_8},  {GPIOA, GPIO_PINS_11}, {GPIOA, GPIO_PINS_15},
  {GPIOC, GPIO_PINS_10}, {GPIOC, GPIO_PINS_11}, {GPIOC, GPIO_PINS_13}, {GPIOC, GPIO_PINS_14},
  {GPIOC, GPIO_PINS_15}, {GPIOC, GPIO_PINS_0},
};

#define DEBOUNCE 4

static uint8_t  cnt[MATRIX_ROWS][MATRIX_COLS];
static bool     state[MATRIX_ROWS][MATRIX_COLS];
static matrix_event_cb cb;

void matrix_set_callback(matrix_event_cb c) { cb = c; }

static void cfg_pin(const pin_t *p, gpio_mode_type mode, gpio_pull_type pull)
{
  gpio_init_type io;
  gpio_default_para_init(&io);
  io.gpio_mode = mode;
  io.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  io.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  io.gpio_pull = pull;
  io.gpio_pins = p->pin;
  gpio_init(p->port, &io);
}

void matrix_init(void)
{
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, TRUE);

  for (int r = 0; r < MATRIX_ROWS; r++) {
    cfg_pin(&row_pin[r], GPIO_MODE_OUTPUT, GPIO_PULL_NONE);
    gpio_bits_set(row_pin[r].port, row_pin[r].pin);   /* idle HIGH (inactive) */
  }
  for (int c = 0; c < MATRIX_COLS; c++)
    cfg_pin(&col_pin[c], GPIO_MODE_INPUT, GPIO_PULL_UP);
}

void matrix_scan(void)
{
  for (int r = 0; r < MATRIX_ROWS; r++) {
    gpio_bits_reset(row_pin[r].port, row_pin[r].pin);          /* select row LOW */
    for (volatile int d = 0; d < 40; d++) __asm volatile("nop"); /* settle */

    for (int c = 0; c < MATRIX_COLS; c++) {
      bool down = (gpio_input_data_bit_read(col_pin[c].port, col_pin[c].pin) == RESET);
      if (down == state[r][c]) { cnt[r][c] = 0; continue; }
      if (++cnt[r][c] >= DEBOUNCE) {
        cnt[r][c] = 0;
        state[r][c] = down;
        /* Rotation: the 8x8 grid always rotates (in setup too, so the layout /
         * app / brightness cells move as a live preview); the perimeter side
         * keys rotate only in play mode (in the UI they are the fixed rotation
         * selector); the 4 corners never rotate (TL must always reach setup). */
        uint8_t rr = (uint8_t)r, cc = (uint8_t)c;
        uint8_t lr = rr, lc = cc;
        bool corner = (rr == 0 || rr == 9) && (cc == 0 || cc == 9);
        bool inner  = (rr >= 1 && rr <= 8 && cc >= 1 && cc <= 8);
        if (!corner && (inner || g_ui == UI_NORMAL))
          phys_to_logical(rr, cc, &lr, &lc);
        pad_id_t pad = map_phys_to_pad(lr, lc);
        if (pad != PAD_NONE && cb) cb(pad, down);
      }
    }
    gpio_bits_set(row_pin[r].port, row_pin[r].pin);            /* deselect */
  }
}
