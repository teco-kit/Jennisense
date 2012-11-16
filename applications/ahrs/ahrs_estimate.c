#include "contiki.h"
#include "contiki-net.h"
#include "shell.h"
#include "hrclock.h"
#include "ahrs.h"
//#include "imu.h"
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include "dev/acc-sensor.h"
#include "dev/l3g4200d-sensor.h"
#include "dev/mag-sensor.h"

PROCESS(tcp_process, "TCP AHRS process");
PROCESS(udp_process, "UDP AHRS process");
PROCESS(ahrs_process,"AHRS Estimator");

AUTOSTART_PROCESSES(&tcp_process, &udp_process);

static char msg[200];

static
PT_THREAD(handle_connection(struct psock *p))
{
  PSOCK_BEGIN(p);

  while (true) {
  if (PSOCK_NEWDATA(p))
    PSOCK_CLOSE(p);
  else
    PSOCK_SEND_STR(p,msg);
  }

  PSOCK_END(p);
}

PROCESS_THREAD(tcp_process, ev, data)
{
  static char incoming[10];
  static struct psock ps;

  PROCESS_BEGIN();

  uart0_set_br(1000000);
  tcp_listen(UIP_HTONS(2020));

  while(1) {
    /* wait for tcp connection */
    PROCESS_WAIT_EVENT_UNTIL(ev==tcpip_event && uip_connected());

    /* start estimator process and init psock connection handler */
    process_start(&ahrs_process, NULL);
    PSOCK_INIT(&ps,incoming,sizeof(incoming));

    /* loop until connection is closed */
    while (!(uip_aborted() || uip_closed() || uip_timedout())) {
      PROCESS_YIELD_UNTIL(ev==tcpip_event);
      handle_connection(&ps);
    }

    /* stop ahrs process */
    process_exit(&ahrs_process);

  }

  PROCESS_END();
}

#define SEND_INTERVAL (CLOCK_SECOND/50)
PROCESS_THREAD(udp_process, ev, data)
{
  static struct uip_udp_conn *s;
  static struct etimer et;

  PROCESS_BEGIN();

  uart0_set_br(1000000);

  /* setup udp connection             */
  s = udp_broadcast_new(UIP_HTONS(2020),NULL); // incoming
  udp_bind(s,UIP_HTONS(2020));                 // outgoing

  /* start estimator process and init psock connection handler */
  process_start(&ahrs_process, NULL);

  etimer_set(&et, SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev==PROCESS_EVENT_TIMER);
    uip_udp_packet_send(s,msg,strlen(msg));
    etimer_restart(&et);
  }

  PROCESS_END();
}

int ftoa(float x, char *s)
{
  size_t i; char *orig=s;

  if (x<0) { // uh, casting -0.5 to int gives 0 (not -0!)
    *s++ = '-';
     x  *= -1;
  }

  s += sprintf(s,"%d.",(int) x);
  x -= (int) x; // remove vorkomma

  for (i=10;i<1000000;i*=10) {
    x   *= 10;
    *s++ = '0'+(int) x;
    x   -= (int) x;
  }

  return s-orig;
}

float gyro2rad(int raw)
{
  static float scale_factor = 140.e-3; //130e-3;
  return raw * scale_factor * M_PI / 180.;
}

float magcal(struct sensors_sensor *s, int which)
{
  /* only offset correction by tracking min-max. */
  static int maxx=INT_MIN, minx=INT_MAX,
             maxy=INT_MIN, miny=INT_MAX,
             maxz=INT_MIN, minz=INT_MAX;

  if (s==NULL) {
    maxx=INT_MIN; minx=INT_MAX;
    maxy=INT_MIN; miny=INT_MAX;
    maxz=INT_MIN; minz=INT_MAX;
    return 0.;
  }

  int value = s->value(which);

  switch (which)  {
  case MAG_VALUE_X:
    maxx=value>maxx ? value : maxx;
    minx=value<minx ? value : minx;
    return value + minx - (minx-maxx)/2;
  case MAG_VALUE_Y:
    maxy=value>maxy ? value : maxy;
    miny=value<miny ? value : miny;
    return value + miny - (miny-maxy)/2;
  case MAG_VALUE_Z:
    maxz=value>maxz ? value : maxz;
    minz=value<minz ? value : minz;
    return value + minz - (miny-maxz)/2;
  }

  return 0;
}


float q0 = 1, q1 = 0, q2 = 0, q3 = 0;	// quaternion elements representing the estimated orientation
float exInt = 0, eyInt = 0, ezInt = 0;	// scaled integral error

PROCESS_THREAD(ahrs_process,ev,data)
{
  static hrclock_t timestamp;
  static float gx,gy,gz,ax,ay,az,mx,my,mz,td;
  static struct sensors_sensor *gyr, *acc, *mag;

  if(ev == PROCESS_EVENT_EXIT) { /* switch off sampling */
    if (gyr!=NULL) gyr->configure(SENSORS_ACTIVE,0);
    if (acc!=NULL) acc->configure(SENSORS_ACTIVE,0);
    if (mag!=NULL) mag->configure(SENSORS_ACTIVE,0);
  } else if (ev == sensors_event) {
    if (data==acc) {
      ax = acc->value(ACC_VALUE_X);
      ay = acc->value(ACC_VALUE_Y);
      az = acc->value(ACC_VALUE_Z);
    } else if (data==mag) {
      mx = magcal(mag,MAG_VALUE_X);
      my = magcal(mag,MAG_VALUE_Y);
      mz = magcal(mag,MAG_VALUE_Z);
    } else if (data==gyr) {
      gx = gyro2rad(gyr->value(GYRO_VALUE_X));
      gy = gyro2rad(gyr->value(GYRO_VALUE_Y));
      gz = gyro2rad(gyr->value(GYRO_VALUE_Z));
    }
  }

  PROCESS_BEGIN();

  /* initialize sensors */
  gyr = sensors_find(GYRO_SENSOR);
  acc = sensors_find(ACC_SENSOR);
  mag = sensors_find(MAG_SENSOR);

  if (gyr==NULL) {
    strncpy(msg, "gyro not found\n",sizeof(msg));
    process_exit(&ahrs_process);
  } else if (acc==NULL) {
    strncpy(msg, "accelerometer not found\n",sizeof(msg));
    process_exit(&ahrs_process);
  } else if (mag==NULL) {
    strncpy(msg, "compass not found\n",sizeof(msg));
    process_exit(&ahrs_process);
  } else {
    gyr->configure(SENSORS_ACTIVE,1);
    acc->configure(SENSORS_ACTIVE,1);
    mag->configure(SENSORS_ACTIVE,1);
  }

  /* configure sensors, gyro to 2000dps@200Hz with bandpass at 15-70Hz, bandpass
   * for zero-rate removal!  acc to 100Hz and 1Hz high-pass */
  gyr->configure(GYRO_L3G_RANGE,GYRO_L3GVAL_2000DPS);
  gyr->configure(GYRO_L3G_DRATE,GYRO_L3GVAL_200_70HZ);
  gyr->configure(GYRO_L3G_HIGHPASS,GYRO_L3GVAL_HP0); // 15Hz@200Hz with ref
  gyr->configure(GYRO_L3G_FILTER,GYRO_L3GVAL_BOTH);
  acc->configure(ACC_LSM303_RANGE,ACC_LSM303VAL_2G);
  acc->configure(ACC_LSM303_DRATE,ACC_LSM303VAL_100HZ);
  acc->configure(ACC_LSM303_HIGHPASS,ACC_LSM303VAL_HP1);
  acc->configure(ACC_LSM303_FILTER,ACC_LSM303VAL_BOTH);

  /* XXX: ignore first measurements */
  while (gx==0 || mx==0 || ax==0)
    PROCESS_YIELD_UNTIL(ev==sensors_event);

  /* reset quaternion and compass calibration */
  q0=1; q1=q2=q3=exInt=eyInt=ezInt=0;
  magcal(NULL,0);

  while (1) { /* estimate continously and write into msg */
    timestamp = clock_hrtime();
    PROCESS_YIELD_UNTIL(ev==sensors_event);

    /* calculate sampling time */
    td = (clock_hrtime()-timestamp) / 1.e6;

    /* update estimate */
    AHRSupdate(gx,gy,gz,ax,ay,az,mx,my,mz,td);
    //IMUupdate(gx,gy,gz,ax,ay,az,td);

    /* rebuild msg string */
    {
      char *s = msg;
      s   += ftoa(td,s);
      *s++ = ' ';
      s   += ftoa(q0,s);
      *s++ = ' ';
      s   += ftoa(q1,s);
      *s++ = ' ';
      s   += ftoa(q2,s);
      *s++ = ' ';
      s   += ftoa(q3,s);
      *s++ = ' ';
      s   += ftoa(exInt,s);
      *s++ = ' ';
      s   += ftoa(eyInt,s);
      *s++ = ' ';
      s   += ftoa(ezInt,s);
      *s++ = '\n';
      *s = '\0';
    }
  }

  PROCESS_END();
}
