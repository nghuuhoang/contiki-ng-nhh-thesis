
/**
 * 
 * Generic module controlling sensors on the cc2538dk
 * @{
 *
 * \file
 * Implementation of a generic module controlling sensors
 */
#include "contiki.h"
#include "dev/cc2538-sensors.h"

#include <string.h>

/** \brief Exports a global symbol to be used by the sensor API */
SENSORS(&cc2538_temp_sensor, &vdd3_sensor);

/**
 * @}
 * @}
 */
