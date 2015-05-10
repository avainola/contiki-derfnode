#include <stdio.h>
#include <util/delay.h>

#include "contiki.h"
#include "usb.h"
#include "io_access.h"
#include "i2c_sensors_interface.h"

/* === TYPES =============================================================== */

/* === MACROS / DEFINES ==================================================== */

/* === GLOBALS ============================================================= */

/* temperature sensor value */
static temperature_t temp;

/* luminosity sensor value */
static luminosity_t lumi;

/* acceleration sensor value */
static acceleration_t accel;

/* === PROTOTYPES ========================================================== */

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Helper function for console printout
 *
 * Prints the measured sensor parameters values.
 *
 */
static void print_results(void)
{
   /* print current temperature */
   printf("Temperature: %c%d.%02d C\n", (temp.sign ? '-' : '+'),
										temp.integralDigit,
										temp.fractionalDigit);

   /* print current luminosity sensor value */
   printf("Luminosity: %d Lux\n", lumi);

   /* print current acceleration values */
   printf("Acceleration: x: %c%d.%02d g\n", accel.acc_x_sign ? '-' : '+',
										accel.acc_x_integral,
										accel.acc_x_fractional);

   printf("              y: %c%d.%02d g\n", accel.acc_y_sign ? '-' : '+',
										accel.acc_y_integral,
										accel.acc_y_fractional);

   printf("              z: %c%d.%02d g\n", accel.acc_z_sign ? '-' : '+',
										accel.acc_z_integral,
										accel.acc_z_fractional);
}


/**
 * @brief Helper function for querying the sensors
 *
 * Determines the current sensor parameters.
 *
 */
static void measure(void)
{
	/* measure temperature */
	while(TMP102_WakeUp());
	while(TMP102_StartOneshotMeasurement());
	while(TMP102_GetTemperature(&temp, true));
	while(TMP102_PowerDown());

	/* measure luminosity */
	while(ISL29020_WakeUp());
	ISL29020_StartOneshotMeasurement();
	// delay needed for correct luminosity measurement
	_delay_ms(100);
	while(ISL29020_GetLuminosity(&lumi));
	while(ISL29020_PowerDown());

	/* measure acceleration */
	while(BMA150_WakeUp());
	//read twice because 1st read after wake-up is trash
	while(BMA150_GetAcceleration(&accel));
	while(BMA150_GetAcceleration(&accel));
	while(BMA150_PowerDown());
}

PROCESS(sensors_process, "Sensorsprocess");
AUTOSTART_PROCESSES(&sensors_process);
PROCESS_THREAD(sensors_process, ev, data)
{
  PROCESS_BEGIN();

  /* initialize LEDs and stuff */
  io_init();

  printf("Sensors application started!\n");

  static struct etimer et;
  static int led = 0;
  etimer_set(&et, CLOCK_SECOND * 10);

  /* loop infinite */
  while(1)
  {
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	if (led == 0)
	{
		led_set(LED_0, LED_ON);
		led_set(LED_1, LED_OFF);
		led_set(LED_2, LED_OFF);
	}
	if (led == 1)
	{
		led_set(LED_1, LED_ON);
		led_set(LED_0, LED_OFF);
		led_set(LED_2, LED_OFF);
	}
	if (led == 2)
	{
		led_set(LED_2, LED_ON);
		led_set(LED_1, LED_OFF);
		led_set(LED_0, LED_OFF);
	}
	led++;
    measure();
    print_results();
    if (led == 3) led = 0;
    etimer_reset(&et);
  }

  PROCESS_END();
}
