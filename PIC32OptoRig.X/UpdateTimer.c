#include "GlobalIncludes.h"

#define PRESCALE               256
#define TOGGLES_PER_SEC        10
#define T1_TICK               (GetPeripheralClock()/PRESCALE/TOGGLES_PER_SEC)

int volatile HBCounter;
int volatile secondCounter;
unsigned char isInDarkMode;
unsigned char volatile timerFlag_100ms;
unsigned char volatile timerFlag_1ms;
unsigned char volatile timerFlag_1sec;

void ConfigureUpdateTimer(void) {
    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_256, T1_TICK);
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);
    HBCounter = 0;
    secondCounter=0;
}

void ConfigureOptoTimer(void) {
    // This timer is set to go off every 1ms.    
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_256, OPTO_TICK);
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_6);
    timerFlag_1ms = 0;
}


void __ISR(_TIMER_2_VECTOR, IPL6AUTO) Timer2Handler(void) {    
    
    timerFlag_1ms=1;
    mT2ClearIntFlag();    
}

// This timer goes off every 100ms
void __ISR(_TIMER_1_VECTOR, IPL2SOFT) Timer1Handler(void) {
    if (HBCounter % (TOGGLES_PER_SEC >> 1) == 0) {
        if (isInDarkMode == 0)
            FLIP_HEARTBEAT_LED();       
    }
    HBCounter++;
    if(secondCounter++>=10) {
        secondCounter=0;
        timerFlag_1sec=1;         
    }
    timerFlag_100ms=1;
    mT1ClearIntFlag();
}
