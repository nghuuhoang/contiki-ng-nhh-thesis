/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#ifndef WEBSERVER_CONF_CFS_CONNS
#define WEBSERVER_CONF_CFS_CONNS 2
#endif

#ifndef BORDER_ROUTER_CONF_WEBSERVER
#define BORDER_ROUTER_CONF_WEBSERVER 1
#endif

#if BORDER_ROUTER_CONF_WEBSERVER
#define UIP_CONF_TCP 1
#endif

/* IEEE802.15.4 PANID */
#define IEEE802154_CONF_PANID 0x0509
// #define CC2538_RF_CONF_TX_POWER 0x00

/* Do not start TSCH at init, wait for NETSTACK_MAC.on() */
// #define TSCH_CONF_AUTOSTART 0

/* 6TiSCH minimal schedule length.
 * Larger values result in less frequent active slots: reduces capacity and saves energy. */
// #define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 3

// #define USB_SERIAL_CONF_ENABLE		1
// #define SLIP_ARCH_CONF_USB          1 /** SLIP over UART by default */
// #define DBG_CONF_USB                1 /** All debugging over UART by default */

// #if WITH_SECURITY

/* Enable security */
// #define LLSEC802154_CONF_ENABLED 1

// #endif /* WITH_SECURITY */

/*******************************************************/
/************* Other system configuration **************/
/*******************************************************/

/* Logging */
// #define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_COAP                        LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_LWM2M                       LOG_LEVEL_DBG
// #define LOG_CONF_LEVEL_6TOP                        LOG_LEVEL_DBG
//#define TSCH_LOG_CONF_PER_SLOT                     1

#endif /* PROJECT_CONF_H_ */
