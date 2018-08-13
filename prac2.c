/**
 * \file
 *         A TCP socket echo server. Listens and replies on port 8080
 * \author
 *         mds
 */

#include "contiki-net.h"
#include "contiki.h"

#include "sys/cc.h"
#include "sys/etimer.h"
#include "sys/stimer.h"
#include "sys/rtimer.h"
#include "buzzer.h"
#include "sys/timer.h"
#include "dev/leds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PORT 8080

static struct tcp_socket socket;

#define INPUTBUFSIZE 400
static uint8_t inputbuf[INPUTBUFSIZE];

#define OUTPUTBUFSIZE 400
static uint8_t outputbuf[OUTPUTBUFSIZE];

PROCESS(tcp_server_process, "TCP echo process");
AUTOSTART_PROCESSES(&tcp_server_process);
static uint8_t get_received;
static int bytes_to_send;

int buzz_state = 0; 
int buzz_freq = 1000;
int led_colour = -1;
int led_toggle = 0; 

static struct ctimer dtimer;
static struct ctimer itimer;
static struct ctimer ledtimer; 

void do_itimer(){
	buzz_freq -=50;
	buzzer_start(buzz_freq);
}
void do_dtimer(){
	buzz_freq +=50;
	buzzer_start(buzz_freq);
}
void do_ledtimer(){
	leds_toggle(led_colour);
	ctimer_reset(&ledtimer);
}


void parse(char * data){
	if(!strcmp(data,"r")){
		if(led_toggle ==1){
			led_toggle = 0;
			led_colour = LEDS_RED;
			ctimer_set(&ledtimer,CLOCK_SECOND /2,do_ledtimer,NULL);
		}else if(led_toggle ==0){
			led_toggle =1;
			ctimer_stop(&ledtimer);
		}
	}
	if(!strcmp(data,"g")){

		if(led_toggle ==1){

			led_toggle = 0;
			led_colour = LEDS_GREEN;
			ctimer_set(&ledtimer,CLOCK_SECOND /2,do_ledtimer,NULL);
		}else if(led_toggle ==0){
			led_toggle =1;
			ctimer_stop(&ledtimer);
		}

	}
	if(!strcmp(data,"a")){
			if(led_toggle ==1){
			led_toggle = 0;
			led_colour = LEDS_ALL;
			ctimer_set(&ledtimer,CLOCK_SECOND /2,do_ledtimer,NULL);
		}else if(led_toggle ==0){
			led_toggle =1;
			ctimer_stop(&ledtimer);
		}
	}
	if(!strcmp(data,"b")){
		if(buzz_state ==0){
		 printf("starting buzzer\n");
		   buzzer_start(buzz_freq);
		   buzz_state =1;
		}else if(buzz_state ==1){
		   buzzer_stop();
		   buzz_state = 0; 				
		}
	}
	if(!strcmp(data,"i")){
		ctimer_set(&itimer,CLOCK_SECOND *5,do_itimer,NULL);
		buzz_freq += 50;
		buzzer_start(buzz_freq);
	
	}
	if(!strcmp(data,"d")){
		ctimer_set(&dtimer,CLOCK_SECOND *5,do_dtimer,NULL);
		buzz_freq -= 50;
		buzzer_start(buzz_freq);
	}
}

/*---------------------------------------------------------------------------*/
//Input data handler
static int input(struct tcp_socket *s, void *ptr, const uint8_t *inputptr, int inputdatalen) {

  	printf("input %d bytes '%s'\n\r", inputdatalen, inputptr);
	const char * data = (char *)inputptr; 
        parse(data);
	tcp_socket_send_str(&socket, inputptr);	//Reflect byte

	//Clear buffer
	memset(inputptr, 0, inputdatalen);
    return 0; // all data consumed 
}

/*---------------------------------------------------------------------------*/
//Event handler
static void event(struct tcp_socket *s, void *ptr, tcp_socket_event_t ev) {
  printf("event %d\n", ev);
}

/*---------------------------------------------------------------------------*/
//TCP Server process
PROCESS_THREAD(tcp_server_process, ev, data) {

  	PROCESS_BEGIN();

	//Register TCP socket
  	tcp_socket_register(&socket, NULL,
               inputbuf, sizeof(inputbuf),
               outputbuf, sizeof(outputbuf),
               input, event);
  	tcp_socket_listen(&socket, SERVER_PORT);

	printf("Listening on %d\n", SERVER_PORT);
	
	while(1) {
	
		//Wait for event to occur
		PROCESS_PAUSE();
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
