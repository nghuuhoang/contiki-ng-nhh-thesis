

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

// #ifdef BOARD_STRING
// #define LWM2M_DEVICE_MODEL_NUMBER BOARD_STRING
// #elif defined(CONTIKI_TARGET_WISMOTE)
// #include "dev/watchdog.h"
// #define LWM2M_DEVICE_MODEL_NUMBER "LWM2M_DEVICE_MODEL_NUMBER"
// #define LWM2M_DEVICE_MANUFACTURER "LWM2M_DEVICE_MANUFACTURER"
// #define LWM2M_DEVICE_SERIAL_NO    "LWM2M_DEVICE_SERIAL_NO"
// #define PLATFORM_REBOOT watchdog_reboot
// #endif
#define LWM2M_DEVICE_MANUFACTURER     "nghuu.hoang@outlook.com"
#define LWM2M_DEVICE_TYPE "CC2538-base BK-Node"
#define LWM2M_DEVICE_MODEL_NUMBER     "1.0"
#define LWM2M_DEVICE_SERIAL_NUMBER    "1"

#define LWM2M_DEFAULT_CLIENT_LIFETIME 60 /* sec */
#if BOARD_SENSORTAG
/* Real sensor is present... */
#else
#define IPSO_TEMPERATURE example_ipso_temperature
#endif /* BOARD_SENSORTAG */

/* Increase rpl-border-router IP-buffer when using more than 64. */
#define COAP_MAX_CHUNK_SIZE            64

/* Multiplies with chunk size, be aware of memory constraints. */
#define COAP_MAX_OPEN_TRANSACTIONS     4

/* Filtering .well-known/core per query can be disabled to save space. */
#define COAP_LINK_FORMAT_FILTERING     0
#define COAP_PROXY_OPTION_PROCESSING   0

/* Enable client-side support for COAP observe */
#define COAP_OBSERVE_CLIENT 1

// #define USB_SERIAL_CONF_ENABLE		1
// #define SLIP_ARCH_CONF_USB          1 /** SLIP over UART by default */
// #define DBG_CONF_USB                1 /** All debugging over UART by default */

/* IEEE802.15.4 PANID */
#define IEEE802154_CONF_PANID 0x0509
// #define CC2538_RF_CONF_TX_POWER 0x00

// #define REGISTER_WITH_LWM2M_BOOTSTRAP_SERVER 1

// #define COAP_DTLS_PSK_DEFAULT_IDENTITY "Client_identity"
// #define COAP_DTLS_PSK_DEFAULT_KEY      "12345678890"

/* Definitions to enable Queue Mode, include the dynamic adaptation and change the default parameters  */
/* #define LWM2M_QUEUE_MODE_CONF_ENABLED 1
   #define LWM2M_QUEUE_MODE_CONF_INCLUDE_DYNAMIC_ADAPTATION 1
   #define LWM2M_QUEUE_MODE_CONF_DEFAULT_CLIENT_AWAKE_TIME 2000
   #define LWM2M_QUEUE_MODE_CONF_DEFAULT_CLIENT_SLEEP_TIME 10000
   #define LWM2M_QUEUE_MODE_CONF_DEFAULT_DYNAMIC_ADAPTATION_FLAG 0
   #define LWM2M_QUEUE_MODE_OBJECT_CONF_ENABLED 1 */
/* Logging */
// #define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_COAP                        LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_LWM2M                       LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_6TOP                        LOG_LEVEL_DBG
//#define TSCH_LOG_CONF_PER_SLOT                     1
#endif /* PROJECT_CONF_H_ */
