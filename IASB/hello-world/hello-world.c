#include "contiki.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
	static struct etimer timer;
	static int count = 0;

	PROCESS_BEGIN();

	// set the etimer module to generate an event in one second.
	etimer_set(&timer, CLOCK_CONF_SECOND*5);
	while (1)
	{
		 // wait here for an event to happen
		 PROCESS_WAIT_EVENT();

		 // if the event is the timer event as expected...
		 if(ev == PROCESS_EVENT_TIMER)
		 {
			 // do the process work
			 printf("HELLO, woRld #%i\n", count);
			 count ++;

			 // reset the timer so it will generate an other event
			 // the exact same time after it expired (periodicity guaranteed)
			 etimer_reset(&timer);
		 }
	 }
	 // any process must end with this, even if it is never reached.
	 PROCESS_END();
}
