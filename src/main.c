/*
  Author: Ramon Arents
  Version number: 0.1
  Date generated: 26-3-2019
  Last changed: 5-4-2019
**/
// define CPU
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//required libraries
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// header files
#include "USART.h"
#include "spi.h"
#include "lcd-routines.h"
#include "utils.h"
#include "bmp085.h"
#include "mfrc522.h"


//constant for servo motor. This is used to get the right turning angle
#define SERVO_OFFSET 800

//variables for temperature and for printing that temperature on the LCD. Volatile because we use them only in the ISR
volatile double temp;
volatile char printBuff[10];
volatile char lcdBuffer[16];
volatile char celsiusSign = (char) 223;

// variables to recognize the rfid. Volatile because we use them only in the ISR
volatile uint8_t rfidByte;
volatile uint8_t str[MAX_LEN];

char* deblank(char* input){
     /**
     * This function removes blanks from a char. This is used to save space on the LCD display when displaying the temperature
     * 
     * @param  char input
     * @return char
    */
    // increment/decrement variables
    int i,j;
    // make the output the input
    char *output=input;
    // loop trough the char
    for (i = 0, j = 0; i<strlen(input); i++,j++)          
    {
        // check if there is no space
        if (input[i]!=' '){
            // save the characters that are no spaces in the output                         
            output[j]=input[i];                     
        }else{
            // decrement j to remove the spaces
            j--;
        }                                     
    }
    // reset the output
    output[j]=0;
    // return the output
    return output;
}


void initTimer1Servo(void){
    /**
     * This function sets up the timer for the servo motor
     * @return void
    */
	// set PB1 for output
  DDRB |= 1 << PB1; 

  TCCR1A |= (1 << WGM11) | (1 << COM1A1); 	// use fast pwm mode, COM1A1 is uesd to set PB1 / OC1A
	TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11); // 8 precaling -- microsecond steps
  ICR1 = 39999; // top value 39999
  
}

// set change intterupt on SS pin (PB2)
void initPinChangeInterrupt(void){
    /**
     * This function sets up the intterupt for PCINT0 (B pins), using PCINT2 as mask (the slave select pin)
     * @return void
    */
  PCICR |= (1 << PCIE0);  // set interrupts for B pins
  PCMSK0 |= (1 << PCINT2); // set PB2 (PCINT2) as mask
  sei(); // set enable intterupt
}

int main(void){
  //setup
  // init USART
  initUSART();
  // init LCD	
	lcd_init();
	// wait for card
	lcd_string("Waiting...");
  // init interrupts
  initPinChangeInterrupt();
  // SPI init
  spi_init();
  // init RFID
  mfrc522_init();
  //loop
  while(1){
    
  }
  return (0); // this line is never reached
}

// interrupt for PCINT0_vect to wait for input from the user on the RFID
ISR(PCINT0_vect){
    // read the card
    rfidByte = mfrc522_request(PICC_REQALL,str);
    // check if the card is found
		if(rfidByte == CARD_FOUND){
        // get the serial code from the card
        rfidByte = mfrc522_get_card_serial(str);
        // check again if the card is found (we do this to check if it is the right card)
        if(rfidByte == CARD_FOUND){
          // clear LCD to display welcome text
          lcd_clear();
          lcd_string("Welcome");
          // init preasure sensor
          bmp085_init();
          // read temperature from sensor
          temp = bmp085_gettemperature();
          // temperature is in double, this funciont makes it a char to display it on the LCD
          dtostrf(temp, 10, 1, printBuff);
          deblank(printBuff);
          // make the string for the LCD. sprinft is uesed to use a variable (from the temperature) in the string
          sprintf(lcdBuffer, "Temp:%s", printBuff);
          // set the cursor of the LCD on the next line
          lcd_setcursor(0, 2);
          // put the string on the LCD
          lcd_string(lcdBuffer);
          // write a nice celsius sign after the temperature
          lcd_writeChar(celsiusSign);
          lcd_writeChar('C');    
          // init timer for servo
          initTimer1Servo();
          // start servo motor on ORC1A (ORC1A is the timer value for PB1)
          OCR1A = 1999 + SERVO_OFFSET;
          // wait 5 seconds
          _delay_ms(5000);
          // put servo back in original position
          OCR1A = 999 - SERVO_OFFSET;
          // clear LCD and display waiting text again
          lcd_clear();
          lcd_string("Waiting ...");  
         // if it is not the right card display that the wrong card is used
			  }else if(rfidByte == CARD_NOT_FOUND){
          // display the that the  wrong card is used and display this on the lcd. Reset after two seconds
          lcd_clear();
          lcd_string("Wrong card");
          _delay_ms(2000);
          lcd_clear();
          lcd_string("Waiting...");
        // otherwise there is some kind of error
        }else{
          // display error on the lcd and reset after two seconds
          lcd_clear();
          lcd_string("Error");
          _delay_ms(2000);
          lcd_clear();
          lcd_string("Waiting...");
        }
    }
}