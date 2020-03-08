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
    OpenTimer2(T2_ON | T2_32BIT_MODE_ON | T2_SOURCE_INT | T2_PS_1_64, T2_TICK);
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_3);        
}

void ConfigurePWM(){
  
    ConfigurePWMTimer();
    // Turn off the module before configuring   
    OC1CON = 0x0000;
    // PWM mode without fault pin.
    OC1CONbits.OCM0=0;
    OC1CONbits.OCM1=1;
    OC1CONbits.OCM2=1;
    
    OC1CONbits.OCTSEL=0; // Use timer 2 as the clock source.
    OC1CONbits.OC32=1; //Use 16-bit compare.
    
    OC1CONbits.SIDL=0; //Continue when in idle mode.
    
    OC1CONbits.ON=0; //Turn off module.
        
    OC1R=PR2>>1;
    OC1RS=PR2>>1;
           
    T2CONSET=0x8000;
    
    ConfigIntOC1(OC_INT_PRIOR_4 | OC_INT_ON);
    
    OC1CONbits.ON=1; // Turn PWM on.
}

// Based on the given prescaler the max frequency is
// 2kHz with duty cycle >= 1%.
void SetOptoParameters(unsigned int freq,unsigned int dc){      
    float tmp, tmp2;
    
    if(freq>2000) freq=2000;
    if(dc>100) dc=100;
    
    tmp=(float)(GetPeripheralClock()/PRESCALE/(float)freq);    
        
    tmp2=tmp*((float)dc/100.0);
            
    dutyCycle=dc;
    hertz=freq;
    
    PR2=(unsigned int)tmp;
    OC1RS= (unsigned int)tmp2;        
}




void inline SetOptoState(unsigned char os){
  OptoState=os;
}

void SetDC(unsigned int dc) {
  SetOptoParameters(hertz, dc);
}

void SetHertz(unsigned int hz) {
  SetOptoParameters(hz, dutyCycle);
}


void inline Opto_On() {
  isOptoOn=1;
  LATESET =OptoState & 0x0F;
}

void inline Opto_Off() {
  isOptoOn=0;
  
  LATECLR = 0x0F;
  
  LATESET =OptoState & (IsLEDConstant.ledField &0x0F);
  
}

void ConfigureOpto(void) {
  LEDSTRING1_TRIS = 0;
  LEDSTRING2_TRIS = 0;
  LEDSTRING3_TRIS = 0;
  LEDSTRING4_TRIS = 0;
  //Default is LED strings 1 and 2 are pulsed.
  // Strings 3 and 4 are not.
  IsLEDConstant.ledField=0x0C;
  OptoState = 0x00;
  Opto_Off();
  
  ConfigurePWM();
  
  SetOptoParameters(40, 8);
 
}

void GetOptoStatus(unsigned char* status){
    status[2]=OptoState;
    status[4]=hertz>>8;
    status[5]=hertz & 0xFF;
    status[6]=dutyCycle>>8;
    status[7]=dutyCycle & 0xFF;
}



void inline ProcessOptoStep(){
    // TODO: Try to eliminate this by activating the interrupts.
    if(PWM_PORT==1)
        Opto_On();
    else
        Opto_Off();
}

// This interrupt is not currently active
void __ISR(_TIMER_2_VECTOR, IPL3SOFT) Timer2Handler(void) {
    //PORTEINV = 0x0002;   
    Opto_On();
    mT2ClearIntFlag();
}
// This interrupt is not currently active
void __ISR(_OUTPUT_COMPARE_1_VECTOR, IPL4SOFT) OC1Handler(void) {
    //PORTEINV = 0x0001;
    Opto_Off();
    mOC1ClearIntFlag();
}