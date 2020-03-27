/*
OR, RL and EX sensor
The ISR for ADC_vect is taking place under the assumption that the list has already been initialized in main.
It would look like the follow code:
List* list = new_list();
Therefore the ISR for ADC_vect corresponds to those assumptions

(also we would only be able to use the destroy function and not delete list function that way we don't need to keep making a new list)

*/

#include <avr/io.h>
#include <stdlib.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>

unsigned int lowest; 
const int Al_Max = 350;
const int Al_Min = 0; 
const int St_Max = 699;
const int St_Min = 351;
const int Bl_Max = 1023;
const int Bl_Min = 941;
const int Wh_Max = 940;
const int Wh_Min = 700;

ISR(ADC_vect)
{
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
            list->push_back(list, ALUMINIUM);
        }
    }
}


//The ISR for the OR sensor
ISR(INT2_vect) // it is hooked up to PORTD2
{
    lowest = 0xffff; 

    ADCSRA |= _BV(ADSC); // Starts the conversion

}

// The ISR for the exit sensor
ISR(INT3_vect)// it is hooked up to PORT D3
{
    //Depends on what we want it to do when the piece first triggers the sensor
}

/*
In the main while loop we can do the following unless you come up with better ideas for it
the dc motor is running and when the exit sensor senses that a piece has come up on its radar then we stop the dc motor
and check for the material to move the stepper motor the amount that it needs to and once the stepper motor is there
then we run the dc motor until the piece is no longer there

so the only code we need for the exit sensor is the following
if((PIND &= 0x08) == 0x08)// this means that there is something in front of the exit sensor 
if((PIND &= 0x08) == 0x00)// this means that there is nothing in front of the exit sensor

*/


void init_ADC () 
{
    // config the external interrupt ======================================
    EIMSK |= (_BV(INT2)); // enable INT2
    EICRA |= (_BV(ISC21) | _BV(ISC20)); // rising edge interrupt
    ADCSRA |= _BV(ADEN); // enable ADC
    ADCSRA |= _BV(ADIE); // enable interrupt of ADC
    ADMUX |= _BV(MUX0) | _BV(REFS0);  
}

void PWM (){
    TCCR0A |= 0b10000011; //first bit sets compare match output mode to clear, last two set whole thing to mode 3 (table 13-7)
    TCCR0B |= 0b00000010; //sets clock prescale to 1/8 (Page 114)
    OCR0A |= 0b01100000; //sets duty cycle to 1/2 
}


void mTimer(int count){
    int i = 0;
    //Set the Waveform Generation mode bit description to clear timer on compare math mode (CTC) only
    TCCR1B |=_BV(WGM12);
    OCR1A = 0x03e8; // sets the output compare register for 1000 cycles = 1ms
    TCNT1 = 0x0000; //sets the initial value of the Timer Counter to 0x0000
    TIFR1 |=_BV(OCF1A); // so it checks whether the values of the timer counter 1 and the data of OCR1A are the same
    while(i < count){
        if((TIFR1 & 0x02) == 0x02){
            TIFR1 |=_BV(OCF1A); //clears the interrupt flag by writing one to a bit
            i++;
        }
    }
    return;
}



