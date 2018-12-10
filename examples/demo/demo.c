/*nghuu.hoang@outlook.com*/
#include "contiki.h"
#include "dev/button-hal.h"
#include "dev/leds.h"
#include "lib/sensors.h"
#include "dev/cc2538-sensors.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(demo, "demo");
AUTOSTART_PROCESSES(&demo);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(demo, ev, data)
{
  PROCESS_BEGIN();
  // gpio_hal_pin_cfg_t interrupt;
  // gpio_hal_callback_t *button_handler;
  // gpio_hal_event_handler(GPIO_HAL_PIN_CFG_EDGE_RISING);
  // gpio_hal_arch_interrupt_enable(7);
  // gpio_hal_register_handler(button_handler);
  while(1){
  		//leds_single_off(LEDS_GREEN);
  		//printf("0\n");
  	PROCESS_WAIT_EVENT();
  	gpio_hal_arch_interrupt_enable(8);
  	if((gpio_hal_arch_pin_cfg_get(8) & GPIO_HAL_PIN_CFG_EDGE_RISING) == GPIO_HAL_PIN_CFG_EDGE_RISING)
  	{
  		leds_single_on(LEDS_GREEN);	
  		// gpio_hal_arch_interrupt_disable(7);
  		printf("1\n");
  	}
  	
  		// gpio_hal_arch_interrupt_disable(7);
  	
  	// interrupt = gpio_hal_arch_pin_cfg_get(7) &
   //        GPIO_HAL_PIN_CFG_EDGE_RISING;
   //      if(interrupt == 0) {
   //        printf("Enabling button interrupt\n");
   //        gpio_hal_arch_interrupt_enable(7);
   //      } else {
   //        printf("Disabling button interrupt\n");
   //        gpio_hal_arch_interrupt_disable(7);
   //      }
}
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
// void button_handler(void)
// {
// 	if(gpio_hal_arch_pin_cfg_get(7)& GPIO_HAL_PIN_CFG_EDGE_RISING)
// 	{
//   		leds_single_on(LEDS_GREEN);	
// 	}
// 	else
// 	{
//   		leds_single_off(LEDS_GREEN);
// 	}
// }