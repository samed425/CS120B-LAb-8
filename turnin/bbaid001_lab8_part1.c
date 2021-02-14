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
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

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

enum States{INIT, wait, playC, playD, playE}state;
unsigned char button;

void Tick(){
    switch(state){
        case INIT:
            state = wait;
            break;

        case wait:
            if(button == 0x01){
                state = playC;
            }
            else if(button == 0x02){
                state = playD;
            }
            else if(button == 0x04){
                state = playE;
            }
            else{
                state = wait;
            }
            break;

        case playC:
            if(button == 0x01){
                state = playC;
            }
            else{
                state = wait;
            }
            break;

        case playD:
            if(button == 0x02){
                state = playD;
            }
            else{
                state = wait;
            }
            break;

        case playE:
            if(button == 0x04){
                state = playE;
            }
            else{
                state = wait;
            }
            break;

        default:
            state = wait;
            break;
    }
    
    switch(state){
        case INIT:
            set_PWM(0);
            break;

        case wait:
            set_PWM(0);
            break;

        case playC:
            set_PWM(261.63);
            break;

        case playD:
            set_PWM(293.66);
            break;

        case playE:
            set_PWM(329.63);
            break;

        default:
            break;
    }
}

int main(void) {
    DDRB = 0xFF; PORTB = 0x00;
    DDRA = 0x00; PORTA = 0xFF;

    button = 0x00;
    state = INIT;
    PWM_on();
    
    while (1) {
        button = ~PINA & 0x07;
        Tick();
    }
    return 1;
}

