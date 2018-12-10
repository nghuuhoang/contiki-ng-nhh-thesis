
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/i2c.h"
#include "dev/leds.h"
#include "dev/bmpx8x.h"
/*---------------------------------------------------------------------------*/
#define SENSOR_READ_INTERVAL (CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
PROCESS(remote_bmpx8x_process, "BMP085/BMP180 test process");
AUTOSTART_PROCESSES(&remote_bmpx8x_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_bmpx8x_process, ev, data)
{
  PROCESS_BEGIN();
  static uint16_t pressure;
  static int16_t temperature;

  /* Use Contiki's sensor macro to enable the sensor */
  SENSORS_ACTIVATE(bmpx8x);

  /* And periodically poll the sensor */

  while(1) {
    etimer_set(&et, SENSOR_READ_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    pressure = bmpx8x.value(BMPx8x_READ_PRESSURE);
    temperature = bmpx8x.value(BMPx8x_READ_TEMP);

    if((pressure != BMPx8x_ERROR) && (temperature != BMPx8x_ERROR)) {
      printf("Pressure = %u.%u(hPa), ", pressure / 10, pressure % 10);
      printf("Temperature = %d.%u(ºC)\n", temperature / 10, temperature % 10);
    } else {
      printf("Error, enable the DEBUG flag in the BMPx8x driver for info, ");
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

