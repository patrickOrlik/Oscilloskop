#include <avr/io.h>
#include "UART.h"
#define BAUD 115200
#define MYUBRRF (F_CPU/(8UL * BAUD) -1) //defines calculation method for baud rate

// initializes UART

void uart_init0(void) {
    UBRR0H = (unsigned char)(MYUBRRF >> 8); // stores the 4 MS bits in the UBRR0H reg
    UBRR0L = (unsigned char)MYUBRRF; // stores the 8 LS bits in the UBBR0L reg

      UCSR0A = (1 << U2X0);  // enable double speed mode
    // Enable receiver and transmitter
    UCSR0B = (1 << TXEN0)|(1 << RXEN0)|(1<<RXCIE0); 
    
    UCSR0C = (1 << UCSZ00)|(1<<(UCSZ01));
}

void uart_init1(void) {
    UBRR1H = (unsigned char)(MYUBRRF >> 8); // stores the 4 MS bits in the UBRR0H reg
    UBRR1L = (unsigned char)MYUBRRF; // stores the 8 LS bits in the UBBR0L reg

      UCSR1A = (1 << U2X1);  // enable double speed mode
    // Enable receiver and transmitter
    UCSR1B = (1 << TXEN1)|(1 << RXEN1)|(1<<RXCIE1); 
    
    UCSR1C = (1 << UCSZ10)|(1<<(UCSZ11));
}


void putcharuart0(char txmsg)
{
    while(!(UCSR0A & (1<<UDRE0))); // waits for data register empty flag to go high
    UDR0 = txmsg; // writes to TX buffer(I/o data register)
}

char getcharuart0()
{
    while(!(UCSR0A & (1<<RXC0))); // waits for the receive flag is high(receive buffer full)
    return UDR0; // Returns the data from the buffer.
}
void putstringuart1(char *str)
{
    while (*str) 
    {
        putcharUsart(*str);
        str++;
    }
}

void putstringuart0(char *str)
{
    while (*str) 
    {
        putcharuart0(*str);
        str++;
    }
}
char getcharuart1()
{
    while(!(UCSR1A & (1<<RXC1))); // waits for the receive flag is high(receive buffer full)
    return UDR1; // Returns the data from the buffer.
}
void putcharuart1(unsigned char txmsg)
{
    while(!(UCSR1A & (1<<UDRE1))); // waits for data register empty flag to go high
    UDR1 = txmsg; // writes to TX buffer(I/o data register)
}

void Uart1Transmit(uint16_t Dlength, char type, unsigned char data[]) {
    if (Protocol == 1)
        Uart1TransmitLRC8(Dlength, type, data);
    else
        Uart1TransmitZ16(Dlength, type, data);
}

void Uart1TransmitZ16(uint16_t Dlength, char type, unsigned char data[]) {
    unsigned char lsb = (unsigned)Dlength & 0xff;
    unsigned char msb = (unsigned)Dlength >> 8;
    putcharuart1(0x55);
    putcharuart1(0xAA);
    putcharuart1(msb);
    putcharuart1(lsb);
    putcharuart1(type);
    for (int i = 0; i < (Dlength - 7); i++)
        putcharuart1(data[i]);
    putcharuart1(0x00);
    putcharuart1(0x00);
}

void Uart1TransmitLRC8(uint16_t Dlength, char type, unsigned char data[]) {
    unsigned char lsb = (unsigned)Dlength & 0xff;
    unsigned char msb = (unsigned)Dlength >> 8;
    unsigned char chksum = 0x55 ^ 0xAA ^ msb ^ lsb ^ (unsigned char)type;
    for (int i = 0; i < (Dlength - 7); i++)
        chksum ^= data[i];
    putcharuart1(0x55);
    putcharuart1(0xAA);
    putcharuart1(msb);
    putcharuart1(lsb);
    putcharuart1(type);
    for (int i = 0; i < (Dlength - 7); i++)
        putcharuart1(data[i]);
    putcharuart1(0x00);
    putcharuart1(chksum);
}