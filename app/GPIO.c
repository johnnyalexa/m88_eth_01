/*
 * GPIO.c
 *
 * Created: 5/29/2014 11:57:13 AM
 *  Author: ionut.alexa
 */ 
#define F_CPU 8000000ULL

#include "config.h"
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