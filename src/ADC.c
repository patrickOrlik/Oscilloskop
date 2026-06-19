#include <avr/io.h>
#include "UART.h"
#include <stdio.h>
void init_adc()
{
    ADCSRA |= (1 << ADPS2); // intern clock
    ADCSRA |= (1 << ADEN) | (1 << ADIE);  // enable adc and interrupt complete
    ADMUX |= (1<<ADLAR); // sets admux register to voltage reference selection and chooses admux= 0 which picks channel ADC0
   
}

void select_channel(char channel)
{
    ADMUX |= (1<<ADLAR); // Clears the register but keeps the mode selection part intact.
    ADMUX |= channel;
}


void CTC_init (void)
{
    TCCR0B |= (1<<CS01);// sets prescaler to 8
    TCCR0B |= (1<<WGM01); // enables CTC mode 
    OCR0A = 9; // sets output compare match amount to produce 10kHz frequency
    TIMSK0 |= (1<<OCIE1A); // enable interrupt
}

void timer1_SetFreq(uint16_t freq)
{
    if (freq == 0) {               
        TIMSK1 &= ~(1<<OCIE1A);
        return;
    }
    TCCR1B = (1<<WGM12) | (1<<CS11) | (1 << CS10);         // CTC, prescaler 64
    OCR1A  = (uint16_t)((uint32_t)(250000) / freq) - 1;
    TIMSK1 |= (1<<OCIE1A);// enables interrupt
}

void external_int()
{
    DDRE &=~(1<<5);
    PORTE |=(1<<5); //internal pullup on digital pin 18

    EICRB = 0x02; //falling edge
    EIMSK |= (1<<INT5);
}