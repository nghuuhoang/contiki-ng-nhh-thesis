
#include "lwm2m-object.h"
#include "lwm2m-network-info.h"
#include "lwm2m-engine.h"
#include "dev/sys-ctrl.h"

#include "contiki-net.h"
#include "sys/node-id.h"

#include <string.h>

/* Log configuration */
#include "coap-log.h"
#define LOG_MODULE "lwm2m-dev"
#define LOG_LEVEL  LOG_LEVEL_LWM2M

#define LWM2M_OBJECT_DEVICE_NETWORK_INFO_ID 1024

static const lwm2m_resource_id_t resources[] =
  { RO(LWM2M_DEVICE_NETWORK_ROUTING_ID),
    RO(LWM2M_DEVICE_NETWORK_NET_ID),
    RO(LWM2M_DEVICE_NETWORK_MAC_ID),
    RO(LWM2M_DEVICE_NETWORK_PAN_ID),
    RO(LWM2M_DEVICE_NETWORK_CHANNEL_ID), /* Multi-resource-instance */
    RO(LWM2M_DEVICE_NETWORK_NODE_ID), /* Multi-resource-instance */
    RO(LWM2M_DEVICE_NETWORK_MAC_ADRESS_ID), /* Multi-resource-instance */
  };





/*---------------------------------------------------------------------------*/
static lwm2m_status_t
write_string(lwm2m_context_t *ctx, const char *text)
{
  lwm2m_object_write_string(ctx, text, strlen(text));
  return LWM2M_STATUS_OK;
}



/*---------------------------------------------------------------------------*/
static lwm2m_status_t
lwm2m_callback(lwm2m_object_instance_t *object, lwm2m_context_t *ctx)
{
  if(ctx->operation == LWM2M_OP_READ) {
    switch(ctx->resource_id) {
    case LWM2M_DEVICE_NETWORK_ROUTING_ID:
      return write_string(ctx, NETSTACK_ROUTING.name);
    case LWM2M_DEVICE_NETWORK_NET_ID:
      return write_string(ctx, NETSTACK_NETWORK.name);
    case LWM2M_DEVICE_NETWORK_MAC_ID:
      return write_string(ctx, NETSTACK_MAC.name);
    
    case LWM2M_DEVICE_NETWORK_NODE_ID:
      lwm2m_object_write_int(ctx, node_id);
      return LWM2M_STATUS_OK;
    case LWM2M_DEVICE_NETWORK_CHANNEL_ID:
      lwm2m_object_write_int(ctx, IEEE802154_DEFAULT_CHANNEL);
      return LWM2M_STATUS_OK;
    case LWM2M_DEVICE_NETWORK_PAN_ID:
      lwm2m_object_write_int(ctx, IEEE802154_CONF_PANID);
      return LWM2M_STATUS_OK;
    default:
      LOG_WARN("Not found: %u\n", ctx->resource_id);
      return LWM2M_STATUS_NOT_FOUND;
    }

  } 

  return LWM2M_STATUS_OPERATION_NOT_ALLOWED;
}
/*---------------------------------------------------------------------------*/
static lwm2m_object_instance_t device = {
  .object_id = LWM2M_OBJECT_DEVICE_NETWORK_INFO_ID,
  .instance_id = 0,
  .resource_ids = resources,
  .resource_count = sizeof(resources) / sizeof(lwm2m_resource_id_t),
  .callback = lwm2m_callback,
};
/*---------------------------------------------------------------------------*/
void
lwm2m_device_network_info_init(void)
{
  lwm2m_engine_add_object(&device);
}
/*---------------------------------------------------------------------------*/
/** @} */
