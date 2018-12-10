#include "contiki.h"

#include <stdio.h> /* For printf() */
#include "dev/gpio-hal.h"

gpio_hal_pin_t out_pin0 = 8;
gpio_hal_pin_t out_pin1 = 9;
gpio_hal_pin_t out_pin2 = 10;
gpio_hal_pin_t out_pin3 = 11;
gpio_hal_pin_t out_pin4 = 12;
gpio_hal_pin_t out_pin5 = 13;

/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
  //static struct etimer timer;

  PROCESS_BEGIN();

  /* Setup a periodic timer that expires after 10 seconds. */
  //etimer_set(&timer, CLOCK_SECOND * 10);
  gpio_hal_arch_pin_set_output(out_pin0);
  gpio_hal_arch_pin_set_output(out_pin1);
  gpio_hal_arch_pin_set_output(out_pin2);
  gpio_hal_arch_pin_set_output(out_pin3);
  gpio_hal_arch_pin_set_output(out_pin4);
  gpio_hal_arch_pin_set_output(out_pin5);
  // while(1) {
    printf("Hello, world\n");
    gpio_hal_arch_set_pin(out_pin0);
    gpio_hal_arch_set_pin(out_pin1);
    gpio_hal_arch_set_pin(out_pin2);
    gpio_hal_arch_set_pin(out_pin3);
    gpio_hal_arch_set_pin(out_pin4);
    gpio_hal_arch_set_pin(out_pin5);
    /* Wait for the periodic timer to expire and then restart the timer. */
  //   PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
  //   etimer_reset(&timer);
  // }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
