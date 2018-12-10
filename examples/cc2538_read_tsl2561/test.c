#include "contiki.h"

#include <stdio.h> /* For printf() */
#include "dev/leds.h"
#include "lib/sensors.h"
#include "dev/tsl2561.h"
#include "dev/i2c.h"
#include "gpio.h"
/*---------------------------------------------------------------------------*/
PROCESS(test, "TSL2561 Lux process");
AUTOSTART_PROCESSES(&test);
/*---------------------------------------------------------------------------*/
#define SENSOR_READ_INTERVAL (CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
// void
// light_interrupt_callback(uint8_t value)
// {
//   printf("* Light sensor interrupt!\n");
//   leds_toggle(LEDS_RED);
// }
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test, ev, data)
{
  PROCESS_BEGIN();
  static uint16_t light;

  /* Print the sensor used, teh default is the TSL2561 (from Grove) */
  if(TSL256X_REF == TSL2561_SENSOR_REF) {
    printf("Light sensor test --> TSL2561\n");
  } else if(TSL256X_REF == TSL2563_SENSOR_REF) {
    printf("Light sensor test --> TSL2563\n");
  } else {
    printf("Unknown light sensor reference, aborting\n");
    PROCESS_EXIT();
  }

  /* Use Contiki's sensor macro to enable the sensor */
  SENSORS_ACTIVATE(tsl256x);

  /* Default integration time is 402ms with 1x gain, use the below call to
   * change the gain and timming, see tsl2563.h for more options
   */
  tsl256x.configure(TSL256X_TIMMING_CFG, TSL256X_G16X_402MS);

  /* Register the interrupt handler */
  // TSL256X_REGISTER_INT(light_interrupt_callback);

  /* Enable the interrupt source for values over the threshold.  The sensor
   * compares against the value of CH0, one way to find out the required
   * threshold for a given lux quantity is to enable the DEBUG flag and see
   * the CH0 value for a given measurement.  The other is to reverse the
   * calculations done in the calculate_lux() function.  The below value roughly
   * represents a 2500 lux threshold, same as pointing a flashlight directly
   */
  // tsl256x.configure(TSL256X_INT_OVER, 0x15B8);

  /* And periodically poll the sensor */

  while(1) {
    etimer_set(&et, SENSOR_READ_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    light = tsl256x.value(TSL256X_VAL_READ);
    if(light != TSL256X_ERROR) {
      printf("Light = %u\n", (uint16_t)light);
    } else {
      printf("Error, enable the DEBUG flag in the tsl256x driver for info, ");
      printf("or check if the sensor is properly connected\n");
      PROCESS_EXIT();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
