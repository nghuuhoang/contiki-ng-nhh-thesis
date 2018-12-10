//nghuu.hoang@outlook.com

#include "contiki.h"
#include "lwm2m-object.h"
#include "lwm2m-engine.h"
#include "dev/button-hal.h"
#include "dev/leds.h"
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define IPSO_INPUT_STATE     5500
#define IPSO_INPUT_COUNTER   5501
#define IPSO_INPUT_DEBOUNCE  5503
#define IPSO_INPUT_EDGE_SEL  5504
#define IPSO_INPUT_CTR_RESET 5505
#define IPSO_INPUT_SENSOR_TYPE 5751

static lwm2m_status_t lwm2m_callback(lwm2m_object_instance_t *object,
                                     lwm2m_context_t *ctx);
PROCESS(demo, "ipso-button");

static int input_state = 0;
static int32_t counter = 0;
static int32_t edge_selection = 3; /* both */
static int32_t debounce_time = 10;

static const lwm2m_resource_id_t resources[] = {
  RO(IPSO_INPUT_STATE), RO(IPSO_INPUT_COUNTER),
  RW(IPSO_INPUT_DEBOUNCE), RW(IPSO_INPUT_EDGE_SEL), EX(IPSO_INPUT_CTR_RESET),
  RO(IPSO_INPUT_SENSOR_TYPE)
};

/* Only support for one button for now */
static lwm2m_object_instance_t reg_object = {
  .object_id = 3200,
  .instance_id = 0,
  .resource_ids = resources,
  .resource_count = sizeof(resources) / sizeof(lwm2m_resource_id_t),
  .callback = lwm2m_callback,
};

/*---------------------------------------------------------------------------*/
static int
read_state(void)
{
  PRINTF("Read input state: %d\n", input_state);
  //button_hal_get_state(btn);
  input_state = gpio_hal_arch_read_pin(RELAY_1_PIN);
  return input_state;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static lwm2m_status_t
lwm2m_callback(lwm2m_object_instance_t *object,
               lwm2m_context_t *ctx)
{
  if(ctx->operation == LWM2M_OP_READ) {
    switch(ctx->resource_id) {
    case IPSO_INPUT_STATE:
      lwm2m_object_write_int(ctx, read_state());
      break;
    case IPSO_INPUT_COUNTER:
      lwm2m_object_write_int(ctx, counter);
      break;
    case IPSO_INPUT_DEBOUNCE:
      lwm2m_object_write_int(ctx, debounce_time);
      break;
    case IPSO_INPUT_EDGE_SEL:
      lwm2m_object_write_int(ctx, edge_selection);
      break;
    case IPSO_INPUT_SENSOR_TYPE:
      lwm2m_object_write_string(ctx, "digital input", strlen("digital input"));
      break;
    default:
      return LWM2M_STATUS_ERROR;
    }
  } else if(ctx->operation == LWM2M_OP_EXECUTE) {
    if(ctx->resource_id == IPSO_INPUT_CTR_RESET) {
      counter = 0;
    } else {
      return LWM2M_STATUS_ERROR;
    }
  }
  return LWM2M_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
void
ipso_digital_input_init(void)
{
  /* register this device and its handlers - the handlers automatically
     sends in the object to handle */
  lwm2m_engine_add_object(&reg_object);
  process_start(&demo, NULL);
}
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
      leds_single_off(LEDS_GREEN);
      //printf("0\n");
    PROCESS_WAIT_EVENT();
    gpio_hal_arch_interrupt_enable(RELAY_1_PIN);
    if((gpio_hal_arch_pin_cfg_get(RELAY_1_PIN) & GPIO_HAL_PIN_CFG_EDGE_BOTH) == GPIO_HAL_PIN_CFG_EDGE_BOTH)
    {
      leds_single_on(LEDS_GREEN); 
      // gpio_hal_arch_interrupt_disable(RELAY_1_PIN);
      // printf("1\n");
      input_state = gpio_hal_arch_read_pin(RELAY_1_PIN);
      lwm2m_notify_object_observers(&reg_object, IPSO_INPUT_STATE);
    }
}
  PROCESS_END();
}