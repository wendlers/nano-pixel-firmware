#ifndef __WS2812B_SPI_H__
#define __WS2812B_SPI_H__

#include <stdint.h>

#include "ws2812b_driver.h"

static nrf_drv_spi_t spi_drv_inst[] = {
  NRF_DRV_SPI_INSTANCE(0),
#if (NUM_SPI_BUF > 1)
  NRF_DRV_SPI_INSTANCE(1),
#if (NUM_SPI_BUF > 2)
  NRF_DRV_SPI_INSTANCE(2),
#endif
#endif
};

class Ws2812bSpiInst
{
public:
  enum spi_inst_t {
    spi0,
#if (NUM_SPI_BUF > 1)
    spi1,
#if (NUM_SPI_BUF > 2)
    spi2,
#endif
#endif
  };
};

template <uint16_t num_leds, Ws2812bSpiInst::spi_inst_t spi_instance>
class Ws2812bSpi
{
public:

  Ws2812bSpi() :
    brightness(1.0)
  {
    spi.spi = spi_drv_inst[spi_instance];

    ws2812b_driver_spi_init(spi_instance, &spi);
  	alloc_spi_buffer(&spi_buffer, num_leds);

    blank();
    update();
  }

  uint16_t getNumLeds()
  {
      return num_leds;
  }

  void set(uint16_t id, uint8_t red, uint8_t green, uint8_t blue)
  {
      led_array[id % num_leds].red = red;
      led_array[id % num_leds].green = green;
      led_array[id % num_leds].blue = blue;
  }

  void blank()
  {
      set_blank(led_array, num_leds);
  }

  void update()
  {
    if(brightness < 1.0) {
        rgb_led_t led_array_tmp[num_leds];

		for(uint16_t i = 0; i < num_leds; i++)
		{
            led_array_tmp[i].red   = led_array[i].red   * brightness;
            led_array_tmp[i].green = led_array[i].green * brightness;
            led_array_tmp[i].blue  = led_array[i].blue  * brightness;
		}
        ws2812b_driver_xfer(led_array_tmp, spi_buffer, spi);
    }
    else {
        ws2812b_driver_xfer(led_array, spi_buffer, spi);
    }
  }

  void setBrightness(float value)
  {
    brightness = value;
  }

  float getBrightness()
  {
     return brightness;
  }

private:

    ws2812b_driver_spi_t spi;
    spi_buffer_t spi_buffer;
    rgb_led_t led_array[num_leds];

    float brightness;
};

#endif
