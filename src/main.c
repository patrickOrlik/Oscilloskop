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
#define Srate 0
#define Rlen 1
#define BTN 1 
#define SEND 2
#define START 3
enum buttons { BTN0,BTN1,BTN2,BTN3};
enum SPI_Types{shape,ampl,freq,run,reset};
int comp(const void *a, const void *b);
// Declarations of all variables and flags
volatile unsigned char Data;
volatile int count = 0;
unsigned char SelectCounter = 0;
volatile int Datalength = 0;
volatile int indexcount = 0;
volatile bool Adcready = false; // Used for checking if Adc is ready.
volatile unsigned char Adcvalue;    // Used for storing ADCvalues
volatile bool Receiveflag = false;
unsigned char Protocol = 0;// change this to change UART protocol 
unsigned char amplitude;
unsigned char Chksum;

// declaration of all data arrays 
unsigned char ADCdataA[1000];
unsigned char ADCdataB[1000];
volatile unsigned char *writeBuffer = ADCdataA;
volatile unsigned char *readBuffer  = ADCdataB;
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
    uart_init0();
    init_adc();
    timer1_SetFreq(OscSettings[0]);
    Uart1Transmit(12,1,Settings);

    
    while (1)
    {
        if (Adcready){
       Uart1Transmit(OscSettings[1]+7,0x02,(unsigned char *)readBuffer);
        Adcready = false;
        }
    if(Receiveflag){   
        switch(RXdata[4]){
        case BTN:
             switch(RXdata[5]){
                case BTN0:
                Settings[SelectCounter+1]= RXdata[6];
                Uart1Transmit(12,1,Settings);
                SPI_FpgaTransmit((SelectCounter),RXdata[6]);
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
        OscSettings[Srate] = (RXdata[5]<< 8) | RXdata[6];
        OscSettings[Rlen] =(RXdata[7]<< 8) | RXdata[8];
        long MinRlen = (7L * OscSettings[Srate]) / (11520L - OscSettings[Srate]);
        if (OscSettings[Rlen] < MinRlen){
            OscSettings[Rlen] = (uint16_t)MinRlen;
            char buff[50];
            sprintf(buff,"Too low Rlen set. Rlen set to: %d",OscSettings[Rlen]);
            putstringuart0(buff);
        }
        timer1_SetFreq(OscSettings[Srate]);
        Receiveflag = false;

        break;
        case START:
        OscSettings[1]= 100;
        SPI_FpgaTransmit(shape,0x03);
        SPI_FpgaTransmit(ampl,0xFF);
        SPI_FpgaTransmit(run,0x00);

        for(int i = 1; i <= 255; i++){
            uint32_t fpga_freq = (i == 0) ? 0 : (24UL + (uint32_t)(i - 1) * 23676UL / 254UL);
            timer1_SetFreq((uint16_t)(fpga_freq * 2));
            SPI_FpgaTransmit(freq,i);
            Adcready = false; while(!Adcready);   // venter et  par ADC interrupt før vi måler
            Adcready = false; while(!Adcready);
            cli(); 
            memcpy(SortingArray,ADCdataB,100);
            sei();
            qsort(SortingArray,100,sizeof(unsigned char),comp);
            unsigned char amplitude = (SortingArray[0]-SortingArray[99]);
            BodeArray[i] = (255-amplitude);
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
   
   writeBuffer[indexcount] = ADCH;
   indexcount++;
   if(indexcount >= OscSettings[Rlen]){
//     memcpy(ADCdataB,ADCdataA,OscSettings[Rlen]);
//    indexcount = 0;
//    Adcready = true;
       volatile unsigned char *tmp = readBuffer;
        readBuffer = writeBuffer;
        writeBuffer = tmp;
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
                if (Protocol){
                        Chksum = 0;
                   for (int i =0; i <(Datalength); i++){
                  Chksum = Chksum ^ RXdata[i];
                   }
                 
                   if (!(Chksum ^ RXdata[Datalength])){
                    count = 0;
                    Receiveflag = true;
                   }
              
                else {
                Receiveflag = false;
                count = 0;
                }
               }
                 else {

            
                     if(RXdata[Datalength] == 0x00 && RXdata[Datalength-1] == 0x00){
                    
                    count = 0;
                    Receiveflag = true;
                }
                else{ count = 0;
                Receiveflag = false;
                }       
               
            }
   
}
}
int comp(const void *a, const void *b) {
    return (*(const unsigned char *)a - *(const unsigned char *)b);
}
