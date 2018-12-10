#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* Set to enable TSCH security */
// #ifndef WITH_SECURITY
// #define WITH_SECURITY 0
// #endif /* WITH_SECURITY */

/* USB serial takes space, free more space elsewhere */
// #define SICSLOWPAN_CONF_FRAG 0
// #define UIP_CONF_BUFFER_SIZE 160
// #define USB_SERIAL_CONF_ENABLE		1
// #define SLIP_ARCH_CONF_USB          0 /** SLIP over UART by default */
// #define DBG_CONF_USB                1 /** All debugging over UART by default */
/*******************************************************/
/******************* Configure TSCH ********************/
/*******************************************************/
// #define USB_PULLUP_PORT         GPIO_C_NUM
// #define USB_PULLUP_PIN          0
/* IEEE802.15.4 PANID */
#define IEEE802154_CONF_PANID 0x0509

/* Do not start TSCH at init, wait for NETSTACK_MAC.on() */
// #define TSCH_CONF_AUTOSTART 0

/* 6TiSCH minimal schedule length.
 * Larger values result in less frequent active slots: reduces capacity and saves energy. */
// #define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 3

// #if WITH_SECURITY

/* Enable security */
// #define LLSEC802154_CONF_ENABLED 1
/* Force button descriptions */
// #define BUTTON_HAL_CONF_WITH_DESCRIPTION 1
// #endif /* WITH_SECURITY */

/*******************************************************/
/************* Other system configuration **************/
/*******************************************************/
/* Force button descriptions */
#define BUTTON_HAL_CONF_WITH_DESCRIPTION 1
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
