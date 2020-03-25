//STEPPER MOTOR W/ RAMP FUNCTION TEST

/*
WIRING SETUP:
PORTC is hooked up to LEDs as usual in all labs to this point
PORTA drives stepper motor with:
	A0 = I4
	A1 = I3
	A2 = E2
	A3 = I2
	A4 = I1
	A5 = E1
PORTD only requires HE sensor pin attached to pin D0
*/


#include <avr/io.h>
#include <stdlib.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>
//#include "lcd.h"
#include <inttypes.h>
 

volatile int current_step = 0;
volatile int current_pos = 4;
volatile int step_delay = 13; //global variable for i5s speed .
int spin[4] = {0b11000000, 0b00011000, 0b10100000, 0b00010100};

void mTimer(int count){
	int i = 0;
	//Set the Waveform Generation mode bit description to clear timer on compare math mode (CTC) only
	TCCR1B |=_BV(WGM12);
	OCR1A = 0x03e8; // sets the output compare register for 1000 cycles = 1ms
	TCNT1 = 0x0000; //sets the initial value of the Timer Counter to 0x0000
	TIMSK1 = TIMSK1 |0b00000010; // enables the output compare interrupt enable
	TIFR1 |=_BV(OCF1A); // so it checks whether the values of the timer counter 1 and the data of OCR1A are the same
	while(i < count){
		if((TIFR1 & 0x02) == 0x02){
			TIFR1 |=_BV(OCF1A); //clears the interrupt flag by writing one to a bit
			i++;
		}
	}
	return;
}

void StepperMotorCW (int steps){

  for(int i = 0; i < steps; i++){
      if(current_step == 3)
      current_step = 0;
      else
      current_step++;
      PORTA = spin[current_step];
      mTimer(step_delay);

    if ((i%2 == 0) && (i <= (steps/5)))
      step_delay--;
    if ((i%2 == 0) && (i >= (steps/5)))
      step_delay++;
  }
  step_delay = 18;
}
 
void StepperMotorCCW (int steps){
    
    for(int i = 0; i < steps; i++){
        if(current_step == 0)
        current_step = 3;
        else
        current_step--;
        PORTA = spin[current_step];
        mTimer(step_delay);  
 
    if ((i%2 == 0) && (i <= (steps/5)))
      step_delay--;
    if ((i%2 == 0) && (i >= (steps/5)))
      step_delay++;
    }
   step_delay = 18;
 }
 
 
 void Stepper(int to_be_sorted_to){ // CALL THIS IN MAIN LOOP IN SORTING STATE
	unsigned int dist = (to_be_sorted_to - current_pos)*50;
	if (dist == 150)
		StepperMotorCCW(50);
	if (dist == -150)
		StepperMotorCW (50);
	else if (dist < 0)
		StepperMotorCCW (-dist);
	else if (dist > 0)
		StepperMotorCW (dist);
}

ISR(INT0_vect) { // HE sensor is hooked up to PORTD0, will set current position to 0 every time HE sensor is triggered
  current_pos = 0;
}
 
 int main()
 {
 
    DDRC = 0xff; //set port c to output
	DDRD = 0x00; //set port d to input
	DDRA = 0xff; //set port a to ouput

	TCCR1B |=_BV(CS10);
	
	sei(); // sets the Global Enable for all interrupts

   //initialize the stepper to get it to the starting position
   while (current_pos != 0){
    StepperMotorCW(1);
   }
   
   //now we test its ability to change positions with the ramp up and down function
   Stepper(2); //go to bin 2 (180 degrees CW)
   PORTC = 0b00000011; // turns on 2 LED to signal pos2
   mTimer(1000);
   
   Stepper(3); //go to bin 3 (90 degrees CW)
   PORTC = 0b00000111; // turns on 3 LED to signal pos3
   mTimer(1000);
   
   Stepper(1); //go to bin 1 (180 degrees CCW)
   PORTC = 0b00000011; // turns on 2 LED to signal pos2
   mTimer(1000);
   
   Stepper(0); //go to bin 0 (90 degrees CCW)
   PORTC = 0b00111111; // turns on 6 LED to signal pos0
   mTimer(1000);
   
   Stepper(3); //go to bin 3 (90 degrees CCW)
   PORTC = 0b00000111; // turns on 3 LED to signal pos3
   mTimer(1000);
   
   Stepper(0); //go to bin 0 (90 degrees CW)
   PORTC = 0b00111111; // turns on 6 LED to signal pos0
   mTimer(1000);

   PORTC = 0b11111111; // turns on all LEDs to signal completion

 
 }
