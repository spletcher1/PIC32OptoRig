
#include "GlobalIncludes.h"


int LEDDecayCounter[NUMLEDS];
int LEDDelayCounter[NUMLEDS];
int LEDMaxTimeOnCounter[NUMLEDS];

unsigned int LEDDecayValues[NUMLEDS];
unsigned int LEDDelayValues[NUMLEDS];
unsigned int LEDMaxTimeOnValues[NUMLEDS];

LEDFLAGS IsLEDOn;
LEDFLAGS LEDLinkFlags[NUMLEDS];
unsigned char IsOverMaxTimeOn[NUMLEDS];

extern int CurrentValues[NUMLEDS];
int LEDThresholdValues[NUMLEDS];

void (*LEDUpdateFunction)(unsigned char);
void SetLEDParams(unsigned int decayval, unsigned int delayval, unsigned int maxtimeonval);

// For now we will force each well on a DFM to have the same delay, decay, maxtime values
unsigned int currentDelay;
unsigned int currentDecay;
unsigned int currentMaxTimeOn;


void ClearLEDLinkFlags(){
    int i;
    for(i=0;i<NUMLEDS;i++){
        LEDLinkFlags[i].ledField=(1<<i);
    }    
}

// Link defs will be an array of length 12, one for each LED
// LEDs with the same number will be linked.  Max number is 12.
void SetLEDLinkFlags(unsigned char *linkdefs){
    int i,j;
    char currentLinkNumber;
    for(i=0;i<NUMLEDS;i++){
        currentLinkNumber=linkdefs[i];
        LEDLinkFlags[i].ledField = 0;
        for(j=0;j<NUMLEDS;j++){
            if(linkdefs[j]==currentLinkNumber){
                LEDLinkFlags[i].ledField |= (1<<j);    
            }
        }
    }
}

void InitializeLEDControl(unsigned int decayval,unsigned int delayval,unsigned int maxtimeval) {    
    int i; 
    for(i=0;i<NUMLEDS;i++)
        LEDThresholdValues[i]=-1;       
    ClearLEDLinkFlags();    
    SetLEDParams(decayval,delayval,maxtimeval);
}

void ClearLEDThresholds(){
    int i;
    for(i=0;i<NUMLEDS;i++)
        LEDThresholdValues[i]=-1;
}

void TestLEDThresholds(){
    int i;
    for(i=0;i<NUMLEDS;i++)
        LEDThresholdValues[i]=100*128;
}

void SetLEDThresholds(int *thresh){
    int i;
    for(i=0;i<NUMLEDS;i++)
        LEDThresholdValues[i]=thresh[i]*128;
}

void inline SetLEDOn(unsigned char led){    
    // Set current and linked LEDs here.  
    IsLEDOn.ledField |= LEDLinkFlags[led].ledField; 
}

//void inline SetLEDOff(unsigned char led){
//    IsLEDOn.ledField &= ~(1<<led);
//}

void UpdateLEDSimplest(unsigned char led) {
    if (LEDThresholdValues[led] == -1) { 
        return;
    }  
    if (LEDThresholdValues[led] == 0) {
        SetLEDOn(led);
        return;
    }
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        SetLEDOn(led);
        return;
    } 
}

void UpdateLEDWithDecay(unsigned char led) {
    if (LEDThresholdValues[led] == -1) {    
        return;
    }
    if (LEDThresholdValues[led] == 0) {
        SetLEDOn(led);
        return;
    }   
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        SetLEDOn(led);
        LEDDecayCounter[led] = LEDDecayValues[led];
    } else if (LEDDecayValues[led] > 0 && LEDDecayCounter[led] > 0) {
        LEDDecayCounter[led]--;
        SetLEDOn(led);
    } 
}

void UpdateLEDWithDecayAndMaxTime(unsigned char led) {
    if (LEDThresholdValues[led] == -1) {    
        return;
    }
    if (LEDThresholdValues[led] == 0) {
        SetLEDOn(led);
        return;
    } 
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        if (LEDDecayValues[led] > 0) {
            LEDDecayCounter[led] = LEDDecayValues[led];
        }
        if (LEDMaxTimeOnValues[led] > 0) {
            if (LEDMaxTimeOnCounter[led]-- > 0) {
                SetLEDOn(led);
            } 
        } else {
            SetLEDOn(led);
        }

    } else {
        if (LEDDecayValues[led] > 0) {
            if (LEDDecayCounter[led]-- > 0) {
                if (LEDMaxTimeOnValues[led] > 0) {
                    if (LEDMaxTimeOnCounter[led]-- > 0) {
                        SetLEDOn(led);
                    } 
                } else {
                    SetLEDOn(led);
                }
            } else {                
                LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
            }
        } else {            
            LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
        }
    }
}

// This function is ONLY used when delay > 0.
// We ignore maxtime on for this because the lights on time
// is determined uniquely by the decay time.
// We also require a decay > 0.

void UpdateLEDWithDelay(unsigned char led) {
     if (LEDThresholdValues[led] == -1) {    
        return;
    }
    if (LEDThresholdValues[led] == 0) {
        SetLEDOn(led);
        return;
    }
   
    if (CurrentValues[led] > LEDThresholdValues[led]) {        
        LEDDelayCounter[led] = LEDDelayValues[led];
        LEDDecayCounter[led] = LEDDecayValues[led];
    } 
    else {
        if (LEDDelayCounter[led]-- > 0) {            
            LEDDecayCounter[led] = LEDDecayValues[led];
        } 
        else {
            if (LEDDecayCounter[led]-- >= 0) {
                SetLEDOn(led);
            } else {                
                LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
            }
        }
    }
}



// Note that one has to be careful to set the parameters in the right manner
// or constraints may result in some unintended changes.
void SetLEDParams(unsigned int decayval, unsigned int delayval, unsigned int maxtimeonval) {
    int i;
    if(delayval>0){
        maxtimeonval=0;
        if(decayval<=0)
            decayval=1000;
    }        
    for (i = 0; i < NUMLEDS; i++) {
        LEDDecayValues[i] = decayval;
        LEDDelayValues[i] = delayval;
        LEDMaxTimeOnValues[i] = maxtimeonval;

        LEDDecayCounter[i] = 0;
        if(delayval>0)
            LEDDelayCounter[i] = 0;
        else
            LEDDelayCounter[i] = LEDDelayValues[i];
        
        LEDMaxTimeOnCounter[i] = LEDMaxTimeOnValues[i];        
    }
    IsLEDOn.ledField=0;
    if (delayval == 0 && maxtimeonval == 0 && decayval == 0) {
        LEDUpdateFunction = &UpdateLEDSimplest;
    } else if (delayval == 0 && maxtimeonval == 0) {
        LEDUpdateFunction = &UpdateLEDWithDecay;
    } else if (delayval == 0) {
        LEDUpdateFunction = &UpdateLEDWithDecayAndMaxTime;
    } else {
        LEDUpdateFunction = &UpdateLEDWithDelay;
    }
    currentDelay = delayval;
    currentDecay = decayval;
    currentMaxTimeOn =maxtimeonval; 
}

void SetDelay(unsigned int delay){
    SetLEDParams(currentDecay,delay,currentMaxTimeOn);
}

void SetDecay(unsigned int decay){
    SetLEDParams(decay,currentDelay,currentMaxTimeOn);
}

void SetMaxTimeOn(unsigned int maxTime){
    SetLEDParams(currentDecay,currentDelay,maxTime);
}


void SetCurrentOptoState() {
    int OptoState;
    OptoState=0;
    if (IsLEDOn.bits.LED1)
        OptoState = 0x01;
    if (IsLEDOn.bits.LED2)
        OptoState = 0x02;
    if (IsLEDOn.bits.LED3)
        OptoState |= 0x04;
    if (IsLEDOn.bits.LED4)
        OptoState |= 0x08;  
    SetOptoState(OptoState);
}



void StepLEDControl() {
    unsigned char i;
    // We now start by assuming everyone is off.
    IsLEDOn.ledField=0;
    for (i = 0; i < NUMLEDS; i++){               
        // LED Update only sets those as on.
        LEDUpdateFunction(i);        
    }
    SetCurrentOptoState();
}
