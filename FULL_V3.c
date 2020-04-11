#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "LL.h" //header file for the linked list

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

Item* head;		
Item* tail;
Item* DQ;


//Linked list functions

void setup(Item** h,Item** t){
	*h = NULL;		
	*t = NULL;		
}

void initLink(Item** newItem){
	*newItem = malloc(sizeof(Item));
	(*newItem)->next = NULL;
}

/* push back */
void enqueue(Item** h, Item** t, Item** nL){

	if (*t != NULL){ //if not empty
		(*t)->next = *nL;
		*t = *nL; //(*t)->next;
	}
	else{ // if empty 
		*h = *nL;
		*t = *nL;
	}
}

void dequeue(Item** h, Item** t, Item** deQueuedItem){
	*deQueuedItem = *h;	// Will set to NULL if Head points to NULL
	/* Ensure it is not an empty queue */
	if (*h != NULL){
		*h = (*h)->next;
	}
	if (*h == NULL){
		*t = NULL;
	}
	
}

Material firstValue(Item** h){ //gives you back the material of the first item
	return((*h)->mat);
}


void clearQueue(Item** h, Item** t){

	Item* temp;

	while (*h != NULL){
		temp = *h;
		*h=(*h)->next;
		free(temp);
	}
	
	/* Last but not least set the tail to NULL */
	*t = NULL;		

	return;
}


char isEmpty(Item** h){
	return(*h == NULL);
}

int size(Item** h, Item** t){

	Item*   temp;			
	int 	numElements;

	numElements = 0;

	temp = *h;			/* point to the first item in the list */

	while(temp != NULL){
		numElements++;
		temp = temp->next;
	}/*while*/
	
	return(numElements);
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

void PWM (){
    TCCR0A |= 0b10000011; //first bit sets compare match output mode to clear, last two set whole thing to mode 3 (table 13-7)
    TCCR0B |= 0b00000010; //sets clock prescale to 1/8 (Page 114)
    OCR0A |= 0b10000000; //sets duty cycle to 1/2 
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
	dist = ((firstValue(&head)) - current_pos);
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
	
	//does the belt need to slow down?
	if((PIND &= 0x08) == 0x08)// this means that there is something in front of the exit sensor
	OCR0A |= 0b01000000; //sets duty cycle to 1/4 to slow down belt and allow bucket to prep

}

//ISRs

ISR(ADC_vect){ //ISR for reflective sensor when ADC conversion complete 
   // current_state = READ_IN_INT; -do we need this?
    if(ADC < lowest) {
        lowest = ADC;
    }

    if( (PIND & 0x04 ) == 0x04){ // if the item is still in front of the OR sensor 
        ADCSRA |= _BV(ADSC); // Starts the conversion
    }
    else{
	Item* newItem; 
	initLink(&newItem);
	LCDClear(); 
        
	if(lowest <= Bl_Max && lowest >= Bl_Min){
			newItem->mat = BLACK; 
			enqueue(&head,&tail,&newItem);
			LCDWriteStringXY(0, 0, "PART: BLACK");
        }
        else if(lowest <= St_Max && lowest >= St_Min){
			newItem->mat = STEEL; 
			enqueue(&head,&tail,&newItem);
			LCDWriteStringXY(0, 0, "PART: STEEL");
        }
        else if(lowest <= Wh_Max && lowest >= Wh_Min){
            newItem->mat = WHITE; 
			enqueue(&head,&tail,&newItem);
			LCDWriteStringXY(0, 0, "PART: WHITE");
        }
        else if(lowest <= Al_Max && lowest >= Al_Min){
            newItem->mat = ALUMINUM; 
			enqueue(&head,&tail,&newItem);
			LCDWriteStringXY(0, 0, "PART: ALUMINUM");
        }
    }
	LCDWriteStringXY(0, 1, "PART PENDING");
}

ISR(INT2_vect){ // OR sensor
    lowest = 0xffff; 
    ADCSRA |= _BV(ADSC); // Starts the conversion
}

ISR(INT3_vect){// EX/EOT sensor, it is hooked up to PORT D3
	
	if(head->mat == STEEL){
		steel_count++;
	}
	else if(head->mat == ALUMINUM){
		aluminum_count++;	
	}
	else if(head->mat == BLACK){
		black_count++;	
	}
	else if(head->mat == WHITE){
		white_count++; 
	}
	
	dequeue(&head, &tail, &DQ);
	free(DQ);
	
	LCDClear();
	LCDWriteStringXY(0, 0, "PART SORTED");
    /*
    if((PIND &= 0x08) == 0x08)// this means that there is something in front of the exit sensor 
    if((PIND &= 0x08) == 0x00)// this means that there is nothing in front of the exit sensor
    */
}



ISR(INT0_vect) { // HE sensor is hooked up to PORTD0, will set current position to 0 every time HE sensor is triggered
	current_pos = 0;
}

ISR(INT1_vect) { //pause button hooked up to D1
	mTimer(20);
	if(current_state == 0){
		current_state = 1;
		}else{
		current_state = 0;
	}
}

int main(){
//initialization
    DDRD = 0x00; // for the interrupts for the sensors
    DDRB = 0xff; // for the dc motor
    DDRC = 0xff; // output for the LCD
    DDRA = 0xff; //output for stepper
    DDRF = 0x00; //input for RL sensor @ F1
    
    InitLCD(LS_BLINK|LS_ULINE); //initialize LCD subsystem
    TCCR1B |=_BV(CS10); // we need this in main to use the timer
    init_int(); //initializes all interrupts
    PWM(); //Though the duty cycle may need to be changed for the DC motor
    sei(); // sets the Global Enable for all interrupts
    setup(&head, &tail); 

	
//initialize the stepper to get it to the starting position
	LCDWriteStringXY(0, 0, "Homing Start");
	while (current_pos != 0){
		StepperGo();
	}
	LCDWriteStringXY(0, 0, "Homing Complete");
	LCDWriteStringXY(0, 1, "Starting Sort");
	mTimer(1500); //notifies that initialization is complete and ready to begin
	
	
	//loop stuff starts  
  goto RUNNING;
  
  RUNNING:
	  //output to lcd that it's running normally
	PORTB = 0b00000010;//turns on DC motor forward (CCW)    
	
	//Here we're gonna do some fucked shit to try to make this thing SMART
		
	if(((firstValue(&head)) != current_pos) && (head != NULL)){ //is the stepper/bucket ready to receive the next item?
		step_what();//sets distance to go, and adjusts the step delay/stepper speed, and slows down belt if necessary
		StepperGo();
		
	} else { // YES, in position
		step_delay = 18;
		OCR0A |= 0b10000000; //sets duty cycle to 1/2 to speed belt back up after bucket aligned

	}
	  switch(current_state){
	  	case(0):
			goto RUNNING; //basically looping this stuff
			break;
		case(1):
			goto PAUSE; //triggered by pause interrupt
			break;

	  } // Changes states, otherwise keeps running belt
  	
  PAUSE:
  	//lcd output that we in the pause state
	PORTB = 0b00000000;	//DC motor STOP
	step_delay = 17; //returns the step delay to a reasonable speed, in case it was paused in the bucket stage
	//might need a small loop here that ramps down the bucket in case it's going full tilt
	
	Item* temp;			
	temp = head;			
	while(temp != NULL){
	   if(temp->mat == STEEL){
	     	pending_steel++; 
	    }
	    else if(temp->mat == ALUMINUM){
	     	pending_aluminum++; 
	    }
	    else if(temp->mat == BLACK){
	     	pending_black++; 
	     } 
	     else if(temp->mat == WHITE){
	     	pending_white++; 
	     }
		temp = temp->next;
	}
	
	//LCD displays number of sorted and pending items
	LCDWriteStringXY(2, 0, "SYSTEM PAUSE");
		for(int i = 0; i < 100; i++){
			mTimer(15);
			if(current_state == 0)
			goto RUNNING;
		}
		LCDClear();

	LCDWriteStringXY(0, 0, "P = Pending");
	LCDWriteStringXY(0, 1, "S = Sorted");
	for(int i = 0; i < 100; i++){
		mTimer(10);
		if(current_state == 0)
		goto RUNNING;
	}
	LCDClear(); 	

	//THIS FIRST PART IS SORTED ITEMS FROM WHEREVER WE STORE THEM
	LCDWriteStringXY(0, 0, "BL: WH: ST: AL: ");
	LCDWriteStringXY(0, 1, "S   S   S   S   S   ");
	LCDWriteIntXY(1,1,black_count, 2);
	LCDWriteIntXY(1,5,white_count, 2);
	LCDWriteIntXY(1,9,steel_count, 2);
	LCDWriteIntXY(1,13,aluminum_count, 2);
	for(int i = 0; i < 100; i++){
			mTimer(30);
			if(current_state == 0)
			goto RUNNING;
	}
	
	//THIS NEXT PART IS PENDING ITEMS FROM LINKED LIST	
	LCDWriteStringXY(0, 0, "BL: WH: ST: AL: ");
	LCDWriteStringXY(0, 1, "P   P   P   P   P   ");
	LCDWriteIntXY(1,1,pending_black, 2);
	LCDWriteIntXY(1,5,pending_white, 2);
	LCDWriteIntXY(1,9,pending_steel, 2);
	LCDWriteIntXY(1,13,pending_aluminum, 2);
	for(int i = 0; i < 100; i++){
		mTimer(30);
		if(current_state == 0)
		goto RUNNING;
	}
	
	if (current_state == 1)
		goto PAUSE;
	else
		goto RUNNING;
	
    
}
