/* Board bring-up for the Keychron C100 (AT32F405RCT7-7) MIDI-grid firmware.
 * Clock sequence is the Artery tinyusb-BSP reference: 216 MHz sclk, USB from PLLU. */
#include "board.h"
#include "at32f402_405_conf.h"
#include "core_cm4.h"
#include "FreeRTOSConfig.h"

/* ---- 216 MHz system clock + PLLU for USB (from tinyusb hw/bsp/at32f402_405) ---- */
static void system_clock_config(void)
{
  crm_reset();
  flash_psr_set(FLASH_WAIT_CYCLE_6);

  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
  pwc_ldo_output_voltage_set(PWC_LDO_OUTPUT_1V3);

  crm_clock_source_enable(CRM_CLOCK_SOURCE_HEXT, TRUE);
  while (crm_hext_stable_wait() == ERROR) {}

  /* sclk = hext(12M) * 72 / (1 * 4) = 216 MHz */
  crm_pll_config(CRM_PLL_SOURCE_HEXT, 72, 1, CRM_PLL_FP_4);
  crm_pllu_div_set(CRM_PLL_FU_18);

  crm_clock_source_enable(CRM_CLOCK_SOURCE_PLL, TRUE);
  while (crm_flag_get(CRM_PLL_STABLE_FLAG) != SET) {}

  crm_ahb_div_set(CRM_AHB_DIV_1);      /* 216 MHz */
  crm_apb2_div_set(CRM_APB2_DIV_1);    /* 216 MHz */
  crm_apb1_div_set(CRM_APB1_DIV_2);    /* 108 MHz */

  crm_auto_step_mode_enable(TRUE);
  crm_sysclk_switch(CRM_SCLK_PLL);
  while (crm_sysclk_switch_status_get() != CRM_SCLK_PLL) {}
  crm_auto_step_mode_enable(FALSE);

  system_core_clock_update();
}

/* Full OTGHS + embedded-HS-PHY bring-up. TinyUSB does NOT do the Artery-specific
 * PHY clocking, and its core soft-reset spins forever if the PHY clock isn't up
 * yet ("do not call tusb_init before the HS PHY clock is stable"). */
static void usb_clock_config(void)
{
  /* OTGHS bring-up from the known-good stock-firmware reverse engineering.
   *
   * C100 matrix column 2 is PA11, which is also OTGFS1 D-, so the FS
   * controller stays disabled and the device runs exclusively on
   * OTGHS / rhport 1 with the dedicated on-chip HS PHY. Both the DWC2 core
   * and the OTGHS PHY clocks must be running before TinyUSB touches the core. */
  crm_periph_reset(CRM_OTGHS_PERIPH_RESET,    TRUE);
  crm_periph_reset(CRM_OTGHSPHY_PERIPH_RESET, TRUE);
  crm_periph_reset(CRM_OTGHS_PERIPH_RESET,    FALSE);
  crm_periph_reset(CRM_OTGHSPHY_PERIPH_RESET, FALSE);
  crm_periph_clock_enable(CRM_OTGHSPHY_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_OTGHS_PERIPH_CLOCK,    TRUE);

  /* The AT32F405 on-chip HS PHY needs its dedicated 12 MHz reference selector
   * set explicitly; leaving it at reset made HS attach visible to the host
   * while EP0 traffic failed intermittently before the device descriptor. */
  crm_usb_phy12_clock_select(CRM_USB_PHY12_CLOCK_HEXT_DIV_1);

  crm_pllu_output_set(TRUE);
  while (crm_flag_get(CRM_PLLU_STABLE_FLAG) != SET) {}
  crm_usb_clock_source_select(CRM_USB_CLOCK_SOURCE_PLLU);

  /* OTGHS GCCFG bit 21: ignore VBUS sensing (this board does not wire VBUS to
   * the OTG comparator). Written once here — GCCFG is outside the DWC2 core
   * reset domain, so it survives TinyUSB's core soft-reset. */
  *(volatile uint32_t *)(OTGHS_BASE + 0x38u) |= (1u << 21);

  /* IRQ priority inside the FreeRTOS syscall range; left disabled + pending
   * cleared — TinyUSB's dcd_init() enables it. */
  NVIC_SetPriority(OTGHS_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
  NVIC_DisableIRQ(OTGHS_IRQn);
  NVIC_ClearPendingIRQ(OTGHS_IRQn);
}

uint32_t board_unique_id(uint8_t out[12])
{
  const volatile uint32_t *uid = (const volatile uint32_t *)0x1FFFF7E8u;
  uint32_t *o = (uint32_t *)(void *)out;
  o[0] = uid[0];
  o[1] = uid[1];
  o[2] = uid[2];
  return 12;
}

/* Independent watchdog off the ~40 kHz LICK.
 * timeout = reload / (40000 / div).  div 256, reload 3125/8 ... pick ~3 s:
 *   40000/128 = 312.5 Hz -> reload 937 ~= 3.0 s */
void board_watchdog_init(void)
{
  crm_clock_source_enable(CRM_CLOCK_SOURCE_LICK, TRUE);
  while (crm_flag_get(CRM_LICK_STABLE_FLAG) != SET) {}

  wdt_register_write_enable(TRUE);
  wdt_divider_set(WDT_CLK_DIV_128);
  wdt_reload_value_set(937);
  wdt_register_write_enable(FALSE);
  wdt_counter_reload();
  wdt_enable();
}

void board_watchdog_kick(void)
{
  wdt_counter_reload();
}

void board_init(void)
{
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  system_clock_config();
  usb_clock_config();

  /* FreeRTOS owns SysTick; make sure nothing is running it yet. */
  SysTick->CTRL = 0;
}

/* ---- Fault handlers ----
 * Brief pause, then reboot; a fault loop then shows as the boot animation
 * repeating. */
static void fault_trap(void)
{
  for (volatile uint32_t d = 0; d < 2000000u; d++) { __asm volatile("nop"); }
  NVIC_SystemReset();
}
void HardFault_Handler(void)   { fault_trap(); }
void MemManage_Handler(void)   { fault_trap(); }
void BusFault_Handler(void)    { fault_trap(); }
void UsageFault_Handler(void)  { fault_trap(); }
