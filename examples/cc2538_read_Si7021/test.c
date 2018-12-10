#include "contiki.h"

#include <stdio.h> /* For printf() */
#include "dev/leds.h"
#include "lib/sensors.h"
#include "dev/si7021.h"
#include "dev/i2c.h"
#include "gpio.h"
/*---------------------------------------------------------------------------*/
PROCESS(test, "Si7021 Temperature and Humidity process");
AUTOSTART_PROCESSES(&test);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test, ev, data)
{
  //static struct etimer timer;
    static int count = 0;

  static struct etimer periodic;
  PROCESS_BEGIN();

  i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_NORMAL_BUS_SPEED);
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
      si7021_readTemp(TEMP_NOHOLD);
      si7021_readHumd(RH_NOHOLD);
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