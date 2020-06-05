/*	Author: Serena Lew
 *	Partner(s) Name: none
 *	Lab Section: 027
 *	Assignment: Lab #11  Exercise #3
 *	Exercise Description: 
 *			Combine keypad and LCD functionality.
 *			When keypad is pressed and released, the character is displayed on the LCD
 *			and stays until a different button press occurs.
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Video link: https://youtu.be/jqf4xuMVsh0
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#include "bit.h"
#include "timer.h"
#include "io.h"



// --------------------find GCD function----------------------------------------------------
unsigned long int findGCD(unsigned long int a, unsigned long int b) {
	unsigned long int c;
	while(1) {
		c = a%b;
		if (c == 0) { return b; }
		a = b;
		b = c;
	}
	return 0;
}
// --------------------end find GCD function------------------------------------------------

// --------------------task scheduler data structure----------------------------------------
// struct for tasks represent a running process in our simple real-time operating system.
typedef struct _task {
	// tasks should have members that include: state, period, a measurement of 
	// elapsed time, and a function pointer
	signed char state;		// task's current state
	unsigned long int period;	// task period
	unsigned long int elapsedTime;	// time elapsed since last task tick
	int (*TickFct) (int);		// task tick function
} _task;
// --------------------end task scheduler data structure------------------------------------

// --------------------shared variables-----------------------------------------------------
// --------------------end shared variables-------------------------------------------------


// returns '\0' if no key pressed, else returns char '1', '2', .. '9', 'A', ...
// if multiple keyspressed, returns leftmost-topmost one
// keybad must be connected to port c
/* keypad arrangement
 
   		PC4 PC5 PC6 PC7
	col	 1   2   3   4 
   row	
   PC0   1  	 1   2   3   A
   PC1   2	 4   5   6   B
   PC2   3	 7   8   9   C
   PC3   4	 *   0   #   D

*/
unsigned char tmp = 0x00;
enum GetKeypadKey_States { keypad };
int GetKeypadSMTick (int state) {
	switch (state) {
		case keypad:
			state = keypad;
			break;
		default:
			state = keypad;
			break;
	}
	switch (state) {
		case keypad:
			PORTC = 0xEF;	// enable col 4 with 0, disable others with 1's
			
//			tmp = '\0';

			asm("nop");	// add a delay to allow PORTC to stabilize before checking 
			if (GetBit(PINC, 0) == 0) { tmp = 1; }
			if (GetBit(PINC, 1) == 0) { tmp = 4; }
			if (GetBit(PINC, 2) == 0) { tmp = 7; }
			if (GetBit(PINC, 3) == 0) { tmp = 0x0E; }

			// check keys in col 2
			PORTC = 0xDF;	// enable col 5 with 0, disable others with 1's
			asm("nop");	// add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC, 0) == 0) { tmp = 2; }
			if (GetBit(PINC, 1) == 0) { tmp = 5; }
			if (GetBit(PINC, 2) == 0) { tmp = 8; }
			if (GetBit(PINC, 3) == 0) { tmp = 0; }

			// check keys in col 3
			PORTC = 0xBF;	// enable col 6 with 0, disable others with 1's
			asm("nop");	// add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC, 0) == 0) { tmp = 3; }
			if (GetBit(PINC, 1) == 0) { tmp = 6; }
			if (GetBit(PINC, 2) == 0) { tmp = 9; }
			if (GetBit(PINC, 3) == 0) { tmp = 0x0F; }

			// check keys in col 4
			PORTC = 0x7F;
			asm("nop");
			if (GetBit(PINC, 0) == 0) { tmp = 0x0A; }
			if (GetBit(PINC, 1) == 0) { tmp = 0x0B; }
			if (GetBit(PINC, 2) == 0) { tmp = 0x0C; }
			if (GetBit(PINC, 3) == 0) { tmp = 0x0D; }
			break;
	}
}

enum ledDisplay_States { ldStart, ledDisplay1, ledDisplay2 };
int ledDisplaySMTick(int state) {
	unsigned char disp;
	unsigned char ch;
	switch(state) {
		case ldStart:
			state = ledDisplay2;
			break;
		case ledDisplay1:
			if (tmp > 9) {
				state = ledDisplay2;
				ch = 1;
			}
			else {
				state = ledDisplay1;
			}
			break;
		case ledDisplay2:
			if (tmp <= 9) { 
				state = ledDisplay1;
				ch = 0;
			}
			else {
				state = ledDisplay2;
			}
			break;
		default:
			state = ldStart;
			break;	
	}
	switch(state) {
		case ldStart:
			disp = ' ';
			ch = 1;
			break;
		case ledDisplay1:
			ch = 0;
			if (tmp == 1) { disp = 1; }
			else if (tmp == 2) { disp = 2; }
			else if (tmp == 3) { disp = 3; }
			else if (tmp == 4) { disp = 4; }
			else if (tmp == 5) { disp = 5; }
			else if (tmp == 6) { disp = 6; }
			else if (tmp == 7) { disp = 7; }
			else if (tmp == 8) { disp = 8; }
			else if (tmp == 9) { disp = 9; }
			else if (tmp == 0) { disp = 0; }
			break;
		case ledDisplay2:
			ch = 1;
			if (tmp == 0x0A) { disp = 'A'; }
			if (tmp == 0x0B) { disp = 'B'; }
			if (tmp == 0x0C) { disp = 'C'; }
			if (tmp == 0x0D) { disp = 'D'; }
			if (tmp == 0x0E) { disp = '*'; }
			if (tmp == 0x0F) { disp = '#'; } 
			break;
		default:
			break;
	}
	LCD_ClearScreen();
	if (ch == 1) {
		LCD_WriteData(disp);
	}
	else {
		LCD_WriteData(disp + '0');
	}
	return state;
}

int main(void) {
//	unsigned char x;
	DDRA = 0x00;	PORTA = 0xFF;
	DDRB = 0xFF;	PORTB = 0x00;	// PORTB set to output, outputs init 0s
	DDRC = 0xF0;	PORTC = 0x0F;	// PC7..4 outputs inits 0s. PC3..0 inputs init 1s
	DDRD = 0xFF;	PORTD = 0x00;

	LCD_init();

	// declare an array of tasks
	static _task task1, task2;
	_task *tasks[] = { &task1, &task2 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(_task*);
	const char start = -1;

	task1.state = start;
	task1.period = 50;
	task1.elapsedTime = task1.period;
	task1.TickFct = &GetKeypadSMTick;

	task2.state = start;
	task2.period = 100;
	task2.elapsedTime = task2.period;
	task2.TickFct = &ledDisplaySMTick;

	unsigned short i;	// scheduler for loop iterator
	unsigned long GCD = tasks[0]->period;
	for (i = 1; i < numTasks; i++) {
		GCD = findGCD(GCD, tasks[i]->period);
	}

	// set timer and turn it on
	TimerSet(GCD);
	TimerOn();

	while(1) {
		for (i = 0; i < numTasks; i++) {	// scheduler code
			if (tasks[i]->elapsedTime == tasks[i]->period) {	// task is ready to tick
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);	// set next state
				tasks[i]->elapsedTime = 0;	// reset the elapsed time for next tick
			}
			tasks[i]->elapsedTime += GCD;
		}
		while (!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}
