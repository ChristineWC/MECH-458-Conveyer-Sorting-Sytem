#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"
#include <inttypes.h>
#include "list.h"

//Global variables
State current_state;

volatile int current_step = 4;
volatile int step_delay = 18; //global variable for i5s speed .
volatile int current_pos = 0;

int steel_count = 0; 
int aluminum_count = 0; 
int black_count = 0; 
int white_count = 0; 

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

//Linked list functions

List* new_list() //this is to initialize the list
{
	List* list = (List*)calloc(1, sizeof(List));

	init_list_table(list);
	list->create(list);

	return list;
}

void delete_list(List* list) //this function is to make sure that the list pointer is null
{
	list->destroy(list);
	free(list);
}

void init_list_table(List* list)// since this is using function pointers, this lets us use the syntax NameOfList->nameOfFunction(parameters); examples can be seen in main
{
	list->create  = create;
	list->destroy = destroy;
	list->size    = size;
	list->empty   = empty;

	list->push_back = push_back;
	list->pop_front = pop_front;
}

void create(List* this) // part of list initialization
{
	this->head  = NULL;
	this->tail  = NULL;
	this->_size = 0;
}

void destroy(List* this)
{
	if (this->empty(this))
	{
		return;
	}

	Item* it;
	while (this->size(this) != 0)
	{
		it = this->pop_front(this);
		free(it);
		it = NULL;
	}
}

unsigned int size(List* this)
{
	return this->_size;
}

bool empty(List* this)
{
	return this->_size == 0;
}

void push_back(List* this, Material m) // use this function to add stuff to the back of the list
{
	Item* new_item = (Item*)calloc(1, sizeof(Item));

	new_item->material = m;

	if (this->empty(this))
	{
		this->head     = new_item;
		this->tail     = new_item;
		new_item->next = NULL;
		new_item->prev = NULL;
	}
	else
	{
		new_item->prev   = this->tail;
		new_item->next   = NULL;
		this->tail->next = new_item;
		this->tail       = new_item;
	}

	this->_size++;
}

Item* pop_front(List* this) // use this function to delete stuff at the front of the list
{
	if (this->empty(this))
	{
		return NULL;
	}

	Item* front;
	if (this->_size == 1)
	{
		front      = this->head;
		this->head = NULL;
		this->tail = NULL;
	}
	else
	{
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

void init_int() //enables all necessary interrupts
{
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
    EICRA |= (_BV(ISC31) | _BV(ISC30)); // rising edge interrupt for EOT sensor
}

void PWM (){
    TCCR0A |= 0b10000011; //first bit sets compare match output mode to clear, last two set whole thing to mode 3 (table 13-7)
    TCCR0B |= 0b00000010; //sets clock prescale to 1/8 (Page 114)
    OCR0A |= 0b01100000; //sets duty cycle to 1/2 
}

void StepperMotorCW (int steps){

	for(int i = 1; i <= steps; i++){
		if(current_step == 3)
		current_step = 0;
		else
		current_step++;
		PORTA = spin[current_step];
		mTimer(step_delay);

		if ((i%2 == 0) && (i <= (steps/4)))
		step_delay--;
		if ((i%2 == 0) && (i >= (steps*4/5)))
		step_delay++;
		if (i%50 == 0)
		current_pos++;
	}
	step_delay = 18;
}

void StepperMotorCCW (int steps){
	
	for(int i = 1; i <= steps; i++){
		if(current_step == 0)
		current_step = 3;
		else
		current_step--;
		PORTA = spin[current_step];
		mTimer(step_delay);
		
		if ((i%2 == 0) && (i <= (steps/4)))
		step_delay--;
		if ((i%2 == 0) && (i >= (steps*4/5)))
		step_delay++;
		if (i%50 == 0)
		current_pos--;

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



//ISRs

ISR(ADC_vect) //ISR for reflective sensor when ADC conversion complete 
{
    current_state = READ_IN_INT;
	
    if(ADC < lowest) 
    {
        lowest = ADC;
    }

    if( (PIND & 0x04 ) == 0x04) // if the item is still in front of the OR sensor 
    {
        ADCSRA |= _BV(ADSC); // Starts the conversion
    }
    else
    {
        if(lowest <= Bl_Max && lowest >= Bl_Min)
        {
            list->push_back(list, BLACK);
        }
        else if(lowest <= St_Max && lowest >= St_Min)
        {
            list->push_back(list, STEEL);
        }
        else if(lowest <= Wh_Max && lowest >= Wh_Min)
        {
            list->push_back(list, WHITE);
        }
        else if(lowest <= Al_Max && lowest >= Al_Min)
        {
            list->push_back(list, ALUMINUM);
        }
    }
}

ISR(INT2_vect) // OR sensor
{
    lowest = 0xffff; 

    ADCSRA |= _BV(ADSC); // Starts the conversion

}

ISR(INT3_vect)// EX/EOT sensor, it is hooked up to PORT D3
{
	State = 1; 
	
	PORTB = 0b00000000;    // this is Brake Vcc
	Item* it = list->head;
	Material mat = it->material;
	
	if(mat == STEEL)
	{
		steel_count++;
	}
	else if(mat == ALUMINUM)
	{
		aluminum_count++;	
	}
	else if(mat == BLACK)
	{
		black_count++;	
	}
	else if(mat == WHITE)
	{
		white_count++; 
	}
	//trigger stepper motor state
    /*
    if((PIND &= 0x08) == 0x08)// this means that there is something in front of the exit sensor 
    if((PIND &= 0x08) == 0x00)// this means that there is nothing in front of the exit sensor
    */
}



ISR(INT0_vect) { // HE sensor is hooked up to PORTD0, will set current position to 0 every time HE sensor is triggered
	current_pos = 0;
}

ISR(INT1_vect) { //pause button hooked up to D1
	//put here whatever the pause button ISR code would be
	last_state = State
	goto PAUSE;
}


int main(){
//initialization
    DDRD = 0x00; // for the interrupts for the sensors
    DDRB = 0xff; // for the dc motor
    DDRC = 0xff; // output for the LCD
    DDRA = 0xff; //output for stepper
    DDRF = 0x00; //input for RL sensor @ F1
    List* list = new_list();
    
    LCDInit(LS_BLINK|LS_ULINE); //initialize LCD subsystem
    TCCR1B |=_BV(CS10); // we need this in main to use the timer
    init_int(); //initializes all interrupts
    PWM(); //Though the duty cycle may need to be changed for the DC motor
    sei(); // sets the Global Enable for all interrupts

	
//initialize the stepper to get it to the starting position
	while (current_pos != 0){
		StepperMotorCW(1);
	}
	
	//loop stuff starts
	
	//needs global variable "volatile int last_state" set in interrupts
  
  goto RUNNING;
  
  RUNNING:
	  //output to lcd that it's running normally
	  //turn on the dc motor to run belt
	PORTB = 0b00000010;		// DC motor forward (CCW)    
	
	//Here we're gonna do some fucked shit to try to make this thing SMART
	if(head.next.mat != current_pos){ //is the stepper/bucket ready to recieve the next item?
		stepper
	}
	  switch(State){
	  	case(0):
			goto RUNNING; //basically looping this stuff
			break;
		case(1):
			goto BUCKET; //triggered by EOT interrupt
			break;
		case(2):
			goto PAUSE; //triggered by pause interrupt
			break;

	  } // Changes states, otherwise keeps running belt
  
  BUCKET:
  	//lcd output that we in the bucket state
	//add stepper motor code here
	
	Stepper(list.color); //This needs to take in the bin number that the object needs to dump into
  	
  	State = 0;
	goto RUNNING;
  	
  PAUSE:
  	//lcd output that we in the pause state
	//lcd output the right things
	
	if (last_state == 1)
		goto BUCKET;
	else()
		goto RUNNING;
    while()
    {
	PORTB = 0b00000010;		// DC motor forward (CCW)      
    }
	
	
	    
	    
	
    

}
