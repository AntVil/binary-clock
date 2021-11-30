#include "Definitions.h"

#define TIMER_1_MAX_VALUE 65535
#define ONE_SECOND         1000

void __interrupt(high_priority) timer_overflow_interrupt(void){
    if(TMR1IF){
        TMR1IF = 0;
        TMR1 = TIMER_1_MAX_VALUE - ONE_SECOND;
    }

    return;
}

void main(void) {
    // enable global interrupts
    INTCON = 0b11000000;

    // enable timer-1 interrupt
    PIE1bits.TMR1IE = 1;

    // setup timer
    T1CON = 0b00000001;

    // reset timer
    TMR1IF = 0;
    TMR1 = TIMER_1_MAX_VALUE - ONE_SECOND;

    // idle
    while(1){
        
    }
    
    return;
}

