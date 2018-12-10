#include "contiki.h"

#include <stdio.h> /* For printf() */
#include "dev/leds.h"


/*---------------------------------------------------------------------------*/
PROCESS(test, "Hello world process");
AUTOSTART_PROCESSES(&test);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test, ev, data)
{
  //static struct etimer timer;
    static int count = 0;

  static struct etimer periodic;
  PROCESS_BEGIN();

  /* Setup a periodic timer that expires after 10 seconds. */
  //etimer_set(&timer, CLOCK_SECOND * 10);
  etimer_set(&periodic, CLOCK_SECOND * 1);
  // while(1) {
  while(1) {
  //   val = cc2538_temp_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED);
  //   printf("temperature : %u mC\n",val);
    
    PROCESS_WAIT_EVENT();
    
    if(ev == PROCESS_EVENT_TIMER && etimer_expired(&periodic)) {
      count++;
      // printf("%d\n",count );
      
    if(count == 9){
      leds_single_on(LEDS_GREEN);
    }
    if(count == 10){
      leds_single_off(LEDS_GREEN);
      count = 0;
    }
      etimer_reset(&periodic);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/