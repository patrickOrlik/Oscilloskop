#define F_CPU 16000000UL
#include <stdint.h>
void uart_init0(void);
void uart_init1(void);
void putcharuart0(char txmsg);
void putcharuart1(unsigned char txmsg);
char getcharuart0();
char getcharuart1();
void putstringuart0(char *str);
void putstringuart1(char *str);

extern unsigned char Protocol;
void Uart1Transmit(uint16_t Dlength, char type, unsigned char data[]);
void Uart1TransmitZ16(uint16_t Dlength, char type, unsigned char data[]);
void Uart1TransmitLRC8(uint16_t Dlength, char type, unsigned char data[]);
