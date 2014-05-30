/*
 * m88_eth_01.c
 *
 * Created: 3/31/2014 8:31:28 PM
 *  Author: John
 */ 


/*********************************************
 * vim:sw=8:ts=8:si:et
 * To use the above modeline in vim you must have "set modeline" in your .vimrc
 * Author: Guido Socher
 * Copyright: GPL V2
 *
 * Read the silicon revision of the ENC28J60 via http or UDP
 *
 * http://tuxgraphics.org/electronics/
 * Chip type           : Atmega88/168/328/644 with ENC28J60
 *********************************************/
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include "../ip_stack/ip_arp_udp_tcp.h"
#include "../ip_stack/enc28j60.h"
#include "../ip_stack/timeout.h"
#include "../ip_stack/net.h"
#include "config.h"
#include <stdio.h>

// please modify the following two lines. mac and ip have to be unique
// in your local area network. You can not have the same numbers in
// two devices:
// how did I get the mac addr? x S L F x x 
static uint8_t defaultmac[6] = {0x00,0x53,0x4C,0x46,0x00,0x01};
static uint8_t defaultip[4] = {192,168,1,5}; // aka http://10.0.0.29/

// listen port for www
#define MYWWWPORT 8082
//// listen port for udp
#define MYUDPPORT_DEFAULT 4601

#define BUFFER_SIZE 550
static uint8_t buf[BUFFER_SIZE+1];

// set output to VCC, red LED off
#define LEDOFF PORTB|=(1<<PORTB1)
// set output to GND, red LED on
#define LEDON PORTB&=~(1<<PORTB1)
// to test the state of the LED
#define LEDISOFF PORTB&(1<<PORTB1)

uint16_t http200ok(void);
uint16_t http200ok(void)
{
        return(fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n")));
}

uint16_t print_webpage(uint8_t *buf);
// prepare the webpage by writing the data to the tcp send buffer
uint16_t print_webpage(uint8_t *buf)
{
        char vstr[5];
        uint16_t plen;
        plen=http200ok();
        plen=fill_tcp_data_p(buf,plen,PSTR("<center><p>ENC28J60 silicon rev is: B"));
        // convert number to string:
        itoa((enc28j60getrev()),vstr,10);
        plen=fill_tcp_data(buf,plen,vstr);
        plen=fill_tcp_data_p(buf,plen,PSTR("</center><hr><br>SLF.RO\n"));
        return(plen);
}

int main(void){
	
	
        uint16_t dat_p,plen;
        uint8_t payloadlen=0;
        char str[20];
        uint8_t SetNewConfig = 0;
		int config_rc = 0;
		Eth_config_t current_config;
		int scan_tmp[5];
		int scanf_rc =0 ;
		int reset_rc=0;
		int led_count=0;

        // Set the clock speed to "no pre-scaler" (8MHz with internal osc or 
        // full external speed)
        // set the clock prescaler. First write CLKPCE to enable setting 
        // of clock the next four instructions.
        // Note that the CKDIV8 Fuse determines the initial
        // value of the CKKPS bits.
		
		
		Init_Uart();
			LOG("Reset");
		GPIO_init();
		
		//check for reset values
		
		reset_rc = GetResetSw();

		
		config_rc=NVM_LoadConfig(&current_config);
		LOG("Mac:%x:%x:%x:%x:%x:%x\n",
									current_config.mac[0],
									current_config.mac[1],
									current_config.mac[2],
									current_config.mac[3],
									current_config.mac[4],
									current_config.mac[5]);
									
		LOG("IP and port: %d.%d.%d.%d:%d",
										current_config.ip[0],
										current_config.ip[1],
										current_config.ip[2],
										current_config.ip[3],
										current_config.port);
		if((config_rc<0)||(reset_rc!=0)){
			current_config.mac[0]=defaultmac[0];
			current_config.mac[1]=defaultmac[1];
			current_config.mac[2]=defaultmac[2];
			current_config.mac[3]=defaultmac[3];
			current_config.mac[4]=defaultmac[4];
			current_config.mac[5]=defaultmac[5];
			current_config.ip[0]=defaultip[0];
			current_config.ip[1]=defaultip[1];
			current_config.ip[2]=defaultip[2];
			current_config.ip[3]=defaultip[3];
			current_config.port=MYUDPPORT_DEFAULT;
			NVM_SaveConfig(&current_config);
			LOG("Config default");
			//goto CONFIG_SET;
		}
			LOG("Config Loaded");
		//we are ready 
		STATUS_ON();
		
		
		//USART_Transmit(0x30 + config_rc);
		
#if 0 // !defined(__AVR_ATmega8__)	
        CLKPR=(1<<CLKPCE);
        CLKPR=0; // 8 MHZ
#endif
        _delay_loop_1(0); // 60us
        
        /*initialize enc28j60*/
        enc28j60Init(current_config.mac);
			LOG("Init enc");
        enc28j60clkout(2); // change clkout from 6.25MHz to 12.5MHz
			LOG("change enc clk to 12.5mhz");
        _delay_loop_1(0); // 60us
        
        /* Magjack leds configuration, see enc28j60 datasheet, page 11 */
        // LEDB=yellow LEDA=green
        //
        // 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
        // enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
        enc28j60PhyWrite(PHLCON,0x476);

        DDRB|= (1<<DDB1); // LED, enable PB1, LED as output
        LEDOFF;
		

        
        //init the ethernet/ip layer:
        init_udp_or_www_server(current_config.mac,current_config.ip);	
			LOG("Init udp server and start tcp");
#ifdef WWW_server			
        www_server_port(MYWWWPORT);
#endif		

        while(1){
				led_count++;
                // handle ping and wait for a tcp packet:
                plen=enc28j60PacketReceive(BUFFER_SIZE, buf);
                dat_p=packetloop_arp_icmp_tcp(buf,plen);
				if(led_count<50){
					STATUS_ON();
				}else if (led_count<100){
					STATUS_OFF();
				}else
					led_count=0;
				if(SetNewConfig !=0 )
					goto CONFIG_SET;
                /* dat_p will ne unequal to zero if there is a valid 
                 * http get */
                if(dat_p==0){
                        // check for udp
                        goto UDP;
                }
#ifdef WWW_server
                // tcp port 80 begin
                if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0){
                        // head, post and other methods:
                        //
                        // for possible status codes see:
                        // http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
                        dat_p=http200ok();
                        dat_p=fill_tcp_data_p(buf,dat_p,PSTR("<h1>200 OK</h1>"));
                        goto SENDTCP;
                }
                if (strncmp("/ ",(char *)&(buf[dat_p+4]),2)==0){
                        dat_p=print_webpage(buf);
                        goto SENDTCP;
                }else{
                        dat_p=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Unauthorized</h1>"));
                        goto SENDTCP;
                }
SENDTCP:
                www_server_reply(buf,dat_p); // send web page data
                continue;
                // tcp port 80 end
#endif //WWW_server				
                //--------------------------
                // udp start, we listen on udp port 1200=0x4B0
UDP:
                // check if ip packets are for us:
                if(eth_type_is_ip_and_my_ip(buf,plen)==0){
                        continue;
                }
                if (buf[IP_PROTO_P] == IP_PROTO_UDP_V &&\
				    buf[UDP_DST_PORT_H_P] == (current_config.port>>8) &&\
					buf[UDP_DST_PORT_L_P] == (current_config.port&0xff)) {
                        payloadlen=buf[UDP_LEN_L_P]-UDP_HEADER_LEN;
                        // you must sent a string starting with v
                        // e.g udpcom version 10.0.0.24
  						if (buf[UDP_DATA_P]=='*' ){
							  scanf_rc = sscanf(&buf[UDP_DATA_P],"*%d.%d.%d.%d:%d", 
												&scan_tmp[0],
												&scan_tmp[1],
												&scan_tmp[2],
												&scan_tmp[3],
												&scan_tmp[4]
												);
																	
											//	USART_Transmit(0x30+scanf_rc);
								if(scanf_rc !=5 )				
									strcpy(str,"Config error: Please use format *IP:port");
								else{
									current_config.ip[0]=scan_tmp[0];
									current_config.ip[1]=scan_tmp[1];
									current_config.ip[2]=scan_tmp[2];
									current_config.ip[3]=scan_tmp[3];
									current_config.port=scan_tmp[4];
									NVM_SaveConfig(&current_config);
									strcpy(str,"Config set OK. Restarting...");
									SetNewConfig = 1;
								}
								
						}else
						if (buf[UDP_DATA_P]==':' ){
							scanf_rc = sscanf(&buf[UDP_DATA_P],":%x:%x",
							&scan_tmp[0],
							&scan_tmp[1]						
							);
							
							//	USART_Transmit(0x30+scanf_rc);
							if(scanf_rc !=2 )
							strcpy(str,"Config error: Please use format :a4:92 as mac5 and mac6!");
							else{
								current_config.mac[4]=scan_tmp[0];
								current_config.mac[5]=scan_tmp[1];
								NVM_SaveConfig(&current_config);
								strcpy(str,"Config set OK. Restarting...");
								SetNewConfig = 1;
							}
							
						}else
						
						{
							// strcpy(str,(char*)buf);
							//extern void USART_Transmit(uint8_t data);
							//USART_Transmit(0x31);
							extern void USART_print(char * text);
                               strcpy(str,"OK");
							   USART_print(&buf[UDP_DATA_P]);
							   
                        }
                        make_udp_reply_from_request(buf,str,strnlen(str,35),current_config.port);
                }
        }
		/// Reach here only if new config must be set
		
CONFIG_SET:		
		//USART_print("Rst_AVR");
		Reset_AVR();
        return (0);
}
