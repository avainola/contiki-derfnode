#include "contiki.h"
#include "net/rime/rime.h"
#include "i2c_sensors_interface.h"
#include "io_access.h"
//#include "sensors.h"
#include <stdio.h>
#include <util/delay.h>

/* This is the structure of unicast messages. */
struct unicast_message {
  temperature_t temp;
  luminosity_t lumi;
  acceleration_t accel;
};

//MEMB(sinkaddress, linkaddr_t, 1);
//static const linkaddr_t *sink_addr;

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
static void print_results(const temperature_t* temp, const luminosity_t* lumi, const acceleration_t* accel)
{
   /* print current temperature */
   printf("Temperature: %c%d.%02d C\n", (temp->sign ? '-' : '+'),
                                    	temp->integralDigit,
                                    	temp->fractionalDigit);

   /* print current luminosity sensor value */
   printf("Luminosity: %d Lux\n", *lumi);

   /* print current acceleration values */
   printf("Acceleration: x: %c%d.%02d g\n", accel->acc_x_sign ? '-' : '+',
									  	accel->acc_x_integral,
                                        accel->acc_x_fractional);

   printf("              y: %c%d.%02d g\n", accel->acc_y_sign ? '-' : '+',
                                        accel->acc_y_integral,
                                        accel->acc_y_fractional);

   printf("              z: %c%d.%02d g\n", accel->acc_z_sign ? '-' : '+',
                                        accel->acc_z_integral,
                                        accel->acc_z_fractional);
}

/* This function is called whenever a broadcast message is received. */
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{

}

/* This is where we define what function to be called when a broadcast
   is received. We pass a pointer to this structure in the
   broadcast_open() call below. */
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

/*---------------------------------------------------------------------------*/

/* This function is called for every incoming unicast packet. */
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
  struct unicast_message *msg;

  /* Grab the pointer to the incoming data. */
  msg = packetbuf_dataptr();

  printf("\n");
  printf("   DATA FROM   %d,%d   \n", from->u8[0], from->u8[1]);
  print_results(&msg->temp, &msg->lumi, &msg->accel);

}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(broadcast_process, ev, data)
{


  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  printf("STAR-SINK started!\n");
  led_set(LED_0, LED_ON);
  led_set(LED_1, LED_ON);

  broadcast_open(&broadcast, 129, &broadcast_call);

  static struct etimer et;
  etimer_set(&et, CLOCK_SECOND * 30);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    packetbuf_copyfrom("I'm a sink!", 12);
    broadcast_send(&broadcast);
    etimer_reset(&et);
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&unicast);)
  PROCESS_BEGIN();

  unicast_open(&unicast, 146, &unicast_callbacks);

  static struct etimer et;
  etimer_set(&et, CLOCK_SECOND * 10);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
