#include "contiki.h"

#include <stdio.h> /* For printf() */
#include "dev/leds.h"
#include "lib/sensors.h"
#include "dev/si7021.h"
#include "dev/i2c.h"
#include "gpio.h"

#include "contiki-net.h"
#include "sys/node-id.h"
#include "project-conf.h"

#include "net/ipv6/uip.h"
#include "net/ipv6/uiplib.h"
#include "net/ipv6/ip64-addr.h"
/*---------------------------------------------------------------------------*/
PROCESS(test, "Si7021 Temperature and Humidity process");
AUTOSTART_PROCESSES(&test);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test, ev, data)
{
  //static struct etimer timer;
    // static int count = 0;

  static struct etimer periodic;
  PROCESS_BEGIN();

  
  /* Setup a periodic timer that expires after 10 seconds. */
  //etimer_set(&timer, CLOCK_SECOND * 10);
  etimer_set(&periodic, CLOCK_SECOND * 1);
  // while(1) {
  while(1) {
  //   val = cc2538_temp_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED);
  //   printf("temperature : %u mC\n",val);
    
    PROCESS_WAIT_EVENT();
    
    if(ev == PROCESS_EVENT_TIMER && etimer_expired(&periodic)) {
      
      printf("- Routing: %s\n", NETSTACK_ROUTING.name);
      printf("- Net: %s\n", NETSTACK_NETWORK.name);
      printf("- MAC: %s\n", NETSTACK_MAC.name);
      printf("- 802.15.4 PANID: 0x%04x\n", IEEE802154_CONF_PANID);
      printf("- 802.15.4 Default channel: %u\n", IEEE802154_DEFAULT_CHANNEL);
      printf("Node ID: %u\n", node_id);
      // printf("Link-layer address: ");
      // LOG_INFO_LLADDR(&linkaddr_node_addr);
      printf("\n");


      etimer_reset(&periodic);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
