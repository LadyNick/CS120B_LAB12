/*	Author: Nicole Navarro
 *  Partner(s) Name: 
 *	Lab Section:21
 *	Assignment: Lab #12  Exercise #3
 *	**For this part I wasn't testing the row or column movement, just the rectangle display
 *	Video: https://youtu.be/QWB2TEUQ9hQ
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

char pattern_ [5] = {0, 0x3C, 0x24, 0x3C, 0};
char row_ [5] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF}; 
unsigned char update = 0;

//unsigned char pattern = 0x80;
//unsigned char row = 0xE0;
unsigned char A1;
unsigned char A0;
unsigned char A2;
unsigned char A3;

enum Row_States {rowwait, rowup,rowdown, rowrelease}Row_State;
int Row_Tick(int Row_State) {
	
	// Transitions
/*	switch (Row_State) {
		case rowwait:
			if(A0){
				Row_State = rowup;
			}
			else if(A1){
				Row_State = rowdown;
			}
			else{
				Row_State = rowwait;
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
		case rowup:
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
				Row_State = rowwait;
			}
			else{
				Row_State = rowrelease;
			}
			break;
		default: Row_State = rowwait;
	}
*/	return Row_State;
}

enum Col_States{colwait,left,right,colrelease}Col_State;
int Col_Tick(int Col_State){
/*	switch(Col_State){
		case colwait:
			if(A2){
				Col_State = left;
			}
			else if(A3){
				Col_State = right;
			}
			else{
				Col_State = colwait;
			}
			break;
		case left:
			if(pattern == 0x80){
				//do nothing, its on the edge
			}
			else{
				pattern = pattern << 1;
			}
			Col_State = colrelease; 
			break;
		case right:
			if(pattern == 0x01){
				//do nothing, on edge
			}
			else{
				pattern = pattern >> 1;
			}
			Col_State = colrelease;
			break;
		case colrelease:
			if(!A2 && !A3){
			       Col_State = colwait;
			}
	 		else{
				Col_State = colrelease;
			}
			break;
		default: Col_State = colwait; break;		
	}
*/	return Col_State;
}

enum Display_States{display}Display_State;
int Display_Tick(int Display_State){
	
	switch(Display_State){

		case display:
			PORTC = pattern_[update];
			PORTD = row_[update];
			++update;
			if(update > 4){
				update = 0;
			}
			Display_State = display;
			break;
		default: Display_State = display; break;
	}
	return Display_State;
}




int main(void) {
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    DDRA = 0x00; PINA = 0xFF;

    static task task1, task2, task3;
    task *tasks[] = {&task1, &task2, &task3};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    //ROW
    task1.state = start;
    task1.period = 100; 
    task1.elapsedTime = task1.period;
    task1.TickFct = &Row_Tick;

    //COL
    task2.state = start;
    task2.period = 100;
    task2.elapsedTime = task2.period;
    task2.TickFct = &Col_Tick;

    task3.state = start;
    task3.period = 1;
    task3.elapsedTime = task3.period;
    task3.TickFct = &Display_Tick;

    TimerSet(1);
    TimerOn();
    unsigned short i;
    
    while (1) {
	    A0 = ~PINA & 0x01;
            A1 = ~PINA & 0x02;
	    A2 = ~PINA & 0x04;
	    A3 = ~PINA & 0x08;

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
