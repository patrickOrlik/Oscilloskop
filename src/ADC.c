#include <avr/io.h>
void init_adc()
{
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // intern clock 125khz
    ADMUX &= ~(1 << REFS0)|(1<<REFS1);      // Voltage reference selection
    ADCSRA |= (1 << ADEN) | (1 << ADIE);                  // enable adc and interrupt complete
    ADMUX = 0x40; // sets admux register to voltage reference selection and chooses admux= 0 which picks channel ADC0
   
}

void select_channel(char channel)
{
    ADMUX = 0x40; // Clears the register but keeps the mode selection part intact.
    ADMUX |= channel;
}


void CTC_init (void)
{
    TCCR0B |= (1<<CS01);// sets prescaler to 8
    TCCR0B |= (1<<WGM01); // enables CTC mode 
    OCR0A = 9; // sets output compare match amount to produce 10kHz frequency
    TIMSK0 |= (1<<OCIE1A); // enable interrupt
}

void external_int()
{
    DDRE &=~(1<<5);
    PORTE |=(1<<5); //internal pullup on digital pin 18

    EICRB = 0x02; //falling edge
    EIMSK |= (1<<INT5);
}