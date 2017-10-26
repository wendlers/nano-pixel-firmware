#ifndef __WS2812B_H__
#define __WS2812B_H__

#include <stdint.h>

template <typename Driver, uint16_t rows>
class Ws2812bMatrix : public Driver
{
public:

  uint16_t getRows()
  {
      return rows;
  }

  uint16_t getCols()
  {
      return Driver::num_leds / rows;
  }

  void set(uint16_t id, uint8_t red, uint8_t green, uint8_t blue)
  {
    Driver::set(id, red, green, blue);
  }

  void set(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue)
  {
     Driver::set(y * rows + x, red, green, blue);
  }
};

#endif
