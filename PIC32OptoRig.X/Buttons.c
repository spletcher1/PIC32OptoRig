#include "GlobalIncludes.h"
unsigned int button1Counter;
unsigned int button2Counter;

unsigned char signalButton1Pressed;
unsigned char signalButton2Pressed;

extern unsigned long volatile halfSecondCounter;

extern struct FullProgram theProgram;

void ConfigureButtons() {
    USERBUTTON1_TRIS = 0x01;
    USERBUTTON2_TRIS = 0x01;
    button1Counter = button2Counter = 0;
    signalButton1Pressed = signalButton2Pressed = 0;
}

void ProcessButtonStep() {
    if (USERBUTTON1_PORT == 0) {
        button1Counter++;
    } else {
        button1Counter = 0;
        signalButton1Pressed = 0;
    }
    if (USERBUTTON2_PORT == 0) {
        button2Counter++;
    } else {
        button2Counter = 0;
        signalButton2Pressed = 0;
    }

    if (button1Counter >= BUTTON1_DEBOUNCE_COUNTS) {
        if (!signalButton1Pressed) {
            ProcessButton1Press();
            signalButton1Pressed = 1;
        }
    }

    if (button2Counter >= BUTTON2_DEBOUNCE_COUNTS) {
        if (!signalButton2Pressed) {
            ProcessButton2Press();
            signalButton2Pressed = 1;
        }
    }

}

void ProcessButton1Press() {  
  StopProgram();
  signalButton1Pressed=0;        
}

void ProcessButton2Press() {
    StageProgram();
    signalButton2Pressed=0;        
}