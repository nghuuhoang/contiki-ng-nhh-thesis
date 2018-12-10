
//nghuu.hoang@outlook.com

#include <stdint.h>
#include "ipso-sensor-template.h"
#include "ipso-objects.h"
#include "lwm2m-object.h"
#include "lwm2m-engine.h"
#include "lib/sensors.h"
#include "dev/cc2538-sensors.h"
#include "dev/cc2538-temp-sensor.h"




/* Temperature reading */
static lwm2m_status_t
read_temp_value(const ipso_sensor_t *s, int32_t *value)
{
  *value = cc2538_temp_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED);
  return LWM2M_STATUS_OK;
}

IPSO_SENSOR(temp_sensor, 3303, read_temp_value,
            .max_range = 100000, /* 100 cel milli celcius */
            .min_range = -10000, /* -10 cel milli celcius */
            .unit = "Cel",
            .update_interval = 30
            );

void
ipso_temperature_cc2538_init(void)
{

  ipso_sensor_add(&temp_sensor);
}
