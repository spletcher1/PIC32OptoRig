#include "GlobalIncludes.h"

#define PRESCALE               256
#define TOGGLES_PER_SEC        1000
#define T1_TICK               (GetPeripheralClock()/PRESCALE/TOGGLES_PER_SEC)

unsigned char volatile updateTrigger1ms;
unsigned char volatile updateTrigger1sec;
unsigned char volatile secondCounter;
int hbCounter;

extern struct FullProgram theProgram;

void ConfigureUpdateTimer(void) {
    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_256, T1_TICK);
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);
    hbCounter = 0;
    secondCounter = 0;

}

// This timer goes off every 100ms

void __ISR(_TIMER_1_VECTOR, IPL2SOFT) Timer1Handler(void) {
    hbCounter++;
    // Deal with heartbeat
    if (hbCounter >= 500) {
        if (theProgram.programStatus == RUNNING || theProgram.programStatus == STAGED)
            FLIP_HEARTBEAT_LED();                
        if (secondCounter == 1) {
            secondCounter = 0;
            updateTrigger1sec = 1;
        }
        else {
            secondCounter++;
        }
        hbCounter = 0;
    }
    ProcessOptoStep();
    updateTrigger1ms = 1;
    mT1ClearIntFlag();

}
