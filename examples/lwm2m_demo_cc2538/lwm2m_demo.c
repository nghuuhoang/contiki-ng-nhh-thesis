//nghuu.hoang@outlook.com
#include "contiki.h"
#include "dev/leds.h"
#include "services/lwm2m/lwm2m-engine.h"
#include "services/lwm2m/lwm2m-rd-client.h"
#include "services/lwm2m/lwm2m-device.h"
#include "services/lwm2m/lwm2m-server.h"
#include "services/lwm2m/lwm2m-security.h"
#include "services/lwm2m/lwm2m-firmware.h"
#include "services/lwm2m/lwm2m-plain-text.h"
// #include "services/lwm2m/lwm2m-network-info.h"

#include "services/ipso-objects/ipso-objects.h"
#include "services/ipso-objects/ipso-sensor-template.h"
#include "services/ipso-objects/ipso-control-template.h"
#include "dev/leds.h"
#include "dev/gpio-hal.h"
#include "lib/sensors.h"
#include "dev/bmpx8x.h"
#include "dev/si7021.h"
#include "dev/tsl2561.h"
#include "dev/relay.h"
#include "dev/board.h"
// #include "dev/cc2538-sensors.h"


#define DEBUG DEBUG_NONE
#include "net/ipv6/uip-debug.h"

#ifndef REGISTER_WITH_LWM2M_BOOTSTRAP_SERVER
#define REGISTER_WITH_LWM2M_BOOTSTRAP_SERVER 0
#endif

#ifndef REGISTER_WITH_LWM2M_SERVER
#define REGISTER_WITH_LWM2M_SERVER 1
#endif

#ifndef LWM2M_SERVER_ADDRESS
#define LWM2M_SERVER_ADDRESS "coap://[fd00::1]"
#endif

#define LIGHT_TSL2561_READ_INTERVAL (1*CLOCK_SECOND)

static uint8_t modeRelay1=1;

/*---------------------------------------------------------------------------*/
static void
fade(leds_mask_t l)
{
  volatile int i;
  int k, j;
  for(k = 0; k < 400; ++k) {
    j = k > 200 ? 400 - k : k;

    gpio_hal_arch_set_pin(l);
    for(i = 0; i < j; ++i) {
      __asm("nop");
    }
    gpio_hal_arch_clear_pin(l);
    for(i = 0; i < 200 - j; ++i) {
      __asm("nop");
    }
  }
}
/*---------------------------------------------------------------------------*/

//#include "board-peripherals.h"
/* LED control */
static lwm2m_status_t
led_green_set(ipso_control_t *control, uint8_t value)
{
  if(value > 0) {
    gpio_hal_arch_set_pin(LED_ON_SHIELD_2);
    gpio_hal_arch_set_pin(LED_ON_SHIELD_3);
  } else {
    gpio_hal_arch_clear_pin(LED_ON_SHIELD_2);
    gpio_hal_arch_clear_pin(LED_ON_SHIELD_3);
  }
  return LWM2M_STATUS_OK;
}
static lwm2m_status_t
led_red_set(ipso_control_t *control, uint8_t value)
{
  if(value > 0) {
    gpio_hal_arch_set_pin(LED_ON_SHIELD_0);
    gpio_hal_arch_set_pin(LED_ON_SHIELD_1);
  } else {
    gpio_hal_arch_clear_pin(LED_ON_SHIELD_0);
    gpio_hal_arch_clear_pin(LED_ON_SHIELD_1);
  }
  return LWM2M_STATUS_OK;
}
static lwm2m_status_t
led_blue_set(ipso_control_t *control, uint8_t value)
{
  if(value > 0) {
    gpio_hal_arch_set_pin(LED_ON_SHIELD_4);
    gpio_hal_arch_set_pin(LED_ON_SHIELD_5);
  } else {
    gpio_hal_arch_clear_pin(LED_ON_SHIELD_4);
    gpio_hal_arch_clear_pin(LED_ON_SHIELD_5);
  }
  return LWM2M_STATUS_OK;
}

/* Module Si7021 */
/* Temperature on Su7021*/
static lwm2m_status_t
read_tempSi7021_value(const ipso_sensor_t *s, int32_t *value)
{
  // s->instance_id = 1;
  // int32_t tempSi7021 = ((si7021_readTemp(TEMP_NOHOLD)) * 175.72) / 65536 - 46.85;
  *value = (((si7021_readTemp(TEMP_NOHOLD)) * 175.72) / 65536 - 46.85) * 1000;
  return LWM2M_STATUS_OK;
}
/* Humidity on Su7021*/
static lwm2m_status_t
read_humdSi7021_value(const ipso_sensor_t *s, int32_t *value)
{
  *value = (((si7021_readHumd(RH_NOHOLD)) * 125) / 65536 - 6) * 1000;
  return LWM2M_STATUS_OK;
}
/* Pressure on BMP180*/
static lwm2m_status_t
read_pressureBMP180_value(const ipso_sensor_t *s, int32_t *value)
{
  *value = bmpx8x.value(BMPx8x_READ_PRESSURE)/10 * 1000;
  return LWM2M_STATUS_OK;
}
/* Illuminance on TSL2561*/
static lwm2m_status_t
read_lightTSL2561_value(const ipso_sensor_t *s, int32_t *value)
{
  *value = tsl256x.value(TSL256X_VAL_READ) * 1000;
  return LWM2M_STATUS_OK;
}
/* Relay 1*/
static lwm2m_status_t
relay_1_set(ipso_control_t *control, uint8_t value)
{
  if(value > 0) {
    gpio_hal_arch_clear_pin(RELAY_1_PIN);
  } else {
    gpio_hal_arch_set_pin(RELAY_1_PIN);
  }
  return LWM2M_STATUS_OK;
}

/* Relay 2*/
static lwm2m_status_t
relay_2_set(ipso_control_t *control, uint8_t value)
{
  if(value > 0) {
    gpio_hal_arch_clear_pin(RELAY_2_PIN);
  } else {
    gpio_hal_arch_set_pin(RELAY_2_PIN);
  }
  return LWM2M_STATUS_OK;
}
static lwm2m_status_t
relay_1_mode_set(ipso_control_t *control, uint8_t value)
{
  if(value > 0){
    //auto mode relay 1
    modeRelay1 = 1;
    printf("auto mode\n");
  }else{
    //manual mode relay 1
    modeRelay1 = 0;
    printf("manual mode\n");
  }
  return LWM2M_STATUS_OK;
}

IPSO_CONTROL(led_control_green, 3311, 0, led_green_set);
IPSO_CONTROL(led_control_red, 3311, 1, led_red_set);
IPSO_CONTROL(led_control_blue, 3311, 2, led_blue_set);

IPSO_SENSOR(temp_sensor_Si7021, 3303, read_tempSi7021_value,
            .max_range = -50000, /* milli celcius */
            .min_range = 80000, /* milli celcius */
            .unit = "Cel",
            .update_interval = 10
            );
IPSO_SENSOR(humd_sensor_Si7021, 3304, read_humdSi7021_value,
            .max_range = 0, /* milli celcius */
            .min_range = 100000, /* milli celcius */
            .unit = "%",
            .update_interval = 11
            );
IPSO_SENSOR(pressure_sensor_BMP180, 3323, read_pressureBMP180_value,
            .unit = "hPa",
            .update_interval = 13
            );
IPSO_SENSOR(light_sensor_TSL2561, 3301, read_lightTSL2561_value,
            .max_range = 0,
            .min_range = 40000000,
            .unit = "lux",
            .update_interval = 12
            );

IPSO_CONTROL(relay_control_1, 3311, 3, relay_1_set);
IPSO_CONTROL(relay_control_2, 3311, 4, relay_2_set);
IPSO_CONTROL(relay_mode_1, 3311, 5, relay_1_mode_set);

static struct etimer et;


/*---------------------------------------------------------------------------*/
PROCESS(example_ipso_objects, "IPSO object example");
PROCESS(relayAutoMode, "Relay Auto mode");
AUTOSTART_PROCESSES(&example_ipso_objects,&relayAutoMode);

/*---------------------------------------------------------------------------*/
static void
setup_lwm2m_servers(void)
{
#ifdef LWM2M_SERVER_ADDRESS
  coap_endpoint_t server_ep;
  if(coap_endpoint_parse(LWM2M_SERVER_ADDRESS, strlen(LWM2M_SERVER_ADDRESS),
                         &server_ep) != 0) {
    lwm2m_rd_client_register_with_bootstrap_server(&server_ep);
    lwm2m_rd_client_register_with_server(&server_ep);
  }
#endif /* LWM2M_SERVER_ADDRESS */

  lwm2m_rd_client_use_bootstrap_server(REGISTER_WITH_LWM2M_BOOTSTRAP_SERVER);
  lwm2m_rd_client_use_registration_server(REGISTER_WITH_LWM2M_SERVER);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_ipso_objects, ev, data)
{
  // static struct etimer periodic;
  // static int val;
  // static int count = 0;
  
  
  gpio_hal_arch_pin_set_output(LED_ON_SHIELD_0);
  gpio_hal_arch_pin_set_output(LED_ON_SHIELD_1);
  gpio_hal_arch_pin_set_output(LED_ON_SHIELD_2);
  gpio_hal_arch_pin_set_output(LED_ON_SHIELD_3);
  gpio_hal_arch_pin_set_output(LED_ON_SHIELD_4);
  gpio_hal_arch_pin_set_output(LED_ON_SHIELD_5);
  gpio_hal_arch_pin_set_output(RELAY_1_PIN);
  gpio_hal_arch_pin_set_output(RELAY_2_PIN);
    gpio_hal_arch_clear_pin(LED_ON_SHIELD_0);
    gpio_hal_arch_clear_pin(LED_ON_SHIELD_1);
    gpio_hal_arch_clear_pin(LED_ON_SHIELD_2);
    gpio_hal_arch_clear_pin(LED_ON_SHIELD_3);
    gpio_hal_arch_clear_pin(LED_ON_SHIELD_4);
    gpio_hal_arch_clear_pin(LED_ON_SHIELD_5);
  PROCESS_BEGIN();
  i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_NORMAL_BUS_SPEED);
  SENSORS_ACTIVATE(bmpx8x);
  SENSORS_ACTIVATE(tsl256x);
  tsl256x.configure(TSL256X_TIMMING_CFG, TSL256X_G16X_402MS);

  fade(LED_ON_SHIELD_0);
  fade(LED_ON_SHIELD_1);
  fade(LED_ON_SHIELD_2);
  fade(LED_ON_SHIELD_3);
  fade(LED_ON_SHIELD_4);
  fade(LED_ON_SHIELD_5);
  fade(LED_ON_SHIELD_4);
  fade(LED_ON_SHIELD_3);
  fade(LED_ON_SHIELD_2);
  fade(LED_ON_SHIELD_1);
  fade(LED_ON_SHIELD_0);

  PROCESS_PAUSE();
  // leds_single_on(LEDS_BLUE);
  PRINTF("Starting IPSO objects example%s\n",
         REGISTER_WITH_LWM2M_BOOTSTRAP_SERVER ? " (bootstrap)" : "");
  /* Initialize the OMA LWM2M engine */
  lwm2m_engine_init();

  /* Register default LWM2M objects */
  lwm2m_device_init();
  // lwm2m_device_network_info_init();
  // lwm2m_security_init();
  // lwm2m_server_init();
  // lwm2m_firmware_init();
  // ipso_button_init();
  // ipso_leds_control_init();
  // ipso_temperature_cc2538_init();
  ipso_control_add(&relay_control_1);
  ipso_control_add(&relay_control_2);
  ipso_sensor_add(&temp_sensor_Si7021);
  ipso_sensor_add(&humd_sensor_Si7021);
  ipso_sensor_add(&pressure_sensor_BMP180);
  ipso_sensor_add(&light_sensor_TSL2561);
  ipso_control_add(&led_control_green);
  ipso_control_add(&led_control_red);
  ipso_control_add(&led_control_blue);
  ipso_control_add(&relay_mode_1);
  
  // ipso_blockwise_test_init();
  ipso_digital_input_init();
  setup_lwm2m_servers();
  while(1) {
    PROCESS_WAIT_EVENT();
    // PROCESS_WAIT_EVENT_UNTIL(modeRelay1 == 1);
      
      // PROCESS_WAIT_WHILE(modeRelay1 == 0);
    
    
    // PROCESS_WAIT_EVENT();
    
    
  }
  PROCESS_END();
}

PROCESS_THREAD(relayAutoMode, ev, data)
{
  static uint16_t lightValueTSL2561;
  PROCESS_BEGIN();
  while(1){
    etimer_set(&et, LIGHT_TSL2561_READ_INTERVAL);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    if(modeRelay1 == 1){
      lightValueTSL2561 = tsl256x.value(TSL256X_VAL_READ);
      if(lightValueTSL2561 != TSL256X_ERROR) {
        printf("Light = %u\n", (uint16_t)lightValueTSL2561);
        if(lightValueTSL2561 < 5){
          gpio_hal_arch_clear_pin(RELAY_1_PIN);
        }else{
          gpio_hal_arch_set_pin(RELAY_1_PIN);
        }
      } else {
        printf("Error, enable the DEBUG flag in the tsl256x driver for info, ");
        printf("or check if the sensor is properly connected\n");
        PROCESS_EXIT();
      }
    }
}
  PROCESS_END();
}