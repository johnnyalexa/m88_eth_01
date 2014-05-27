/*
 * NVM.c
 *
 * Created: 4/30/2014 10:36:25 PM
 *  Author: John
 */ 
#include "config.h"
#include <avr/io.h>
#include <avr/eeprom.h>


#define EEPROM_START		0u
#define EEPROM_LEN			255u

#define CONFIG_LEN		sizeof(Eth_config_t)
#define EEPROM_LOOP		(EEPROM_LEN/CONFIG_LEN)

#define STATUS_START_ADDRESS (CONFIG_LEN - 1)
#define FIRST_USE_LOCATION 0

/* Shows current buffer position */
static uint8_t FirstRound = 0;
static uint8_t ConfigIndex;
static uint8_t StatusBufferValue;

uint8_t get_loop(void){
	
	return EEPROM_LOOP;
}


/*-------------------- EEPROM_GetCurrentPosition   -------------------------
*    Function:    EEPROM_GetCurrentPosition
*    Purpose:    Loads Current position in EEPROM.
*
*    Parameters: none
*    Returns: none
*------------------------------------------------------------*/
uint8_t NVM_GetCurrentPosition(void){
	uint8_t U8_value1,U8_value2;
	uint8_t U8_position= 0;
	
	//Read first status byte	
	U8_value1 = eeprom_read_byte((const uint8_t *)STATUS_START_ADDRESS);
	if(U8_value1 == 0xff){
		ConfigIndex = U8_position;
		StatusBufferValue = 1;
		FirstRound = 1;
		return U8_position;
	}
	
	for(U8_position=0;U8_position<EEPROM_LOOP-1;U8_position++){
		//Read next status byte
		U8_value2 = eeprom_read_byte((const uint8_t *)(STATUS_START_ADDRESS + ((U8_position+1)*CONFIG_LEN)));
		//If next status byte is current status byte incremented	
		if(U8_value2 == (uint8_t)(U8_value1+1)){
				//Change the values and continue the loop
				U8_value1 = U8_value2;
			//	continue;
			}else{
				ConfigIndex = U8_position;
				StatusBufferValue = U8_value1;
				break;
			}
		}
	return U8_position;	
}

static void IncrementLocation(void){
	ConfigIndex ++;
	if(ConfigIndex >= EEPROM_LOOP)
		ConfigIndex = 0;
	StatusBufferValue = (uint8_t)(StatusBufferValue + 1);		
}

int NVM_LoadConfig(Eth_config_t *data){
	int rc = 0;
	rc = NVM_GetCurrentPosition();
	eeprom_read_block(data,(void *)(ConfigIndex*CONFIG_LEN),CONFIG_LEN);
	if((data->ip[0]==0xff)||
		(data->ip[0]==0))
		rc =-1;
	return rc;
}

void NVM_SaveConfig(Eth_config_t *data){
	//Save the data to a new location
	if (FirstRound != 1){
		IncrementLocation();
	}else
		FirstRound = 0;
	data->status=StatusBufferValue;
	eeprom_write_block((uint8_t *)data,(void *)(ConfigIndex*CONFIG_LEN),CONFIG_LEN);
}