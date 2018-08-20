/**
 * \file
 *         A TCP socket echo server. Listens and replies on port 8080
 * \author
 *         mds
 */

#include "contiki-net.h"
#include "sys/cc.h"
#include "dev/leds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys/etimer.h"
#include "sys/stimer.h"
#include "sys/rtimer.h"
#include "board-peripherals.h"
#include "ti-lib.h"

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
int isBuzzerOn = 0;

char * resolve_http_request(char * req){
 	char *currPtr = strtok(req,"/");
	for(int i =0;i<2;i++){		
		if(i==1){
 			return currPtr; 
		}
		currPtr = strtok (NULL, " ");
	}
	return NULL;
}

void toggleLeds(char *led,char* param){
	if(!strcmp(led,"r")){
		leds_toggle(LEDS_RED);
	}
}

int toggleBuzzer(char *freq){
	if(isBuzzerOn ==0){
		int bFreq = atoi(freq);
		buzzer_start(bFreq);
		isBuzzerOn =1;
		return isBuzzerOn;
	}else{
		isBuzzerOn =0;
		buzzer_stop();
		return isBuzzerOn;
	}	
}
char * getPressureReading(char *numSamples){
	int num_samples = atoi(numSamples);
	char *response = malloc(sizeof(char) * num_samples);
	int j = 0;
	for(int i= 0;i<num_samples;i++){
		response[j] = bmp_280_sensor.value(BMP_280_SENSOR_TYPE_TEMP);
		j++;
		response[j] = ' ';
		j++;
	}
	return response; 
}

char * resolve_request(char *req){
	char * currPtr = strtok(req,"/");
	char * response; 
	printf("request is %s",currPtr);
	if(!strcmp(currPtr,"leds")){
		char *led = strtok(NULL,"/");
		char *param = strtok(NULL,"/");
		toggleLeds(led,param);
		response = "leds on";
	}else if(!strcmp(currPtr,"buzzer")){
		char *freq = strtok(NULL,"/");
		int on = toggleBuzzer(freq);
		if(on ==1){
			response = "buzzer on";
		}else{
			response = "buzzer off";	
		}
	}else if (!strcmp(currPtr,"pressure")){
		char *numSamples = strtok(NULL,"/");
		response =getPressureReading(numSamples);

	}else if(!strcmp(currPtr,"humidity")){
		response ="OK";
	}else{
		response = "request not recognised";
	}
return response;
}
/*---------------------------------------------------------------------------*/
//Input data handler
static int input(struct tcp_socket *s, void *ptr, const uint8_t *inputptr, int inputdatalen) {

	printf("input %d bytes '%s'\n\r", inputdatalen, inputptr);

	char * data = (char *)inputptr;
	char * url = resolve_http_request(data);
	char * response = resolve_request(url);

	//Clear buffer
	tcp_socket_send_str(&socket, response);	//Reflect byte
	//memset(inputptr, 0, inputdatalen);
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
  	SENSORS_ACTIVATE(bmp_280_sensor); //start pressure sensor. 
	//Register TCP socket
  	tcp_socket_register(&socket, NULL,
               inputbuf, sizeof(inputbuf),
               outputbuf, sizeof(outputbuf),
               input, event);
  	tcp_socket_listen(&socket, SERVER_PORT);

	printf("Listening on %d\n", SERVER_PORT);
	
	while(1) {
	
		//Wait for event to occur
		//PROCESS_PAUSE();
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
