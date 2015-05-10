#include "contiki.h"
#include "net/rime/rime.h"
#include "i2c_sensors_interface.h"
#include "sensors.h"
#include <stdio.h>
#include <util/delay.h>

/* This is the structure of unicast messages. */
struct unicast_message {
  temperature_t temp;
  luminosity_t lumi;
  acceleration_t accel;
};

MEMB(sinkaddress, linkaddr_t, 1);
static const linkaddr_t *sink_addr;

/* These hold the broadcast and unicast structures, respectively. */
static struct broadcast_conn broadcast;
static struct unicast_conn unicast;

/*---------------------------------------------------------------------------*/

/* We first declare our two processes. */
PROCESS(broadcast_process, "Broadcast process");
PROCESS(unicast_process, "Unicast process");

/* The AUTOSTART_PROCESSES() definition specifices what processes to
   start when this module is loaded. We put both our processes there. */
AUTOSTART_PROCESSES(&broadcast_process, &unicast_process);

/*---------------------------------------------------------------------------*/

/* Helper function for console printout
   Prints the received sensor parameters values.*/
//static void print_results(const temperature_t* temp, const luminosity_t* lumi, const acceleration_t* accel)
//{
//   /* print current temperature */
//   printf("Temperature: %c%d.%02d C\n", (temp->sign ? '-' : '+'),
//                                    	temp->integralDigit,
//                                    	temp->fractionalDigit);
//
//   /* print current luminosity sensor value */
//   printf("Luminosity: %d Lux\n", *lumi);
//
//   /* print current acceleration values */
//   printf("Acceleration: x: %c%d.%02d g\n", accel->acc_x_sign ? '-' : '+',
//									  	accel->acc_x_integral,
//                                        accel->acc_x_fractional);
//
//   printf("              y: %c%d.%02d g\n", accel->acc_y_sign ? '-' : '+',
//                                        accel->acc_y_integral,
//                                        accel->acc_y_fractional);
//
//   printf("              z: %c%d.%02d g\n", accel->acc_z_sign ? '-' : '+',
//                                        accel->acc_z_integral,
//                                        accel->acc_z_fractional);
//}

/* This function is called whenever a broadcast message is received. */
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  char *m;


  /* The packetbuf_dataptr() returns a pointer to the first data byte
     in the received packet. */
  m = packetbuf_dataptr();

  // allocate memory for sink address
  if(sink_addr == 0) {
	  sink_addr = memb_alloc(&sinkaddress);
  }
  if(!strcmp((char *)m, "Im a sink!")) {
	  if(!linkaddr_cmp(sink_addr, from)){
		  linkaddr_copy(sink_addr, from);
		  printf("New sink discovered: %d,%d\n", from->u8[0], from->u8[1]);
	  }

  }
}

/* This is where we define what function to be called when a broadcast
   is received. We pass a pointer to this structure in the
   broadcast_open() call below. */
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

/*---------------------------------------------------------------------------*/

/* This function is called for every incoming unicast packet. */
//static void
//recv_uc(struct unicast_conn *c, const linkaddr_t *from)
//{
//  struct unicast_message *msg;
//
//  /* Grab the pointer to the incoming data. */
//  msg = packetbuf_dataptr();
//
//  printf("\n");
//  printf("   DATA FROM   %d,%d   \n", from->u8[0], from->u8[1]);
//  print_results(&msg->temp, &msg->lumi, &msg->accel);
//
//}
//static const struct unicast_callbacks unicast_callbacks = {recv_uc};

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(broadcast_process, ev, data)
{

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  static struct etimer et;
  etimer_set(&et, CLOCK_SECOND * 10);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&unicast);)
    
  PROCESS_BEGIN();

  printf("STAR-NODE started!\n");

  unicast_open(&unicast, 146, NULL /*&unicast_callbacks*/);

  memb_init(&sinkaddress);

  static struct etimer et;
  etimer_set(&et, CLOCK_SECOND * 17);

  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if(sink_addr != 0)
    {
      struct unicast_message msg;

      /* measure temperature */
      while(TMP102_WakeUp());
      while(TMP102_StartOneshotMeasurement());
      while(TMP102_GetTemperature(&msg.temp, true));
      while(TMP102_PowerDown());

      /* measure luminosity */
      while(ISL29020_WakeUp());
      ISL29020_StartOneshotMeasurement();
      // delay needed for correct luminosity measurement
      _delay_ms(100);
      while(ISL29020_GetLuminosity(&msg.lumi));
      while(ISL29020_PowerDown());

      /* measure acceleration */
      while(BMA150_WakeUp());
      //read twice because 1st read after wake-up is trash
      while(BMA150_GetAcceleration(&msg.accel));
      while(BMA150_GetAcceleration(&msg.accel));
      while(BMA150_PowerDown());

      printf("Sending data!\n");

      packetbuf_copyfrom(&msg, sizeof(msg));
      unicast_send(&unicast, sink_addr);
    }
    etimer_reset(&et);
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
