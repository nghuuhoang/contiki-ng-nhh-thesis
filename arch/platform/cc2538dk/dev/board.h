
/**
 *		nghuu.hoang@outlook.com
 *
 *
 * This file provides connectivity information on LEDs, Buttons, UART 
 *
 * @{
 *
 *
 * \note   Do not include this file directly. It gets included by contiki-conf
 *         after all relevant directives have been set.
 */
#ifndef BOARD_H_
#define BOARD_H_

#include "dev/gpio.h"
#include "dev/nvic.h"
/*---------------------------------------------------------------------------*/
/** \name SmartRF LED configuration
 *
 * LEDs on the SmartRF06 (EB and BB) are connected as follows:
 * - LED1 (Green)    -> PC0
 * - LED2 (Red) -> PC1
 * - LED3 (Blue)  -> PC2
 *
 * @{
 */
/*---------------------------------------------------------------------------*/
#define LEDS_CONF_GREEN            0
#define LEDS_CONF_RED              1
#define LEDS_CONF_BLUE	           2


#define LEDS_ARCH_L1_PORT GPIO_C_NUM
#define LEDS_ARCH_L1_PIN           1
#define LEDS_ARCH_L2_PORT GPIO_C_NUM
#define LEDS_ARCH_L2_PIN           2
#define LEDS_ARCH_L3_PORT GPIO_C_NUM
#define LEDS_ARCH_L3_PIN           3


#define LEDS_CONF_COUNT            3


/*Shield*/
#define RELAY_1_PIN			21	
#define RELAY_2_PIN			22

#define LED_ON_SHIELD_0 8
#define LED_ON_SHIELD_1 9
#define LED_ON_SHIELD_2 10
#define LED_ON_SHIELD_3 11
#define LED_ON_SHIELD_4 12
#define LED_ON_SHIELD_5 13

/** @} */
/*---------------------------------------------------------------------------*/
/** \name USB configuration
 *
 * The USB pullup is driven by PC0 and is shared with LED1
 */
#define USB_PULLUP_PORT         GPIO_C_NUM
#define USB_PULLUP_PIN          0
/** @} */
/*---------------------------------------------------------------------------*/
/** \name UART configuration
 *
 * The UART (XDS back channel) is connected to the
 * following ports/pins
 * - RX:  PA0
 * - TX:  PA1
 * - CTS: PB0 (Can only be used with UART1)
 * - RTS: PD3 (Can only be used with UART1)
 *
 * We configure the port to use UART0. To use UART1, replace UART0_* with
 * UART1_* below.
 * @{
 */
#define UART0_RX_PORT           GPIO_A_NUM
#define UART0_RX_PIN            0

#define UART0_TX_PORT           GPIO_A_NUM
#define UART0_TX_PIN            1

#define UART1_CTS_PORT          GPIO_B_NUM
#define UART1_CTS_PIN           0

#define UART1_RTS_PORT          GPIO_D_NUM
#define UART1_RTS_PIN           3
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SmartRF Button configuration
 *
 * Buttons 
 * - BUTTON_SELECT -> PA3
 * @{
 */
/** BUTTON_SELECT -> PA3 */
#define BUTTON_SELECT_PORT      GPIO_A_NUM
#define BUTTON_SELECT_PIN       7
#define BUTTON_SELECT_VECTOR    GPIO_A_IRQn

/* Notify various examples that we have Buttons */
#define PLATFORM_HAS_BUTTON     1
#define PLATFORM_SUPPORTS_BUTTON_HAL 1
/** @} */

/*---------------------------------------------------------------------------*/
/**
 * \name SPI configuration
 *
 * These values configure which CC2538 pins to use for the SPI lines. Both
 * SPI instances can be used independently by providing the corresponding
 * port / pin macros.
 * @{
 */


#define I2C_SDA_PORT GPIO_A_NUM
#define I2C_SDA_PIN  6

#define I2C_SCL_PORT GPIO_A_NUM
#define I2C_SCL_PIN 5

#define I2C_INT_PORT             GPIO_C_NUM
#define I2C_INT_PIN              7
#define I2C_INT_VECTOR           GPIO_C_IRQn




/**
 * \name CC2538 TSCH configuration
 *
 * @{
 */
#define RADIO_PHY_OVERHEAD        CC2538_PHY_OVERHEAD
#define RADIO_BYTE_AIR_TIME       CC2538_BYTE_AIR_TIME
#define RADIO_DELAY_BEFORE_TX     CC2538_DELAY_BEFORE_TX
#define RADIO_DELAY_BEFORE_RX     CC2538_DELAY_BEFORE_RX
#define RADIO_DELAY_BEFORE_DETECT CC2538_DELAY_BEFORE_DETECT
/** @} */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 * \name Device string used on startup
 * @{
 */
#define BOARD_STRING "cc2538dk by nghuu.hoang@outlook.com"
/** @} */

#endif /* BOARD_H_ */

/**
 * @}
 * @}
 */
