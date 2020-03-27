/*
Tal_Christine RL and OR sensor Test.c
The purpose of this test is to test the functionality of the sensors with this code
Wiring setup:
RL sensor goes to F0, OR sensor goes to D2
The DC motor should be as follows: DC Motor B7 to PWM, B3 to EA, B2 to EB, B1 to IA, B0 to IB
Alu = 0 ->350, Ste = 351 ->699, Wht = 700 -> 940, Blk = 941 - 1023
*/

#include <avr/io.h>
#include <stdlib.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>

unsigned int lowest; 
int material; 
int result_flag = 0; 

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
            material = 0; 
        }
        else if(lowest <= St_Max && lowest >= St_Min)
        {
            material = 1;
        }
        else if(lowest <= Wh_Max && lowest >= Wh_Min)
        {
            material = 2; 
        }
        else if(lowest <= Al_Max && lowest >= Al_Min)
        {
            material = 3; 
        }
         result_flag = 1; // result flag will only turn to one once the item is no longer in front of the sensor and material has a value
    }
}


//The ISR for the OR sensor
ISR(INT2_vect) // it is hooked up to PORTD2
{
    lowest = 0xffff; 

    ADCSRA |= _BV(ADSC); // Starts the conversion

}

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

int main()
{
    DDRD = 0x00; // for the interrupts for the sensors
    DDRC = 0xff; // for the leds
    DDRB = 0xff; // for the dc motor
	
    TCCR1B |=_BV(CS10);
    init_ADC();
    PWM(); 
    sei(); 
	
    while(result_flag != 1) 
    {
	PORTB = 0b00000010;		// DC motor forward (CCW)
    }
    
    PORTB = 0b00000000;    // this is Brake Vcc
 
    mTimer(1000); 

    if (material == 0)
    {
        PORTC = 0b00000011;
    }
    else if(material == 1)
    {
        PORTC = 0b00001111;
    }
    else if(material == 2)
    {
        PORTC = 0b00111111;
    }
    else if(material == 3)
    {
        PORTC = 0b11111111; 
    }
    

}

