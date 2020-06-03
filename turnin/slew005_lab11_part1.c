/*	Author: lab
 *	Partner(s) Name: none
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

#include "bit.h"
#include "timer.h"


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
unsigned char led0_output = 0x00;
unsigned char led1_output = 0x00;
unsigned char pause = 0;
// --------------------end shared variables-------------------------------------------------

// monitors button connected to PA0
// when button is pressed, shared variable "pause" is toggled
enum pauseButtonSM_States { pauseButton_wait, pauseButton_press, pauseButton_release };
int pauseButtonSMTick(int state) {
	unsigned char press = ~PINA & 0x01;
	switch (state) {
		case pauseButton_wait:
			state = press == 0x01 ? pauseButton_press : pauseButton_wait;
			break;
		case pauseButton_press:
			state = pauseButton_release;
			break;
		case pauseButton_release:
			state = press == 0x00 ? pauseButton_wait : pauseButton_press;
			break;
		default: 
			state = pauseButton_wait;
			break;
	}
	switch (state) {
		case pauseButton_wait:
			break;
		case pauseButton_press:
			pause = (pause == 0) ? 1 : 0;	// toggle pause
			break;
		case pauseButton_release:
			break;
	}
	return state;
}

// if paused: do NOT toggle LED connected to PB0
// if unpaused: toggle LED connected to PB0
enum toggleLED0_States { toggleLED0_wait, toggleLED0_blink };
int toggleLED0SMTick(int state) {
	switch (state) {
		case toggleLED0_wait:
			state = !pause ? toggleLED0_blink : toggleLED0_wait;
			break;
		case toggleLED0_blink:
			state = pause ? toggleLED0_wait : toggleLED0_blink;
			break;
		default:
			state = toggleLED0_wait;
			break;
	}
	switch (state) {
		case toggleLED0_wait:
			break;
		case toggleLED0_blink:
			led0_output = (led0_output == 0x00) ? 0x01 : 0x00;	// toggle LED
			break;
	}
	return state;
}

// if paused: Do NOT toggle LED connected to PB1
// if unpaused: toggle LED connected to PB1
enum toggleLED1_States { toggleLED1_wait, toggleLED1_blink };
int toggleLED1SMTick(int state) {
	switch (state) {
		case toggleLED1_wait: 
			state = !pause ? toggleLED1_blink : toggleLED1_wait;
			break;
		case toggleLED1_blink:
			state = pause ? toggleLED1_wait : toggleLED1_blink;
			break;
		default:
			state = toggleLED1_wait;
			break;
	}
	switch (state) {
		case toggleLED1_wait:
			break;
		case toggleLED1_blink:
			led1_output = (led1_output == 0x00) ? 0x01 : 0x00;	// toggle LED
			break;
	}
	return state;
}

// combine blinking LED outputs from toggleLED0 SM and toggleLED1 SM, and output on PORTB
enum display_States { display_display };
int displaySMTick(int state) {
	unsigned char output;
	switch (state) {
		case display_display:
			state = display_display;
			break;
		default:
			state = display_display;
			break;
	}
	switch (state) {
		case display_display:
			output = led0_output | led1_output << 1;	// write shared outputs to local variables
			break;
	}
	PORTB = output;	// write combined shared output variables to PORTB
	return state;
}

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
			pause = 1;
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


int main(void) {
//	unsigned char x;
	DDRA = 0x00;	PORTA = 0xFF;
	DDRB = 0xFF;	PORTB = 0x00;	// PORTB set to output, outputs init 0s
	DDRC = 0xF0;	PORTC = 0x0F;	// PC7..4 outputs inits 0s. PC3..0 inputs init 1s

	// declare an array of tasks
	static _task task1, task2, task3, task4, task5;
	_task *tasks[] = { &task1, &task2, &task3, &task4, &task5 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(_task*);
	const char start = -1;

	// task 1 pauseButtonToggleSM
	task1.state = start;	// task initial state;
	task1.period = 50;	// task period
	task1.elapsedTime = task1.period;	// task current elapsed time
	task1.TickFct = &pauseButtonSMTick;	// function pointer for the tick

	// task 2 toggleLED0SM
	task2.state = start;
	task2.period = 500;
	task2.elapsedTime = task2.period;
	task2.TickFct = &toggleLED0SMTick;

	// task 3 toggleLED1SM
	task3.state = start;
	task3.period = 1000;
	task3.elapsedTime = task3.period;
	task3.TickFct = &toggleLED1SMTick;

	// task 4 displaySM
	task4.state = start;
	task4.period = 10;
	task4.elapsedTime = task4.period;
	task4.TickFct = &displaySMTick;
	
	// task 5 keypad
	task5.state = start;
	task5.period = 10;
	task5.elapsedTime = task5.period;
	task5.TickFct = &GetKeypadSMTick;

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
