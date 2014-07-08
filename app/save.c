/*
 * GPIO.c
 *
 * Created: 6/15/2014 4:54:41 PM
 *  Author: John
 */ 

#define F_CPU 8000000ULL

#include "drivers.h"
#include <util/delay.h>


void GPIO_init(void){
	//pulldown pd6, pull-up pd7
	PORTD=(0<<PD6) | (1<<PD7);
	//pd6 - out, pd7 - input
	DDRD=(1<<PD6) | (0<<PD7);
}



int GetResetSw(void){
	int rc = 0;
	int i;
	if(RESET_SW_IS_PUSHED()){
		for(i=0;i<30;i++){
			STATUS_TOGGLE();
			_delay_ms(100);
		}
		if(RESET_SW_IS_PUSHED())
		rc = 1;
	}
	
	return rc;
}



// PD6 - led de stare
// PD7- pentru reset IP Adress


#define STATUS_LED_PORT		PORTD
#define STATUS_LED			(1<<PD6)
#define RESET_SW_PORT		PIND
#define RESET_SW_PIN		(1<<PIND7)

#define STATUS_ON()		STATUS_LED_PORT|=STATUS_LED
#define STATUS_OFF()	STATUS_LED_PORT&=~STATUS_LED
#define STATUS_TOGGLE()	STATUS_LED_PORT^=STATUS_LED

#define RESET_SW_IS_PUSHED()	!(RESET_SW_PORT&RESET_SW_PIN)


memcpy(mymac,currentConfig.local_mac,sizeof(mymac));
	memcpy(otherside_www_ip,currentConfig.server_ip,sizeof(otherside_www_ip));
	otherside_port = currentConfig.server_port;
		
	SYS_LOG("Config loaded\n");
	SYS_LOG("My mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
				currentConfig.local_mac[0],
				currentConfig.local_mac[1],
				currentConfig.local_mac[2],
				currentConfig.local_mac[3],
				currentConfig.local_mac[4],
				currentConfig.local_mac[5])	;
	SYS_LOG("Server : %d.%d.%d.%d:%d\n",
				currentConfig.server_ip[0],
				currentConfig.server_ip[1],
				currentConfig.server_ip[2],
				currentConfig.server_ip[3],
				currentConfig.server_port);	


