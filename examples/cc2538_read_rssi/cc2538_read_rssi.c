
#include "contiki.h"

#include "dev/cc2538-rf.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "net/packetbuf.h"

#include <stdio.h> /* For printf() */
#include "sys/log.h"
#define LOG_MODULE "RPL BR"
#define LOG_LEVEL LOG_LEVEL_INFO

/*---------------------------------------------------------------------------*/
PROCESS(read_rssi_process, "read rssi process");
AUTOSTART_PROCESSES(&read_rssi_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(read_rssi_process, ev, data)
{
  static struct etimer timer;
  radio_value_t rssi_value;
  PROCESS_BEGIN();

  /* Setup a periodic timer that expires after 10 seconds. */
  etimer_set(&timer, CLOCK_SECOND * 10);

  while(1) {
    //printf("Hello, world\n");
    NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_RSSI, &rssi_value);
    printf("last_rssi %d\n",rssi_value);

    /* Wait for the periodic timer to expire and then restart the timer. */
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
