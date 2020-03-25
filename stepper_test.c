//STEPPER MOTOR W/ RAMP FUNCTION TEST

#include <avr/io.h>
#include <stdlib.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"
#include <inttypes.h>
 

volatile int current_step = 0;
volatile int current_pos = 4;
volatile int step_delay = 13; //global variable for i5s speed .
int spin[4] = {0b11000000, 0b00011000, 0b10100000, 0b00010100};



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
 
    DDRC = 0xff; //set port c to output

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
