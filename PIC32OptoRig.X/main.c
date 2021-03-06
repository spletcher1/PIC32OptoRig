/******************************************************************************/
/*  Files to Include                                                          */
/******************************************************************************/

#include "GlobalIncludes.h"         /* System funct/params, like osc/periph config    */
#include "SP_ConfigP32.h"           /* User funct/params, such as InitApp             */

extern unsigned char volatile updateTrigger500us;
extern unsigned char volatile updateTrigger1ms;
extern unsigned char volatile updateTrigger1sec;
extern unsigned char signalButton1Pressed;
extern unsigned char signalButton2Pressed;
extern unsigned char packetReceived;

//extern rtc_time_t local_time;

errorFlags_t ErrorsRegister;

// Eventually when we implement closed loop, we may have to
// use these CurrentValues in the LEDControl.c
int CurrentValues[NUMLEDS];

void InitApp(void) {        
    Startup();
    InitializeBoard(); 
     
    ConfigureUpdateTimer();        
    ConfigureUART1();              
    ConfigureButtons();
    ConfigureI2C5();    
    DelayMs(500);
    InitRTC();    
    ClearProgram();

    DelayMs(500);

    ConfigureOpto();
       
    ErrorsRegister.byte = 0;

    if (IsDefaultProgramSaved_24LC256())
        LoadProgram_24LC256();    
    else
        ConfigureSimpleProgram(5, 10);

    //ConfigureSimpleProgram(20,20);
    //SaveProgram_24LC256();   

    // Becareful when setting.  Increase the above delay to 5000ms.
    // I think the controller resets prior to programming and
    // can reset the clock a second time when programming with the
    // final version.
    //SetRTC();
    //Delay_ms(1000);
    //SendString("Done");
    //while(1);
}

void main(void) {
    InitApp();
    HB_OFF();             
    StageProgram();     
    while (1) {
        if (updateTrigger500us) {
            StepLEDControl();
            ProcessOptoStep();
            updateTrigger500us = 0;
        }
        if (updateTrigger1ms) {
            ProcessButtonStep();
            ProcessProgramStep();
            updateTrigger1ms = 0;
        }
        if (updateTrigger1sec) {
            //StepRTC(); Not currently used.
            updateTrigger1sec = 0;
        }
        if (signalButton1Pressed) {
            ProcessButton1Press();
        }
        if (signalButton2Pressed) {
            ProcessButton2Press();
        }
        if (packetReceived) {
            ProcessCommandBuffer();
            packetReceived = 0;
        }
    }
}