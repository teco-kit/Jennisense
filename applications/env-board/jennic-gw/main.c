#include "contiki.h"
#include "contiki-net.h"
#include "dev/leds.h"
#include "hrclock.h"
#include "uart0.h"

#include <AppHardwareApi.h>

PROCESS(sync_master, "sync_master");
AUTOSTART_PROCESSES(&sync_master);

#define UDP_HDR ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

/////////////////////////
// HDLC implementation //

/** start, end and escaping characters **/
static uint8_t RF_END=0xC0;
static uint8_t RF_ESC=0xDB;
static uint8_t RF_T_END=0xDC;
static uint8_t RF_T_ESC=0xDD;


void sendStartEndByte(){
    uart0_writeb(RF_END);
}


/** returns the length of the encoded data, encoded data is in tx buffer **/
void encodeAndSendData(uint8_t databyte){

    // encode data
    if(databyte==RF_END){
      uart0_writeb(RF_ESC);
      uart0_writeb(RF_T_END);
    }else
    if(databyte==RF_ESC){
      uart0_writeb(RF_ESC);
      uart0_writeb(RF_T_ESC);
    }else
    {
      uart0_writeb(databyte);
    }
}

/////////////////////////




void con_com(uint8_t* dest, uint8_t* id, uint8_t data_len, void* data)
{
  uint16_t ii = (id[0] - 0x33) + ((id[1] - 0x33) * 40) + ((id[2] - 0x33) * 1600);

  dest[0] = (uint8_t)((ii >> 8) & 0x00ff);
  dest[1] = (uint8_t)(ii & 0x00ff);
  dest[2] = data_len;
  memcpy(&dest[3], data, data_len);
}





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
#if 0
    PROCESS_YIELD_UNTIL(ev == tcpip_event);
    int i;
    for (i = 0; i < 16; i++)
      uart0_writeb(UDP_HDR->srcipaddr.u8[i]);
    for (i = 0; i < uip_datalen(); i++)
    {
      uart0_writeb(((uint8_t*)uip_appdata)[i]);
    }

#endif

#if 0
    PROCESS_YIELD_UNTIL(ev == tcpip_event);
    uint16_t i; 

    uint8_t ch='E';
    uart0_writeb(ch);
#else  
    uint8_t ch='E';
    uart0_writeb(ch);

    PROCESS_YIELD_UNTIL(ev == tcpip_event);
    uint16_t i; 
    uint8_t ipaddr[16+3];

    // encode ip address 
    con_com(ipaddr, "ADR", 16, UDP_HDR->srcipaddr.u8); // ipaddr len= 19
    // send start byte
    sendStartEndByte();
    // calculate checksum over all data (concom ip and buffer data) and send and encode data at same time
    uint32_t sum=0;
    uint8_t crc[2]={0,0};
    for(i=0; i < 16+3; i++){
        sum+=ipaddr[i];
        encodeAndSendData(ipaddr[i]);
    }
    for(i=0; i < uip_datalen(); i++){
        sum+=((uint8_t*)uip_appdata)[i];
        encodeAndSendData( ((uint8_t*)uip_appdata) [i] );
    }

    crc[0]=(~sum +1) & 0xff;
    crc[1]=((~sum +1) >>8) & 0xff;

    // send crc
    encodeAndSendData( crc[0] );
    encodeAndSendData( crc[1] );


    // send end byte
    sendStartEndByte();
    
#endif

  }

  PROCESS_END();
}
