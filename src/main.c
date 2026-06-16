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
// Enums and definitions
#define BTN 1 
#define SEND 2
#define START 3
enum buttons { BTN0,BTN1,BTN2,BTN3};
enum SPI_Types{shape,ampl,freq,run,reset};
void Uart1Transmit(uint16_t Dlength, char type, unsigned char data[]);
int comp(const void *a, const void *b);
// Declarations of all variables and flags
volatile unsigned char Data;
volatile int count = 0;
unsigned char SelectCounter =0;
volatile int Datalength =0;
volatile int indexcount = 0;
volatile bool Adcready = false; // Used for checking if Adc is ready.
volatile bool RX1_COMPLETE_FLAG = false;
volatile unsigned int channel = 1; // channel selected
volatile unsigned char Adcvalue;    // Used for storing ADCvalues
volatile bool Receiveflag = false;
unsigned char amplitude;


// declaration of all data arrays 
unsigned char ADCdataA[1000];
unsigned char ADCdataB[1000];
unsigned char RXdata[256];
unsigned char Settings[4] = {0,3,0x7f,0x05};
uint16_t OscSettings[2] = {10000,30}; //stores Osciloscope settings [0]= Sample rate [1]= Packet length
unsigned char SortingArray[100];
unsigned char BodeArray[256];



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
    timer1_SetFreq(OscSettings[0]);
    Uart1Transmit(12,1,Settings);

    
    while (1)
    {
        if (Adcready){
       Uart1Transmit(OscSettings[1]+7,0x02,ADCdataB);
        Adcready = false;
        }
    if(Receiveflag){
          
        switch(RXdata[4]){
        case BTN:
             switch(RXdata[5]){
                case BTN0:
                Settings[SelectCounter+1]= RXdata[6];
                Uart1Transmit(12,1,Settings);
                SPI_FpgaTransmit((SelectCounter-1),RXdata[6]);
               Receiveflag = false;

                break;
                
                case BTN1:
                SelectCounter++;
                Settings[0]=SelectCounter;
                Uart1Transmit(12,1,Settings);
                if (SelectCounter == 3){
                    SelectCounter = 0;
                }
                Receiveflag = false;

                break;
                case BTN2:
                SPI_FpgaTransmit(0x03,0x00);
                Receiveflag = false;

                break;

                case BTN3:
                SPI_FpgaTransmit(0x04,0x00);
                memset(Settings,0,4);
                Uart1Transmit(12,1,Settings);

                Receiveflag = false;

                break;


                default:
                Receiveflag = false;
                break;


                
             }

            
        break;



        case SEND:
        OscSettings[0] = (RXdata[5]<< 8) | RXdata[6];
        OscSettings[1] =(RXdata[7]<< 8) | RXdata[8];
        timer1_SetFreq(OscSettings[0]);
        Receiveflag = false;


        break;
        case START:
        OscSettings[1]= 100;
        SPI_FpgaTransmit(shape,0x03);
        SPI_FpgaTransmit(ampl,0xFF);
        SPI_FpgaTransmit(run,0x00);

        for(int i = 0; i <= 255; i++){
            timer1_SetFreq(1000+(i*300));
            SPI_FpgaTransmit(freq,i);
            while(!Adcready);
            memcpy(SortingArray,ADCdataB,100);
            qsort(SortingArray,100,sizeof(unsigned char),comp);
            unsigned char amplitude = (SortingArray[99]-SortingArray[0]);
            BodeArray[i] = (1023-amplitude);
        }
        Uart1Transmit(263,0x03,BodeArray);
          Receiveflag = false;
        break;
    
        default:
         Receiveflag = false;
        continue;


        }
        
       
    }

   
}
}

ISR(ADC_vect)
{

   ADCdataA[indexcount] = ADC;
   indexcount++;
   if(indexcount >= OscSettings[1]){
    memcpy(ADCdataB,ADCdataA,OscSettings[1]);
   indexcount = 0;
   Adcready = true;
   }

}

// Timer interrupt service routine. updates channel and enables adc
ISR(TIMER1_COMPA_vect)
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
int comp(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}