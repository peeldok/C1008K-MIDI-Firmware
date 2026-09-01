/* C100 recovery path into the AT32F405 system-memory ROM DFU bootloader.
 * Reverse-engineered from the stock firmware. NOT the TinyUSB DFU class. */
#include "board.h"
#include "at32f402_405_conf.h"
#include "core_cm4.h"

#define SCB_VTOR_ADDR   0xE000ED08u
#define SYST_CSR_ADDR   0xE000E010u
#define SCB_AIRCR_ADDR  0xE000ED0Cu
#define AIRCR_VECTKEY   (0x5FAu << 16)
#define AIRCR_SYSRESETREQ (1u << 2)
#define DFU_REG32(a) (*(volatile uint32_t *)(a))

__attribute__((noreturn))
static void jump_rom_dfu(void)
{
  const uint32_t msp = *(const volatile uint32_t *)(C100_ROM_DFU_BASE + 0u);
  const uint32_t rst = *(const volatile uint32_t *)(C100_ROM_DFU_BASE + 4u);

  if (msp < 0x20000000u || msp > 0x20018000u ||
      !(rst & 1u) ||
      (rst & ~1u) < C100_ROM_DFU_BASE || (rst & ~1u) >= 0x1FFFF400u) {
    for (;;) { __asm volatile("nop"); }
  }

  __asm volatile("cpsid i" ::: "memory");
  DFU_REG32(SYST_CSR_ADDR) = 0u;
  DFU_REG32(SCB_VTOR_ADDR) = C100_ROM_DFU_BASE;
  __asm volatile(
    "dsb          \n"
    "isb          \n"
    "msr msp, %0  \n"
    "bx %1        \n"
    : : "r"(msp), "r"(rst) : "memory");
  __builtin_unreachable();
}

/* Minimal direct-GPIO scan of the recovery key (the top-left corner key of the
 * grid): row0 = PC12 (push-pull LOW), col0 = PC7 (input pull-up), key held =
 * PC7 reads LOW for 8 consecutive samples. Saves/restores the touched GPIOC
 * state so normal init is not disturbed. */
static int boot_key_held(void)
{
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);

  /* snapshot the registers gpio_init() will touch for PC7 / PC12 */
  uint32_t s_cfgr  = GPIOC->cfgr;
  uint32_t s_omode = GPIOC->omode;
  uint32_t s_odrvr = GPIOC->odrvr;
  uint32_t s_pull  = GPIOC->pull;
  uint32_t s_odt   = GPIOC->odt;

  gpio_init_type io;
  gpio_default_para_init(&io);

  io.gpio_mode = GPIO_MODE_INPUT;
  io.gpio_pull = GPIO_PULL_UP;
  io.gpio_pins = GPIO_PINS_7;
  gpio_init(GPIOC, &io);

  io.gpio_mode = GPIO_MODE_OUTPUT;
  io.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  io.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  io.gpio_pull = GPIO_PULL_NONE;
  io.gpio_pins = GPIO_PINS_12;
  gpio_init(GPIOC, &io);
  gpio_bits_reset(GPIOC, GPIO_PINS_12);

  for (volatile uint32_t d = 0; d < 20000; d++) { __asm volatile("nop"); }

  int held = 1;
  for (unsigned i = 0; i < 8u; i++) {
    if (gpio_input_data_bit_read(GPIOC, GPIO_PINS_7) != RESET) { held = 0; break; }
    for (volatile uint32_t d = 0; d < 2000; d++) { __asm volatile("nop"); }
  }

  /* restore */
  GPIOC->cfgr  = s_cfgr;
  GPIOC->omode = s_omode;
  GPIOC->odrvr = s_odrvr;
  GPIOC->pull  = s_pull;
  GPIOC->odt   = s_odt;

  return held;
}

void board_early_dfu_check(void)
{
  uint32_t magic = *C100_DFU_MAGIC_ADDR;
  *C100_DFU_MAGIC_ADDR = 0u;   /* clear first so a wedged jump can't loop forever */

  if (magic == C100_DFU_MAGIC || boot_key_held()) {
    jump_rom_dfu();
  }
}

__attribute__((noreturn))
void board_enter_rom_dfu(void)
{
  *C100_DFU_MAGIC_ADDR = C100_DFU_MAGIC;
  __asm volatile("dsb" ::: "memory");
  DFU_REG32(SCB_AIRCR_ADDR) = AIRCR_VECTKEY | AIRCR_SYSRESETREQ;
  __asm volatile("dsb \n isb" ::: "memory");
  for (;;) { __asm volatile("nop"); }
}
