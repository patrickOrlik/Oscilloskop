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


char buffer[32];
char Data;
int count = 0;
int Datalength;
char RXdata[256];
bool Receiveflag = false;

volatile bool Adcready = false; // Used for checking if Adc is ready.
volatile bool RX1_COMPLETE_FLAG = false;

volatile unsigned int channel = 1; // channel selected
volatile unsigned int Adcvalue;    // Used for storing ADCvalues

int main()
{

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

        if (RX1_COMPLETE_FLAG)
        {
            RXdata[count] = Data;
            count++;
            if (count == 2)
            {
                if (RXdata[0] != 0x55 || RXdata[1] != 0xAA)
                {
                    count = 0;
                }
            }
            else if (count == 4)
            {
                Datalength = (int)((RXdata[3] << 8) | RXdata[2]);
            }
            else if (count == Datalength +7)
            {
                if(RXdata[Datalength+6] != 0x00 || RXdata[Datalength+5] != 0x00){
                    Receiveflag = true;
                    count = 0;
                }
                else {
                Receiveflag = false;
                count = 0;
            }
        }
        RX1_COMPLETE_FLAG = false;
    }
    if(Receiveflag){
        clear_display();
        sendCharXY(RXdata[0],1,1);
        sendCharXY(RXdata[1],2,2);
        _delay_ms(3000);
        clear_display();
        Receiveflag = false;
    }
}
}

ISR(ADC_vect)
{
    Adcready = true;
    Adcvalue = ADC;
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
    RX1_COMPLETE_FLAG = true;
}

