/*	Author: Serena Lew
 *	Partner(s) Name: none
 *	Lab Section: 027
 *	Assignment: Lab #11  Exercise #2
 *	Exercise Description: 
 *		Using the LCD code, display "CS120B is Legend... wait for it DARY!"
 *		The string will not fit all at once, so scroll the text.
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Video link: https://youtu.be/Sop3dcnrAvQ
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
unsigned  char str[] = "CS120B is Legend... wait for it DARY!                ";	// 37 + 16 chars
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
			
			tmp = '\0';

			asm("nop");	// add a delay to allow PORTC to stabilize before checking 
			if (GetBit(PINC, 0) == 0) { tmp = '1'; }
			if (GetBit(PINC, 1) == 0) { tmp = '4'; }
			if (GetBit(PINC, 2) == 0) { tmp = '7'; }
			if (GetBit(PINC, 3) == 0) { tmp = '*'; }

			// check keys in col 2
			PORTC = 0xDF;	// enable col 5 with 0, disable others with 1's
			asm("nop");	// add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC, 0) == 0) { tmp = '2'; }
			if (GetBit(PINC, 1) == 0) { tmp = '5'; }
			if (GetBit(PINC, 2) == 0) { tmp = '8'; }
			if (GetBit(PINC, 3) == 0) { tmp = '0'; }

			// check keys in col 3
			PORTC = 0xBF;	// enable col 6 with 0, disable others with 1's
			asm("nop");	// add a delay to allow PORTC to stabilize before checking
			if (GetBit(PINC, 0) == 0) { tmp = '3'; }
			if (GetBit(PINC, 1) == 0) { tmp = '6'; }
			if (GetBit(PINC, 2) == 0) { tmp = '9'; }
			if (GetBit(PINC, 3) == 0) { tmp = '#'; }

			// check keys in col 4
			PORTC = 0x7F;
			asm("nop");
			if (GetBit(PINC, 0) == 0) { tmp = 'A'; }
			if (GetBit(PINC, 1) == 0) { tmp = 'B'; }
			if (GetBit(PINC, 2) == 0) { tmp = 'C'; }
			if (GetBit(PINC, 3) == 0) { tmp = 'D'; }
			break;
	}
	switch(tmp) {
		case '\0': PORTB = 0x1F;	break;	// all 5 LEDs on
		case '1': PORTB = 0x01;		break;	// hex equivalent
		case '2': PORTB = 0x02; 	break;
		case '3': PORTB = 0x03;		break;
		case '4': PORTB = 0x04;		break;
		case '5': PORTB = 0x05;		break;
		case '6': PORTB = 0x06;		break;
		case '7': PORTB = 0x07;		break;
		case '8': PORTB = 0x08;		break;
		case '9': PORTB = 0x09;		break;
		case 'A': PORTB = 0x0A;		break;
		case 'B': PORTB = 0x0B;		break;
		case 'C': PORTB = 0x0C;		break;
		case 'D': PORTB = 0x0D;		break;
		case '*': PORTB = 0x0E;		break;
		case '0': PORTB = 0x00;		break;
		case '#': PORTB = 0x0F;		break;
		default: PORTB = 0x1B;		break;	// should never occur.  middle LED off.
	}
	
}

enum ledDisplay_States { ledDisplay };
int ledDisplaySMTick(int state) {
	char s = 0;
	switch(state) {
		case ledDisplay:
			state = ledDisplay;
			break;
		default:
			state = ledDisplay;
			break;	
	}
	switch(state) {
		case ledDisplay:
			for (char i = s; i < 52; i++) {
				char temp = str[i];
				str[i] = str[i + 1];
				str[i + 1] = temp;
			}
			if (s >= 52) {
				s = 0;
			}
			s++;
			break;
		default:
			break;
	}
	LCD_DisplayString(1, str);
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
	static _task task1;
	_task *tasks[] = { &task1 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(_task*);
	const char start = -1;

	task1.state = start;
	task1.period = 500;
	task1.elapsedTime = task1.period;
	task1.TickFct = &ledDisplaySMTick;

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
