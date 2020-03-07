#include "GlobalIncludes.h"

unsigned int volatile optoOnCounter;
unsigned int volatile optoOffCounter;
unsigned int volatile pulseWidth_ms;
unsigned int volatile hertz;
int volatile optoOffThreshold;
unsigned char volatile OptoState;
unsigned char volatile isOptoOn;

LEDFLAGS IsLEDPulsed;

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
    
    OC1CONbits.ON=1; // Turn PWM on.
}

// Based on the given prescaler the max frequency is
// 2kHz with duty cycle >= 1%.
void SetPWM(unsigned int freq,unsigned int dc){      
    float tmp, tmp2;
    
    if(freq>2000) freq=2000;
    if(dc>100) dc=100;
    
    tmp=(float)(GetPeripheralClock()/PRESCALE/(float)freq);    
        
    tmp2=tmp*((float)dc/100.0);
            
    PR2=(unsigned int)tmp;
    OC1RS= (unsigned int)tmp2;        
}

void __ISR(_TIMER_2_VECTOR, IPL3SOFT) Timer2Handler(void) {
    PORTEINV = 0x0002;    
    mT2ClearIntFlag();

}

void __ISR(_OUTPUT_COMPARE_1_VECTOR, IPL4SOFT) OC1Handler(void) {
    PORTEINV = 0x0001;
    mOC1ClearIntFlag();
}


void inline SetOptoState(unsigned char os){
  OptoState=os;
}


void SetOptoParameters(unsigned int hz, unsigned int pw) {
    hertz = hz;
    if (hertz < 1) hertz = 1;
    if (hertz > 500) hertz = 500;
    pulseWidth_ms = pw;
    optoOffThreshold = (unsigned int) (1000 / hertz) - (pulseWidth_ms);
    // If the parameters don't work, then accept the hertz and
    // adjust the pulse width to be maximum given that hertz.
    if (optoOffThreshold < 0) {
        pulseWidth_ms = 1000 / (hertz * 2);
        optoOffThreshold = (unsigned int) (1000 / hertz) - (pulseWidth_ms);
    }
    optoOnCounter = optoOffCounter = 0;
}


void SetPulseWidth_ms(unsigned int pw) {
  SetOptoParameters(hertz, pw);
}

void SetHertz(unsigned int hz) {
  SetOptoParameters(hz, pulseWidth_ms);
}


void inline Opto_On() {
  isOptoOn=1;
  if (OptoState & 0x01)
    LEDSTRING1_ON();
  if (OptoState & 0x02)
    LEDSTRING2_ON();
  if (OptoState & 0x04)
    LEDSTRING3_ON();
  if (OptoState & 0x08)
    LEDSTRING4_ON();
}

void inline Opto_Off() {
  isOptoOn=0;
  ALLLED_OFF();
  if((OptoState & 0x01) & IsLEDPulsed.bits.LED1)
      LEDSTRING1_ON();
  if((OptoState & 0x02) & IsLEDPulsed.bits.LED2)
      LEDSTRING2_ON();
  if((OptoState & 0x04) & IsLEDPulsed.bits.LED3)
      LEDSTRING3_ON();
  if((OptoState & 0x08) & IsLEDPulsed.bits.LED4)
      LEDSTRING4_ON();
  
}

void ConfigureOpto(void) {
  LEDSTRING1_TRIS = 0;
  LEDSTRING2_TRIS = 0;
  LEDSTRING3_TRIS = 0;
  LEDSTRING4_TRIS = 0;
  Opto_Off();
  SetOptoParameters(40, 8);
  //Default is LED strings 1 and 2 are pulsed.
  // Strings 3 and 4 are not.
  IsLEDPulsed.ledField=0x03;
  //SetOptoParameters(200 , 1);
  OptoState = 0x00;

}

void GetOptoStatus(unsigned char* status){
    status[2]=OptoState;
    status[4]=hertz>>8;
    status[5]=hertz & 0xFF;
    status[6]=pulseWidth_ms>>8;
    status[7]=pulseWidth_ms & 0xFF;
}

void ToggleOpto(){
    if(OptoState!=0) {
        OptoState=0x00;
    }
    else {
        OptoState=0x3F;
    }
}

void inline ProcessOptoStep(){
  if (isOptoOn == 0) {// All lights off
        optoOffCounter++;
        if (optoOffCounter >= optoOffThreshold) {
            optoOffCounter = 0;
            Opto_On();
        }
  }
  else if (isOptoOn == 1) { //Column 1 lights attended to
        optoOnCounter++;
        if (optoOnCounter >= pulseWidth_ms) {
            Opto_Off();
            optoOnCounter = 0;
        }
  }
}