//**************************************************************
// ****** FUNCTIONS FOR SD RAW DATA TRANSFER *******
//**************************************************************
//Controller: ATmega32 (8 Mhz internal)
//Compiler	: AVR-GCC
//Version	: 2.2
//Author	: CC Dharmani, Chennai (India)
// 			  www.dharmanitech.com
//Date		: 15 July 2009
//**************************************************************

//**************************************************
// ***** SOURCE FILE : SD_routines.c ******
//**************************************************
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "SPI_routines.h"
#include "SD_routines.h"
#include "UART_routines.h"

//******************************************************************
//Function: to initialize the SD card in SPI mode
//Arguments: none
//return: unsigned char; will be 0 if no error,
// otherwise the response byte will be sent
//******************************************************************
unsigned char SD_init(void)
{
unsigned char i, response=0x00, retry=0 ;

SD_CS_DEASSERT;
PORTB|=(1<<5);
for(i=0;i<10;i++)
SPI_transmit(0xFF);
SD_CS_ASSERT;
retry=0;
	
SD_sendCommand(0,0,0x95);
do 
{
response=SPI_receive();

retry++;
if(retry==2)
{

	retry=0;
SD_sendCommand(0,0,0x95);	
}
} while (response!=0x01);


if(response==0x01)
{
//transmitByte('c');
//retry=0;
do{
	SD_sendCommand(0x77,  0 , 0xFF);
	response=SPI_receive();
	do 
	{
	response=SPI_receive();
	retry++;	
	} while ((response!=0x01)&&(retry<10));

SPI_receive();
retry=0;
SD_sendCommand(0x69,  0x40000000 , 0xFF);
response=SPI_receive();
do
{
	response=SPI_receive();
	retry++;
} while ((response!=0x00)&&(retry<10));
}while(response);

if(!response)
return 1;
else 
return 0;
}
else
return 0;
 }

//******************************************************************
//Function: to send a command to SD card
//Arguments: unsigned char (8-bit command value)
// & unsigned long (32-bit command argument)
//return: unsigned char; response byte
//******************************************************************
unsigned char SD_sendCommand(unsigned char cmd, unsigned long int arg,char CRC_bit)
{
unsigned char response, retry=0;

SD_CS_ASSERT;

SPI_transmit(0xFF);
SPI_transmit(cmd | 0x40); //send command, first two bits always '01'
SPI_transmit((unsigned char)(arg>>24));
SPI_transmit((unsigned char)(arg>>16));
SPI_transmit((unsigned char)(arg>>8));
SPI_transmit((unsigned char)(arg));
SPI_transmit(CRC_bit);
}








void Card_write(unsigned long sector,unsigned char *str,unsigned long sector_count)
{
unsigned char response;
unsigned long i,start_addr,n;
start_addr=512*sector;	
SD_sendCommand(25,start_addr,0xff);
while(response=SPI_receive());
SPI_transmit(0xFF);
SPI_transmit(0xFF);
SPI_transmit(0b11111100);
for(i=0;i<sector_count;i++)	
{
for(n=0;((n<512)&&(*str));n++)
{
	SPI_transmit(*str);
	str++;	
	
}
if(!(*str))
{
SPI_transmit(*str);
break;
}
}
SPI_transmit(0xff);
SPI_transmit(0xff);
response=SPI_receive();
 while ((response & 0b00011111) != 0x05)
 {
	response=SPI_receive();
 }
 while (response!= 0xff)
 {
response=SPI_receive();
 }
SPI_transmit(0xff);
SPI_transmit(0xff);
SPI_transmit(0b11111101);                   //stop token
response=SPI_receive();
response=SPI_receive();
 while (response!= 0xff)
 {
response=SPI_receive();
 }
}

void Card_read(unsigned long sector, unsigned char* buffer,unsigned long sector_count)
{
	
	unsigned long i, Start_Addr;
	unsigned int n;
	unsigned char response;
    unsigned char *result=buffer;
	Start_Addr = sector * 512;
	SD_sendCommand(18, Start_Addr, 0xff);
	while (response!= 0)
	{
		response=SPI_receive();
	}
	while (response!= 0xfe)
	{
		response=SPI_receive();
	}                                                               // Add platform specific sector (512 bytes) read code here
	                                                                       // Add platform specific sector (512 bytes) write code here
		for (n = 0;n<512; n++)
		{
			response=SPI_receive();
			*(buffer+n) =response;
			if(!(*(buffer+n)))
			break;
			}
		

	SPI_transmit(0xff);
	SPI_transmit(0xff);

	SD_sendCommand(12, 0, 0xff);
	response=SPI_receive();
	while (response!= 0)
	{
		response=SPI_receive();
	}
	while (response!= 0xff)
	{
		response=SPI_receive();
	}

}