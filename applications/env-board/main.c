#include "contiki.h"
#include "contiki-net.h"
#include "leds.h"
#include "dev/SHT21-sensor.h"
#include "clock.h"

#include <AppHardwareApi.h>

PROCESS(send_sensor, "send_sensor");
AUTOSTART_PROCESSES(&send_sensor);

#define SEND_INTERVAL 1

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
  static struct sensors_sensor *sht21_humid, *sht21_temp;
  PROCESS_BEGIN();

  udpc = udp_broadcast_new(UIP_HTONS(10000), NULL);

/*  gyro = sensors_find(GYRO_SENSOR);
  gyro->configure(SENSORS_ACTIVE, 1);
  acc = sensors_find(ACC_SENSOR);
  acc->configure(SENSORS_ACTIVE, 1);
  mag = sensors_find(MAG_SENSOR);
  mag->configure(SENSORS_ACTIVE, 1);
*/
  sht21_humid = sensors_find(SHT21_HUMIDITY_SENSOR);
  sht21_humid->configure(SENSORS_ACTIVE,1);
  sht21_temp = sensors_find(SHT21_TEMPERATURE_SENSOR);
  sht21_temp->configure(SENSORS_ACTIVE,1);

  while(1) {
    etimer_set(&send_et, SEND_INTERVAL);
    leds_toggle(LEDS_ALL);

    /* collect data */
    int8_t ajn = 3;
    clock_time_t msecs = clock_time();
    uint64_t cts = ((msecs/1000) << 16) | (msecs % 1000);
 /*   int16_t sgx = gyro->value(GYRO_VALUE_X);
    int16_t sgy = gyro->value(GYRO_VALUE_Y);
    int16_t sgz = gyro->value(GYRO_VALUE_Z);
    int16_t sax = acc->value(ACC_VALUE_X);
    int16_t say = acc->value(ACC_VALUE_Y);
    int16_t saz = acc->value(ACC_VALUE_Z);
    int16_t scx = mag->value(MAG_VALUE_X);
    int16_t scy = mag->value(MAG_VALUE_Y);
    int16_t scz = mag->value(MAG_VALUE_Z); */

    int16_t shu = sht21_humid->value(SHT21_HUMIDITY_VALUE_PERCENT);
    int16_t ste = sht21_temp->value(SHT21_TEMPERATURE_VALUE_MILLICELSIUS);

    /* create conCom Package */
    uint8_t buffer[60];
    con_com(buffer, "AJN", 1, &ajn); //4 Byte
    con_com(&buffer[4], "CTS", 8, &cts); //11 Byte
/*    con_com(&buffer[15], "SAX", 2, &sax); //5 Byte
    con_com(&buffer[20], "SAY", 2, &say); //5 Byte
    con_com(&buffer[25], "SAZ", 2, &saz); //5 Byte
    con_com(&buffer[30], "SGX", 2, &sgx); //5 Byte
    con_com(&buffer[35], "SGY", 2, &sgy); //5 Byte
    con_com(&buffer[40], "SGZ", 2, &sgz); //5 Byte
    con_com(&buffer[45], "SCX", 2, &scx); //5 Byte
    con_com(&buffer[50], "SCY", 2, &scy); //5 Byte
    con_com(&buffer[55], "SCZ", 2, &scz); //5 Byte*/
    con_com(&buffer[20], "SHU", 2, &shu); //5 Byte
    con_com(&buffer[25], "STE", 2, &ste); //5 Byte

    /* send and wait on timer */
    uip_udp_packet_send(udpc, buffer, sizeof(buffer));
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_TIMER);
  }

  PROCESS_END();
}
