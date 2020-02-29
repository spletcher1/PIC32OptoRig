/******************************************************************************/
/*  Files to Include                                                          */
/******************************************************************************/

#include "GlobalIncludes.h"         /* System funct/params, like osc/periph config    */
#include "SP_ConfigP32.h"           /* User funct/params, such as InitApp             */

extern unsigned char volatile updateTrigger1ms;
extern unsigned char volatile updateTrigger500ms;
extern unsigned char volatile updateTrigger1sec;
extern unsigned char signalButton1Pressed;
extern unsigned char signalButton2Pressed;
extern unsigned char packetReceived;

//extern rtc_time_t local_time;

errorFlags_t ErrorsRegister;

void InitApp(void){
  
  Startup();  
  InitializeBoard();
  ConfigureUpdateTimer();
  ConfigureUART2();
  ConfigureButtons();
  //Initialize_24LC256();// 500ms delay
  //InitRTC();
  //ClearProgram();

  //Delay_ms(1000);
  
  //ConfigureOpto();
  //ConfigureMainTimer();
  //ErrorsRegister=0;
  
  //if(IsDefaultProgramSaved_24LC256())
  //  LoadProgram_24LC256();
  //else
  //  ConfigureSimpleProgram(30,30);

  //ConfigureSimpleProgram(5,10);
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
  /*
  StageProgram();
  while (1) {
    if(updateTrigger1ms){
      ProcessButtonStep();
      ProcessProgramStep();
      updateTrigger1ms=0;
    }
    if(updateTrigger1sec){
      //StepRTC();
      updateTrigger1sec=0;
    }
    if(signalButton1Pressed){
      ProcessButton1Press();
    }
    if(signalButton2Pressed){
      ProcessButton2Press();
    }
    if(packetReceived){
     ProcessCommandBuffer();
     packetReceived=0;
    }
  }
  */
}