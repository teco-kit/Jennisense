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

/*---------------------------------------------------------------------------*/
PROCESS(imu_serial, "imu-serial process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(imu_serial, ev, data)
{
  static u16_t sample_count=0;
  static struct sensors_sensor *mag = NULL,
                               *acc = NULL,
                               *gyr = NULL,
                               *tem = NULL,
                               *pre = NULL,
                               *lls = NULL,
                               *pro = NULL;

  PROCESS_BEGIN();

  /* make sure that the sensor process is running! */
  process_start(&sensors_process, NULL);
  PROCESS_PAUSE();

  mag = sensors_find(MAG_SENSOR);
  acc = sensors_find(ACC_SENSOR);
  gyr = sensors_find(GYRO_SENSOR);
  tem = sensors_find(TEMPERATURE_SENSOR);
  pre = sensors_find(PRESSURE_SENSOR);
  lls = sensors_find(LIGHTLEVEL_SENSOR);
  pro = sensors_find(PROXIMITY_SENSOR);
  sample_count = 0;

  uart0_set_br(1000000);

  if (mag==NULL || acc==NULL || gyr==NULL || tem==NULL || pre==NULL ||
      lls==NULL || pro==NULL)
  {
    while(1)
    {
      PROCESS_PAUSE();
      printf("not all sensors are there\n");
    }
  }

  gyr->configure(SENSORS_ACTIVE, 1);
  pre->configure(SENSORS_ACTIVE, 1);
  tem->configure(SENSORS_ACTIVE, 1);
  mag->configure(SENSORS_ACTIVE, 1);
  acc->configure(SENSORS_ACTIVE, 1);
  lls->configure(SENSORS_ACTIVE, 1);
  pro->configure(SENSORS_ACTIVE, 1);

  printf("# sample_count,accx,accy,accz,gyrx,gyry,gyrz,magx,magy,magz,temp\n");

  /* sample at maximum frequency */
  while (1)
  {
    static int i=0;
    static int ax,ay,az,gx,gy,gz,mx,my,mz,tm,tm2,pr,li,px;

    PROCESS_YIELD_UNTIL(ev==sensors_event);

    if (data==acc) {
      ax = acc->value(ACC_VALUE_X);
      ay = acc->value(ACC_VALUE_Y);
      az = acc->value(ACC_VALUE_Z);
      printf("acc:%d %d %d\n",ax,ay,az);
    } else if (data==mag) {
      mx = mag->value(MAG_VALUE_X);
      my = mag->value(MAG_VALUE_Y);
      mz = mag->value(MAG_VALUE_Z);
      printf("mag:%d %d %d\n",mx,my,mz);
    } else if (data==gyr) {
      gx = gyr->value(GYRO_VALUE_X_DGS);
      gy = gyr->value(GYRO_VALUE_Y_DGS);
      gz = gyr->value(GYRO_VALUE_Z_DGS);
      printf("gyr:%d %d %d\n",gx,gy,gz);
    } else if (data==tem) {
      tm = tem->value(TEMPERATURE_VALUE_MILLICELSIUS);
      printf("tem:%d\n",tm);
    } else if (data==pre) {
      pr = pre->value(PRESSURE_VALUE_PASCAL);
      printf("pre:%d\n",pr);
    } else if (data==lls) {
      li = lls->value(LIGHT_VALUE_VISIBLE_CENTILUX);
      printf("lig:%d\n",li);
    } else if (data==pro) {
      px = pro->value(PROXIMITY_VALUE);
      printf("pro:%d\n",px);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
AUTOSTART_PROCESSES(&imu_serial);
/*---------------------------------------------------------------------------*/
