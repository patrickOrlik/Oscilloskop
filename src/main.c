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

char buffer[32];
char Datatest;
volatile bool Adcready = false; // Used for checking if Adc is ready.
volatile unsigned int channel = 1; // channel selected
volatile unsigned int Adcvalue; // Used for storing ADCvalues

int main(){

DDRB &= ~(1<<PA0);// sets ADC0 as input (Pin(22))
DDRH |= (1<<PH4); // sets pin 7 as output.
DDRB |= (PB0); 

sei();
SPI_MasterInit();
uart_init();
init_adc();
CTC_init();
Datatest = 0x01;



while(1){

_delay_ms(1000);
for (int i = 0; i < 16; i++){
SPI_MasterTransmit(Datatest);
sprintf(buffer, "number is %d \n",Datatest);
putstringuart(buffer);

Datatest++;
_delay_ms(1000);

}
Datatest = 0;
}

}



ISR(ADC_vect)
{
    Adcready = true;
    Adcvalue = ADC;
}

// Timer interrupt service routine. updates channel and enables adc 
ISR(TIMER0_COMPA_vect){
    // Start ADC conversion
    ADCSRA |= (1 << ADSC); // enables adc
  
}