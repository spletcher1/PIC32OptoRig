#include "GlobalIncludes.h"

unsigned int volatile optoOnCounter;
unsigned int volatile optoOffCounter;
unsigned int volatile dutyCycle;
unsigned int volatile hertz;
int volatile optoOffThreshold;
unsigned char volatile OptoState;
unsigned char volatile isOptoOn;

extern LEDFLAGS IsLEDConstant;

#define PRESCALE               64
#define TOGGLES_PER_SEC        1000
#define T2_TICK               (GetPeripheralClock()/PRESCALE/TOGGLES_PER_SEC)

void ConfigurePWMTimer(void) {
    OpenTimer2(T2_OFF | T2_32BIT_MODE_ON | T2_SOURCE_INT | T2_PS_1_64, T2_TICK);
}

void ConfigurePWM2(){
    unsigned int tmp;
    tmp = T2_TICK>>1;
    ConfigurePWMTimer();
    OpenOC1(OC_ON | OC_TIMER_MODE32 | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE ,tmp,tmp);    
    T2CONSET = 0x8000;
}

void ConfigurePWM() {    
    ConfigurePWMTimer();    
    // Turn off the module before configuring   
    OC1CON = 0x0000;
    
    PR2=156250;
    OC1R = PR2 >> 1;
    OC1RS = PR2 >> 1;
    
    // PWM mode without fault pin.
    OC1CON = 0x0006;    
    
    //OC1CONbits.OCM0 = 0;
    //OC1CONbits.OCM1 = 1;
    //OC1CONbits.OCM2 = 1;

    OC1CONbits.OCTSEL = 0; // Use timer 2 as the clock source.
    OC1CONbits.OC32 = 1; //Use 32-bit compare.

    OC1CONbits.SIDL = 0; //Continue when in idle mode.
    
    // An OC interrupt is not generated in PWM mode.
    // To use it this way I think I would have to configure
    // Dual pulse continuous mode.
    OC1CONbits.ON = 1; // Turn PWM on.
    T2CONSET = 0x8000;
   
}

// Based on the given prescaler the max frequency is
// 2kHz with duty cycle >= 1%.

void SetOptoParameters(unsigned int freq, unsigned int dc) {
    float tmp, tmp2;    
    if (freq > 2000) freq = 2000;
    if (dc > 100) dc = 100;

    tmp = (float) (GetPeripheralClock() / PRESCALE / (float) freq);  
    tmp2 = tmp * ((float) dc / 100.0);

    dutyCycle = dc;
    hertz = freq;
    
    // Need to reset timer here so we don't miss the new period and run forever.
    TMR2=TMR3=0;
    PR2 = (unsigned int) tmp;
    OC1RS = (unsigned int) tmp2;
}

void inline SetOptoState(unsigned char os) {
    OptoState = os;
}

void SetDC(unsigned int dc) {
    SetOptoParameters(hertz, dc);
}

void SetHertz(unsigned int hz) {
    SetOptoParameters(hz, dutyCycle);
}

// For now, the suppressor will only suppress LED1 and LED2
// Will assume the IR is controlled independently of Suppressor.

// Also the indicator only cares about LED1 and LED2 (the red ones))
void inline Opto_On() {
    isOptoOn = 1;
    if(SUPRESSOR_PORT)
        LATESET = OptoState & 0x0C;    
    else 
        LATESET = OptoState & 0x0F;
    if (PORTE & 0x03)
        INDICATOR_LAT = 1;
}

void inline Opto_Off() {
    isOptoOn = 0;

    LATECLR = 0x0F;
    INDICATOR_LAT = 0;
    if(SUPRESSOR_PORT)
        LATESET = OptoState & (IsLEDConstant.ledField & 0x0C);        
    else 
        LATESET = OptoState & (IsLEDConstant.ledField & 0x0F);

    if (PORTE & 0x03)
        INDICATOR_LAT = 1;
}

void ConfigureOpto(void) {
    LEDSTRING1_TRIS = 0;
    LEDSTRING2_TRIS = 0;
    LEDSTRING3_TRIS = 0;
    LEDSTRING4_TRIS = 0;
    //Default is LED strings 1 and 2 are pulsed.
    // Strings 3 and 4 are not.
    IsLEDConstant.ledField = 0x0C;
    OptoState = 0x00;
    Opto_Off();

    ConfigurePWM();
    InitializeLEDControl (0, 0, 0);
    SetOptoParameters(40, 50);
}

void GetOptoStatus(unsigned char* status) {
    status[2] = OptoState;
    status[4] = hertz >> 8;
    status[5] = hertz & 0xFF;
    status[6] = dutyCycle >> 8;
    status[7] = dutyCycle & 0xFF;
}



void inline ProcessOptoStep() {  
    if (PWM_PORT == 1)
        Opto_On();
    else
        Opto_Off();
}

