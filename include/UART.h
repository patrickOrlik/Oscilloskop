#define F_CPU 16000000UL
void uart_init0(void);
void uart_init1(void);
void putcharuart0(char txmsg);
void putcharuart1(char txmsg);
char getcharuart0();
char getcharuart1();
void putstringuart0(char *str);
void putstringuart1(char *str);
