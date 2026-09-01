/* Keychron C100 — USB-MIDI grid controller firmware.
 *
 * Core wiring only: clocks/USB bring-up, the two FreeRTOS tasks, and the
 * top-level event dispatch. The behaviour lives in the feature modules —
 *   Performance/  the normal MIDI-grid app (UI_NORMAL)
 *   Setup/        the setup + brightness screens (UI_SETUP / UI_BRIGHTNESS)
 * entered from normal play with the top-left corner key.
 *
 * Layout / rotation / app / brightness / palette persist to flash (nvs.c) when
 * the setup screen is left.
 */
#include "board.h"
#include "app_state.h"
#include "mapping.h"
#include "matrix.h"
#include "leds.h"
#include "midi.h"
#include "boot_anim.h"
#include "performance.h"
#include "setup.h"

#include "tusb.h"
#include "FreeRTOS.h"
#include "task.h"

/* ---- USB IRQ (OTGHS = rhport 1) ---- */
void OTGHS_IRQHandler(void)      { tusb_int_handler(1, true); }
void OTGHS_WKUP_IRQHandler(void) { tusb_int_handler(1, true); }

/* ---- shared app state (declared in app_state.h) ---- */
volatile layout_t   g_layout   = LAYOUT_DRUM;
volatile ui_state_t g_ui       = UI_NORMAL;
volatile uint8_t    g_rotation = 0;
volatile uint8_t    g_app      = APP_PERFORMANCE;
volatile uint8_t    g_palette  = 0;        /* 0 = Original */

/* ---- event dispatch ---- */
static void on_key(pad_id_t pad, bool pressed)
{
  if (pad == PAD_CORNER_TL) {              /* top-left corner = UI navigation, never MIDI */
    if (pressed) {
      if (g_ui == UI_NORMAL)          g_ui = UI_SETUP;
      else if (g_ui == UI_BRIGHTNESS) g_ui = UI_SETUP;
      else { g_ui = UI_NORMAL; setup_save_settings(); }   /* leaving setup */
      leds_clear();
    }
    return;
  }

  if (g_ui == UI_NORMAL) performance_key(pad, pressed);
  else                   setup_key(pad, pressed);
}

static void on_midi_note(uint8_t note, uint8_t velocity, bool on)
{
  if (ui_owns_panel()) return;             /* a setup screen owns the panel */
  performance_midi_note(note, velocity, on);
}

/* ---- tasks ---- */
#define USBD_STACK   (3 * configMINIMAL_STACK_SIZE)
#define APP_STACK    (6 * configMINIMAL_STACK_SIZE)

static StackType_t  usbd_stack[USBD_STACK];
static StaticTask_t usbd_tcb;
static StackType_t  app_stack[APP_STACK];
static StaticTask_t app_tcb;

static void usbd_task(void *arg)
{
  (void)arg;
  /* init the stack from inside a task, after the scheduler is running.
   * .speed = AUTO: let dcd_dwc2 pick from CFG_TUD_MAX_SPEED / the HS PHY. */
  tusb_rhport_init_t rh = { .role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO };
  tusb_init(1, &rh);
  for (;;) tud_task();
}

static void app_task(void *arg)
{
  (void)arg;

  setup_load_settings();            /* layout / rotation / app / brightness / palette from flash */
  leds_init();
  leds_set_brightness(setup_brightness());
  matrix_init();
  matrix_set_callback(on_key);
  midi_init(on_midi_note);

  boot_anim_play(board_watchdog_kick);   /* usbd task services USB concurrently */

  TickType_t last = xTaskGetTickCount();
  uint32_t frame = 0;
  for (;;) {
    board_watchdog_kick();           /* if usbd_task wedges, this stops -> reboot */
    matrix_scan();
    midi_poll();

    /* Render every 3rd 1 kHz pass -> ~333 fps (target >= 250). The SNLED push
     * (~1 ms at the /64 SPI clock) blocks this task; USB keeps running on the
     * higher-priority usbd task. */
    if (++frame >= 3) {
      frame = 0;
      if (g_ui != UI_NORMAL) {        /* a setup screen owns the panel */
        leds_clear();
        setup_render();
      }
      leds_render();
    }

    vTaskDelayUntil(&last, pdMS_TO_TICKS(1));
  }
}

int main(void)
{
  board_early_dfu_check();   /* must run before anything else */
  board_init();
  board_watchdog_init();     /* ~3 s; kicked from app_task */

  xTaskCreateStatic(usbd_task, "usbd", USBD_STACK, NULL, configMAX_PRIORITIES - 1,
                    usbd_stack, &usbd_tcb);
  xTaskCreateStatic(app_task, "app", APP_STACK, NULL, 2, app_stack, &app_tcb);

  vTaskStartScheduler();
  for (;;) {}
}

/* ---- FreeRTOS static allocation + hooks ---- */
void vApplicationGetIdleTaskMemory(StaticTask_t **tcb, StackType_t **stack, uint32_t *sz)
{
  static StaticTask_t t; static StackType_t s[configMINIMAL_STACK_SIZE];
  *tcb = &t; *stack = s; *sz = configMINIMAL_STACK_SIZE;
}
void vApplicationGetTimerTaskMemory(StaticTask_t **tcb, StackType_t **stack, uint32_t *sz)
{
  static StaticTask_t t; static StackType_t s[configTIMER_TASK_STACK_DEPTH];
  *tcb = &t; *stack = s; *sz = configTIMER_TASK_STACK_DEPTH;
}
void vApplicationStackOverflowHook(TaskHandle_t t, char *name) { (void)t; (void)name; for (;;) {} }
void vApplicationMallocFailedHook(void) { for (;;) {} }
