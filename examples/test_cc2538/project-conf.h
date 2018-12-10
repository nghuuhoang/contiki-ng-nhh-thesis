#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* Set maximum debug level on all modules. See os/sys/log-conf.h for
 * a list of supported modules. The different log levels are defined in
 * os/sys/log.h:
 *     LOG_LEVEL_NONE         No log
 *     LOG_LEVEL_ERR          Errors
 *     LOG_LEVEL_WARN         Warnings
 *     LOG_LEVEL_INFO         Basic info
 *     LOG_LEVEL_DBG          Detailled debug
  */
/* IEEE802.15.4 PANID */
#define IEEE802154_CONF_PANID 0xb1b3

#define SLIP_ARCH_CONF_USB          0 /** SLIP over UART by default */
#define DBG_CONF_USB                0 /** All debugging over UART by default */
#define UART_CONF_ENABLE            1 /**< Enable/Disable UART I/O */


/* Enable security */
// #define LLSEC802154_CONF_ENABLED 1
/* 6TiSCH minimal schedule length.
 * Larger values result in less frequent active slots: reduces capacity and saves energy. */
// #define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 3
// #define TSCH_CONF_WITH_SIXTOP 1

#define ENERGEST_CONF_ON 1

// #define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_COAP                        LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_LWM2M                       LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_6TOP                        LOG_LEVEL_DBG
// #define TSCH_LOG_CONF_PER_SLOT                     1
/* Enable cooja annotations */
//#define LOG_CONF_WITH_ANNOTATE                1

#endif