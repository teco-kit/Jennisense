/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 *
 * $Id: telnet-server.c,v 1.4 2009/03/01 23:37:49 oliverschmidt Exp $
 */

/**
 * \file
 *         Example showing how to use the Telnet server
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "dev/acc-sensor.h"
#include "dev/l3g4200d-sensor.h"
#include "dev/mag-sensor.h"
#include "dev/temperature-sensor.h"
#include "dev/pressure-sensor.h"
#include "dev/lightlevel-sensor.h"
#include "dev/proximity-sensor.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(imu_serial, "imu-serial process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(imu_serial, ev, data)
{
  static struct sensors_sensor *s;
  PROCESS_BEGIN();

  /* make sure that the sensor process is running! */
  process_start(&sensors_process, NULL);
  PROCESS_PAUSE();

  /* activate all sensors */
  for (s=sensors_first(); s; s=sensors_next(s)) {
    if (s->configure(SENSORS_ACTIVE,1))
      printf("%s activated.\n", s->type);
    else
      while (1) {
        PROCESS_YIELD();
        printf("%s not activated.\n", s->type);
      }
  }

  /* sample at maximum frequency */
  while (1)
  {
    int a,b,c;

    PROCESS_YIELD_UNTIL(ev==sensors_event);
    s = (struct sensors_sensor*) data;

    if (s==&acc_sensor) {
      a = s->value(ACC_VALUE_X);
      b = s->value(ACC_VALUE_Y);
      c = s->value(ACC_VALUE_Z);
      printf("acc:%d %d %d\n",a,b,c);
    } else if (s==&mag_sensor) {
      a = s->value(MAG_VALUE_X);
      b = s->value(MAG_VALUE_Y);
      c = s->value(MAG_VALUE_Z);
      printf("mag:%d %d %d\n",a,b,c);
    } else if (s==&l3g4200d_sensor) {
      a = s->value(GYRO_VALUE_X);
      b = s->value(GYRO_VALUE_Y);
      c = s->value(GYRO_VALUE_Z);
      printf("gyr:%d %d %d\n",a,b,c);
    } else if (s==&temperature_sensor) {
      a = s->value(TEMPERATURE_VALUE_MILLICELSIUS);
      printf("tem:%d\n",a);
    } else if (s==&pressure_sensor) {
      a = s->value(PRESSURE_VALUE_PASCAL);
      printf("pre:%d\n",a);
    } else if (s==&lightlevel_sensor) {
      a = s->value(LIGHT_VALUE_VISIBLE_CENTILUX);
      printf("lig:%d\n",a);
    } else if (s==&proximity_sensor) {
      a = s->value(PROXIMITY_VALUE);
      printf("pro:%d\n",a);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
AUTOSTART_PROCESSES(&imu_serial);
/*---------------------------------------------------------------------------*/
