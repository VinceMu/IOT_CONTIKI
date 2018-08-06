/**
 * \file
 *         A simple Contiki application showing serial interfacing
 * \author
 *         mds
 */
#include "contiki.h"
#include "dev/serial-line.h"
#include "buzzer.h"
#include "sys/timer.h"
#include "dev/leds.h"
#include <stdio.h> /* For printf() */
#include <string.h>
#include "dev/cc26xx-uart.h"
#include "ieee-addr.h"
#include "sys/etimer.h"
#include "sys/stimer.h"
#include "sys/rtimer.h"
unsigned char addr[256];
int buzz_state = 0; 
int buzz_freq = 100000;

static struct ctimer dtimer;
static struct ctimer itimer;
/*---------------------------------------------------------------------------*/
PROCESS(test_serial, "Serial line test process");
AUTOSTART_PROCESSES(&test_serial);
/*---------------------------------------------------------------------------*/

void do_itimer(){
	buzz_freq -=50000;
}
void do_dtimer(){
	buzz_freq +=50000;
}

//Serial Interface
PROCESS_THREAD(test_serial, ev, data) {

	PROCESS_BEGIN();

	cc26xx_uart_set_input(serial_line_input_byte);	//Initalise UART in serial driver
	
   	while(1) {

     	PROCESS_YIELD();	//Let other threads run

		//Wait for event triggered by serial input
     	
		//******************************************
		//NOTE: MUST HOLD CTRL and then press ENTER 
		//at the end of typing for the serial driver 
		//to work. Serial driver expects 0x0A as
		//last character, to tigger the event.
		//******************************************
		if(ev == serial_line_event_message) {
		

			if(strcmp((char *)data,"r")){
				leds_toggle(LEDS_RED);
			}
			if(strcmp((char *)data,"g")){
				leds_toggle(LEDS_GREEN);
			}
			if(strcmp((char *)data,"a")){
				leds_toggle(LEDS_ALL);
			}
			if(strcmp((char *)data,"b")){
				if(buzz_state ==0){
				   buzzer_start(buzz_freq);
				   buzz_state =1;
				}else if(buzz_state ==1){
				   buzzer_stop();
				   buzz_state = 0; 				
				}
			}
			if(strcmp((char *)data,"i")){
				ctimer_set(&itimer,CLOCK_SECOND *5,do_itimer,NULL);
				buzz_freq += 50000;
				
			}
			if(strcmp((char *)data,"d")){
				ctimer_set(&dtimer,CLOCK_SECOND *5,do_dtimer,NULL);
				buzz_freq -= 50000;
			}
			if(strcmp((char *)data,"n")){
				uint8_t* dst = NULL;
				for(int i=0;i<5;i++){
					ieee_addr_cpy_to (dst, i);
				}
				printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
					 dst[0] & 0xff, dst[1] & 0xff, dst[2] & 0xff,
					 dst[3] & 0xff, dst[4] & 0xff, dst[5] & 0xff);
				
			}


     		}
		

		clock_wait(CLOCK_SECOND/10);

   	}
   	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
