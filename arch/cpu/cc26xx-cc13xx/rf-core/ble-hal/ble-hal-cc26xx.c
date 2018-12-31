/*
 * Copyright (c) 2017, Graz University of Technology
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
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
 * \file
 *    BLE radio hardware abstraction implementation for the TI CC26XX controller
 *
 * \author
 *    Michael Spoerk <michael.spoerk@tugraz.at>
 */
/*---------------------------------------------------------------------------*/

#include "lpm.h"

#include "sys/rtimer.h"
#include "sys/process.h"

#include "os/dev/ble-hal.h"
#include "dev/oscillators.h"

#include "ble-addr.h"

#include "net/netstack.h"
#include "net/packetbuf.h"

#include "rf_data_entry.h"
#include "rf-core/rf-core.h"
#include "rf_ble_cmd.h"
#include "lib/random.h"

#include "ioc.h"
#include "ti-lib.h"
#include "inc/hw_types.h"
#include "inc/hw_rfc_dbell.h"

#include <string.h>

#include "rf-core/ble-hal/rf-ble-cmd.h"
/*---------------------------------------------------------------------------*/
#include "sys/log.h"
#define LOG_MODULE "BLE-RADIO"
#define LOG_LEVEL LOG_LEVEL_MAIN
/*---------------------------------------------------------------------------*/
#define CMD_GET_STATUS(X)         (((rfc_radioOp_t *)X)->status)
#define RX_ENTRY_STATUS(X)        (((rfc_dataEntry_t *)X)->status)
#define RX_ENTRY_LENGTH(X)        (((rfc_dataEntry_t *)X)->length)
#define RX_ENTRY_TYPE(X)        (((rfc_dataEntry_t *)X)->config.type)
#define RX_ENTRY_NEXT_ENTRY(X)      (((rfc_dataEntry_t *)X)->pNextEntry)
#define RX_ENTRY_DATA_LENGTH(X)     ((X)[8])
#define RX_ENTRY_DATA_PTR(X)      (&(X)[9])
#define TX_ENTRY_STATUS(X)        RX_ENTRY_STATUS(X)
#define TX_ENTRY_LENGTH(X)        RX_ENTRY_LENGTH(X)
#define TX_ENTRY_TYPE(X)        RX_ENTRY_TYPE(X)
#define TX_ENTRY_NEXT_ENTRY(X)      RX_ENTRY_NEXT_ENTRY(X)
#define TX_ENTRY_FRAME_TYPE(X)      ((X)[8])
#define TX_ENTRY_DATA_PTR(X)      (&(X)[9])
/*---------------------------------------------------------------------------*/
/* LPM                                                                       */
/*---------------------------------------------------------------------------*/
static uint8_t
request(void)
{
  if(rf_core_is_accessible()) {
    return LPM_MODE_SLEEP;
  }

  return LPM_MODE_MAX_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
LPM_MODULE(cc26xx_ble_lpm_module, request, NULL, NULL, LPM_DOMAIN_NONE);
/*---------------------------------------------------------------------------*/
/* timing utilities                                */
#define TIME_UNIT_MS          1000    /* 1000 times per second */
#define TIME_UNIT_0_625_MS        1600    /* 1600 times per second */
#define TIME_UNIT_1_25_MS        800    /*  800 times per second */
#define TIME_UNIT_10_MS          100    /*  100 times per second */
#define TIME_UNIT_RF_CORE      4000000    /*  runs at 4 MHz */
#define TIME_UNIT_RTIMER        RTIMER_SECOND

rtimer_clock_t
ticks_from_unit(uint32_t value, uint32_t unit)
{
  double temp = (((double)value) / unit) * RTIMER_SECOND;
  return (rtimer_clock_t)temp;
}
uint32_t
ticks_to_unit(rtimer_clock_t value, uint32_t unit)
{
  double temp = (((double)value) / RTIMER_SECOND) * unit;
  return (uint32_t)temp;
}
/*---------------------------------------------------------------------------*/
#define CMD_BUFFER_SIZE         24
#define PARAM_BUFFER_SIZE       36
#define OUTPUT_BUFFER_SIZE        24
/*---------------------------------------------------------------------------*/
/* ADVERTISING data structures												 */
#define ADV_RX_BUFFERS_OVERHEAD     8
#define ADV_RX_BUFFERS_DATA_LEN     60
#define ADV_RX_BUFFERS_LEN        (ADV_RX_BUFFERS_OVERHEAD + ADV_RX_BUFFERS_DATA_LEN)
#define ADV_RX_BUFFERS_NUM        2

#define ADV_PREPROCESSING_TIME_TICKS  65

typedef struct {
  /* PARAMETER */
  uint16_t adv_interval;
  ble_adv_type_t adv_type;
  ble_addr_type_t own_addr_type;
  uint8_t channel_map;
  uint8_t adv_data_len;
  uint8_t adv_data[BLE_ADV_DATA_LEN];
  uint8_t scan_rsp_data_len;
  uint8_t scan_rsp_data[BLE_ADV_DATA_LEN];
  /* STATE information */
  uint8_t active;
  rtimer_clock_t start_rt;
  struct rtimer timer;
  /* utility */
  uint8_t cmd_buf[CMD_BUFFER_SIZE];
  uint8_t param_buf[PARAM_BUFFER_SIZE];
  uint8_t output_buf[OUTPUT_BUFFER_SIZE];
  dataQueue_t rx_queue;
  uint8_t rx_buffers[ADV_RX_BUFFERS_NUM][ADV_RX_BUFFERS_LEN];
  uint8_t *rx_queue_current;
} ble_adv_param_t;

static ble_adv_param_t adv_param;
static void advertising_event(struct rtimer *t, void *ptr);
/*---------------------------------------------------------------------------*/
/* CONNECTION data structures                          */
#define BLE_MODE_MAX_CONNECTIONS    1

/* maximum packet length that is transmitted during a single connection event*/
#ifdef BLE_MODE_CONF_CONN_MAX_PACKET_SIZE
#define BLE_MODE_CONN_MAX_PACKET_SIZE   BLE_MODE_CONF_CONN_MAX_PACKET_SIZE
#else
#define BLE_MODE_CONN_MAX_PACKET_SIZE   256
#endif

#define CONN_BLE_BUFFER_SIZE        27  /* maximum size of the data buffer */

#define CONN_RX_BUFFERS_OVERHEAD    8
#define CONN_RX_BUFFERS_DATA_LEN    60
#define CONN_RX_BUFFERS_LEN       (CONN_RX_BUFFERS_OVERHEAD + CONN_RX_BUFFERS_DATA_LEN)
#define CONN_RX_BUFFERS_NUM       12

/* custom status used for tx buffers */
#define DATA_ENTRY_FREE         5
#define DATA_ENTRY_QUEUED       6

#define CONN_TX_BUFFERS_OVERHEAD    9
#define CONN_TX_BUFFERS_DATA_LEN    27
#define CONN_TX_BUFFERS_LEN       (CONN_TX_BUFFERS_OVERHEAD + CONN_TX_BUFFERS_DATA_LEN)
#define CONN_TX_BUFFERS_NUM       12

#define CONN_WIN_SIZE           1
#define CONN_WIN_OFFSET          20

#define CONN_EVENT_LATENCY_THRESHOLD     10
#define CONN_WINDOW_WIDENING_TICKS     30   /* appr. 0.46 ms */
#define CONN_PREPROCESSING_TIME_TICKS 100   /* 1.5 ms */

#define CONN_UPDATE_DELAY         6

typedef struct {
  /* PARAMETER */
  uint8_t peer_address[BLE_ADDR_SIZE];
  uint32_t access_address;
  uint8_t crc_init_0;
  uint8_t crc_init_1;
  uint8_t crc_init_2;
  uint8_t win_size;
  uint16_t win_offset;
  uint16_t interval;
  uint16_t latency;
  uint16_t timeout;
  uint64_t channel_map;
  uint8_t num_used_channels;
  uint8_t hop;
  uint8_t sca;
  rtimer_clock_t timestamp_rt;
  /* STATE information */
  uint8_t active;
  uint16_t counter;
  uint8_t unmapped_channel;
  uint8_t mapped_channel;
  rtimer_clock_t start_rt;
  uint16_t conn_handle;
  struct rtimer timer;
  /* utility */
  uint8_t cmd_buf[CMD_BUFFER_SIZE];
  uint8_t param_buf[PARAM_BUFFER_SIZE];
  uint8_t output_buf[OUTPUT_BUFFER_SIZE];
  dataQueue_t rx_queue;
  uint8_t rx_buffers[CONN_RX_BUFFERS_NUM][CONN_RX_BUFFERS_LEN];
  uint8_t *rx_queue_current;
  dataQueue_t tx_queue;
  uint8_t tx_buffers[CONN_TX_BUFFERS_NUM][CONN_TX_BUFFERS_LEN];
  uint8_t tx_buffers_sent;
  uint16_t skipped_events;
  /* channel map update */
  uint64_t channel_update_channel_map;
  uint16_t channel_update_counter;
  uint8_t channel_update_num_used_channels;
  /* connection parameter update */
  uint8_t conn_update_win_size;
  uint16_t conn_update_win_offset;
  uint16_t conn_update_interval;
  uint16_t conn_update_latency;
  uint16_t conn_update_timeout;
  uint16_t conn_update_counter;
} ble_conn_param_t;

static ble_conn_param_t conn_param[BLE_MODE_MAX_CONNECTIONS];

static uint16_t conn_counter = 0;

static void connection_event_slave(struct rtimer *t, void *ptr);
/*---------------------------------------------------------------------------*/
PROCESS(ble_hal_conn_rx_process, "BLE/CC26xx connection RX process");
process_event_t rx_data_event;
/*---------------------------------------------------------------------------*/
static void
setup_buffers(void)
{
  uint8_t conn_count;
  ble_conn_param_t *conn;
  uint8_t i;
  rfc_dataEntry_t *entry;

  /* setup advertisement RX buffer (circular buffer) */
  memset(&adv_param, 0x00, sizeof(ble_adv_param_t));
  memset(&adv_param.rx_queue, 0x00, sizeof(adv_param.rx_queue));
  adv_param.rx_queue.pCurrEntry = adv_param.rx_buffers[0];
  adv_param.rx_queue.pLastEntry = NULL;
  adv_param.rx_queue_current = adv_param.rx_buffers[0];
  for(i = 0; i < ADV_RX_BUFFERS_NUM; i++) {
    memset(&adv_param.rx_buffers[i], 0x00, ADV_RX_BUFFERS_LEN);
    entry = (rfc_dataEntry_t *)adv_param.rx_buffers[i];
    entry->pNextEntry = adv_param.rx_buffers[(i + 1) % ADV_RX_BUFFERS_NUM];
    entry->config.lenSz = 1;
    entry->length = ADV_RX_BUFFERS_DATA_LEN;
  }

  memset(conn_param, 0x00, sizeof(ble_conn_param_t) * BLE_MODE_MAX_CONNECTIONS);
  for(conn_count = 0; conn_count < BLE_MODE_MAX_CONNECTIONS; conn_count++) {
    /* setup connection RX buffer (circular buffer) */
    conn = &conn_param[conn_count];
    memset(&conn->rx_queue, 0x00, sizeof(conn->rx_queue));
    conn->rx_queue.pCurrEntry = conn->rx_buffers[0];
    conn->rx_queue.pLastEntry = NULL;
    conn->rx_queue_current = conn->rx_buffers[0];

    for(i = 0; i < CONN_RX_BUFFERS_NUM; i++) {
      memset(&conn->rx_buffers[i], 0x00, CONN_RX_BUFFERS_LEN);
      entry = (rfc_dataEntry_t *)conn->rx_buffers[i];
      entry->pNextEntry = conn->rx_buffers[(i + 1) % CONN_RX_BUFFERS_NUM];
      entry->config.lenSz = 1;
      entry->length = CONN_RX_BUFFERS_DATA_LEN;
    }

    /* setup connection TX buffer (buffers are added on demand to the queue) */
    memset(&conn->tx_queue, 0x00, sizeof(conn->tx_queue));
    conn->tx_queue.pCurrEntry = NULL;
    conn->tx_queue.pLastEntry = NULL;

    for(i = 0; i < CONN_TX_BUFFERS_NUM; i++) {
      memset(&conn->tx_buffers[i], 0x00, CONN_TX_BUFFERS_LEN);
      entry = (rfc_dataEntry_t *)conn->tx_buffers[i];
      entry->config.lenSz = 1;
      entry->status = DATA_ENTRY_FREE;
    }
  }
}
/*---------------------------------------------------------------------------*/
static ble_conn_param_t *
get_connection_for_handle(uint8_t conn_handle)
{
  uint8_t i;
  for(i = 0; i < BLE_MODE_MAX_CONNECTIONS; i++) {
    if(conn_param[i].conn_handle == conn_handle) {
      return &conn_param[i];
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static uint8_t *
tx_queue_get_buffer(ble_conn_param_t *param)
{
  uint8_t i;
  rfc_dataEntry_t *entry;
  for(i = 0; i < CONN_TX_BUFFERS_NUM; i++) {
    entry = (rfc_dataEntry_t *)param->tx_buffers[i];
    if(entry->status == DATA_ENTRY_FREE) {
      return (uint8_t *)entry;
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static uint16_t
tx_queue_count_free_buffers(ble_conn_param_t *param)
{
  uint16_t i;
  uint16_t free_bufs = 0;
  for(i = 0; i < CONN_TX_BUFFERS_NUM; i++) {
    if(TX_ENTRY_STATUS(param->tx_buffers[i]) == DATA_ENTRY_FREE) {
      free_bufs++;
    }
  }
  return free_bufs;
}
/*---------------------------------------------------------------------------*/
static uint8_t
tx_queue_data_to_transmit(ble_conn_param_t *param)
{
  uint16_t i;
  for(i = 0; i < CONN_TX_BUFFERS_NUM; i++) {
    if(TX_ENTRY_STATUS(param->tx_buffers[i]) == DATA_ENTRY_QUEUED) {
      return 1;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
ble_result_t
on(void)
{
  oscillators_request_hf_xosc();
  if(!rf_core_is_accessible()) {
    /* boot the rf core */
    if(rf_core_boot() != RF_CORE_CMD_OK) {
      LOG_ERR("ble_controller_reset() could not boot rf-core\n");
      return BLE_RESULT_ERROR;
    }

    rf_core_setup_interrupts(0);
    oscillators_switch_to_hf_xosc();

    if(rf_ble_cmd_setup_ble_mode() != RF_BLE_CMD_OK) {
      LOG_ERR("could not setup rf-core to BLE mode\n");
      return BLE_RESULT_ERROR;
    }
  }
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
void
off(void)
{
  rf_core_power_down();
  oscillators_switch_to_hf_rc();
}
/*---------------------------------------------------------------------------*/
static ble_result_t
reset(void)
{
  LOG_INFO("maximum connections: %4d\n", BLE_MODE_MAX_CONNECTIONS);
  LOG_INFO("max. packet length:  %4d\n", BLE_MODE_CONN_MAX_PACKET_SIZE);
  lpm_register_module(&cc26xx_ble_lpm_module);
  rf_core_set_modesel();
  setup_buffers();
  if(on() != BLE_RESULT_OK) {
    return BLE_RESULT_ERROR;
  }
  off();
  if(!process_is_running(&ble_hal_conn_rx_process)) {
    rx_data_event = process_alloc_event();
    process_start(&ble_hal_conn_rx_process, NULL);
  }
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
read_bd_addr(uint8_t *addr)
{
  ble_addr_cpy_to(addr);
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
read_buffer_size(unsigned int *buf_len, unsigned int *num_buf)
{
  uint16_t i;
  uint16_t ll_buffers = CONN_TX_BUFFERS_NUM;
  uint16_t packet_buffers;
  uint16_t buffer_size;
  for(i = 0; i < conn_counter; i++) {
    ll_buffers = MIN(ll_buffers, tx_queue_count_free_buffers(&conn_param[i]));
  }
  packet_buffers = ll_buffers / (BLE_MODE_CONN_MAX_PACKET_SIZE / CONN_BLE_BUFFER_SIZE);
  buffer_size = BLE_MODE_CONN_MAX_PACKET_SIZE;
  memcpy(buf_len, &buffer_size, 2);
  memcpy(num_buf, &packet_buffers, 2);
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
set_adv_param(unsigned int adv_int, ble_adv_type_t type,
              ble_addr_type_t own_type, unsigned short adv_map)
{
  adv_param.adv_interval = adv_int;
  adv_param.adv_type = type;
  adv_param.own_addr_type = own_type;
  adv_param.channel_map = adv_map;

  LOG_INFO("advertising parameter: interval: %4d, channels: %2d\n",
           adv_param.adv_interval, adv_param.channel_map);

  LOG_DBG("interval: %16u (ms)\n", adv_param.adv_interval);
  LOG_DBG("type:     %16u\n", adv_param.adv_type);
  LOG_DBG("addr_type:%16u\n", adv_param.own_addr_type);
  LOG_DBG("channels: %16u\n", adv_param.channel_map);

  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
read_adv_channel_tx_power(short *power)
{
  return BLE_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
set_adv_data(unsigned short data_len, char *data)
{
  if(data_len > BLE_ADV_DATA_LEN) {
    LOG_WARN("BLE-HAL: adv_data too long\n");
    return BLE_RESULT_INVALID_PARAM;
  }
  adv_param.adv_data_len = data_len;
  memcpy(adv_param.adv_data, data, data_len);
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
set_scan_resp_data(unsigned short data_len, char *data)
{
  if(data_len > BLE_SCAN_RESP_DATA_LEN) {
    LOG_WARN("BLE-HAL: scan_resp_data too long\n");
    return BLE_RESULT_INVALID_PARAM;
  }
  adv_param.scan_rsp_data_len = data_len;
  memcpy(adv_param.scan_rsp_data, data, data_len);
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
set_adv_enable(unsigned short enable)
{
  uint32_t now = RTIMER_NOW();
  if((enable) && (!adv_param.active)) {
    adv_param.start_rt = now + ticks_from_unit(adv_param.adv_interval,
                                               TIME_UNIT_1_25_MS);
    rtimer_set(&adv_param.timer, adv_param.start_rt,
               0, advertising_event, (void *)&adv_param);
  }
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
send_frame(ble_conn_param_t *conn, uint8_t *data, uint8_t data_len, uint8_t frame_type)
{
  uint8_t *tx_buffer = tx_queue_get_buffer(conn);
  if(tx_buffer == NULL) {
    LOG_WARN("BLE-HAL: send_frame: no TX buffer available (conn_handle: 0x%04X)\n", conn->conn_handle);
    return BLE_RESULT_ERROR;
  }
  if(data_len > CONN_BLE_BUFFER_SIZE) {
    LOG_WARN("BLE-HAL: send_frame: data too long (%d bytes)\n", data_len);
    return BLE_RESULT_ERROR;
  }

  memcpy(TX_ENTRY_DATA_PTR(tx_buffer), data, data_len);
  TX_ENTRY_LENGTH(tx_buffer) = data_len + 1;
  TX_ENTRY_STATUS(tx_buffer) = DATA_ENTRY_QUEUED;
  TX_ENTRY_FRAME_TYPE(tx_buffer) = frame_type;
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
connection_update(unsigned int connection_handle, unsigned int conn_interval,
                  unsigned int conn_latency, unsigned int supervision_timeout)
{
  uint8_t len = 0;
  uint8_t data[24];
  ble_conn_param_t *conn = get_connection_for_handle(connection_handle);

  if(conn == NULL) {
    return BLE_RESULT_ERROR;
  }

  LOG_INFO("connection_update: handle: 0x%04X, interval: %4d, latency: %2d, timeout: %4d\n",
           connection_handle, conn_interval, conn_latency, supervision_timeout);
#if UIP_CONF_ROUTER
  uint16_t instant = conn->counter + CONN_UPDATE_DELAY;
  /* prepare connection update packet */
  data[0] = BLE_LL_CONN_UPDATE_REQ;
  data[1] = conn->win_size;
  data[2] = 0;
  data[3] = 0;
  memcpy(&data[4], &conn_interval, 2);
  memcpy(&data[6], &conn_latency, 2);
  memcpy(&data[8], &supervision_timeout, 2);
  memcpy(&data[10], &instant, 2);
  len = 12;
  /* set new connection */
  conn->conn_update_win_size = conn->win_size;
  conn->conn_update_interval = conn_interval;
  conn->conn_update_latency = conn_latency;
  conn->conn_update_timeout = supervision_timeout;
  conn->conn_update_counter = instant;

  if(send_frame(conn, data, len, BLE_DATA_PDU_LLID_CONTROL) != BLE_RESULT_OK) {
    LOG_ERR("connection_update: send frame was NOT successful\n");
    return BLE_RESULT_ERROR;
  }
#else
  data[0] = BLE_LL_CONN_PARAM_REQ;
  memcpy(&data[1], &conn_interval, 2);  /* interval min */
  memcpy(&data[3], &conn_interval, 2);  /* interval max */
  memcpy(&data[5], &conn_latency, 2);   /* latency */
  memcpy(&data[7], &supervision_timeout, 2);    /* supervision timeout */
  memcpy(&data[9], &conn_interval, 1);  /* preferred periodicity */
  memcpy(&data[10], &conn->counter, 2); /* referenc conn event count */
  memset(&data[12], 0xFF, 12);      /* offset 0 to 5 */
  len = 24;

  if(send_frame(conn, data, len, BLE_DATA_PDU_LLID_CONTROL) != BLE_RESULT_OK) {
    LOG_ERR("connection_update: send frame was NOT successful\n");
    return BLE_RESULT_ERROR;
  }
#endif
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
send(void *buf, unsigned short buf_len)
{
  uint16_t loop_data;
  uint16_t loop_conn;
  ble_conn_param_t *conn;
  uint8_t *data;
  uint16_t data_len;
  linkaddr_t dest_addr;
  linkaddr_t conn_addr;
  uint8_t result;

  linkaddr_copy(&dest_addr, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));

  LOG_DBG("ble-hal: sending %d bytes\n", buf_len);

  for(loop_conn = 0; loop_conn < conn_counter; loop_conn++) {
    conn = &conn_param[loop_conn];
    ble_addr_to_eui64(conn_addr.u8, conn->peer_address);
    if((linkaddr_cmp(&dest_addr, &linkaddr_null) != 0) || (linkaddr_cmp(&dest_addr, &conn_addr) != 0)) {
      for(loop_data = 0; loop_data < buf_len; loop_data += CONN_BLE_BUFFER_SIZE) {
        data = &((uint8_t *)buf)[loop_data];
        data_len = MIN((buf_len - loop_data), CONN_BLE_BUFFER_SIZE);
        if(loop_data == 0) {
          result = send_frame(conn, data, data_len, BLE_DATA_PDU_LLID_DATA_MESSAGE);
        } else {
          result = send_frame(conn, data, data_len, BLE_DATA_PDU_LLID_DATA_FRAGMENT);
        }
        if(result != BLE_RESULT_OK) {
          LOG_WARN("ble-hal: send was unsuccessful\n");
          return result;
        }
      }
    }
  }
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static ble_result_t
read_connection_interval(unsigned int conn_handle, unsigned int *conn_interval)
{
  ble_conn_param_t *conn = get_connection_for_handle(conn_handle);
  if(conn == NULL) {
    memset(conn_interval, 0x00, sizeof(uint16_t));
    return BLE_RESULT_ERROR;
  }
  memcpy(conn_interval, &conn->interval, sizeof(uint16_t));
  return BLE_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
const struct ble_hal_driver ble_hal =
{
  reset,
  read_bd_addr,
  read_buffer_size,
  set_adv_param,
  read_adv_channel_tx_power,
  set_adv_data,
  set_scan_resp_data,
  set_adv_enable,
  NULL,
  NULL,
  NULL,
  NULL,
  connection_update,
  NULL,
  send,
  NULL,
  read_connection_interval
};
/*---------------------------------------------------------------------------*/
static void
advertising_rx(ble_adv_param_t *param)
{
  uint8_t i;
  uint8_t offset = 14;
  uint8_t *rx_data;
  ble_conn_param_t *c_param = &conn_param[0];
  rtimer_clock_t wakeup;

  while(RX_ENTRY_STATUS(param->rx_queue_current) == DATA_ENTRY_FINISHED) {
    rx_data = RX_ENTRY_DATA_PTR(param->rx_queue_current);

    if(CMD_GET_STATUS(param->cmd_buf) == RF_CORE_RADIO_OP_STATUS_BLE_DONE_CONNECT) {
      /* parsing connection parameter */
      for(i = 0; i < BLE_ADDR_SIZE; i++) {
        c_param->peer_address[i] = rx_data[BLE_ADDR_SIZE + 1 - i];
      }
      memcpy(&c_param->access_address, &rx_data[offset], 4);
      memcpy(&c_param->crc_init_0, &rx_data[offset + 4], 1);
      memcpy(&c_param->crc_init_1, &rx_data[offset + 5], 1);
      memcpy(&c_param->crc_init_2, &rx_data[offset + 6], 1);
      memcpy(&c_param->win_size, &rx_data[offset + 7], 1);
      memcpy(&c_param->win_offset, &rx_data[offset + 8], 2);
      memcpy(&c_param->interval, &rx_data[offset + 10], 2);
      memcpy(&c_param->latency, &rx_data[offset + 12], 2);
      memcpy(&c_param->timeout, &rx_data[offset + 14], 2);
      memcpy(&c_param->channel_map, &rx_data[offset + 16], 5);
      memcpy(&c_param->hop, &rx_data[offset + 21], 1);
      memcpy(&c_param->sca, &rx_data[offset + 21], 1);
      memcpy(&c_param->timestamp_rt, &rx_data[offset + 24], 4);

      /* convert all received timing values to rtimer ticks */

      c_param->timestamp_rt = ticks_from_unit(c_param->timestamp_rt, TIME_UNIT_RF_CORE);
      c_param->hop = c_param->hop & 0x1F;
      c_param->sca = (c_param->sca >> 5) & 0x07;

      LOG_INFO("connection created: conn_int: %4u, latency: %3u, channel_map: %8llX\n",
               c_param->interval, c_param->latency, c_param->channel_map);

      LOG_DBG("access address: 0x%08lX\n", c_param->access_address);
      LOG_DBG("crc0: 0x%02X\n", c_param->crc_init_0);
      LOG_DBG("crc1: 0x%02X\n", c_param->crc_init_1);
      LOG_DBG("crc2: 0x%02X\n", c_param->crc_init_2);
      LOG_DBG("win_size:       %4u\n", c_param->win_size);
      LOG_DBG("win_offset:     %4u\n", c_param->win_offset);
      LOG_DBG("interval:       %4u\n", c_param->interval);
      LOG_DBG("latency:        %4u\n", c_param->latency);
      LOG_DBG("timeout:        %4u\n", c_param->timeout);
      LOG_DBG("channel_map:    %llX\n", c_param->channel_map);

      /* calculate the first anchor point
       * (add an interval, because we skip the first connection event ) */
      wakeup = c_param->timestamp_rt + ticks_from_unit(c_param->win_offset, TIME_UNIT_1_25_MS) - CONN_WINDOW_WIDENING_TICKS;
      wakeup += ticks_from_unit(c_param->interval, TIME_UNIT_1_25_MS) - CONN_PREPROCESSING_TIME_TICKS;
      rtimer_set(&c_param->timer, wakeup, 0, connection_event_slave, (void *)c_param);

      /* initialization for the connection */
      c_param->counter = 0;
      c_param->unmapped_channel = 0;
      c_param->conn_handle = conn_counter;
      c_param->active = 1;
      conn_counter++;
      LOG_INFO("BLE-HAL: connection (0x%04X) created\n", c_param->conn_handle);
    }

    /* free current entry (clear BLE data length & reset status) */
    RX_ENTRY_DATA_LENGTH(param->rx_queue_current) = 0;
    RX_ENTRY_STATUS(param->rx_queue_current) = DATA_ENTRY_PENDING;
    param->rx_queue_current = RX_ENTRY_NEXT_ENTRY(param->rx_queue_current);
  }
}
/*---------------------------------------------------------------------------*/
static void
advertising_event(struct rtimer *t, void *ptr)
{
  ble_adv_param_t *param = (ble_adv_param_t *)ptr;
  uint32_t wakeup;

  if(on() != BLE_RESULT_OK) {
    LOG_ERR("BLE-HAL: advertising event: could not enable rf core\n");
    return;
  }

  rf_ble_cmd_create_adv_params(param->param_buf, &param->rx_queue,
                               param->adv_data_len, param->adv_data,
                               param->scan_rsp_data_len, param->scan_rsp_data,
                               param->own_addr_type, (uint8_t *)BLE_ADDR_LOCATION);

  /* advertising on advertisement channel 1*/
  if(param->channel_map & BLE_ADV_CHANNEL_1_MASK) {
    rf_ble_cmd_create_adv_cmd(param->cmd_buf, BLE_ADV_CHANNEL_1,
                              param->param_buf, param->output_buf);
    rf_ble_cmd_send(param->cmd_buf);
    rf_ble_cmd_wait(param->cmd_buf);
  }

  off();
  advertising_rx(param);

  if(conn_param[0].active == 1) {
    LOG_INFO("stop advertising\n");
    return;
  }

  param->start_rt = param->start_rt + ticks_from_unit(param->adv_interval, TIME_UNIT_MS);
  wakeup = adv_param.start_rt - ADV_PREPROCESSING_TIME_TICKS;
  rtimer_set(&param->timer, wakeup, 0, advertising_event, (void *)param);
}
/*---------------------------------------------------------------------------*/
static void
update_data_channel(ble_conn_param_t *param)
{
  uint8_t i;
  uint8_t j;
  uint8_t remap_index;
  /* perform the data channel selection according to BLE standard */

  /* calculate unmapped channel*/
  param->unmapped_channel = (param->unmapped_channel + param->hop) % (BLE_DATA_CHANNEL_MAX + 1);

  /* map the calculated channel */
  if(param->channel_map & (1ULL << param->unmapped_channel)) {
    /* channel is marked as used */
    param->mapped_channel = param->unmapped_channel;
  } else {
    remap_index = param->unmapped_channel % param->num_used_channels;
    j = 0;
    for(i = 0; i < (BLE_DATA_CHANNEL_MAX + 1); i++) {
      if(param->channel_map & (1ULL << i)) {
        if(j == remap_index) {
          param->mapped_channel = i;
        }
        j++;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
process_ll_ctrl_msg(ble_conn_param_t *conn, uint8_t input_len, uint8_t *input, uint8_t *output_len, uint8_t *output)
{
  uint8_t op_code = input[0];
  uint16_t interval;
  uint16_t latency;
  uint16_t timeout;
  uint64_t channel_map = 0;
  uint16_t instant = 0;
  uint8_t i;

  if(op_code == BLE_LL_CONN_UPDATE_REQ) {
    LOG_INFO("BLE-HAL: connection update request received\n");
    memcpy(&conn->conn_update_win_size, &input[1], 1);
    memcpy(&conn->conn_update_win_offset, &input[2], 2);
    memcpy(&conn->conn_update_interval, &input[4], 2);
    memcpy(&conn->conn_update_latency, &input[6], 2);
    memcpy(&conn->conn_update_timeout, &input[8], 2);
    memcpy(&conn->conn_update_counter, &input[10], 2);
  } else if(op_code == BLE_LL_CHANNEL_MAP_REQ) {
    LOG_INFO("BLE-HAL: channel map update received\n");
    memcpy(&channel_map, &input[1], 5);
    memcpy(&instant, &input[6], 2);

    conn->channel_update_channel_map = channel_map;
    conn->channel_update_counter = instant;
    conn->channel_update_num_used_channels = 0;
    for(i = 0; i <= BLE_DATA_CHANNEL_MAX; i++) {
      if(channel_map & (1ULL << i)) {
        conn->channel_update_num_used_channels++;
      }
    }
  } else if(op_code == BLE_LL_FEATURE_REQ) {
    LOG_INFO("BLE-HAL: feature request received\n");
    output[0] = BLE_LL_FEATURE_RSP;
    memset(&output[1], 0x00, 8);
    *output_len = 9;
  } else if(op_code == BLE_LL_VERSION_IND) {
    LOG_INFO("BLE-HAL: version request received\n");
    output[0] = BLE_LL_VERSION_IND;
    output[1] = 7;
    memset(&output[2], 0xAA, 4);
    *output_len = 6;
  } else if(op_code == BLE_LL_CONN_PARAM_REQ) {
    LOG_INFO("BLE-HAL: connection parameter request received\n");
    memcpy(&interval, &input[1], 2); /* use interval min */
    memcpy(&latency, &input[5], 2);
    memcpy(&timeout, &input[7], 2);
    connection_update(conn->conn_handle, interval, latency, timeout);
  } else {
    LOG_WARN("BLE-HAL: unknown LL control code: %02X\n", op_code);
  }
}
/*---------------------------------------------------------------------------*/
static void
connection_rx(ble_conn_param_t *param)
{
  uint8_t header_offset = 2;
  uint8_t *rx_data;
  uint16_t len;
  uint8_t channel;
  uint8_t frame_type;
  uint8_t more_data;
  uint8_t rssi;
  linkaddr_t sender_addr;
  rfc_bleMasterSlaveOutput_t *out_buf = (rfc_bleMasterSlaveOutput_t *)param->output_buf;

  uint8_t output_len = 0;
  uint8_t output[26];

  while(RX_ENTRY_STATUS(param->rx_queue_current) == DATA_ENTRY_FINISHED) {
    rx_data = RX_ENTRY_DATA_PTR(param->rx_queue_current);
    len = RX_ENTRY_DATA_LENGTH(param->rx_queue_current) - 6 - 2;  /* last 8 bytes are status, timestamp, ... */
    channel = (rx_data[len + 3] & 0x3F);
    frame_type = rx_data[0] & 0x03;
    more_data = (rx_data[0] & 0x10) >> 4;

    if(frame_type == BLE_DATA_PDU_LLID_CONTROL) {
      process_ll_ctrl_msg(param, (len - header_offset), &rx_data[header_offset], &output_len, output);
      if(output_len > 0) {
        send_frame(param, output, output_len, BLE_DATA_PDU_LLID_CONTROL);
      }
    } else if(frame_type == BLE_DATA_PDU_LLID_DATA_MESSAGE) {
      packetbuf_clear();
      memcpy(packetbuf_dataptr(), &rx_data[header_offset], len);
      packetbuf_set_datalen(len);
      rssi = out_buf->lastRssi;
      ble_addr_to_eui64(sender_addr.u8, param->peer_address);
      packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rssi);
      packetbuf_set_attr(PACKETBUF_ATTR_CHANNEL, channel);
      packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_node_addr);
      packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &sender_addr);
      packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME_BLE_RX_EVENT);
      if((!more_data) || (len < CONN_BLE_BUFFER_SIZE)) {
        NETSTACK_MAC.input();
      }
    } else if(frame_type == BLE_DATA_PDU_LLID_DATA_FRAGMENT) {
      memcpy((packetbuf_dataptr() + packetbuf_datalen()), &rx_data[header_offset], len);
      packetbuf_set_datalen(packetbuf_datalen() + len);
      if((!more_data) || (len < CONN_BLE_BUFFER_SIZE)) {
        NETSTACK_MAC.input();
      }
    }

    /* free current entry (clear BLE data length & reset status) */
    RX_ENTRY_DATA_LENGTH(param->rx_queue_current) = 0;
    RX_ENTRY_STATUS(param->rx_queue_current) = DATA_ENTRY_PENDING;
    param->rx_queue_current = RX_ENTRY_NEXT_ENTRY(param->rx_queue_current);
  }
}
/*---------------------------------------------------------------------------*/
static void
connection_event_slave(struct rtimer *t, void *ptr)
{

  ble_conn_param_t *conn = (ble_conn_param_t *)ptr;
  rfc_bleMasterSlaveOutput_t *output = (rfc_bleMasterSlaveOutput_t *)conn->output_buf;
  uint8_t first_packet = 0;
  rtimer_clock_t wakeup;
  uint8_t i;
  uint8_t tx_data = tx_queue_data_to_transmit(conn);

  if(conn->counter == 0) {
    /* the slave skips connection event 0, because it is usually too early */
    conn->start_rt = conn->timestamp_rt + ticks_from_unit(conn->win_offset, TIME_UNIT_1_25_MS) - CONN_WINDOW_WIDENING_TICKS;
    update_data_channel(conn);
    first_packet = 1;
  }
  conn->counter++;

  /* connection timing */
  if(conn->counter == conn->conn_update_counter) {
    conn->start_rt += ticks_from_unit(conn->interval + conn->conn_update_win_offset, TIME_UNIT_1_25_MS);

    conn->win_size = conn->conn_update_win_size;
    conn->win_offset = conn->conn_update_win_offset;
    conn->interval = conn->conn_update_interval;
    conn->latency = conn->conn_update_latency;
    conn->timeout = conn->conn_update_timeout;
    conn->conn_update_win_size = 0;
    conn->conn_update_win_offset = 0;
    conn->conn_update_interval = 0;
    conn->conn_update_latency = 0;
    conn->conn_update_timeout = 0;
  } else if(output->pktStatus.bTimeStampValid) {
    conn->start_rt = ticks_from_unit(output->timeStamp, TIME_UNIT_RF_CORE) +
      ticks_from_unit(conn->interval, TIME_UNIT_1_25_MS) - CONN_WINDOW_WIDENING_TICKS;
  } else {
    conn->start_rt += ticks_from_unit(conn->interval, TIME_UNIT_1_25_MS);
  }

  /* connection channel */
  if(conn->channel_update_counter == conn->counter) {
    conn->channel_map = conn->channel_update_channel_map;
    conn->num_used_channels = conn->channel_update_num_used_channels;
    conn->channel_update_counter = 0;
    conn->channel_update_channel_map = 0;
    conn->channel_update_num_used_channels = 0;
  }
  update_data_channel(conn);

  if(tx_data || (conn->skipped_events >= conn->latency) || (conn->counter < CONN_EVENT_LATENCY_THRESHOLD)) {
    /* participating in the connection event */
    conn->skipped_events = 0;
    rf_ble_cmd_create_slave_params(conn->param_buf, &conn->rx_queue, &conn->tx_queue, conn->access_address,
                                   conn->crc_init_0, conn->crc_init_1, conn->crc_init_2,
                                   ticks_to_unit(ticks_from_unit(conn->win_size, TIME_UNIT_1_25_MS), TIME_UNIT_RF_CORE),
                                   ticks_to_unit(CONN_WINDOW_WIDENING_TICKS, TIME_UNIT_RF_CORE), first_packet);

    rf_ble_cmd_create_slave_cmd(conn->cmd_buf, conn->mapped_channel, conn->param_buf, conn->output_buf,
                                ticks_to_unit(conn->start_rt, TIME_UNIT_RF_CORE));

    if(on() != BLE_RESULT_OK) {
      LOG_ERR("connection_event: could not enable radio core\n");
      return;
    }

    /* append TX buffers */
    for(i = 0; i < CONN_TX_BUFFERS_NUM; i++) {
      if(TX_ENTRY_STATUS(conn->tx_buffers[i]) == DATA_ENTRY_QUEUED) {
        TX_ENTRY_STATUS(conn->tx_buffers[i]) = DATA_ENTRY_PENDING;
        rf_ble_cmd_add_data_queue_entry(&conn->tx_queue, conn->tx_buffers[i]);
      }
    }
    rf_ble_cmd_send(conn->cmd_buf);
    rf_ble_cmd_wait(conn->cmd_buf);
    off();

    if(CMD_GET_STATUS(conn->cmd_buf) != RF_CORE_RADIO_OP_STATUS_BLE_DONE_OK) {
      LOG_DBG("command status: 0x%04X; connection event counter: %d, channel: %d\n",
              CMD_GET_STATUS(conn->cmd_buf), conn->counter, conn->mapped_channel);
    }

    /* free finished TX buffers */
    for(i = 0; i < CONN_TX_BUFFERS_NUM; i++) {
      if(TX_ENTRY_STATUS(conn->tx_buffers[i]) == DATA_ENTRY_FINISHED) {
        TX_ENTRY_STATUS(conn->tx_buffers[i]) = DATA_ENTRY_FREE;
        TX_ENTRY_LENGTH(conn->tx_buffers[i]) = 0;
        TX_ENTRY_NEXT_ENTRY(conn->tx_buffers[i]) = NULL;
      }
    }
  } else {
    /* skipping connection event */
    conn->skipped_events++;
    output->pktStatus.bTimeStampValid = 0;
  }
  wakeup = conn->start_rt + ticks_from_unit(conn->interval, TIME_UNIT_1_25_MS) - CONN_PREPROCESSING_TIME_TICKS;
  rtimer_set(&conn->timer, wakeup, 0, connection_event_slave, ptr);
  process_post(&ble_hal_conn_rx_process, rx_data_event, ptr);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ble_hal_conn_rx_process, ev, data) {
  ble_conn_param_t *conn = (ble_conn_param_t *)data;
  rfc_bleMasterSlaveOutput_t *output = (rfc_bleMasterSlaveOutput_t *)conn->output_buf;
  uint8_t tx_buffers_sent;
  PROCESS_BEGIN();
  LOG_DBG("BLE-HAL: conn rx process start\n");

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == rx_data_event);
    /* notify upper layers (L2CAP) when TX buffers were successfully transmitted */
    tx_buffers_sent = output->nTxEntryDone - conn->tx_buffers_sent;
    if(tx_buffers_sent != 0) {
      conn->tx_buffers_sent = output->nTxEntryDone;
      packetbuf_set_datalen(0);
      packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME_BLE_TX_EVENT);
      NETSTACK_MAC.input();
    }

    /* handle RX buffers */
    connection_rx(conn);

    /* generate an event if the connection parameter were updated */
    if(conn->counter == conn->conn_update_counter) {
      packetbuf_set_datalen(0);
      packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME_BLE_CONNECTION_UPDATED);
      NETSTACK_MAC.input();
    }
  }

  PROCESS_END();
}
