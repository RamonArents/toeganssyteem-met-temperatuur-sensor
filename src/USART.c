/*
    The code from the book to make USART work
/**/

#include <avr/io.h>
#include "USART.h"

#define BAUD 9600
#include <util/setbaud.h>

void initUSART(void){
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
#if USE_2X
    UCSR0A |= (1 << U2X0);
#else
    UCSR0A &= ~(1 << U2X0);
#endif

    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void transmiteByte(uint8_t data){
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = data;
}

uint8_t receiveByte(void){
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

void printString(const char myString[]){
    uint8_t i = 0;
    while(myString[i]){
        transmiteByte(myString[i]);
        i++;
    }
}