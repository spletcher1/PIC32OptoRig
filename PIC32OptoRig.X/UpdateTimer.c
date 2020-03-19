#include "GlobalIncludes.h"

#define PRESCALE               256
#define TOGGLES_PER_SEC        2000
#define T1_TICK               (GetPeripheralClock()/PRESCALE/TOGGLES_PER_SEC)

unsigned char volatile updateTrigger500us;
unsigned char volatile updateTrigger1ms;
unsigned char volatile updateTrigger1sec;
unsigned char volatile secondCounter;
unsigned char volatile msCounter;
int hbCounter;

extern struct FullProgram theProgram;

void ConfigureUpdateTimer(void) {
    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_256, T1_TICK);
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);
    hbCounter = 0;
    secondCounter = 0;
    msCounter=0;
}

// This timer goes off every 1ms
void __ISR(_TIMER_1_VECTOR, IPL2SOFT) Timer1Handler(void) {
    hbCounter++;
    // Deal with heartbeat
    if (hbCounter >= 1000) {       
        FLIP_HEARTBEAT_LED();  
        if (theProgram.programStatus == RUNNING || theProgram.programStatus == STAGED)
            FLIP_SIGNALLED_LED();                
        if (secondCounter == 1) {
            secondCounter = 0;
            updateTrigger1sec = 1;
        }
        else {
            secondCounter++;
        }
        hbCounter = 0;
    }
    
    msCounter++;
    if(msCounter >=2){
        updateTrigger1ms=1;
        msCounter=0;
    }
    
    // Moved to interrupt.
    //ProcessOptoStep();
    updateTrigger500us = 1;
    mT1ClearIntFlag();

}
