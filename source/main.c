/*	Author: Nicole Navarro
 *  Partner(s) Name: 
 *	Lab Section:21
 *	Assignment: Lab #12  Exercise #4
 *	Video: https://youtu.be/rnHT7BRAF1M
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <stdbool.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#include "scheduler.h"
#endif

unsigned char pattern[5] = {0x00, 0x3C, 0x24, 0x3C, 0x00};
unsigned char row[5] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF}; 
unsigned char update = 0;
bool edge = false;


unsigned char A0;
unsigned char A1;
unsigned char A2;
unsigned char A3;

void transmit_data(unsigned char data, unsigned char reg) {
	unsigned char lower = ~((data & 0x0F) << 4); //takes bits 3-0 --> 7-4 and flips
	unsigned char higher = ~((data & 0xF0) >> 4); //this makes the bits 7-4 --> 3-0 and flips
	higher = higher & 0x0F; //so it becomes the lower and the higher btis it holds is 0
	lower = lower & 0xF0; //this ensures the lower bits of this char are now 0 so we can now just or higher and lower and assign it to data once again
	
	data = higher | lower;
	
	
	
    int i;
    if (reg == 1) {
        for (i = 0; i < 8 ; ++i) {
        // Sets SRCLR to 1 allowing data to be set
        // Also clears SRCLK in preparation of sending data
        PORTC = 0x08;
            // set SER = next bit of data to be sent.
            PORTC |= ((data >> i) & 0x01);
            // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
            PORTC |= 0x02;  
        }
        // set RCLK = 1. 
        PORTC |= 0x04;
    }

    else if (reg == 2) {
        for (i = 0; i < 8 ; ++i) {
        // Sets SRCLR to 1 allowing data to be set
        // Also clears SRCLK in preparation of sending data
        PORTD = 0x08;
            // set SER = next bit of data to be sent.
            PORTD |= ((data >> i) & 0x01);
            // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
            PORTD |= 0x02;  
        }
        // set RCLK = 1. 
        PORTD |= 0x04;
    }
}

enum Move_States {wait, up, down, left, right, release}Move_State;
int Move_Tick(int Move_State) {
	
	switch (Move_State) {
		case wait:
			if(A0){
				Move_State = up;
			}
			else if(A1){
				Move_State = down;
			}
			else if(A2){
				Move_State = left;
			}
			else if(A3){
				Move_State = right;
			}
			else{
				Move_State = wait;
			}
			break;
		case down:
			if(pattern[4] > 0){
					edge = true;
			}
			if(edge == true){
				edge = false; //reset edge bool 
				//do nothing, keep it the same
			}
			else{
				for(int i = 4; i > 0; --i){
					pattern[i] = pattern[i-1]; 
				}
				pattern[0] = 0x00; 
			}
			Move_State = release;
			break;
		case up:
			if(pattern[0] > 0x00){
				edge = true;
			
			}
			if(edge == true){
				edge = false; //reset
				//do nothing
			}
			else{
				for(int i = 0; i < 4; ++i){
					pattern[i] = pattern[i+1];
				}
				pattern[4] = 0x00;
			}
			Move_State = release;
			break;
		case left:
			for(int i = 0; i < 5; ++i){
				if(pattern[i] & 0x80){
					edge = true;
				}
			}
			if(edge == true){
				edge = false;
			}
			else{
				for(int i = 0; i < 5; ++i){
					pattern[i] = pattern[i] << 1;
				}
			}
			Move_State = release; 
			break;
		case right:
			for(int i = 0; i < 5; ++i){
				if(pattern[i] & 0x01){
					edge = true;
				}
			}
			if(edge == true){
				edge = false;
			}
			else{
				for(int i = 0; i < 5; ++i){
					pattern[i] = (pattern[i] >> 1) & 0x7F;	
				}
			}
			Move_State = release;
			break;
		case release:
			edge = false;
			if(!A0 && !A1 && !A2 && !A3){
				Move_State = wait;
			}
			else{
				Move_State = release;
			}
			break;
		default: Move_State = wait;
	}
	return Move_State;
}

enum Display_States{display}Display_State;
int Display_Tick(int Display_State){
	
	switch(Display_State){

		case display:
			transmit_data(pattern[update]);
			transmit_data(row[update]);
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

    static task task1, task2;
    task *tasks[] = {&task1, &task2};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    //MOVE
    task1.state = start;
    task1.period = 100; 
    task1.elapsedTime = task1.period;
    task1.TickFct = &Move_Tick;

    task2.state = start;
    task2.period = 1;
    task2.elapsedTime = task2.period;
    task2.TickFct = &Display_Tick;

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
