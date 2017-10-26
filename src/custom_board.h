/**
 * Definitions for RedBaer BLE Nano v2
 */

#ifndef CUSTOM_BOARD_H
#define CUSTOM_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// LEDs definitions
#define LEDS_NUMBER    1

#define LED_START      11
#define LED_1          11
#define LED_STOP       11

#define LEDS_ACTIVE_STATE 0

#define LEDS_INV_MASK  LEDS_MASK

#define LEDS_LIST { LED_1 }

#define BSP_LED_0      LED_1

#define BUTTONS_NUMBER 0

#define BUTTONS_LIST { }

#define RX_PIN_NUMBER  30
#define TX_PIN_NUMBER  29
#define RTS_PIN_NUMBER 2
#define CTS_PIN_NUMBER 28
#define HWFC           true

#define SPIM0_SCK_PIN   8   // SPI clock GPIO pin number.
#define SPIM0_MOSI_PIN  6   // SPI Master Out Slave In GPIO pin number.
#define SPIM0_MISO_PIN  7   // SPI Master In Slave Out GPIO pin number.
#define SPIM0_SS_PIN    3   // SPI Slave Select GPIO pin number.

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC { \
  .source        = NRF_CLOCK_LF_SRC_XTAL,            \
  .rc_ctiv       = 0,                                \
  .rc_temp_ctiv  = 0,                                \
  .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM }

#ifdef __cplusplus
}
#endif

#endif // CUSTOM_BOARD_H
