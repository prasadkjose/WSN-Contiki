 
#include "contiki.h"
#include "net/rime/rime.h"
#include "dev/button-sensor.h"
#include "dev/light-sensor.h"
#include "dev/leds.h"
 #include "sys/timer.h"
#include <stdio.h>
#define MAX_RETRANSMISSIONS 1
/*---------------------------------------------------------------------------*/
PROCESS(mainProcess, "Starting Cascade Program");
PROCESS(ACKcheck, "check for no ACK process");
PROCESS(BlueLED, "Blue recieve LED process");
PROCESS(DisplyACK, "Print the ACK message and the nodeID process");
PROCESS(ACKrecieved, "ACK recieved process");
PROCESS(StringAppend, "append string to be sent process");
PROCESS(SendString, "send concatenated process");
PROCESS(FatherNode, "Father Node check ");
AUTOSTART_PROCESSES(&mainProcess);

static void sendACK();
static struct unicast_conn unicast;
/*---------------------------------------------------------------------------*/

static char alert[100];
static int childcount = 0;
static int ACKflag= 0;
/*---------------------------------------------------------------------------*/

/*
RED is Green
BlUE is RED
GREEN is Blue

*/


PROCESS_THREAD(BlueLED, ev, data){
  PROCESS_BEGIN();

  static struct etimer et1;
  etimer_set(&et1, CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1)); 
  leds_on(LEDS_BLUE);
  etimer_set(&et1, CLOCK_SECOND*3);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));       
  leds_off(LEDS_BLUE);
  //etimer_reset(&et1);
  
  PROCESS_END();
}

PROCESS_THREAD(StringAppend, ev, data){
  PROCESS_BEGIN();

  char newchild[10];
  char newchildmessage[10] = "child";
  //cascade message count
  childcount = strlen(alert)/6;
  snprintf(newchild, 10, "%d", childcount+1);
  strcat(newchildmessage,newchild); //append previous children to "Child"
  strcat(alert,newchildmessage); //append "child*n" to previous message

  PROCESS_END();
}

PROCESS_THREAD(DisplyACK, ev, data){
  PROCESS_BEGIN();

  int i;
  printf("Cascade message received from %d\n", data);
  strcpy(alert, (char *)packetbuf_dataptr());
  printf("%s\n ", alert ); 
  process_start(&BlueLED,NULL); 

  PROCESS_END();
}


PROCESS_THREAD(ACKcheck, ev, data){
  PROCESS_BEGIN();

  int wait ;
  int loop;
  wait = data;
  static struct etimer et3; 
    
  etimer_set(&et3, CLOCK_SECOND*wait); 
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et3));     

  if(ACKflag == 0){
  printf("Waited for 5 seconds. Have not recieved a ACK from the next node.\n");
    for(loop=0;loop<2;loop++){
      //send string concatenated
      /*For some reason I am not able to put a delay here.
      static struct etimer etz; 
      etimer_set(&etz, CLOCK_SECOND); 
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&etz)); */
      printf("Attempting Retranmission %dth time\n", loop);
      process_start(&SendString,NULL);

    }
  }
 
  if(ACKflag == 0){
    process_start(&FatherNode, NULL);

  }
  etimer_reset(&et3);
  ACKflag=0;
  PROCESS_END();
}

PROCESS_THREAD(SendString, ev, data){
  PROCESS_BEGIN();

  static struct etimer et5;  
  linkaddr_t addr;
  etimer_set(&et5, CLOCK_SECOND); 
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et5)); 
  leds_on(LEDS_GREEN);      
  etimer_set(&et5, CLOCK_SECOND); 
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et5)); 
  leds_off(LEDS_GREEN);
  packetbuf_copyfrom(alert,strlen(alert)); //send the appended message 
  addr.u8[0] = linkaddr_node_addr.u8[0]+1; // ID+1 mote address
  addr.u8[1] = 0;
  unicast_send(&unicast, &addr ); //send to ID+1 mote 

  PROCESS_END();

}
PROCESS_THREAD(ACKrecieved, ev, data){
  PROCESS_BEGIN();

  static int j;
  printf("BUFF VALUE %d\n",data);
  if(linkaddr_node_addr.u8[0]+1==data){
    printf("ACK received from %d\n",data);
    ACKflag = 1;
  }

  PROCESS_END();
}

PROCESS_THREAD(FatherNode, ev, data){
  PROCESS_BEGIN();
  static int k;    
  static struct etimer et4;   
  printf("Father Node and I am going to blink my Red LED %d times\n",childcount);
  for(k=0;k<childcount;k++){ //blink the number of children
    etimer_set(&et4, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et4)); 
    leds_on(LEDS_RED);
    etimer_set(&et4, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et4));       
    leds_off(LEDS_RED);
    printf("LED Blinking %d time\n",k+1);
  }	
  printf("*************************************Cascade complete***************************************\n");
  printf("Restart Cascade with another Node\n");
  PROCESS_END();	
}
/*--------------------------Functions -------------------------------------------------*/




static void sendACK(){
  linkaddr_t addr1;
  packetbuf_copyfrom("ACK",3);
  addr1.u8[0] = linkaddr_node_addr.u8[0]-1; // ID-1 mote address
  addr1.u8[1] = 0;
  unicast_send(&unicast, &addr1 ); //send to ID-1 mote 
}

static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from){
  //variable declarations
  //Message callback function 	 
  if(linkaddr_node_addr.u8[0]-1==from->u8[0] ){
    process_start(&DisplyACK,from->u8[0]);                        
    //ACK process
    sendACK();
    //send string concatenated
    process_start(&StringAppend,NULL);
    process_start(&SendString,NULL);
    }
  if(linkaddr_node_addr.u8[0]+1==from->u8[0] ){
    process_start(&ACKrecieved,from->u8[0]);
    }  
  process_start(&ACKcheck,5);           
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static uint8_t active;
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(mainProcess, ev, data){
  PROCESS_EXITHANDLER(unicast_close(&unicast);)
  PROCESS_BEGIN();
  unicast_open(&unicast, 146, &unicast_callbacks);  
  SENSORS_ACTIVATE(button_sensor);
  active = 0;
  while(1) {
    static struct etimer et;
    linkaddr_t addr;
    PROCESS_WAIT_EVENT_UNTIL( ev == sensors_event && data == &button_sensor);
    if(!active) {
      /* First Child Process */
      char message[12]= "child1";
      packetbuf_copyfrom(message, strlen(message)); //Value to send    	
      addr.u8[0] = linkaddr_node_addr.u8[0]+1;
      addr.u8[1] = 0;
      //For sender	
      if(unicast_send(&unicast, &addr)){
        printf("The first child %d is sending to %d\n",linkaddr_node_addr.u8[0],linkaddr_node_addr.u8[0]+1);
        etimer_set(&et, CLOCK_SECOND); 
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        leds_on(LEDS_GREEN);
        etimer_set(&et, CLOCK_SECOND);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));       
        leds_off(LEDS_GREEN);
        etimer_reset(&et);        

      }
    }
    /* Listen */
    printf("Always Listening\n");  
    active ^= 1;
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/