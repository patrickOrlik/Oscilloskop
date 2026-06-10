#include <avr/io.h>
#define BAUD 115200
#define MYUBRRF (F_CPU/(8UL * BAUD) -1) //defines calculation method for baud rate
// initializes UART

void uart_init(void) {
    UBRR0H = (unsigned char)(MYUBRRF >> 8); // stores the 4 MS bits in the UBRR0H reg
    UBRR0L = (unsigned char)MYUBRRF; // stores the 8 LS bits in the UBBR0L reg

      UCSR0A = (1 << U2X0);  // enable double speed mode
    // Enable receiver and transmitter
    UCSR0B = (1 << TXEN0)|(1 << RXEN0)|(1<<RXCIE0); 
    
    UCSR0C = (1 << UCSZ00)|(1<<(UCSZ01));
}


void putcharUsart(char txmsg)
{
    while(!(UCSR0A & (1<<UDRE0))); // waits for data register empty flag to go high
    UDR0 = txmsg; // writes to TX buffer(I/o data register)
}

char getcharuart0()
{
    while(!(UCSR0A & (1<<RXC0))); // waits for the receive flag is high(receive buffer full)
    return UDR0; // Returns the data from the buffer.
}
void putstringuart(char *str)
{
    while (*str) 
    {
        putcharUsart(*str);
        str++;
    }
}
