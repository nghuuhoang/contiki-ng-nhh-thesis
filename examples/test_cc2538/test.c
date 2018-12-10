#include "contiki.h"
#include "dev/gpio-hal-arch.h"
#include <stdio.h> /* For printf() */

#include "sys/log.h"
#define LOG_MODULE "Test"
#define LOG_LEVEL LOG_LEVEL_INFO
// void ctimer_callback(void *ptr);

// static struct ctimer timer_ctimer;
/*---------------------------------------------------------------------------*/
PROCESS(test, "Logging example process");
AUTOSTART_PROCESSES(&test);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test, ev, data)
{
	// static struct etimer et;
	// static uint8_t state_button;

  PROCESS_BEGIN();
  // etimer_set(&et, CLOCK_SECOND *5);
  while(1)
  {
  	PROCESS_WAIT_EVENT();
  	// etimer_set(&timer_ctimer, CLOCK_SECOND, ctimer_callback, NULL);
  	// if(etimer_expired(&et))
  	// {
      // state_button = gpio_hal_arch_read_pin( 0x80);
      // LOG_INFO("%d\n",state_button );
  	// 	LOG_INFO("etimer expired.\n");
  	// 	etimer_restart(&et);
  		printf("test.\n");
  	// }
  }

  PROCESS_END();
}

// void
// ctimer_callback(void *ptr)
// {
//   /* rearm the ctimer */
//   ctimer_reset(&timer_ctimer);
//   printf("CTimer callback called\n");
// }