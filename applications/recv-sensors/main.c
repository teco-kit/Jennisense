#include "contiki.h"
#include "contiki-net.h"
#include "dev/leds.h"
#include "hrclock.h"

#include <AppHardwareApi.h>

PROCESS(sync_master, "sync_master");
AUTOSTART_PROCESSES(&sync_master);

#define UDP_HDR ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

PROCESS_THREAD(sync_master, ev, data)
{
  static int i=0;
  static struct uip_udp_conn *udp;

  PROCESS_BEGIN();
    
  udp = udp_new(NULL, UIP_HTONS(10000), NULL);
  uip_udp_bind(udp, UIP_HTONS(10000));
  uart0_init(1000000);

  while(1)
  {
    PROCESS_YIELD_UNTIL(ev == tcpip_event);
    int i;
    for (i = 0; i < 16; i++)
      uart0_writeb(UDP_HDR->srcipaddr.u8[i]);
    for (i = 0; i < uip_datalen(); i++)
    {
      uart0_writeb(((uint8_t*)uip_appdata)[i]);
    }
  }

  PROCESS_END();
}
