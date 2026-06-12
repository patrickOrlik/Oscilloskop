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
#define BTN 1 
#define SEND 2
#define START 3

enum buttons { BTN0,BTN1,BTN2,BTN3};

void Uart1Transmit(uint16_t Dlength, char type, unsigned char data[]);
char buffer[32];
volatile unsigned char Data;
volatile int count = 0;
unsigned char counter =0;
volatile int Datalength =0;
int indexcount = 0;
unsigned char ADCdataA[30];
unsigned char ADCdataB[30];
unsigned char RXdata[256];
unsigned char Settings[4] = {0,3,0x7f,0x05};
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
        
        
        switch(RXdata[4]){
        case BTN:
             switch(RXdata[5]){
                case BTN0:
                SPI_FpgaTransmit(0x00,0x01);
               SPI_FpgaTransmit(0x01,0x7F);
               SPI_FpgaTransmit(0x02,0x05);
             
               Receiveflag = false;

                break;
                
                case BTN1:
                counter++;
                Settings[0]=counter;
                Uart1Transmit(12,1,Settings);
                if (counter == 3){
                    counter = 0;
                }
                Receiveflag = false;

                break;
                case BTN2:
                Receiveflag = false;

                break;

                case BTN3:
                SPI_FpgaTransmit(0x04,0x00);
                memset(Settings,0,4);
                Receiveflag = false;

                break;


                default:
                Receiveflag = false;
                break;


                
             }

            
        break;



        case SEND:
        sendStrXY("SEND has been pressed",2,2);
        break;
        case START:
          sendStrXY("START has been pressed",2,2);
        break;
    
        default:
        continue;


        }
        
       
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
            else if (count == Datalength)
            {
                 
                if(RXdata[Datalength] == 0x00 && RXdata[Datalength-1] == 0x00){
                    
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