/* SNLED2735 x2 over SPI1 (software CS), blocking 8-bit transfers.
 * Protocol reverse-engineered from the stock C100 firmware:
 *   frame = [ (page & 0x0F) | 0x20 ][ start_reg ][ data ... ]   (CS low for the whole frame)
 * Pages:  0 = LED on/off enable (24 regs)
 *         1 = PWM (192 regs, 0x00..0xBF)  <- stock LED map offsets index here
 *         3 = function/config
 *         4 = current tune (12 regs)
 * Init sequence and magic register values are taken from the stock firmware.
 */
#include "snled2735.h"
#include "stock_led_map.h"
#include "at32f402_405_conf.h"

/* ---- pin map ---- */
#define SNLED_SPI            SPI1
#define SNLED_SCK_PIN        GPIO_PINS_5     /* PA5  AF5 */
#define SNLED_MOSI_PIN       GPIO_PINS_7     /* PA7  AF5 */
#define SNLED_MISO_PIN       GPIO_PINS_14    /* PB14 AF5 (unused) */
#define SNLED_CS0_PORT       GPIOA
#define SNLED_CS0_PIN        GPIO_PINS_8     /* PA8  -> SNLED #0 */
#define SNLED_CS1_PORT       GPIOC
#define SNLED_CS1_PIN        GPIO_PINS_9     /* PC9  -> SNLED #1 */
#define SNLED_SDB_PORT       GPIOB
#define SNLED_SDB_PIN        GPIO_PINS_7     /* PB7  shared shutdown/wake */

#define SNLED_PAGE_ENABLE    0
#define SNLED_PAGE_PWM       1
#define SNLED_PAGE_FUNC      3
#define SNLED_PAGE_TUNE      4

/* SPI bit clock = PCLK2 (216 MHz) / divider. Stock firmware uses /64 (3.4 MHz);
 * a full 2x194-byte frame then takes ~1 ms, so a 3 ms render cadence still
 * gives ~333 fps. Faster dividers (/32, /16) are available but were seen to
 * disturb the panel on this board, so stay on the stock value by default. */
#ifndef SNLED_SPI_MCLK_DIV
#define SNLED_SPI_MCLK_DIV   SPI_MCLK_DIV_64
#endif

/* SNLED2735 page-4 global current tune (0x00..0xFF), applied to all 12 CS lines.
 * Stock firmware uses 0x78 (~47 %); this build runs full scale (0xFF).
 * NOTE: a fully-lit 100-LED panel at 0xFF can pull well past the 500 mA USB
 * budget -> possible brown-out (LEDs stutter / go dark) on a bus-powered port.
 * Build with -DSNLED_CURRENT_TUNE=0xNN to dial it back. */
#ifndef SNLED_CURRENT_TUNE
#define SNLED_CURRENT_TUNE   0xFF
#endif

static void delay_loops(volatile uint32_t n) { while (n--) __asm volatile("nop"); }
static void delay_ms_rough(uint32_t ms)      { delay_loops(ms * 40000u); } /* ~216MHz, pre-scheduler */

/* ---- low level ---- */
static inline void spi_put8(uint8_t v)
{
  while (spi_i2s_flag_get(SNLED_SPI, SPI_I2S_TDBE_FLAG) == RESET) {}
  *(volatile uint8_t *)&SNLED_SPI->dt = v;          /* 8-bit access is mandatory */
}
static inline void spi_wait_idle(void)
{
  while (spi_i2s_flag_get(SNLED_SPI, SPI_I2S_BF_FLAG) == SET) {}
}

static void cs_low(uint8_t dev)
{
  if (dev == 0) gpio_bits_reset(SNLED_CS0_PORT, SNLED_CS0_PIN);
  else          gpio_bits_reset(SNLED_CS1_PORT, SNLED_CS1_PIN);
}
static void cs_high_all(void)
{
  gpio_bits_set(SNLED_CS0_PORT, SNLED_CS0_PIN);
  gpio_bits_set(SNLED_CS1_PORT, SNLED_CS1_PIN);
}

static void snled_write(uint8_t dev, uint8_t page, uint8_t start_reg,
                        const uint8_t *data, uint16_t len)
{
  cs_high_all();
  cs_low(dev);
  spi_put8((uint8_t)((page & 0x0F) | 0x20));
  spi_put8(start_reg);
  for (uint16_t i = 0; i < len; i++) spi_put8(data[i]);
  spi_wait_idle();
  cs_high_all();
}

static void snled_write_reg(uint8_t dev, uint8_t page, uint8_t reg, uint8_t val)
{
  snled_write(dev, page, reg, &val, 1);
}

/* ---- init ---- */
static void snled_init_dev(uint8_t dev)
{
  uint8_t zero[192];
  uint8_t tune[12];
  for (int i = 0; i < 192; i++) zero[i] = 0;
  for (int i = 0; i < 12;  i++) tune[i] = SNLED_CURRENT_TUNE;

  snled_write_reg(dev, SNLED_PAGE_FUNC, 0x00, 0x00);   /* config mode */
  snled_write_reg(dev, SNLED_PAGE_FUNC, 0x13, 0xAA);
  snled_write_reg(dev, SNLED_PAGE_FUNC, 0x14, 0x00);
  snled_write_reg(dev, SNLED_PAGE_FUNC, 0x15, 0x04);
  snled_write_reg(dev, SNLED_PAGE_FUNC, 0x16, 0x40);
  snled_write_reg(dev, SNLED_PAGE_FUNC, 0x1A, 0x00);

  snled_write(dev, SNLED_PAGE_ENABLE, 0x00, zero, 24);   /* enable page cleared */
  snled_write(dev, SNLED_PAGE_PWM,    0x00, zero, 192);  /* PWM page cleared    */
  snled_write(dev, SNLED_PAGE_TUNE,   0x00, tune, 12);   /* global current tune */

  snled_write_reg(dev, SNLED_PAGE_FUNC, 0x00, 0x01);   /* normal operation */

  for (int i = 0; i < 24; i++) zero[i] = 0xFF;           /* enable every channel */
  snled_write(dev, SNLED_PAGE_ENABLE, 0x00, zero, 24);
}

static void gpio_out(gpio_type *port, uint32_t pin)
{
  gpio_init_type io;
  gpio_default_para_init(&io);
  io.gpio_mode = GPIO_MODE_OUTPUT;
  io.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  io.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  io.gpio_pull = GPIO_PULL_NONE;
  io.gpio_pins = pin;
  gpio_init(port, &io);
}

void snled_init(void)
{
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_SPI1_PERIPH_CLOCK, TRUE);

  /* CS + SDB as GPIO output */
  gpio_out(SNLED_CS0_PORT, SNLED_CS0_PIN);
  gpio_out(SNLED_CS1_PORT, SNLED_CS1_PIN);
  gpio_out(SNLED_SDB_PORT, SNLED_SDB_PIN);
  cs_high_all();

  /* SPI1 SCK/MOSI as AF5 */
  gpio_init_type io;
  gpio_default_para_init(&io);
  io.gpio_mode = GPIO_MODE_MUX;
  io.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  io.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  io.gpio_pull = GPIO_PULL_NONE;
  io.gpio_pins = SNLED_SCK_PIN | SNLED_MOSI_PIN;
  gpio_init(GPIOA, &io);
  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE5, GPIO_MUX_5);
  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE7, GPIO_MUX_5);

  spi_init_type spi;
  spi_default_para_init(&spi);
  spi.transmission_mode    = SPI_TRANSMIT_HALF_DUPLEX_TX;   /* TX only, MISO free */
  spi.master_slave_mode    = SPI_MODE_MASTER;
  spi.mclk_freq_division   = SNLED_SPI_MCLK_DIV;
  spi.first_bit_transmission= SPI_FIRST_BIT_MSB;
  spi.frame_bit_num        = SPI_FRAME_8BIT;
  spi.clock_polarity       = SPI_CLOCK_POLARITY_LOW;         /* mode 0 */
  spi.clock_phase          = SPI_CLOCK_PHASE_1EDGE;
  spi.cs_mode_selection    = SPI_CS_SOFTWARE_MODE;
  spi_init(SNLED_SPI, &spi);
  spi_software_cs_internal_level_set(SNLED_SPI, SPI_SWCS_INTERNAL_LEVEL_HIGHT);
  spi_enable(SNLED_SPI, TRUE);

  /* SDB wake: LOW -> HIGH + ~1 ms */
  gpio_bits_reset(SNLED_SDB_PORT, SNLED_SDB_PIN);
  delay_ms_rough(2);
  gpio_bits_set(SNLED_SDB_PORT, SNLED_SDB_PIN);
  delay_ms_rough(2);

  snled_init_dev(0);
  snled_init_dev(1);
}

/* ---- render ---- */
void snled_render(const rgb_t frame[SNLED_PHYS_LEDS], uint8_t brightness)
{
  /* static: 384 B kept off the caller's task stack (single render task, no reentry) */
  static uint8_t pwm[2][192];
  for (int i = 0; i < 192; i++) { pwm[0][i] = 0; pwm[1][i] = 0; }

  for (int i = 0; i < STOCK_LED_COUNT && i < SNLED_PHYS_LEDS; i++) {
    const snled_led_map_t *m = &STOCK_LED_MAP[i];
    uint16_t r = (uint16_t)frame[i].r * brightness / 255;
    uint16_t g = (uint16_t)frame[i].g * brightness / 255;
    uint16_t b = (uint16_t)frame[i].b * brightness / 255;
    pwm[m->dev][m->r] = (uint8_t)r;
    pwm[m->dev][m->g] = (uint8_t)g;
    pwm[m->dev][m->b] = (uint8_t)b;
  }

  snled_write(0, SNLED_PAGE_PWM, 0x00, pwm[0], 192);
  snled_write(1, SNLED_PAGE_PWM, 0x00, pwm[1], 192);
}
