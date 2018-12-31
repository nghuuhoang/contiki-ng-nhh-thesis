/*
 * Copyright (c) 2015, Yanzi Networks AB.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \addtogroup lwm2m
 * @{
 */

/**
 * \file
 *         Implementation of the Contiki OMA LWM2M device
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#include "lwm2m-object.h"
#include "lwm2m-device.h"
#include "lwm2m-engine.h"
#include "dev/sys-ctrl.h"

#include <string.h>

#include "contiki-net.h"
#include "sys/node-id.h"

/* Log configuration */
#include "coap-log.h"
#define LOG_MODULE "lwm2m-dev"
#define LOG_LEVEL  LOG_LEVEL_LWM2M

static const lwm2m_resource_id_t resources[] =
  { RO(LWM2M_DEVICE_MANUFACTURER_ID),
    RO(LWM2M_DEVICE_MODEL_NUMBER_ID),
    RO(LWM2M_DEVICE_SERIAL_NUMBER_ID),
    RO(LWM2M_DEVICE_FIRMWARE_VERSION_ID),
    RO(LWM2M_DEVICE_AVAILABLE_POWER_SOURCES), /* Multi-resource-instance */
    RO(LWM2M_DEVICE_TYPE_ID),
    EX(LWM2M_DEVICE_REBOOT_ID),
    EX(LWM2M_DEVICE_FACTORY_DEFAULT_ID),
    RO(LWM2M_DEVICE_NETWORK_ROUTING_ID),
    RO(LWM2M_DEVICE_NETWORK_NET_ID),
    RO(LWM2M_DEVICE_NETWORK_MAC_ID),
    RO(LWM2M_DEVICE_NETWORK_PAN_ID),
    RO(LWM2M_DEVICE_NETWORK_CHANNEL_ID), /* Multi-resource-instance */
    RO(LWM2M_DEVICE_NETWORK_NODE_ID), /* Multi-resource-instance */
  };

#ifndef LWM2M_DEVICE_MANUFACTURER
#define LWM2M_DEVICE_MANUFACTURER     "RISE SICS"
#endif
#ifndef LWM2M_DEVICE_MODEL_NUMBER
#define LWM2M_DEVICE_MODEL_NUMBER     "1"
#endif
#ifndef LWM2M_DEVICE_SERIAL_NUMBER
#define LWM2M_DEVICE_SERIAL_NUMBER    "1"
#endif
#ifndef LWM2M_DEVICE_FIRMWARE_VERSION
#define LWM2M_DEVICE_FIRMWARE_VERSION CONTIKI_VERSION
#endif
#ifndef LWM2M_DEVICE_TYPE
#define LWM2M_DEVICE_TYPE "Contiki-NG LWM2M"
#endif



/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static lwm2m_status_t
write_string(lwm2m_context_t *ctx, const char *text)
{
  lwm2m_object_write_string(ctx, text, strlen(text));
  return LWM2M_STATUS_OK;
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static lwm2m_status_t
lwm2m_callback(lwm2m_object_instance_t *object, lwm2m_context_t *ctx)
{
  if(ctx->operation == LWM2M_OP_READ) {
    switch(ctx->resource_id) {
    case LWM2M_DEVICE_MANUFACTURER_ID:
      return write_string(ctx, LWM2M_DEVICE_MANUFACTURER);
    case LWM2M_DEVICE_MODEL_NUMBER_ID:
      return write_string(ctx, LWM2M_DEVICE_MODEL_NUMBER);
    case LWM2M_DEVICE_SERIAL_NUMBER_ID:
      return write_string(ctx, LWM2M_DEVICE_SERIAL_NUMBER);
    case LWM2M_DEVICE_FIRMWARE_VERSION_ID:
      return write_string(ctx, LWM2M_DEVICE_FIRMWARE_VERSION);
    case LWM2M_DEVICE_TYPE_ID:
      return write_string(ctx, LWM2M_DEVICE_TYPE);
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

  } else if(ctx->operation == LWM2M_OP_EXECUTE) {
    if(ctx->resource_id == LWM2M_DEVICE_REBOOT_ID) {
      /* Do THE REBOOT */
      sys_ctrl_reset();
      LOG_INFO("REBOOT\n");
      return LWM2M_STATUS_OK;
    }

  }

  return LWM2M_STATUS_OPERATION_NOT_ALLOWED;
}
/*---------------------------------------------------------------------------*/
static lwm2m_object_instance_t device = {
  .object_id = LWM2M_OBJECT_DEVICE_ID,
  .instance_id = 0,
  .resource_ids = resources,
  .resource_count = sizeof(resources) / sizeof(lwm2m_resource_id_t),
  .callback = lwm2m_callback,
};
/*---------------------------------------------------------------------------*/
void
lwm2m_device_init(void)
{
  lwm2m_engine_add_object(&device);
}
/*---------------------------------------------------------------------------*/
/** @} */
