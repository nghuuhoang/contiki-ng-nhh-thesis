/*nghuu.hoang@outlook.com*/
#include "contiki.h"
#include "dev/button-hal.h"
#include "dev/leds.h"
#include <stdio.h>

#define GREEN_LED LEDS_LED1
#define RED_LED   LEDS_LED2
#define BLUE_LED  LEDS_LED3
/*---------------------------------------------------------------------------*/
PROCESS(button_hal_example, "Button HAL Example");
AUTOSTART_PROCESSES(&button_hal_example);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(button_hal_example, ev, data)
{
  button_hal_button_t *btn;

  PROCESS_BEGIN();

  btn = button_hal_get_by_index(0);

  printf("Button HAL example.\n");
  printf("Device button count: %u.\n", button_hal_button_count);

  if(btn) {
    printf("%s on pin %u with ID=0, Logic=%s, Pull=%s\n",
           BUTTON_HAL_GET_DESCRIPTION(btn), btn->pin,
           btn->negative_logic ? "Negative" : "Positive",
           btn->pull == GPIO_HAL_PIN_CFG_PULL_UP ? "Pull Up" : "Pull Down");
  }

  while(1) {

    PROCESS_YIELD();

    if(ev == button_hal_press_event) {
      btn = (button_hal_button_t *)data;
      printf("Press event (%s)\n", BUTTON_HAL_GET_DESCRIPTION(btn));

      if(btn == button_hal_get_by_id(BUTTON_HAL_ID_BUTTON_FOUR)) {
        printf("This was button 0, on pin %u\n", btn->pin);
        leds_single_toggle(GREEN_LED);
      }
    } else if(ev == button_hal_release_event) {
      btn = (button_hal_button_t *)data;
      printf("Release event (%s)\n", BUTTON_HAL_GET_DESCRIPTION(btn));
      leds_single_toggle(RED_LED);
    } else if(ev == button_hal_periodic_event) {
      btn = (button_hal_button_t *)data;
      printf("Periodic event, %u seconds (%s)\n", btn->press_duration_seconds,
             BUTTON_HAL_GET_DESCRIPTION(btn));

      if(btn->press_duration_seconds > 5) {
        printf("%s pressed for more than 5 secs. Do custom action\n",
               BUTTON_HAL_GET_DESCRIPTION(btn));
        leds_single_toggle(BLUE_LED);
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
