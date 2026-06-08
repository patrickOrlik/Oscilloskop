
#include <avr/io.h>
#include "SPI.h"

void SPI_MasterInit(void)
{
    DDRB |= (1<<DDB2) | (1<<DDB1) | (1<<DDB0);  // MOSI, SCK, SS as output
    DDRB &=~ (1<<DDB3);                         // MISO as input
    SPCR = 0;                                   // Clear register first
    SPCR |= (1<<SPE) | (1<<MSTR);               // Enable SPI, Master
    SPCR |= (1<<CPOL);                         // CPOL = 1
    SPCR |= (1<<CPHA);                         // CPHA = 1
    SPCR |= (1<<SPR1);                          // fclk/64 (250 kHz if 16MHz)
}

unsigned char SPI_MasterTransmit(unsigned char cData)
{
PORTB &=~ (1<< PB0); // set SS low
SPCR &= ~(1 << CPOL);   // CPOL = 0
SPCR &= ~(1 << CPHA);   // CPHA = 0 

SPDR = cData;// transmission start
while(!(SPSR & (1<<SPIF)));//wait for complete transmit
PORTB |=(1<<PB0); //set SS high
return SPDR;

;
}


void SPI_SlaveInit(void)
{
DDRB |=(1<<PB3); //direction for MISO (OUTPUT) must be set first!
SPCR |=(1<<SPE)|(1<<CPOL);
PORTB|=(1<<PB0) ;// a pull up on PB0 FOR slave select (SS)
}

char SPI_SlaveReceive(char data)
{
    SPDR = data;
/* Wait for reception complete */
while(!(SPSR & (1<<SPIF)))
;
/* Return Data Register */
return SPDR;
}



int ReadTemperature(void)
{
    int FirstByte, SecondByte, result;
    PORTB &= ~(1 << PB0);   //sætter SS lav

    SPDR = 0x00;            // Dummy byte
    while (!(SPSR & (1 << SPIF)));
    FirstByte = SPDR;

    SPDR = 0x00;            // Dummy byte
    while (!(SPSR & (1 << SPIF)));
    SecondByte = SPDR;

    PORTB |= (1 << PB0);    // sætter SS Høj

    result = (FirstByte << 8) | SecondByte; // kombinere de to bytes ved at bitshifte den første byte 8 bit til venstre også OR med den mindre byte

    return result;
}