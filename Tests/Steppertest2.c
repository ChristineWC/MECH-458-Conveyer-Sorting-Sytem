#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>
//#include <util/delay.h>
#include "list.h"
#include "lcd.h"

//Global variables
volatile int current_state = 0;//state (running or paused)
volatile int current_step = 0;//for the stepper motor, (0-3)
volatile int current_pos = 200;//current position of the bucket (0-199)
volatile int step_delay = 18; //global variable for i5s speed .
volatile int dir = 1; //this should be either +1 or -1, set in step_what() to give direction for bucket
volatile int dist = 0; //holds absolute value of # of steps to get where needs to go

int steel_count = 0;
int aluminum_count = 0;
int black_count = 0;
int white_count = 0;

int pending_steel = 0;
int pending_aluminum = 0;
int pending_black = 0;
int pending_white = 0;

int spin[4] = {0b00110000, 0b00000110, 0b00101000, 0b00000101};

unsigned int lowest;
int ADC_resultflag = 0;




//Functions
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


void StepperGo(){
	current_step += dir;
	current_pos += dir;
	
	if (current_pos < 0)
	current_pos = 199;
	
	if(current_step == 4)
	current_step = 0;
	if(current_step == -1)
	current_step = 3;
	
	PORTA = spin[current_step];
	mTimer(step_delay);
}

void step_what(){ //sets the distance and speed
	dist = (list->head->material - current_pos);
	if (dist >= 100)
	dist = dist - 200;
	if (dist < -100)
	dist = dist + 200;
	dir = (dist)/ abs(dist);

	//how far to go? set "acc_or_dec" based on current distance and step_delay
	if (abs(dist) > 20 && (step_delay > 9) && (dist%3 == 0))
	step_delay--;
	if (abs(dist) < 16 && (step_delay < 18) && (dist%3 == 0))
	step_delay++;
	
}

void init_int() {//enables all necessary interrupts
	// config the external interrupt ======================================
	EIMSK |= (_BV(INT2)); // enable INT2 for OR sensor
	EICRA |= (_BV(ISC21) | _BV(ISC20)); // rising edge interrupt for OR sensor
	ADCSRA |= _BV(ADEN); // enable ADC
	ADCSRA |= _BV(ADIE); // enable interrupt of ADC
	ADMUX |= _BV(MUX0) | _BV(REFS0); //ADC Multiplexer selection register bits 5 and 6 set to 1, ADLAR = ADC left adjust result;
	//REFS0 set to 1 which selects voltage reference selection to core voltage (3.3v)
	EICRA |= _BV(ISC01);   // Falling Edge on INT0 for hall sensor
	EIMSK |= _BV(INT0);    // Enable INT0 for hall sensor
	EIMSK |= _BV(INT3); //Enable INT3 for EOT sensor
	EICRA |= _BV(ISC31); // falling edge interrupt for EOT sensor
	EIMSK |= _BV(INT1); // Enable INT1 for the pause button
	EICRA |= (_BV(ISC11) | _BV(ISC10)); // rising edge interrupt for pause button
}



ISR(INT0_vect) { // HE sensor is hooked up to PORTD0, will set current position to 0 every time HE sensor is triggered
	current_pos = 0;
}



int main(){
	//initialization
	DDRD = 0x00; // for the interrupts for the sensors
	DDRB = 0xff; // for the dc motor
	DDRC = 0xff; // output for the LCD
	DDRA = 0xff; //output for stepper
	DDRF = 0x00; //input for RL sensor @ F1
	List* list = new_list();
	
	InitLCD(LS_BLINK|LS_ULINE); //initialize LCD subsystem
	TCCR1B |=_BV(CS10); // we need this in main to use the timer
	init_int(); //initializes all interrupts
	sei(); // sets the Global Enable for all interrupts

	
	//initialize the stepper to get it to the starting position
	LCDWriteStringXY(0, 0, "Homing Start");
	while (current_pos != 0){
		StepperGo();
	}
	LCDWriteStringXY(0, 0, "Homing Complete");
	LCDWriteStringXY(0, 1, "Start Steptest");
	mTimer(1500); //notifies that initialization is complete and ready to begin
	
	//run a test, having stepper motor just go to a couple different locations
	
	int togoto = 50;
	
	while (togoto != current_pos){
		step_what();
		StepperGo();
	}
	step_delay = 18;
	LCDWriteStringXY(0, 0, "Items sorted: 1" );
	LCDWriteStringXY(0, 1, "Moving on" );
	mTimer(500);
	
	togoto = 150;
	
	while (togoto != current_pos){
		step_what();
		StepperGo();
	}
	step_delay = 18;
	LCDWriteStringXY(0, 0, "Items sorted: 2" );
	LCDWriteStringXY(0, 1, "Moving on" );
	mTimer(500);
	
	togoto = 100;
	
	while (togoto != current_pos){
		step_what();
		StepperGo();
	}
	step_delay = 18;
	LCDWriteStringXY(0, 0, "Items sorted: 3" );
	LCDWriteStringXY(0, 1, "Moving on" );
	mTimer(500);
	
	togoto = 0;
	
	while (togoto != current_pos){
		step_what();
		StepperGo();
	}
	step_delay = 18;
	LCDWriteStringXY(0, 0, "Items sorted: 4" );
	LCDWriteStringXY(0, 1, "Moving on" );
	mTimer(500);
	
	togoto = 150;
	
	while (togoto != current_pos){
		step_what();
		StepperGo();
	}
	step_delay = 18;
	LCDWriteStringXY(0, 0, "Items sorted: 5" );
	LCDWriteStringXY(0, 1, "Done" );
	mTimer(1500);
	
	
	
}
