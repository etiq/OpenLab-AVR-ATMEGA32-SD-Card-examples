//**********************************************************
// **** MAIN routine FOR Interfacing microSD/SD CARD ******
/**********************************************************
Controller: ATmega32 (16 Mhz internal)
Compiler	: AVR-GCC
Author	     :Etiq technologies
Date		: 5/18/2016
DESCRIPTION: The code is for raw data writing to the SD card using ATmega32 controller.
********************************************************/



#define F_CPU 16000000UL		//freq 16 MHz
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "SPI_routines.h"
#include "SD_routines.h"
#include "UART_routines.h"
unsigned int last_secnum;

void port_init(void)
{
PORTA = 0x00;
DDRA  = 0x00;
//PORTB = 0xEF;
PORTB = 0x40;
DDRB  = 0xBF;    //MISO line i/p, rest o/p
PORTC = 0x00;
DDRC  = 0xFF;
PORTD = 0x00;
DDRD  = 0xFE;
}

void uart0_init(void)
{
 unsigned int baudrate_value=103;
 UBRRL=baudrate_value;                             //Loading the baud rate generator registers
 UBRRH=(baudrate_value>>8);

 UCSRB=(1<<TXEN)|(1<<RXEN);                       //Enabling transmission and reception

 UCSRC=(1<<URSEL)|(3<<UCSZ0);                    //accessing the UCSRC register and selecting 8 bit data format
}

//call this routine to initialize all peripherals
void init_devices(void)
{
 cli();
 port_init();
 spi_init();
 uart0_init();

 MCUCR = 0x00;
 GICR  = 0x00;
 TIMSK = 0x00; //timer interrupt sources
 //all peripherals are now initialized
}

/* function to write into the card from last written sector*/

void SD_write(unsigned char *ptr)
{
	static unsigned  int Sect_num=2;
    Card_write(Sect_num,ptr,1);
	Sect_num++;
	_delay_ms(50);
	last_secnum=Sect_num;
}


/* reads string from uart and write int the card  to next sector after the last written*/

void Enter_string()
{
unsigned char *ptr;
unsigned int i;
ptr=(char *)malloc(512*sizeof(char));
transmitString("\r\n enter the string ending with ~ \r\n");
for(i=0;i<512;i++)
{
*(ptr+i)=receiveByte();
transmitByte(*(ptr+i));
if(*(ptr+i)=='~')
break;	
}
*(ptr+i)='\0';
SD_write(ptr);     //function call to write into SD card
free(ptr);
}

/*functi0n which reads the SD card from sect 2 to last written sector*/
void Read_string()
{
unsigned char *str;
unsigned int i;
str=(char *)malloc(512*sizeof(char));
for(i=2;i<last_secnum;i++)
{
Card_read(i,str,1);
transmitString("\r\n");
transmitString(str);	
}
free(str);
}


/* function to write a specific sector preferably from 0-99 for simplification of code */
void Write_sector()
{   
	unsigned char sect_num,*ptr;
	unsigned int i;
	transmitString("Enter sector number 00-99");
	sect_num=(receiveByte()-48)*10;
	sect_num=sect_num|(receiveByte()-48);
	transmitString("enter the string");
	ptr=(char *)malloc(512*sizeof(char));
	transmitString("\r\n enter the string ending with ~ \r\n");
	for(i=0;i<512;i++)
	{
		*(ptr+i)=receiveByte();
		transmitByte(*(ptr+i));
		if(*(ptr+i)=='~')
		break;
	}
	*(ptr+i)='\0';
	 Card_write(sect_num,ptr,1);
	 free(ptr);
}


/* function to read a specific sector preferably from 0-99 for simplification of code */
void Read_sector()
{
		unsigned char sect_num,*ptr;
		unsigned int i;
		transmitString("Enter sector number 00-99");
	    sect_num=(receiveByte()-48)*10;
	   sect_num=sect_num|(receiveByte()-48);
	   ptr=(char *)malloc(512*sizeof(char));
	   Card_read(sect_num,ptr,1);
	   transmitString("\r\n");
	   transmitString(ptr);
}

/*select the appropriate operation*/

void switch_operation()
{

	do
	{
		transmitString("Enter the operation\r\n1.Write \r\n2.Read \r\n3.Write sector \r\n4.Read Sector");
		switch(receiveByte())
		{
			case '1':
			Enter_string();
			break;
			case '2':
			Read_string();
			break;
			case '3':
			Write_sector();
			break;
			case '4':
			Read_sector();
			break;
		}
	} while (1);
	
	
}




//*************************** MAIN *******************************//
int main(void)
{

_delay_ms(100);  //delay for VCC stabilization

init_devices();

TX_NEWLINE;
TX_NEWLINE;
transmitString_F (PSTR("***********************************"));
TX_NEWLINE;
transmitString_F (PSTR("UART Communication OK!!!!!."));
TX_NEWLINE;
transmitString_F (PSTR("***********************************"));
TX_NEWLINE;

if(SD_init())                                       //initialises the SD card
transmitString_F (PSTR("card initialized"));
else
transmitString_F (PSTR("card not initialized"));
switch_operation();


while(1)                                          //infinite loop
{
	
}
}

