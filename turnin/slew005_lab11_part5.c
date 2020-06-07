/*	Author: Serena Lew
 *	Partner(s) Name: none
 *	Lab Section: 027
 *	Assignment: Lab #11  Exercise #5
 *	Exercise Description: 
 *			Design a game where a player controlled char avoids oncoming obstacles.
 *			Three buttons are used to operate the game.
 *			The cursor is the character.
 *			# represents the obstacles.
 *			One button starts and restarts the game.
 *			One button moves the player to the top row.
 *			One button moves the player to the borrom row.
 *			When the player touches an obstacle, it displays a gameover screen.
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Video link: https://youtu.be/3ukSgjzTkHI
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
unsigned char status = 0;
// --------------------end shared variables-------------------------------------------------


unsigned char ind = 17;
unsigned char ind2 = 34;
unsigned char t = 0;
enum Obs_States { Obs };
int ObsSMTick(int state) {
	char s = 0;
	switch(state) {
		case Obs:
			state = Obs;
			break;
		default:
			state = Obs;
			break;	
	}
	switch(state) {
		case Obs:
			if (status) {
				ind--;
				if (ind <= 0) {
					ind = 16;
				}
				if (ind < 10) {
					t = 1;
				}
				if (t) {
					ind2--;
					if (ind2 <= 16) {
						ind2 = 32;
					}
				}
			}
			break;
		default:
			break;
	}
		if (status) {
			LCD_Cursor(ind);
			LCD_WriteData('#');
		}
		if (!status) {
			LCD_Cursor(17);
			LCD_WriteData(' ');
		}
		else if (status && ind == 16) {
			LCD_Cursor(1);
		}
		else if (status) {
			LCD_Cursor(ind + 1);
		}
		LCD_WriteData(' ');

		if (t && status) {
			LCD_Cursor(ind2);
			LCD_WriteData('#');
		}
		if (status && ind2 == 32) {
			LCD_Cursor(17);
			t = 0;
		}
		else if (status) {
			LCD_Cursor(ind2 + 1);
		}
		LCD_WriteData(' ');

	return state;
}

unsigned char p = 0;
enum Player_States { Up, Down };
int PlayerSMTick(int state) {
	unsigned char button = ~PINA & 0x07;
	switch (state) {
		case Up:
			if (button == 0x02 && status) {
				state = Down;
			}
			else if (status){
				state = Up;
			}
			break;
		case Down:
			if (button == 0x04 && status) {
				state = Up;
			}
			else if (status) {
				state = Down;
			}
			break;
		default:
			state = Up;
			break;
	}
	switch (state) {
		case Up:
			p = 1;
			break;
		case Down:
			p = 17;
			break;
		default:
			break;
	}
	LCD_Cursor(p);
	if (!status) {
		LCD_Cursor(8);
	}
	return state;
}

unsigned char mes = 0;
enum Game_States{ Init, On, Restart, GameOver, OverMessage };
int GameSMTick(int state) {
	unsigned char b = ~PINA & 0x01; 
	switch (state) {
		case Init:
			if (b) {
				state = On;
			}
			else {
				state = Init;
			}
			break;
		case On:
			if (b) {
				state = Restart;
			}
			else if (p == ind || p == ind2) {
				state = GameOver;
			}
			else {
				state = On;
			}
			break;
		case Restart:
			state = On;
			break;
		case GameOver:
			state = OverMessage;
			LCD_ClearScreen();
			mes = 1;
			break;
		case OverMessage:
			if (b) {
				state = On;
				mes = 0;
			}
			else {
				state = OverMessage;
			}
			break;
		default:
			state = Init;
			break;
			
	}
	switch (state) {
		case Init:
			status = 0;
			break;
		case On:
			status = 1;
			break;
		case Restart:
			status = 0;
			LCD_ClearScreen();
			ind = 17;
			ind2 = 34;
			break;
		case GameOver:
			status = 0;
		case OverMessage:
			break;
		default:
			status = 0;
			break;

	}
	if (mes) {
		LCD_ClearScreen();
		LCD_DisplayString(1, "WASTED!");
	}
	return state;
}

int main(void) {
	DDRA = 0x00;	PORTA = 0xFF;
	DDRB = 0xFF;	PORTB = 0x00;	// PORTB set to output, outputs init 0s
	DDRC = 0xF0;	PORTC = 0x0F;	// PC7..4 outputs inits 0s. PC3..0 inputs init 1s
	DDRD = 0xFF;	PORTD = 0x00;

	LCD_init();

	// declare an array of tasks
	static _task task1, task2, task3;
	_task *tasks[] = { &task1, &task2, &task3 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(_task*);
	const char start = -1;

	task1.state = start;
	task1.period = 100;
	task1.elapsedTime = task1.period;
	task1.TickFct = &GameSMTick;

	task2.state = start;
	task2.period = 500;
	task2.elapsedTime = task2.period;
	task2.TickFct = &ObsSMTick;

	task3.state = start;
	task3.period = 50;
	task3.elapsedTime = task3.period;
	task3.TickFct = &PlayerSMTick;

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
