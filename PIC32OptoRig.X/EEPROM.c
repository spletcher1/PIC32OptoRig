#include "GlobalIncludes.h"

extern struct FullProgram theProgram;
extern unsigned char isTimeCallAnswered;
extern errorFlags_t ErrorsRegister;

unsigned char IsDefaultProgramSaved_24LC256(){
 unsigned char result;
 ReadByte_24LC256(1,&result);
 if(result=='Y')
   return 1;
 else
   return 0;
}

void SaveProgram_24LC256(){
  unsigned int i, counter;
  unsigned short lights,a,b,c,d,triggers;
  a = theProgram.NumSteps & 0xFF;
  WriteByte_24LC256(1, 'Y',0);
  WriteByte_24LC256(2, a,1);
  
  if(theProgram.programType == LINEAR)
    WriteByte_24LC256(3, 0x01,1);  
  else if(theProgram.programType == LOOPING)
    WriteByte_24LC256(3, 0x02,1);
  else if(theProgram.programType == CIRCADIAN)
    WriteByte_24LC256(3, 0x03,1);
  else
    WriteByte_24LC256(3, 0x02,1);
    
  WriteByte_24LC256(4,theProgram.startTime.seconds,1);
  WriteByte_24LC256(5,theProgram.startTime.minutes,1);
  WriteByte_24LC256(6,theProgram.startTime.hours,1);
  WriteByte_24LC256(7,theProgram.startTime.monthday,1);
  WriteByte_24LC256(8,theProgram.startTime.month,1);
  WriteByte_24LC256(9,theProgram.startTime.year,1);
  
  counter=10;
  for(i=0;i<theProgram.NumSteps;i++){
    lights=theProgram.Steps[i].LED1Threshold;
    WriteByte_24LC256(counter++,lights,1);
    lights=theProgram.Steps[i].LED2Threshold;
    WriteByte_24LC256(counter++,lights,1);
    lights=theProgram.Steps[i].LED3Threshold;
    WriteByte_24LC256(counter++,lights,1);
    lights=theProgram.Steps[i].LED4Threshold;
    WriteByte_24LC256(counter++,lights,1);
    a = theProgram.Steps[i].Frequency>>8;
    a = a & 0x00FF;
    b = theProgram.Steps[i].Frequency & 0x00FF;
    WriteByte_24LC256(counter++,a,1);
    WriteByte_24LC256(counter++,b,1);
    a = theProgram.Steps[i].DutyCycle>>8;
    a = a & 0x00FF;
    b = theProgram.Steps[i].DutyCycle & 0x00FF;
    WriteByte_24LC256(counter++,a,1);
    WriteByte_24LC256(counter++,b,1);
    triggers=theProgram.Steps[i].ActiveTriggers;
    WriteByte_24LC256(counter++,triggers,1);
    a = theProgram.Steps[i].DurationSeconds>>24;
    a = a & 0x00FF;
    b = theProgram.Steps[i].DurationSeconds>>16;
    b = b & 0x00FF;
    c = theProgram.Steps[i].DurationSeconds>>8;
    c = c & 0x00FF;
    d = theProgram.Steps[i].DurationSeconds & 0xFF;
    WriteByte_24LC256(counter++,a,1);
    WriteByte_24LC256(counter++,b,1);
    WriteByte_24LC256(counter++,c,1);
    WriteByte_24LC256(counter++,d,1);
  }
}

void LoadProgram_24LC256(){
  unsigned int i, counter;
  unsigned char led1,led2,led3,led4,a,b,c,d,steps,tmp,triggers;
  unsigned int freq,pw;
  unsigned long int duration;
  ClearProgram();
  ReadByte_24LC256(2,&steps);
  ReadByte_24LC256(3,&tmp);
  
  if(tmp == 1)
    theProgram.programType=LINEAR;
  else if(tmp == 2)
    theProgram.programType=LOOPING;
  else if(tmp == 3)
    theProgram.programType=CIRCADIAN;
  else
    theProgram.programType=LOOPING;

  ReadByte_24LC256(4,&theProgram.startTime.seconds);
  ReadByte_24LC256(5,&theProgram.startTime.minutes);
  ReadByte_24LC256(6,&theProgram.startTime.hours);
  ReadByte_24LC256(7,&theProgram.startTime.monthday);
  ReadByte_24LC256(8,&theProgram.startTime.month);
  ReadByte_24LC256(9,&theProgram.startTime.year );
  
  counter=10;
  for(i=0;i<steps;i++){
    ReadByte_24LC256(counter++,&led1);
    ReadByte_24LC256(counter++,&led2);
    ReadByte_24LC256(counter++,&led3);
    ReadByte_24LC256(counter++,&led4);
    ReadByte_24LC256(counter++,&a);
    ReadByte_24LC256(counter++,&b);
    freq=(a<<8)+b;
    ReadByte_24LC256(counter++,&a);
    ReadByte_24LC256(counter++,&b);
    pw=(a<<8)+b;
    ReadByte_24LC256(counter++,&triggers);
    ReadByte_24LC256(counter++,&a);
    ReadByte_24LC256(counter++,&b);
    ReadByte_24LC256(counter++,&c);
    ReadByte_24LC256(counter++,&d);
    duration = (a << 24) + (b << 16) + (c << 8) + d;
    AddProgramStep(led1,led2,led3,led4,freq,pw,triggers,duration);
  }
  UpdateProgram();
  theProgram.programStatus = LOADED;
}

// Memory address will be from 0x0000 to 0x7FFF


void WriteString_24LC256(unsigned int address, char* wData) {
  char i;
  i=0;
  while(wData[i]!=0){
     WriteByte_24LC256(address++,wData[i++],1);     
  }
  WriteByte_24LC256(address,0,0);
}

//---------------Read string from EEPROM
void ReadString_24LC256(unsigned int address, char* rData) {
  char tmp,i;
  i=0;
  tmp = 255;
  while(tmp!=0){
    ReadByte_24LC256(address++,&tmp);
    rData[i++]=tmp;
  }
}