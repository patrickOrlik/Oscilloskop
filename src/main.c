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
volatile unsigned char Adcvalue;    // Used for storing ADCvalues
volatile bool Receiveflag = false;
bool bode = false;
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
unsigned char Settings[4] = {0,3,0x7f,0x05};
uint16_t OscSettings[2] = {10000,50}; //stores Osciloscope settings [0]= Sample rate [1]= Packet length
unsigned char SortingArrayInput[100];
unsigned char SortingArrayOutput[100];
unsigned char BodeArray[256];
unsigned char bodeCh0[100];
unsigned char bodeCh1[100];



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
        bode = true;
        OscSettings[Rlen] = 100;
        memset(ADCdataA,0,100);
        memset(ADCdataB,0,100);
        SPI_FpgaTransmit(shape,0x03);
        SPI_FpgaTransmit(ampl,0xFF);
        SPI_FpgaTransmit(run,0x00);

        for(int i = 1; i <= 255; i++){
            dataAcnt = 0;
           dataBcnt = 0;
            channel = 0;
            lastChannel = 0;
            bodeCollect = false;
           uint32_t fpga_freq = 24UL + (uint32_t)(i - 1) * 23676UL / 254UL;
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
            if(i == 1){
                 qsort(SortingArrayInput,100,sizeof(unsigned char),comp);
                 amplitudeRef = (SortingArrayInput[99]-SortingArrayInput[0]);
                 BodeArray[i] = amplitudeRef;
                 char buff[50];
            sprintf(buff,"ref amplitude is %d \n",amplitudeRef);
            putstringuart0(buff);
            }
            qsort (SortingArrayOutput,100,sizeof(unsigned char),comp);
             amplitude = (SortingArrayOutput[99]-SortingArrayOutput[0]);
            
             BodeArray[i] = (unsigned char)((255UL * amplitude) / amplitudeRef);
             char buff[50];
             sprintf(buff,"dampening is %d %d",amplitude,BodeArray[i]);
            putstringuart0(buff);
        
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
        
        if (lastChannel == 0 && dataAcnt < 100) {
            ADCdataA[dataAcnt++] = ADCH;
        } else if (lastChannel == 1 && dataBcnt < 100) {
            ADCdataB[dataBcnt++] = ADCH;
        }
        if (dataBcnt >= 100 && dataAcnt >= 100) {
            dataAcnt = 0;
            dataBcnt = 0;
            bodeCollect = true;
            
        }
    }
    else{
    writeBuffer[indexcount] = ADCH;
    indexcount++;
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
    if (bode) {
        lastChannel = channel;
        select_channel(channel);
        channel++;
        if (channel == 2) { channel = 0; }
    } else {
        select_channel(0);
    }
    ADCSRA |= (1 << ADSC);
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
