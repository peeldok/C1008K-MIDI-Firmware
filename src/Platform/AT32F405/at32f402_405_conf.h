/* AT32F402/405 SDK peripheral module selection for the Keychron C100 firmware.
 * HEXT is 12 MHz on this board (matches the SDK default and tinyusb BSP). */
#ifndef __AT32F402_405_CONF_H
#define __AT32F402_405_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined HEXT_VALUE
#define HEXT_VALUE            ((uint32_t)12000000)
#endif
#define HEXT_STARTUP_TIMEOUT  ((uint16_t)0x3000)
#define HICK_VALUE            ((uint32_t)8000000)
#define LEXT_VALUE            ((uint32_t)32768)

/* Only the peripherals the port actually uses. */
#define CRM_MODULE_ENABLED
#define DEBUG_MODULE_ENABLED
#define DMA_MODULE_ENABLED
#define EXINT_MODULE_ENABLED
#define FLASH_MODULE_ENABLED
#define GPIO_MODULE_ENABLED
#define MISC_MODULE_ENABLED
#define PWC_MODULE_ENABLED
#define SCFG_MODULE_ENABLED
#define SPI_MODULE_ENABLED
#define TMR_MODULE_ENABLED
#define USART_MODULE_ENABLED
#define USB_MODULE_ENABLED
#define ACC_MODULE_ENABLED
#define WDT_MODULE_ENABLED

#ifdef CRM_MODULE_ENABLED
#include "at32f402_405_crm.h"
#endif
#ifdef DEBUG_MODULE_ENABLED
#include "at32f402_405_debug.h"
#endif
#ifdef DMA_MODULE_ENABLED
#include "at32f402_405_dma.h"
#endif
#ifdef EXINT_MODULE_ENABLED
#include "at32f402_405_exint.h"
#endif
#ifdef FLASH_MODULE_ENABLED
#include "at32f402_405_flash.h"
#endif
#ifdef GPIO_MODULE_ENABLED
#include "at32f402_405_gpio.h"
#endif
#ifdef MISC_MODULE_ENABLED
#include "at32f402_405_misc.h"
#endif
#ifdef PWC_MODULE_ENABLED
#include "at32f402_405_pwc.h"
#endif
#ifdef SCFG_MODULE_ENABLED
#include "at32f402_405_scfg.h"
#endif
#ifdef SPI_MODULE_ENABLED
#include "at32f402_405_spi.h"
#endif
#ifdef TMR_MODULE_ENABLED
#include "at32f402_405_tmr.h"
#endif
#ifdef USART_MODULE_ENABLED
#include "at32f402_405_usart.h"
#endif
#ifdef USB_MODULE_ENABLED
#include "at32f402_405_usb.h"
#endif
#ifdef ACC_MODULE_ENABLED
#include "at32f402_405_acc.h"
#endif
#ifdef WDT_MODULE_ENABLED
#include "at32f402_405_wdt.h"
#endif

#define assert_param(expr) ((void)0U)

#ifdef __cplusplus
}
#endif
#endif
