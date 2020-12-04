/*	Author: Heng Tan
 *  Partner(s) Name: 
 *	Lab Section: 024
 *	Assignment: Lab 12  Exercise 3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *  
 *  Demmo Link: https://youtu.be/T-CTfpg4yDc
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

unsigned char row1 = 0x15;
unsigned char row2 = 0xFB;
unsigned char pattern1 = 0x3C;
unsigned char pattern2 = 0x24;
unsigned char row_out = 0x00;
unsigned char pattern_out = 0x00;
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
enum Shift_States {shift};
int Shift_Tick(int state){
    static unsigned char flep = 0x00;
    switch(state){
        case shift: break;
        default: state = shift; break;
    }

    switch(state){
        case shift: 
            if(flep == 0x00){
                row_out = row1;
                pattern_out = pattern1;
            } else {
                row_out = row2;
                pattern_out = pattern2;
            }
            flep = ~flep;
            break;
        default: break;
    }
    PORTC = pattern_out;	// Pattern to display
	PORTD = row_out;		// Row(s) displaying pattern	
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
    tasks[0]->period = 1;
    tasks[0]->elapsedTime = tasks[0]->period;
    tasks[0]->TickFct = &Shift_Tick;


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