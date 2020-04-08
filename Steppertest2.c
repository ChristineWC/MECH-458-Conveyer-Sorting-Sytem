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

const int Al_Max = 350;
const int Al_Min = 0; 
const int St_Max = 699;
const int St_Min = 351;
const int Bl_Max = 1023;
const int Bl_Min = 941;
const int Wh_Max = 940;
const int Wh_Min = 700;

List*list; 
//Linked list functions

List* new_list(){ //this is to initialize the list

	List* list = (List*)calloc(1, sizeof(List));

	init_list_table(list);
	list->create(list);

	return list;
}

void delete_list(List* list){ //this function is to make sure that the list pointer is null
	list->destroy(list);
	free(list);
}

void init_list_table(List* list){// since this is using function pointers, this lets us use the syntax NameOfList->nameOfFunction(parameters); examples can be seen in main

	list->create  = create;
	list->destroy = destroy;
	list->size    = size;
	list->empty   = empty;

	list->push_back = push_back;
	list->pop_front = pop_front;
}

void create(List* this){ // part of list initialization
	this->head  = NULL;
	this->tail  = NULL;
	this->_size = 0;
}

void destroy(List* this){
	if (this->empty(this)){
		return;
	}

	Item* it;
	while (this->size(this) != 0){
		it = this->pop_front(this);
		free(it);
		it = NULL;
	}
}

unsigned int size(List* this){
	return this->_size;
}

bool empty(List* this){
	return this->_size == 0;
}

void push_back(List* this, Material m){ // use this function to add stuff to the back of the list
	Item* new_item = (Item*)calloc(1, sizeof(Item));
	new_item->material = m;
	if (this->empty(this)){
		this->head     = new_item;
		this->tail     = new_item;
		new_item->next = NULL;
		new_item->prev = NULL;
	}
	else{
		new_item->prev   = this->tail;
		new_item->next   = NULL;
		this->tail->next = new_item;
		this->tail       = new_item;
	}
	this->_size++;
}

Item* pop_front(List* this){ // use this function to delete stuff at the front of the list
	if (this->empty(this)){
		return NULL;
	}

	Item* front;
	if (this->_size == 1){
		front      = this->head;
		this->head = NULL;
		this->tail = NULL;
	}
	else{
		front            = this->head;
		this->head       = this->head->next;
		this->head->prev = NULL;
		front->next      = NULL;
	}

	this->_size--;
	return front;
}


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
	if (abs(dist) > 20 && (step_delay > 9) && (dist%2 == 0))
	step_delay--;
	if (abs(dist) < 16 && (step_delay < 18) && (dist%2 == 0))
	step_delay++;
	
	//does the belt need to slow down?
	if((PIND &= 0x08) == 0x08)// this means that there is something in front of the exit sensor
	OCR0A |= 0b01000000; //sets duty cycle to 1/4 to slow down belt and allow bucket to prep

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
    PWM(); //Though the duty cycle may need to be changed for the DC motor
    sei(); // sets the Global Enable for all interrupts

	
//initialize the stepper to get it to the starting position
	LCDWriteStringXY(0, 0, "Homing Start");
	while (current_pos != 0){
		StepperGo();
	}
	LCDWriteStringXY(0, 0, "Homing Complete");
	LCDWriteStringXY(0, 1, "Starting Sort");
	mTimer(1500); //notifies that initialization is complete and ready to begin
	
//run a test, adding items to the linked list
	
	list->push_back(list, STEEL);
	list->push_back(list, ALUMINUM);
	list->push_back(list, WHITE);
	list->push_back(list, BLACK);

	LCDClear();
	LCDWriteStringXY(0, 0, "list size is: );
	LCDWriteIntXY(14,0,list->size(), 2);


	
	//loop stuff starts  
  goto RUNNING;
  
  RUNNING:
	  //output to lcd that it's running normally
	PORTB = 0b00000010;//turns on DC motor forward (CCW)    
	
	//Here we're gonna do some fucked shit to try to make this thing SMART
		
	if((list->head->material != current_pos) && (list->head != NULL)){ //is the stepper/bucket ready to receive the next item?
		step_what();//sets distance to go, and adjusts the step delay/stepper speed, and slows down belt if necessary
		StepperGo();
		
	} else { // YES, in position
		Item* it = list->head;
		Item* front = list->pop_front(list); // delete that item from the list
		free(front); 
		LCDWriteIntXY(14,0,list->size(list), 2);
		
		step_delay = 18;
		OCR0A |= 0b10000000; //sets duty cycle to 1/2 to speed belt back up after bucket aligned

	}
	  switch(current_state){
	  	case(0):
			goto RUNNING; //basically looping this stuff
			break;

	  } // Changes states, otherwise keeps running belt
			 }

