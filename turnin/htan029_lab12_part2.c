/*	Author: Heng Tan
 *  Partner(s) Name: 
 *	Lab Section: 024
 *	Assignment: Lab 12  Exercise 2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *  
 *  Demmo Link: https://youtu.be/Z6XZe2oxu0M
 */
#include <avr/io.h>
#include <timer.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

// ======== task struct ===================

typedef struct task {
  signed char state; // Current state of the task
  unsigned long int period; // Rate at which the task should tick
  unsigned long int elapsedTime; // Time since task's previous tick
  int (*TickFct)(int); // Function to call for task's tick
} task;

// ======== End Task scheduler data structure =======

// ======== Shared Variables ========================
unsigned char up = 0;
unsigned char down = 0;
unsigned char row = 0x00;
unsigned char pattern = 0x80;
// ======== End Shared Variables ====================

// ======= find GCD =========================
unsigned long int findGCD(unsigned long int a, unsigned long int b){
    unsigned long int c;
    while (1){
        c= a%b;
        if(c==0){return b;}
        a = b; 
        b = c;
    }
    return 0;
}
// ====== end find GCD ======================

// ====== Tasks =============================
enum Button_States {wait, up_state, down_state, wait2};
int Button_Tick(int state){
    switch(state){
        case wait: 
            if(((~PINA) & 0x01) == 0x01){
                state = up_state;
            } else if (((~PINA) & 0x02) == 0x02){
                state = down_state;
            }
            break;
        case up_state: 
            up = 0;
            state = wait2;
            break;
        case down_state: 
            down = 0;
            state = wait2;
            break;
        case wait2: 
            if(((~PORTA) & 0x01) == 0x01) state = wait2;
            else if (((~PORTA) & 0x02) == 0x02) state = wait2;
            else state = wait;
            break;
        default: state = wait; break;
    }

    switch(state){
        case wait: break;
        case up_state: 
            if(pattern != 0x80){
                pattern = (pattern << 1);
            }
            break;
        case down_state: 
            if(pattern != 0x01){
                pattern = (pattern >> 1);
            }
            break;
        case wait2: break;
        default: break;
    }
    PORTC = pattern;	// Pattern to display
	PORTD = row;		// Row(s) displaying pattern	
    return state;
}
// ====== Tasks struct end ===================

// ====== main ==============================
int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    static task task1, task2, task3, task4;
    task *tasks[] = { &task1};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    const char start = -1;

    tasks[0]->state = start;
    tasks[0]->period = 100;
    tasks[0]->elapsedTime = tasks[0]->period;
    tasks[0]->TickFct = &Button_Tick;


    unsigned short i;
    unsigned long gcd = tasks[0]->period;
    for(i = 1; i < numTasks; i++){
        gcd = findGCD(gcd, tasks[i]->period);
    }

    TimerSet(gcd);
    TimerOn();
    
    /* Insert your solution below */
    while (1) {
        for(i = 0; i < numTasks; i++){
            if(tasks[i]->elapsedTime == tasks[i]->period){
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += gcd;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 0;
}