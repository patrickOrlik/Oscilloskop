#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "UART.h"
#include "ADC.h"
#include <avr/io.h>
#include "SPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ssd1306.h"
#include "I2C.h"
#include <string.h>

void Uart1Transmit(uint16_t Dlength, char type, unsigned char data[]);
char buffer[32];
volatile unsigned char Data;
volatile int count = 0;
volatile int Datalength =0;
int indexcount = 0;
volatile unsigned char ADCdataA[30];
volatile unsigned char ADCdataB[30];
volatile unsigned char RXdata[256];
volatile bool Receiveflag = false;

volatile bool Adcready = false; // Used for checking if Adc is ready.
volatile bool RX1_COMPLETE_FLAG = false;

volatile unsigned int channel = 1; // channel selected
volatile unsigned char Adcvalue;    // Used for storing ADCvalues
unsigned char buf[6] = {0x01,0x010,0x17,0x04,0x10};
int main()
{

    DDRB |= (1 << PB7);
    DDRB &= ~(1 << PA0); // sets ADC0 as input (Pin(22))
    DDRH |= (1 << PH4);  // sets pin 7 as output.
    DDRB |= (PB0);

    sei();
    SPI_MasterInit();
    uart_init1();
    init_adc();
    CTC_init();
    I2C_Init();
    InitializeDisplay();
    clear_display();

    while (1)
    {
        if (Adcready){
        Uart1Transmit(37,0x02,ADCdataB);
        Adcready = false;
        
        }
    if(Receiveflag){
        clear_display();
        
        sprintf(buffer,"[0]:%02X,[1]:%02X",RXdata[0],RXdata[1]);
        sendStrXY(buffer,2,2);
        _delay_ms(10000);
        Receiveflag = false;
        

    }

   
}
}

ISR(ADC_vect)
{

   ADCdataA[indexcount] = ADC;
   indexcount++;
   if(indexcount == 30){
    memcpy(ADCdataB,ADCdataA,30);
   indexcount = 0;
   Adcready = true;
   }

}

// Timer interrupt service routine. updates channel and enables adc
ISR(TIMER0_COMPA_vect)
{
    // Start ADC conversion
    ADCSRA |= (1 << ADSC); // enables adc
}
ISR(USART1_RX_vect)
{
    Data = UDR1;
    PORTB ^= (1<< PB7);
           if (Receiveflag) return;// sikrer at vi når at kører alt koden i main når vi har modtaget en pakke
          RXdata[count] = Data;
             count++;
             if (count == 3)
             {
                 if (RXdata[0] == 0x55 && RXdata[1] == 0xAA)
                 {
                    
                }
                else {count = 0;}
            
            }
            else if (count == 4)
            {
                Datalength = (int)((RXdata[2] << 8) | RXdata[3]);
                
    
            }
            else if (count == Datalength +7)
            {
                 
                if(RXdata[Datalength+6] != 0x00 || RXdata[Datalength+5] != 0x00){
                    
                    count = 0;
                    Receiveflag = true;
                }
                else {
                Receiveflag = false;
                count = 0;
            }
   
}
}

void Uart1Transmit(uint16_t Dlength,char type,unsigned char data[]){
PORTB ^= (1<< PB7);
   unsigned char lsb = (unsigned)Dlength & 0xff; // mask the lower 8 bits
 unsigned char msb = (unsigned)Dlength >> 8;   // shift the higher 8 bits
 putcharuart1(0x55);
   putcharuart1(0xAA);
   putcharuart1(msb);
   putcharuart1(lsb);
   putcharuart1(type);
   for( int i = 0; i < (Dlength-7); i++ ){
    putcharuart1(data[i]);
   }
   putcharuart1(0x00);
   putcharuart1(0x00);
}