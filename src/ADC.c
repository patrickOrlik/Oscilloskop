#include <avr/io.h>
void init_adc()
{
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // intern clock 125khz
    ADMUX = (1 << REFS0);                                 // Voltage reference selection
    ADCSRA |= (1 << ADEN) | (1 << ADIE);                  // enable adc and interrupt complete
}

void select_channel(char channel)
{
    ADMUX = 0x40; // Clears the register but keeps the mode selection part intact.
    ADMUX |= channel;
}