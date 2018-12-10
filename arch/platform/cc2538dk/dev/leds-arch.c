
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/leds.h"
#include "dev/gpio-hal.h"
#include "dev/gpio-hal-arch.h"

#include <stdbool.h>
/*---------------------------------------------------------------------------*/
const leds_t leds_arch_leds[] = {
  {
    .pin = GPIO_PORT_PIN_TO_GPIO_HAL_PIN(LEDS_ARCH_L1_PORT, LEDS_ARCH_L1_PIN),
    .negative_logic = false
  },
  {
    .pin = GPIO_PORT_PIN_TO_GPIO_HAL_PIN(LEDS_ARCH_L2_PORT, LEDS_ARCH_L2_PIN),
    .negative_logic = false
  },
  {
    .pin = GPIO_PORT_PIN_TO_GPIO_HAL_PIN(LEDS_ARCH_L3_PORT, LEDS_ARCH_L3_PIN),
    .negative_logic = false
  },
/*#if !USB_SERIAL_CONF_ENABLE
  {
    .pin = GPIO_PORT_PIN_TO_GPIO_HAL_PIN(LEDS_ARCH_L4_PORT, LEDS_ARCH_L4_PIN),
    .negative_logic = false
  },
#endif*/
};
/*---------------------------------------------------------------------------*/
