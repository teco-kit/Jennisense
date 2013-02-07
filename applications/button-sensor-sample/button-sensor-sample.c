#include "contiki.h"
#include "dev/button-sensor.h"

/*---------------------------------------------------------------------------*/
PROCESS(button_serial, "button_serial");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(button_serial, ev, data)
{
  static struct sensors_sensor *btn;
  static struct etimer et;

  PROCESS_BEGIN();
  
  PROCESS_PAUSE();

  btn = sensors_find(BUTTON_SENSOR);

  if (btn!=NULL)
    SENSORS_ACTIVATE(*btn);

  etimer_set(&et, CLOCK_SECOND/2);
  static int pressed = 0;

  while(1) {
    PROCESS_YIELD_UNTIL(ev==sensors_event && data==btn);
    
    if(btn->value(BUTTON_0)!=pressed){
        printf("btn0 pressed/released \n");
        pressed=btn->value(BUTTON_0);
    } 
     etimer_reset(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
AUTOSTART_PROCESSES(&button_serial);
/*---------------------------------------------------------------------------*/
