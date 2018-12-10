#ifndef LWM2M_NETWORK_INFO_H_
#define LWM2M_NETWORK_INFO_H_

#include "contiki.h"

#define LWM2M_DEVICE_NETWORK_ROUTING_ID             0
#define LWM2M_DEVICE_NETWORK_NET_ID                 1
#define LWM2M_DEVICE_NETWORK_MAC_ID                 2
#define LWM2M_DEVICE_NETWORK_PAN_ID                 3
#define LWM2M_DEVICE_NETWORK_CHANNEL_ID             4
#define LWM2M_DEVICE_NETWORK_NODE_ID                5
#define LWM2M_DEVICE_NETWORK_MAC_ADRESS_ID          6






void lwm2m_device_network_info_init(void);

#endif /* LWM2M_NETWORK_INFO_H_ */
/** @} */