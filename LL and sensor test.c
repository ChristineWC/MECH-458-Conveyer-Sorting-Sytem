/*
 This is to test the linked list to make sure that its working in the way that it should be along with each of the sensors
 
 Test: (If helpful, the speed of the DC motor can be slowed down in the PWM() function)
 Press reset, put parts on the conveyor belt, parts should run through system and not be sorted. 
 After 4 pieces have gone past the EX sensor, press pause and the conveyor belt should stop.
 If everything has been done correctly, during the test the LCD screen would alternate between 
 "PART PENDING" and "PART SORTED". 
 At the end, the LCD should display the size of the list which should be 0 at that point
 and will display the number of each piece that has gone past the EX sensor. 
 */ 

#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>
#include "list.h"
#include "lcd.h"

int steel_count = 0;
int aluminum_count = 0;
int black_count = 0;
int white_count = 0;

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

List* list;

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

void init_int() {//enables all necessary interrupts
	// config the external interrupt ======================================
	EIMSK |= (_BV(INT2)); // enable INT2 for OR sensor
	EICRA |= (_BV(ISC21) | _BV(ISC20)); // rising edge interrupt for OR sensor
	ADCSRA |= _BV(ADEN); // enable ADC
	ADCSRA |= _BV(ADIE); // enable interrupt of ADC
	ADMUX |= _BV(MUX0) | _BV(REFS0); //ADC Multiplexer selection register bits 5 and 6 set to 1, ADLAR = ADC left adjust result;
	//REFS0 set to 1 which selects voltage reference selection to core voltage (3.3v)
	EIMSK |= _BV(INT3); //Enable INT3 for EOT sensor
	EICRA |= _BV(ISC31); // falling edge interrupt for EOT sensor
	EIMSK |= _BV(INT1); // Enable INT1 for the pause button
	EICRA |= (_BV(ISC11) | _BV(ISC10)); // rising edge interrupt for pause button
}

void PWM (){
	TCCR0A |= 0b10000011; //first bit sets compare match output mode to clear, last two set whole thing to mode 3 (table 13-7)
	TCCR0B |= 0b00000010; //sets clock prescale to 1/8 (Page 114)
	OCR0A |= 0b10000000; //sets duty cycle to 1/2
}

//ISRs

ISR(ADC_vect){ //ISR for reflective sensor when ADC conversion complete

	if(ADC < lowest) {
		lowest = ADC;
	}

	if( (PIND & 0x04 ) == 0x04){ // if the item is still in front of the OR sensor
		ADCSRA |= _BV(ADSC); // Starts the conversion
	}
	else{
		
		if(lowest <= Bl_Max && lowest >= Bl_Min){
			list->push_back(list, BLACK);
		}
		else if(lowest <= St_Max && lowest >= St_Min){
			list->push_back(list, STEEL);
		}
		else if(lowest <= Wh_Max && lowest >= Wh_Min){
			list->push_back(list, WHITE);
		}
		else if(lowest <= Al_Max && lowest >= Al_Min){
			list->push_back(list, ALUMINUM);
			
		}
	}
	LCDClear();
	LCDWriteStringXY(0, 1, "PART PENDING");
}

ISR(INT2_vect){ // OR sensor
    lowest = 0xffff; 
    ADCSRA |= _BV(ADSC); // Starts the conversion
}

ISR(INT3_vect){// EX/EOT sensor, it is hooked up to PORT D3
	//current_state = 1; -do we need this?
	//PORTB = 0b00000000;    // this is Brake Vcc

	Item* it = list->head;
	Material mat = it->material;
	
	if(mat == STEEL){
		steel_count++;
	}
	else if(mat == ALUMINUM){
		aluminum_count++;	
	}
	else if(mat == BLACK){
		black_count++;	
	}
	else if(mat == WHITE){
		white_count++; 
	}
	
	Item* front = list->pop_front(list); // delete that item from the list
	free(front); 
	LCDClear();
	LCDWriteStringXY(0, 0, "PART SORTED");
    /*
    if((PIND &= 0x08) == 0x08)// this means that there is something in front of the exit sensor 
    if((PIND &= 0x08) == 0x00)// this means that there is nothing in front of the exit sensor
    */
}

ISR(INT1_vect) { //pause button hooked up to D1
	mTimer(20);
	PORTB = 0b00000000; //Stop motor
	
	LCDClear();
	LCDWriteStringXY(0, 0, "list size is:" );
	LCDWriteIntXY(14,0,list->size(list), 2);
	mTimer(3000); 
	
	LCDWriteStringXY(0, 0, "BL: WH: ST: AL: ");
	LCDWriteStringXY(0, 1, "S   S   S   S   S   ");
	LCDWriteIntXY(1,1,black_count, 2);
	LCDWriteIntXY(1,5,white_count, 2);
	LCDWriteIntXY(1,9,steel_count, 2);
	LCDWriteIntXY(1,13,aluminum_count, 2);
	
}

int main(void)
{
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
	
	PORTB = 0b00000010;//turns on DC motor forward (CCW)    
		
}

