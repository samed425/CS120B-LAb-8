/*	Author: bbaid001
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
    TCCR1B = 0x0B;
    OCR1A = 125;
    TIMSK1 = 0x02;
    TCNT1 = 0;
    _avr_timer_cntcurr = _avr_timer_M;
    SREG |= 0x80;
}

void TimerOff(){
    TCCR1B = 0x00;
}

void TimerISR(){
    TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect){
    _avr_timer_cntcurr--;
    if(_avr_timer_cntcurr == 0){
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}

void TimerSet(unsigned long M){
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

void set_PWM(double frequency){
    static double current_frequency;
    
    if(frequency != current_frequency){
        if(!frequency){
            TCCR3B &= 0x08;
        }
        else{
            TCCR3B |= 0x03;
        }
        if(frequency < 0.954){
            OCR3A = 0xFFFF;
        }
        else if(frequency > 31250){
            OCR3A = 0x0000;
        }
        else{
            OCR3A = (short) (8000000 / (128 * frequency)) - 1;
        }
        TCNT3 = 0;
        current_frequency = frequency;
    }
}

void PWM_on(){
    TCCR3A = (1 << COM3A0);
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
    set_PWM(0);
}

void PWM_off(){
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}

unsigned char tmp, i, check;
double note[8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};


enum Scale_States{INIT, wait, higher, lower}scale_state;

void Scale() {
    switch(scale_state) {
        case INIT:
            scale_state = wait;
            break;

        case wait:
            if (tmp == 0x02){
                scale_state = higher;
            }
            else if(tmp == 0x04){
                scale_state = lower;
            }
            else{
                scale_state = wait;
            }
            break;

        case higher:
            if(tmp == 0x02){
                scale_state = higher;
            }
            else{
                scale_state = wait;
            }
            break;

        case lower:
            if (tmp == 0x04){
                scale_state = lower;
            }
            else {
                scale_state = wait;
            }
            break;

        default:
            scale_state = wait;
            break;
    }
    switch (scale_state){
        case INIT:
            break;

        case wait:
            check = 0;
            break;

        case higher:
            if(check == 0 && i < 7){
                i++;
                check = 1;
            }
            set_PWM(note[i]);
            break;

        case lower:
            if(check == 0 && i > 0){
                i--;
                check = 1;
            }
            set_PWM(note[i]);
            break;

        default:
            break;
    }
}

enum Sound_States{OFF, ON, WAIT}sound_state;
void Sound() {
    switch(sound_state){
        case OFF:
            if(tmp == 0x01){
                sound_state = ON;
            }
            else if(tmp == 0x00){
                sound_state = OFF;
            }
            break;

        case ON:
            if(tmp == 0x00){
                sound_state = WAIT;
            }
            else if(tmp == 0x01){
                sound_state = ON;
            }
            break;

        case WAIT:
            if(tmp == 0x00){
                sound_state = WAIT;
            }
            else if(tmp == 0x01){
                sound_state = OFF;
            }
            break;

        default:
            sound_state = OFF;
            break;
    }
    switch(sound_state){
        case OFF:
            PWM_off();
            break;

        case ON:
            PWM_on();
            set_PWM(note[i]);
            break;

        case WAIT:
	    set_PWM(note[i]);
            break;

        default:
            PWM_off();
            break;
    }
}

int main(void) {
    DDRB = 0xFF; PORTB = 0x00;
    DDRA = 0x00; PORTA = 0xFF;

    tmp = 0;
    i = 0;
    check = 0;
    
    scale_state = INIT;
    sound_state = OFF;
    
    TimerSet(200);
    TimerOn();
    
    while (1) {
        tmp = ~PINA & 0x07;
        Scale();
        Sound();
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}

