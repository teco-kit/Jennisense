/*
 * Copyright (c) 2013. TECO/KIT, All rights reserved.
 * Maybe redistributed without permission :).
 *
 * authors: scholz, gordon, scholl
 */

#include "contiki.h"
#include "contiki-net.h"
#include "leds.h"
#include "clock.h"
/*
#include "dev/acc-sensor.h"
#include "dev/l3g4200d-sensor.h"
#include "dev/mag-sensor.h"
#include "dev/temperature-sensor.h"
#include "dev/pressure-sensor.h"
#include "dev/lightlevel-sensor.h"
#include "dev/proximity-sensor.h"
#include "dev/inttemp-sensor.h"
*/
#include "dev/ext_temp_sensor.h"
#include "dev/SHT21-sensor.h"
#include <stdio.h>

#include <AppHardwareApi.h>

PROCESS(send_sensor, "send_sensor");
AUTOSTART_PROCESSES(&send_sensor);

#define SEND_INTERVAL 30000

void con_com(uint8_t* dest, uint8_t* id, uint8_t data_len, void* data)
{
  uint16_t ii = (id[0] - 0x33) + ((id[1] - 0x33) * 40) + ((id[2] - 0x33) * 1600);

  dest[0] = (uint8_t)((ii >> 8) & 0x00ff);
  dest[1] = (uint8_t)(ii & 0x00ff);
  dest[2] = data_len;
  memcpy(&dest[3], data, data_len);
}

PROCESS_THREAD(send_sensor, ev, data)
{
  static struct etimer send_et;
  static struct uip_udp_conn *udpc;
  static struct sensors_sensor *sht21_humid, *sht21_temp, *ext_temp;
  static int16_t ste, shu, st1, st2;
  PROCESS_BEGIN();

  udpc = udp_broadcast_new(UIP_HTONS(10000), NULL);

 
  sht21_humid = sensors_find(SHT21_HUMIDITY_SENSOR);
  sht21_humid->configure(SENSORS_ACTIVE,1);
  sht21_temp = sensors_find(SHT21_TEMPERATURE_SENSOR);
  sht21_temp->configure(SENSORS_ACTIVE,1); 

  ext_temp = &ext_temp_sensor;
  ext_temp->configure(SENSORS_HW_INIT, 0);


  while(1) {
//    etimer_set(&send_et, 1);
    leds_toggle(LEDS_ALL);

    /* collect data */

   

    int i=0;

    ext_temp->configure( SENSORS_HW_EXT1 ,0 );
    etimer_set(&send_et, 2);
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_TIMER);
    st1 = ext_temp->value(EXT_TEMPERATURE_VALUE_MILLICELSIUS);
    
    
    ext_temp->configure( SENSORS_HW_EXT2 ,0 );
    etimer_set(&send_et, 2);
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_TIMER);
    // call update on value of ext_temp (dirty hack)
    st2 = ext_temp->value(EXT_TEMPERATURE_VALUE_MILLICELSIUS);

    // switch adc/pt1000 off
    ext_temp->configure( SENSORS_HW_OFF ,0 );
    
    ste = sht21_temp->value(SHT21_TEMPERATURE_VALUE_MILLICELSIUS);
    shu = sht21_humid->value(SHT21_HUMIDITY_VALUE_PERCENT);

    
    
    int8_t ajn = 3;
    clock_time_t msecs = clock_time();
    uint64_t cts = ((msecs/1000) << 16) | (msecs % 1000);

    /* create conCom Package */
    uint8_t buffer[35];
    con_com(buffer, "AJN", 1, &ajn); //4 Byte
    con_com(&buffer[4], "CTS", 8, &cts); //11 Byte
    con_com(&buffer[15], "STA", 2, &st1); //5 Byte
    con_com(&buffer[20], "STB", 2, &st2); //5 Byte
    con_com(&buffer[25], "SHU", 2, &shu); //5 Byte
    con_com(&buffer[30], "STE", 2, &ste); //5 Byte
    
    /* send and wait on timer */
    uip_udp_packet_send(udpc, buffer, sizeof(buffer));

    etimer_set(&send_et, SEND_INTERVAL);
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_TIMER);
  }

  PROCESS_END();
}
