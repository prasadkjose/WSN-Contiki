
#include "contiki.h"
#include "net/rime/rime.h"

#include "dev/button-sensor.h"
#include "dev/light-sensor.h"
#include "dev/leds.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Example unicast");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
  printf("unicast message received from %d.%d\n",
	 from->u8[0], from->u8[1]);
 int alert = (int *)packetbuf_dataptr();
	printf("%d\n", alert);
	if(alert < 150) {
		leds_off(LEDS_GREEN);
		leds_on(LEDS_RED);
		printf("ALERT \n");
	}
	else{
		leds_off(LEDS_RED);
		leds_on(LEDS_GREEN);
	}
	
    		
}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc);)
  // cc2420_set_channel(7);
  PROCESS_BEGIN();

  unicast_open(&uc, 146, &unicast_callbacks);
  
  SENSORS_ACTIVATE(light_sensor);

  while(1) {
    static struct etimer et;
    linkaddr_t addr;
    int val;
    etimer_set(&et, CLOCK_SECOND*3);
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	//val =10 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC) / 7;
	val =light_sensor.value(0);
    packetbuf_copyfrom(&val, sizeof(val));
    	
    addr.u8[0] = 1;
    addr.u8[1] = 0;
    if(!linkaddr_cmp(&addr, &linkaddr_node_addr)) {
	//For sender	
      		if(unicast_send(&uc, &addr)){
		printf("data sent %d %d",val, &val);
	}
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
