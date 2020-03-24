/*
OR and RL sensor
*/

#include <avr/io.h>
#include <stdlib.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>

volatile unsigned int ADC_result;

unsigned int lowest; 

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
        if(lowest <= 984 && lowest >=972)
        {
            //Material is set to black
        }
        else if(lowest <= 570 && lowest >= 411)
        {
            //Material is set to steel
        }
        else if(lowest <= 966 && lowest >=942)
        {
            //Material is set to white
        }
        else if(lowest <= 202 && lowest >= 50)
        {
            //Material is set to aluminum 
        }
    }
}


//The ISR for the OR sensor
ISR(INT2_vect) // it is hooked up to PORTD2
{
    //create a new "item"
    lowest = 0xffff; 

    ADCSRA |= _BV(ADSC); // Starts the conversion

}

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