/*	Author: lab
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
#include "timer.h"
#include "scheduler.h"
#endif

unsigned char pattern = 0xFF;
unsigned char row = 0xFE;
unsigned char A1;
unsigned char A0;
//--------------------------------------
// LED Row Shift SynchSM
// Period: 100 ms
//--------------------------------------
enum Row_States {wait, rowup,rowdown, rowrelease}Row_State;
int Row_Tick(int Row_State) {
	
	// Transitions
	switch (Row_State) {
		case wait:
			if(A0){
				Row_State = rowup;
			}
			else if(A1){
				Row_State = rowdown;
			}
			else{
				Row_State = wait;
			}
			break;
		case rowdown:
			if(row == 0xEF){
				//do nothing, keep it the same
			}
			else{
				row = (row << 1) | 0x01;
			}
			Row_State = rowrelease;
			break;
		case rowdown:
			if(row == 0xFE){
				//do nothing, keep it the same
			}
			else{
				row = (row >> 1) | 0x80;
			}
			Row_State = rowrelease;
			break;
		case rowrelease:
			if(!A0 && !A1){
				Row_State = wait;
			}
			else{
				Row_State = rowrelease;
			}
			break;
		default: Row_State = wait;
	}
}

enum Display_States{display}Display_State;
Display_Tick(int Display_State){
	switch(Display_State){
		case display:
			PORTC = pattern;
			PORTD = row;
			Display_State = display;
			break;
		default: Display_State = display; break;
	}
}




int main(void) {
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    DDRA = 0x00; PINA = 0xFF;

    static task task1, task2;
    task *tasks[] = {&task1, &task2};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    //Prelab
    task1.state = start;
    task1.period = 100; 
    task1.elapsedTime = task1.period;
    task1.TickFct = &Row_Tick;
	
    task2.state = start;
    task2.period = 50;
    task2.elapsedTime = task2.period;
    task2.TickFct = &Display_Tick;

    TimerSet(1);
    TimerOn();
    unsigned short i;
	A0 = ~PINA & 0x01;
	A1 = ~PINA & 0x02;

    while (1) {
	    for(i=0; i<numTasks; i++){ //Scheduler code
			if(tasks[i]->elapsedTime == tasks[i]->period){
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
    }
    return 1;
}
