#include "GlobalIncludes.h"

int LEDDecayCounter[NUMLEDS];
int LEDDelayCounter[NUMLEDS];
int LEDMaxTimeOnCounter[NUMLEDS];

unsigned int LEDDecayValues[NUMLEDS];
unsigned int LEDDelayValues[NUMLEDS];
unsigned int LEDMaxTimeOnValues[NUMLEDS];

LEDFLAGS IsLEDOn;
LEDFLAGS IsLEDConstant;
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

struct FullProgram theProgram;

void inline SetLEDOn(unsigned char led){    
    // Set current and linked LEDs here.  
    IsLEDOn.ledField |= LEDLinkFlags[led].ledField; 
}

void ClearLEDLinkFlags(){
    int i;
    for(i=0;i<NUMLEDS;i++){
        LEDLinkFlags[i].ledField=(1<<i);
    }    
}

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

void InitializeLEDControl(unsigned int decayval,unsigned int delayval,unsigned int maxtimeval) {    
    int i; 
    for(i=0;i<NUMLEDS;i++)
        LEDThresholdValues[i]=-1;   
   
    IsLEDConstant.ledField=0x00;
        
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

// For the PIC32 Arena Board V2, we will define three threshold values.
// 0 = LED Off
// 1= LED On and Pulsed
// 2 = LED on and Constant
void SetLEDThresholds(int *thresh){
    int i;
    IsLEDConstant.ledField=0x00;
    for(i=0;i<NUMLEDS;i++){
        if(thresh[i]==0)
            LEDThresholdValues[i]=-1;
        else if(thresh[i]==1)
            LEDThresholdValues[i]=0;
        else if(thresh[i]==2){
            LEDThresholdValues[i]=0;
            IsLEDConstant.ledField |= (1<<i);
        }
    }
}



//void inline SetLEDOff(unsigned char led){
//    IsLEDOn.ledField &= ~(1<<led);
//}



void StepLEDControl() {
    unsigned char i; 
    unsigned char os;
    // We now start by assuming everyone is off.
    IsLEDOn.ledField=0;
    for (i = 0; i < NUMLEDS; i++){               
        // LED Update only sets those as on.
        LEDUpdateFunction(i);        
    }
    os = IsLEDOn.ledField & 0x0F;
    if(theProgram.Steps[theProgram.CurrentStep].ActiveTriggers != 0 && IR_TRIGGER_PORT==0){
        os =os & (!theProgram.Steps[theProgram.CurrentStep].ActiveTriggers);
    }
    /*
    if(theProgram.Steps[theProgram.CurrentStep].ActiveTriggers & 0x01) {
        if(IR_TRIGGER_PORT==0){
            os = os & 0x0E;
        }        
    }
    if(theProgram.Steps[theProgram.CurrentStep].ActiveTriggers & 0x02) {
        if(IR_TRIGGER_PORT==0){
            os = os & 0x0D;
        }        
    }
    if(theProgram.Steps[theProgram.CurrentStep].ActiveTriggers & 0x04) {
        if(TRIGGER_PORT==0){
            os = os & 0x0B;
        }        
    }
    if(theProgram.Steps[theProgram.CurrentStep].ActiveTriggers & 0x08) {
        if(TRIGGER_PORT==0){
            os = os & 0x07;
        }        
    }
    */
    SetOptoState(os);    
}


////////////////////////////////////////////////////
// NOT CURRENTLY IN USE
////////////////////////////////////////////////////


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

