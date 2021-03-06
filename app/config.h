/*
 * config.h
 *
 * Created: 5/23/2014 12:47:20 PM
 *  Author: John
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_
#include <avr/io.h>
#include <avr/wdt.h>


#define STATUS_LED_PORT		PORTD
#define STATUS_LED			(1<<PD6)
#define RESET_SW_PORT		PIND
#define RESET_SW_PIN		(1<<PIND7)

#define STATUS_ON()		STATUS_LED_PORT|=STATUS_LED
#define STATUS_OFF()	STATUS_LED_PORT&=~STATUS_LED
#define STATUS_TOGGLE()	STATUS_LED_PORT^=STATUS_LED

#define RESET_SW_IS_PUSHED()	!(RESET_SW_PORT&RESET_SW_PIN)

//#define LOGGING 1

#ifdef LOGGING
#define LOG(...)	printf(__VA_ARGS__)
#else
#define LOG(...)	{}
#endif

void GPIO_init(void);
int GetResetSw(void);


void Init_Uart(void);
void USART_print(char * text);
uint8_t USART_Receive(uint8_t * data);
void USART_Transmit(uint8_t data);

typedef struct Eth_config{
	uint8_t		mac[6];
	uint8_t		ip[4];
	uint16_t	port;
	uint8_t		status;
	}Eth_config_t;


uint8_t get_loop(void);
uint8_t NVM_GetCurrentPosition(void);
int NVM_LoadConfig(Eth_config_t *data);
void NVM_SaveConfig(Eth_config_t *data);

// Reset AVR using watchdog
#define Reset_AVR() wdt_enable(WDTO_500MS); while(1) {}


#endif /* CONFIG_H_ */