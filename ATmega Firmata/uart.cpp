/*
 * uart.c
 *
 * Created: 03/06/2012 12:46:48 �
 *  Author: HP
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"


#if UART_RX0_INTERRUPT == ENABLED
//volatile unsigned char uartRx0Flag=0;
volatile unsigned char uartRx0databuffer[128];
volatile int index0=0;
#endif 

#if UART_RX1_INTERRUPT == ENABLED
//volatile unsigned char uartRx1Flag=0;
#define UART_RX1_BUFFER_SIZE 64
#define UART_RX1_BUFFER_MASK ( UART_RX1_BUFFER_SIZE - 1)
#define UART1_STATUS   UCSR1A
#define UART1_CONTROL  UCSR1B
#define UART1_DATA     UDR1
volatile unsigned char uartRx1databuffer[BUFFER_SIZE];
static volatile uint8_t UART1_RxBuf[UART_RX1_BUFFER_SIZE];
volatile int index1=0;
static volatile uint16_t UART1_RxHead;
static volatile uint16_t UART1_RxTail;
static volatile uint8_t  UART1_LastRxError;
#endif 

void UartInit(uint8 serialPort,uint16 baudRate){
	
	
			//UBRR0H=0x00;

	
	switch(serialPort)
	{
		case 0:
		    UCSR0A=0x00;
		  	#if UART_RX0_INTERRUPT == DISABLED	  
			UCSR0B=UCSR0B|(1<<TXEN0)|(1<<RXEN0);		
			UCSR0C=UCSR0C|(1<<URSEL0)|(1<<UCSZ00)|(1<<UCSZ01);
			
			#else
		
		    UCSR0B=UCSR0B|(1<<TXEN0)|(1<<RXEN0)|(1<<RXCIE0);		
			UCSR0C=UCSR0C|(1<<URSEL0)|(1<<UCSZ00)|(1<<UCSZ01);
		
			#endif
			/*
			UBRR0L = (byte) (baudRate &0x00ff) ;
			UBRR0H = (byte)((baudRate &0xff00)>>8);
				*/
			UBRR0L= 16; // 57600 single speed
			
		break;
		
		case 1:
		    UCSR1A=0x00;
		 	#if UART_RX1_INTERRUPT == DISABLED
			UCSR1B=UCSR1B|(1<<TXEN1)|(1<<RXEN1);		
			UCSR1C=UCSR1C|(1<<URSEL1)|(1<<UCSZ10)|(1<<UCSZ11);
			#else
			UCSR1B=UCSR1B|(1<<TXEN1)|(1<<RXEN1)|(1<<RXCIE1);		
			UCSR1C=UCSR1C|(1<<URSEL1)|(1<<UCSZ10)|(1<<UCSZ11);
			#endif 
		  /*  UBRR1L = (byte) (baudRate &0x00ff) ;
		    UBRR1H =(byte)((baudRate &0xff00)>>8);
             */
		  UBRR1L= 16; // 57600 single speed
			
		break;
		
		default:
			break;
		
	}
	
}

void UartEnd(uint8 serialPort)
{
	if (serialPort==0)
	{
		UCSR0B &=0x67;
	}
	else
	{
		UCSR1B &=0x67;
	}

	
}

void UartTx0(unsigned char data){
	
	while((UCSR0A&(1<<UDRE0))==0)
	{
		
	}
	UDR0=data;
	
}

void UartTx1(unsigned char data){
	
	while((UCSR1A&(1<<UDRE1))==0)
	{
		
	}
	UDR1=data;
	
}

#if UART_RX0_INTERRUPT == ENABLED

int UartRx0(){
	
	int data;
	 int i;
	if(index0 <= 0) return-1;
	
	else
	{
	data=uartRx0databuffer[0];
	
	
	for(i=0;i<index0;i++)
		uartRx0databuffer[i]=uartRx0databuffer[i+1];
	
	index0--;	
	/*if(index0<0)
		index0=0;
		*/
	return data;
	}
}


int getuartRx0Flag(){
	return index0;
	
}
ISR (USART0_RXC_vect){
	
	//uartRx0Flag=1;
	uartRx0databuffer[index0]=UDR0;
	index0++;
}



int UartRx1(){
	
	uint16_t tmptail;
	uint8_t data;

	if ( UART1_RxHead == UART1_RxTail ) {
		return UART_NO_DATA;   /* no data available */
	}

	/* calculate /store buffer index */
	tmptail = (UART1_RxTail + 1) & UART_RX1_BUFFER_MASK;
	UART1_RxTail = tmptail;

	/* get data from receive buffer */
	data = UART1_RxBuf[tmptail];

	return (UART1_LastRxError << 8) + data;

}


int getuartRx1Flag(){
	return index1;
	
}

ISR (USART1_RXC_vect){
	
	//uartRx1Flag=1;
	//uartRx1databuffer[index1]=UDR1;
	//index1++;
	uint16_t tmphead;
	uint8_t data;
	uint8_t usr;
	uint8_t lastRxError;

	/* read UART status register and UART data register */
	usr  = UART1_STATUS;
	data = UART1_DATA;

	/* */
	lastRxError = (usr & (_BV(FE1)|_BV(DOR1)) );

	/* calculate buffer index */
	tmphead = ( UART1_RxHead + 1) & UART_RX1_BUFFER_MASK;

	if ( tmphead == UART1_RxTail ) {
		/* error: receive buffer overflow */
		lastRxError = UART_BUFFER_OVERFLOW >> 8;
		} else {
		/* store new index */
		UART1_RxHead = tmphead;
		/* store received data in buffer */
		UART1_RxBuf[tmphead] = data;
	}
	UART1_LastRxError = lastRxError;
	
}

#else
int UartRx0(){
	
	while ( (UCSR0A & (1<<RXC0))==0);
	return UDR0;
}

int UartRx1(){
	
	
	while ( !(UCSR1A & (1<<RXC1)) );
	return UDR1;
}

#endif
