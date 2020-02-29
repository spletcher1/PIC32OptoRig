#include "GlobalIncludes.h"


#define OPTOPRESCALE               256
#define OPTOTOGGLES_PER_SEC        1000
#define OPTO_TICK               (GetPeripheralClock()/OPTOPRESCALE/OPTOTOGGLES_PER_SEC)

enum OptoTimerState {
    COL1_OLDPORT_AND_ALL_NEWPORT,
    COL2_OLDPORT,
    OFF,
} currentOptoTimerState;

unsigned char volatile OptoState1;
unsigned char volatile OptoState2;
int opto_msOFF;
int volatile optoOnCounter;
int volatile optoOffCounter;
unsigned int volatile pulseWidth_ms;
unsigned int volatile hertz;
unsigned char volatile timerFlag_1ms;

// This parameter (UsingNewPortOnly) will be defined at startup and indicated by an LED, at the same time DFMID is determined.  
// To change it, the user will need to reset after the change.
extern unsigned char usingNewPortOnly;

void ConfigureOptoTimer(void);

void ConfigureOpto() {

    // Definitions for legacy port
    TRISFCLR = 0x40;
    TRISGCLR = 0x1C0;
    TRISDCLR = 0x1D0;

    ODCFSET = 0x40;
    ODCGSET = 0x1C0;
    ODCDSET = 0x1D0;

    LATFCLR = 0x40;
    LATGCLR = 0x1C0;
    LATDCLR = 0x1D0;

    // Definitions for new port
    // Note that RD4, RD6, RD7, RD8, and RD9 are present on both ports.
    // Rewriting these bits won't hurt...
    TRISDCLR = 0x0FFF;
    ODCDSET = 0xFFF;
    LATDCLR = 0xFFF;

    OptoState1 = OptoState2 = 0;    
    ConfigureOptoTimer();
}

void Col1_Opto_On() {    
    COL1ON();
    if (OptoState1 & 0x01)
        ROW1ON();
    if (OptoState1 & 0x02)
        ROW2ON();
    if (OptoState1 & 0x04)
        ROW3ON();
    if (OptoState1 & 0x08)
        ROW4ON();
    if (OptoState1 & 0x10) // need to change this back to 0x40 when new board is implemented.
        ROW5ON();
    if (OptoState1 & 0x20)
        ROW6ON();
}

void Col2_Opto_On() {    
    COL2ON();
    if (OptoState2 & 0x01)
        ROW1ON();
    if (OptoState2 & 0x02)
        ROW2ON();
    if (OptoState2 & 0x04)
        ROW3ON();
    if (OptoState2 & 0x08)
        ROW4ON();
    if (OptoState2 & 0x10) // need to change this back to 0x40 when new board is implemented.
        ROW5ON();
    if (OptoState2 & 0x20)
        ROW6ON();
}

void NewPort_Opto_On() {   
    LATDSET = ((OptoState2 & 0x3F) << 6)+ (OptoState1 & 0x3F); // The and here is to avoid undefined Optostate bits.    
}

void inline Opto_Off() {   
    // Legacy port definitions are combined with new port
    // here to avoid the cost of writing to the D port twice.
    LATFCLR = 0x40;
    LATGCLR = 0x1C0;
    LATDCLR = 0xFFF;
}

void SetOptoParameters(unsigned int hz, unsigned int pw) {
    // Accept hertz up to 500.  If it is greater than 250, the legacy
    // port will not function
    hertz = hz;
    if (hertz < 1) hertz = 1;
    if(usingNewPortOnly)
        if(hertz>500) hertz = 500;
    else 
        if(hertz>250) hertz=250;
    pulseWidth_ms = pw;
    opto_msOFF = (unsigned int) (1000 / hertz) - (pulseWidth_ms);
    // If the parameters don't work, then accept the hertz and
    // adjust the pulse width to 50% duty cycle given that hertz.
    if (opto_msOFF < 0) {
        pulseWidth_ms = 1000 / (hertz * 2);
        opto_msOFF = (unsigned int) (1000 / hertz) - (pulseWidth_ms);
    }
    optoOnCounter = optoOffCounter = 0;   
    currentOptoTimerState = OFF;
}

void SetPulseWidth_ms(unsigned int pw) {
    SetOptoParameters(hertz, pw);
}

void SetOptoState(unsigned char os1, unsigned char os2) {
    OptoState1 = os1;
    OptoState2 = os2;
}

void SetHertz(unsigned int hz) {
    SetOptoParameters(hz, pulseWidth_ms);
}

void ConfigureOptoTimer(void) {
    // This timer is set to go off every 1ms.    
    //SetHertz(40);
    //SetHertz(100);
    //Set101();
    SetOptoParameters(40, 8);
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_256, OPTO_TICK);
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_6);
    currentOptoTimerState = OFF;
    OptoState1=OptoState2=0x00;
    timerFlag_1ms = 0;
}

// When scoped, this Timer goes off properly at 1ms.
// The time it takes to complete the interrupt is 500ns.

void __ISR(_TIMER_2_VECTOR, IPL6AUTO) Timer2Handler(void) {    
    if (currentOptoTimerState == OFF) {// All lights off
        optoOffCounter++;
        if (optoOffCounter >= opto_msOFF) {
            currentOptoTimerState = COL1_OLDPORT_AND_ALL_NEWPORT;
            optoOnCounter = 0;
            if (usingNewPortOnly) {
                NewPort_Opto_On();
            } else {
                Col1_Opto_On();                
            }
        }
    } else if (currentOptoTimerState == COL1_OLDPORT_AND_ALL_NEWPORT) { //Column 1 lights attended to
        optoOnCounter++;
        if (optoOnCounter >= pulseWidth_ms) {
            Opto_Off();
            optoOffCounter=0;
            if(!usingNewPortOnly) {
                Col2_Opto_On();
                currentOptoTimerState = COL2_OLDPORT;
                optoOnCounter = 0;
            }
        }
    } else if (currentOptoTimerState == COL2_OLDPORT) { //Column 2 lights being attended to
        optoOnCounter++;
        optoOffCounter++; // This is here because column 2 is not being flashed during the off time for the new port and column 1.
        if (optoOnCounter >= pulseWidth_ms) {
            Opto_Off();       
            currentOptoTimerState = OFF;
        }
    }
    timerFlag_1ms = 1;
    mT2ClearIntFlag();    
}