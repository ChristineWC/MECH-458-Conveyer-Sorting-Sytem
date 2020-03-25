/*
OR, RL and EX sensor

The ISR for ADC_vect is taking place under the assumption that the list has already been initialized in main.
It would look like the follow code:
List* list = new_list();
Therefore the ISR for ADC_vect corresponds to those assumptions

*/

#include <avr/io.h>
#include <stdlib.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>

volatile unsigned int ADC_result;

unsigned int lowest; 
int Al_Max = 202;
int Al_Min = 50; 
int St_Max = 570;
int St_Min = 411;
int Bl_Max = 984;
int Bl_Min = 972;
int Wh_Max = 966;
int Wh_Min = 942;

ISR(ADC_vect)
{
    ADC_result = (ADC &= 0x3ff); // This is to get the 10 bit part of it
    
    if(ADC_result < lowest)
    {
        lowest = ADC_result; 
    }

    if( (PIND &= 0x04 ) == 0x04) // if the item is still in front of the OR sensor 
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


void init_ADC () // I'm going to assume we are going to use PORTF0 for the reflective sensor because I am not quite sure how we change the ADC input pin
{
    // config the external interrupt ======================================
    EIMSK |= (_BV(INT2)); // enable INT2
    EICRA |= (_BV(ISC21) | _BV(ISC20)); // rising edge interrupt
    ADCSRA |= _BV(ADEN); // enable ADC
    ADCSRA |= _BV(ADIE); // enable interrupt of ADC
    ADMUX |= _BV(ADLAR) | _BV(REFS0); //ADC Multiplexer selection register bits 5 and 6 set to 1, ADLAR = ADC left adjust result; REFS0 set to 1 which selects voltage reference selection to core voltage (3.3v)
}

//In main everything is running just the way it should so this should work as long as it is hooked up properly 
/*
    



*/