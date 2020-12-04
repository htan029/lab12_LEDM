/*	Author: Heng Tan
 *  Partner(s) Name: 
 *	Lab Section: 024
 *	Assignment: Lab 12  Exercise 5
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *  
 *  Demmo Link: https://youtu.be/sFSAQKXEmrQ
 */
#include <avr/io.h>
#include <timer.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
// ======== Shift Register ==========================
void transmit_data(unsigned char data, unsigned char shifter){
    // PORTC[3] connected to SRCLR //independent
    // PORTC[2] connected to RCLK //independent
    // PORTC[1] connected to SRCLK //same
    // PORTC[0] connected to SER //same

    int i;
    for(i = 0; i < 8; i++){
        //sets SRCLR and clears SRCLK
        if(shifter == 0x00){
            PORTC = 0x08;
        } else if (shifter == 0x01){
            PORTC = 0x20;
        }
        
        //SER //same 
        PORTC |= ((data >> i) & 0x01);
        //SRCLK //same
        PORTC |= 0x02;
    }
    //RCLK 
    if(shifter == 0x00){
        PORTC |= 0x04;
    } else if (shifter == 0x01){
        PORTC |= 0x10;
    }
    
    //all lines
    PORTC = 0x00;
}
// ======== End Shift Register ======================

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

// ======== task struct ===================

typedef struct task {
  signed char state; // Current state of the task
  unsigned long int period; // Rate at which the task should tick
  unsigned long int elapsedTime; // Time since task's previous tick
  int (*TickFct)(int); // Function to call for task's tick
} task;

// ======== End Task scheduler data structure =======

// ======== Shared Variables ========================

unsigned char row1 = 0xF5;
unsigned char row2 = 0xFB;
unsigned char pattern1 = 0x3C;
unsigned char pattern2 = 0x24;
unsigned char row_out = 0x00;
unsigned char pattern_out = 0x00;
// ======== End Shared Variables ====================

// ====== Tasks =============================
enum Button_States {wait, up_state, left_state, right_state, down_state, wait2};
int Button_Tick(int state){
    switch(state){
        case wait: 
            if(((~PINA) & 0x01) == 0x01){
                state = up_state;
            } else if (((~PINA) & 0x04) == 0x04){
                state = down_state;
            } else if (((~PINA) & 0x02) == 0x02){
                state = left_state;
            } else if (((~PINA) & 0x08) == 0x08){
                state = right_state;
            }
            break;
        case up_state:  // A = 0x01 up
            state = wait2;
            break;
        case left_state: // A = 0x02 left
            state = wait2;
            break;
        case right_state: // A = 0x08 right
            state = wait2;
            break;
        case down_state: // A = 0x04 down
            state = wait2;
            break;
        case wait2: 
            if(((~PORTA) & 0x01) == 0x01) state = wait2;
            else if (((~PORTA) & 0x02) == 0x02) state = wait2;
            else if (((~PORTA) & 0x04) == 0x04) state = wait2;
            else if (((~PORTA) & 0x08) == 0x08) state = wait2;
            else state = wait;
            break;
        default: state = wait; break;
    }

    switch(state){
        case wait: break;
        case left_state: 
            if(pattern1 != 0xF0){
                pattern1 = (pattern1 << 1);
                pattern2 = (pattern2 << 1);
            }
            break;
        case right_state: 
            if(pattern1 != 0x0F){
                pattern1 = (pattern1 >> 1);
                pattern2 = (pattern2 >> 1);
            }
            break;
        case up_state: 
            if(row1 != 0xFA){
                row1 = (row1 >> 1) | 0x80;
                row2 = (row2 >> 1) | 0x80;
            }
            break;
        case down_state: 
            if(row1 != 0xEB){
                row1 = (row1 << 1) | 0x01;
                row2 = (row2 << 1) | 0x01;
            }
            break;
        case wait2: break;
        default: break;
    }	
    return state;
}
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
    transmit_data(pattern_out, 0);
    transmit_data(row_out, 1);
    // PORTC = pattern_out;	// Pattern to display
	// PORTD = row_out;		// Row(s) displaying pattern	
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
    task *tasks[] = { &task1, &task2};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    const char start = -1;

    tasks[0]->state = start;
    tasks[0]->period = 1;
    tasks[0]->elapsedTime = tasks[0]->period;
    tasks[0]->TickFct = &Shift_Tick;

    tasks[1]->state = start;
    tasks[1]->period = 100;
    tasks[1]->elapsedTime = tasks[1]->period;
    tasks[1]->TickFct = &Button_Tick;


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