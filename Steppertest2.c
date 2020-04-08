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
