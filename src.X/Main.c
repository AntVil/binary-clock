/*
-------------------------------------------------------------------
File: Main.c
Title: binary-clock
Simulation: possible
Controller: PIC18F4520
Frequency: 4 MHz (PLL disabled)
Compiler: XC8 ver. 2.32
MPLAB X 5.50
Version: 1.2
Date: 30.12.2021
Author: AntVil, Jandorid, A-lex323
DESCRIPTION:
    LATA: seconds (binary)
    LATB: minutes (binary)
    LATD: hours (binary)
    RC0:  reset-button
-------------------------------------------------------------------
*/

#include "Definitions.h"
#include <stdio.h>

// constants
#define TIMER_1_MAX_VALUE  65535
#define ONE_MILISECOND      1000
#define TIMER_1_CORRECTION    35

// subroutine definitions
void print_binary(int num);

// global variables
int milisecond = 0;
int second = -1;
int minute = 0;
int hour = 0;

void __interrupt(high_priority) timer_overflow_interrupt(void){
    // check for timer-1 interrupt flag
    if(TMR1IF){
        // reset timer-1
        TMR1IF = 0;
        TMR1 = TIMER_1_MAX_VALUE - ONE_MILISECOND + TIMER_1_CORRECTION;
        
        // increase time (handels overflows too)
        milisecond++;
        if(milisecond >= 1000){
            milisecond = 0;
            second++;
            if(second >= 60){
                second = 0;
                minute++;
                if(minute >= 60){
                    minute = 0;
                    hour = (hour + 1) % 24;
                }
            }
            
            // print clock to console every second
            print_binary(hour);
            printf(":");
            print_binary(minute);
            printf(":");
            print_binary(second);
            printf("\n");
            
            // output clock on pins every second
            LATA = (unsigned char)second;
            LATB = (unsigned char)minute;
            LATD = (unsigned char)hour;
        }
    }

    return;
}

void main(void) {
    /* -=-=-=-=-=-=- PORT RELATED IO -=-=-=-=-=-=- */
    
    // make PORTA digital (default analog)
    ADCON1bits.PCFG = 0b1111;
    
    // define clock outputs
    TRISA = 0;
    TRISB = 0;
    TRISD = 0;
    
    // define reset button as input
    TRISCbits.RC0 = 1;
    
    // turn off all leds
    LATA = 0;
    LATB = 0;
    LATD = 0;
    
    
    /* -=-=-=-=-=-=- CONSOLE IO -=-=-=-=-=-=- */
    
    // initialize uart (used for printing)
    TRISCbits.RC7 = 1;
    TRISCbits.RC6 = 1;
    TXSTAbits.TXEN = 1;
    RCSTAbits.SPEN = 1;
    
    printf("\n\n");
    
    
    /* -=-=-=-=-=-=- INTERRUPT SETUP -=-=-=-=-=-=- */
    
    // enable global interrupts
    INTCON = 0b11000000;

    // enable timer-1 interrupt
    PIE1bits.TMR1IE = 1;

    // setup timer
    T1CONbits.TMR1ON = 1;

    // reset timer
    TMR1IF = 0;
    TMR1 = TIMER_1_MAX_VALUE - ONE_MILISECOND + TIMER_1_CORRECTION;

    // polling for reset
    while(1){
        if(PORTCbits.RC0){
            milisecond = 0;
            second = -1;
            minute = 0;
            hour = 0;
        }
    }
    
    // turn of timer-1 (actually never called)
    T1CONbits.TMR1ON = 0;
    
    return;
}

// used for printf
void putch(unsigned char data) {
    while(!PIR1bits.TXIF){
        continue;
    }
    TXREG = data;
}

// used to print integer as binary number
void print_binary(int num){
    // initialize output String
    char binary[] = "00000000";
    
    // edit String (dezimal to binary conversion)
    for(int i=0;i<8;i++){
        if(num % 2 == 1){
            binary[7 - i] = '1';
        }else{
            binary[7 - i] = '0';
        }
        num /= 2;
    }
    
    // output to console
    printf(binary);
    
    return;
}
