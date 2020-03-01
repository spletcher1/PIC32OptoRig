#include "GlobalIncludes.h"

extern struct FullProgram theProgram;
extern unsigned char isTimeCallAnswered;
extern errorFlags_t ErrorsRegister;

unsigned char IsDefaultProgramSaved_24LC256(){
 unsigned char result;
 result=ReadByte_24LC256(1);
 if(result=='Y')
   return 1;
 else
   return 0;
}

void SaveProgram_24LC256(){
  unsigned int i, counter;
  unsigned short lights,a,b,c,d,triggers;
  a = theProgram.NumSteps & 0xFF;
  WriteByte_24LC256(1, 'Y');
  WriteByte_24LC256(2, a);
  
  if(theProgram.programType == LINEAR)
    WriteByte_24LC256(3, 0x01);  
  else if(theProgram.programType == LOOPING)
    WriteByte_24LC256(3, 0x02);
  else if(theProgram.programType == CIRCADIAN)
    WriteByte_24LC256(3, 0x03);
  else
    WriteByte_24LC256(3, 0x02);
    
  WriteByte_24LC256(4,theProgram.startTime.seconds);
  WriteByte_24LC256(5,theProgram.startTime.minutes);
  WriteByte_24LC256(6,theProgram.startTime.hours);
  WriteByte_24LC256(7,theProgram.startTime.monthday);
  WriteByte_24LC256(8,theProgram.startTime.month);
  WriteByte_24LC256(9,theProgram.startTime.year);
  
  counter=10;
  for(i=0;i<theProgram.NumSteps;i++){
    lights=theProgram.Steps[i].IsLightOn;
    WriteByte_24LC256(counter++,lights);
    a = theProgram.Steps[i].Frequency>>8;
    a = a & 0x00FF;
    b = theProgram.Steps[i].Frequency & 0x00FF;
    WriteByte_24LC256(counter++,a);
    WriteByte_24LC256(counter++,b);
    a = theProgram.Steps[i].PulseWidth>>8;
    a = a & 0x00FF;
    b = theProgram.Steps[i].PulseWidth & 0x00FF;
    WriteByte_24LC256(counter++,a);
    WriteByte_24LC256(counter++,b);
    triggers=theProgram.Steps[i].ActiveTriggers;
    WriteByte_24LC256(counter++,triggers);
    a = theProgram.Steps[i].DurationSeconds>>24;
    a = a & 0x00FF;
    b = theProgram.Steps[i].DurationSeconds>>16;
    b = b & 0x00FF;
    c = theProgram.Steps[i].DurationSeconds>>8;
    c = c & 0x00FF;
    d = theProgram.Steps[i].DurationSeconds & 0xFF;
    WriteByte_24LC256(counter++,a);
    WriteByte_24LC256(counter++,b);
    WriteByte_24LC256(counter++,c);
    WriteByte_24LC256(counter++,d);
  }
}

void LoadProgram_24LC256(){
  unsigned int i, counter;
  unsigned short lights,a,b,c,d,steps,tmp;
  unsigned int freq,pw,triggers;
  unsigned long int duration;
  ClearProgram();
  steps=ReadByte_24LC256(2);
  tmp = ReadByte_24LC256(3);
  
  if(tmp == 1)
    theProgram.programType=LINEAR;
  else if(tmp == 2)
    theProgram.programType=LOOPING;
  else if(tmp == 3)
    theProgram.programType=CIRCADIAN;
  else
    theProgram.programType=LOOPING;

  theProgram.startTime.seconds = ReadByte_24LC256(4);
  theProgram.startTime.minutes = ReadByte_24LC256(5);
  theProgram.startTime.hours = ReadByte_24LC256(6);
  theProgram.startTime.monthday = ReadByte_24LC256(7);
  theProgram.startTime.month = ReadByte_24LC256(8);
  theProgram.startTime.year = ReadByte_24LC256(9);
  
  counter=10;
  for(i=0;i<steps;i++){
    lights=ReadByte_24LC256(counter++);
    a=ReadByte_24LC256(counter++);
    b=ReadByte_24LC256(counter++);
    freq=(a<<8)+b;
    a=ReadByte_24LC256(counter++);
    b=ReadByte_24LC256(counter++);
    pw=(a<<8)+b;
    triggers=ReadByte_24LC256(counter++);
    a=ReadByte_24LC256(counter++);
    b=ReadByte_24LC256(counter++);
    c=ReadByte_24LC256(counter++);
    d=ReadByte_24LC256(counter++);
    duration = (a << 24) + (b << 16) + (c << 8) + d;
    AddProgramStep(lights,freq,pw,triggers,duration);
  }
  UpdateProgram();
  theProgram.programStatus = LOADED;
}

void I2C1_TimeoutCallback(char errorCode){
  if(isTimeCallAnswered==2) // this indicates that the RTC is trying to be initialized.
    isTimeCallAnswered=0; // Assume for now an I2C error only happens when the RTC is unplugged.
  I2CTimeoutError=1;
}

void Initialize_24LC256(){
  I2C1_Init(100000);
  I2C1_SetTimeoutCallback(1000, I2C1_TimeoutCallback);
  Delay_ms(500);
}

// Memory address will be from 0x0000 to 0x7FFF


void WriteString_24LC256(unsigned int address, char* wData) {
  char i;
  i=0;
  while(wData[i]!=0){
     WriteByte_24LC256(address++,wData[i++]);
  }
  WriteByte_24LC256(address,0);
}

//---------------Read string from EEPROM
void ReadString_24LC256(unsigned int address, char* rData) {
  char tmp,i;
  i=0;
  tmp = 255;
  while(tmp!=0){
    tmp = ReadByte_24LC256(address++);
    rData[i++]=tmp;
  }
}