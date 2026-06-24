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
#include "math.h"
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
volatile bool Receiveflag = false;
bool bode = false;
bool FirstRun = true;
volatile unsigned char lastChannel = 0;
volatile bool bodeCollect = false;

unsigned char Protocol = 0;// change this to change UART protocol 
unsigned char amplitude;
unsigned char amplitudeRef;
unsigned char Chksum;
volatile int channel = 0;
int dataAcnt = 0;
int dataBcnt = 0;


// declaration of all data arrays 
unsigned char ADCdataA[1000];
unsigned char ADCdataB[1000];
volatile unsigned char *writeBuffer = ADCdataA; 
volatile unsigned char *readBuffer  = ADCdataB;
unsigned char RXdata[256];
unsigned char Settings[4] = {0,0,0,0};
uint16_t OscSettings[2] = {10000,50}; //stores Osciloscope settings [0]= Sample rate [1]= Packet length
unsigned char SortingArrayInput[100];
unsigned char SortingArrayOutput[100];
unsigned char BodeArray[256];




int main()
{

    DDRB |= (1 << PB7);
    DDRB &= ~(1 << PA0); // sets ADC0 as input (Pin(22))
    DDRH |= (1 << PH4);  // sets pin 7 as output.
    DDRB |= (1 << PB0);
    

    sei();
    SPI_MasterInit();
    uart_init1();
    uart_init0();
    init_adc();
    timer1_SetFreq(OscSettings[0]);
    Uart1Transmit(12,1,Settings);

    
    while (1)
    {
        // transmits osciloscope data to labview
        if (Adcready){
       Uart1Transmit(OscSettings[1]+7,0x02,(unsigned char *)readBuffer);
        Adcready = false;
        }

    if(Receiveflag){   
        // checks what type of data has been received
        switch(RXdata[4]){
        case BTN:
             switch(RXdata[5]){
                case BTN0: // enter button
                Settings[SelectCounter+1]= RXdata[6];
                Uart1Transmit(12,1,Settings);
                SPI_FpgaTransmit((SelectCounter),RXdata[6]);
               Receiveflag = false;

                break;
                
                case BTN1: // select button
                SelectCounter++;
                Settings[0]=SelectCounter;
                Uart1Transmit(12,1,Settings);
                if (SelectCounter == 3){
                    SelectCounter = 0;
                }
                Receiveflag = false;

                break;
                case BTN2: // run/stop button
                SPI_FpgaTransmit(0x03,0x00);
                Receiveflag = false;

                break;

                case BTN3: // reset
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
        if (OscSettings[Rlen] < MinRlen){  //Checks Rlen below min
            OscSettings[Rlen] = (uint16_t)MinRlen;
            char buff[50];
            sprintf(buff,"Too low Rlen set. Rlen set to: %d",OscSettings[Rlen]);
            putstringuart0(buff);
        }
        timer1_SetFreq(OscSettings[Srate]);
        Receiveflag = false;

        break;
        case START:
        bode = true;
        OscSettings[Rlen] = 100;
        memset(ADCdataA,0,100);
        memset(ADCdataB,0,100);
        SPI_FpgaTransmit(shape,0x03);
        SPI_FpgaTransmit(ampl,0xFF);
        SPI_FpgaTransmit(run,0x00);

        for(int i = 1; i <= 255; i++){
            bodeCollect = false;
            //calculates appropiate sample rate
           uint32_t fpga_freq  = 24UL + (uint32_t)(i - 1) * 93UL;
            uint32_t sampleRate = fpga_freq * 20;
            if (sampleRate > 40000) sampleRate = 40000;
            if (sampleRate < 2000)  sampleRate = 2000;
              timer1_SetFreq((uint16_t)sampleRate);
            SPI_FpgaTransmit(freq,i);
            bodeCollect = false;
           while(!bodeCollect);
            cli();
            memcpy(SortingArrayInput,ADCdataA,100);
            memcpy(SortingArrayOutput,ADCdataB,100);
            // sets the reference amplitude in the first run
            if(i == 1){
                FirstRun = true;
                 qsort(SortingArrayInput,100,sizeof(unsigned char),comp);
                 amplitudeRef = (SortingArrayInput[99]-SortingArrayInput[0]);
                 BodeArray[i] = amplitudeRef;
          
            }
            qsort (SortingArrayOutput,100,sizeof(unsigned char),comp);
             amplitude = (SortingArrayOutput[99]-SortingArrayOutput[0]);
            
             BodeArray[i] = (unsigned char)((255UL * amplitude) / amplitudeRef);
          
        
             sei();
        }
        Uart1Transmit(263,0x03,BodeArray);
        SPI_FpgaTransmit(run,0x00);
        bode = false;
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
    if (bode) {
        //checks if this is the first time bode is running
        if (FirstRun){
            ADCdataA[indexcount++]=ADCH;}
            else {
            ADCdataB[indexcount++]=ADCH;
            }
              if(indexcount >= 100){
                FirstRun = false;
                indexcount =0;
                bodeCollect = true;
               
    }

    } else {
        writeBuffer[indexcount++] = ADCH;
        if(indexcount >= OscSettings[Rlen]){
            volatile unsigned char *tmp = readBuffer;
            readBuffer = writeBuffer;
            writeBuffer = tmp;
            indexcount = 0;
            Adcready = true;
        }
    }
}

ISR(TIMER1_COMPA_vect)
{
    //checks if bodeplot function is active
    if (bode) {
        if(FirstRun){
            select_channel(0); 
        }
        else {
            select_channel(1);
        }
    } else {
        select_channel(0);
    }
    ADCSRA |= (1 << ADSC);
}

ISR(USART1_RX_vect)
{
    Data = UDR1;
    
    PORTB ^= (1<< PB7);
           if (Receiveflag) return;//secures that we havde handled previous received packet before handling a new one
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
                if (Protocol){//checks which protocol is selected
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
//comp function used to sort in the bodeplot
int comp(const void *a, const void *b) {
    return (*(const unsigned char *)a - *(const unsigned char *)b);
}
