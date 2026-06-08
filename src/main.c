#define F_CPU 16000000UL  // your clock speed
#include <util/delay.h>
#include <avr/interrupt.h>
#include "UART.h"
#include "ADC.h"
#include <avr/io.h>
#include "SPI.h"
#include <stdio.h>
#include <stdlib.h>
char buffer[32];


char Datatest;
int main(){
SPI_MasterInit();
uart_init();

Datatest = 0x01;
for (int i = 0; i < 16; i++){
SPI_MasterTransmit(Datatest);
Datatest = Datatest + 1;
sprintf(buffer, "number is %d \n", );
putstringuart(buffer);
_delay_ms(1000);
}



}