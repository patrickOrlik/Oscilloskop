#define F_CPU 16000000UL
void uart_init(void);
void putcharUsart(char txmsg);
char getcharuart0();
void putstringuart(char *str);
