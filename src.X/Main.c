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
#define TIMER_1_MAX_VALUE  65535 // us
#define ONE_MILISECOND      1000 // us
#define TIMER_1_CORRECTION    35 // us
#define BUTTON_DELAY         500 // ms


// subroutine definitions
void print_binary(int num);
void output(void);


// global variables
int milisecond = 0;
int second = -1; // ticks to 0 (instead of 1)
int minute = 0;
int hour = 0;


/* -=-=-=-=-=-=- INTERRUPT -=-=-=-=-=-=- */

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
            
            // display result every second
            output();
        }
    }

    return;
}


void main(void) {
    /* -=-=-=-=-=-=- PIN/LED RELATED IO -=-=-=-=-=-=- */
    
    // make PORTA digital (default analog)
    ADCON1bits.PCFG = 0b1111;
    
    // define clock outputs
    TRISA = 0; // sec
    TRISB = 0; // min
    TRISD = 0; // hr
    
    // define reset button as input
    TRISCbits.RC0 = 1;
    // define edit buttons as input
    TRISCbits.RC1 = 1; // min-
    TRISCbits.RC2 = 1; // min+
    TRISCbits.RC3 = 1; // hr-
    TRISCbits.RC4 = 1; // hr+
    
    // turn off all pins/leds
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
    
    
    /* -=-=-=-=-=-=- LOOP -=-=-=-=-=-=- */

    // polling for buttons
    while(1){
        if(PORTCbits.RC0){
            // reset clock
            milisecond = 0;
            second = -1; // ticks to 0 (instead of 1)
            minute = 0;
            hour = 0;
            
        }else if(PORTCbits.RC1){
            // decrement minute (deactivate timer-1 meanwhile)
            T1CONbits.TMR1ON = 0;
            while(PORTCbits.RC1){
                minute = (minute + 59) % 60;
                output();
                __delay_ms(BUTTON_DELAY);
            }
            TMR1 = TIMER_1_MAX_VALUE - ONE_MILISECOND + TIMER_1_CORRECTION;
            T1CONbits.TMR1ON = 1;
            
        }else if(PORTCbits.RC2){
            // increment minute (deactivate timer-1 meanwhile)
            T1CONbits.TMR1ON = 0;
            while(PORTCbits.RC2){
                minute = (minute + 1) % 60;
                output();
                __delay_ms(BUTTON_DELAY);
            }
            TMR1 = TIMER_1_MAX_VALUE - ONE_MILISECOND + TIMER_1_CORRECTION;
            T1CONbits.TMR1ON = 1;
            
        }else if(PORTCbits.RC3){
            // decrement hour (deactivate timer-1 meanwhile)
            T1CONbits.TMR1ON = 0;
            while(PORTCbits.RC3){
                hour = (hour + 23) % 24;
                output();
                __delay_ms(BUTTON_DELAY);
            }
            TMR1 = TIMER_1_MAX_VALUE - ONE_MILISECOND + TIMER_1_CORRECTION;
            T1CONbits.TMR1ON = 1;
            
        }else if(PORTCbits.RC4){
            // increment hour (deactivate timer-1 meanwhile)
            T1CONbits.TMR1ON = 0;
            while(PORTCbits.RC4){
                hour = (hour + 1) % 24;
                output();
                __delay_ms(BUTTON_DELAY);
            }
            TMR1 = TIMER_1_MAX_VALUE - ONE_MILISECOND + TIMER_1_CORRECTION;
            T1CONbits.TMR1ON = 1;
        }
    }
    
    // turn of timer-1 (good practice)
    T1CONbits.TMR1ON = 0;
    
    return;
}


// used for printf
void putch(unsigned char data) {
    while(!PIR1bits.TXIF){
        continue;
    }
    TXREG = data;
    
    return;
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


void output(void){
    // output clock on console
    print_binary(hour);
    printf(":");
    print_binary(minute);
    printf(":");
    print_binary(second);
    printf("\n");

    // output clock on pins/leds
    LATA = (unsigned char)second;
    LATB = (unsigned char)minute;
    LATD = (unsigned char)hour;
    
    return;
}
