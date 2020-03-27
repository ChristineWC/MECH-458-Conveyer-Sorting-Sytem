#include <avr/io.h>
#include <stdlib.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"
#include <inttypes.h>

//here are all the global variables
volatile int current_step = 0;
volatile int step_delay = 13; //global variable for i5s speed .
volatile int current_pos = 0;




void mTimer(int count){
    int i = 0;
    //Set the Waveform Generation mode bit description to clear timer on compare math mode (CTC) only
    TCCR1B |=_BV(WGM12);
    OCR1A = 0x03e8; // sets the output compare register for 1000 cycles = 1ms
    TCNT1 = 0x0000; //sets the initial value of the Timer Counter to 0x00
    TIFR1 |=_BV(OCF1A); // so it checks whether the values of the timer counter 1 and the data of OCR1A are the same
    while(i < count){
        if((TIFR1 & 0x02) == 0x02){
            TIFR1 |=_BV(OCF1A); //clears the interrupt flag by writing one to a bit
            i++;
        }
    }
    return;
}











int main(){


TCCR1B |=_BV(CS10); // we need this in main to use the timer






}
