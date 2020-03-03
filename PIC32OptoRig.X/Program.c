#include "GlobalIncludes.h"

#define VALIDATIONSECONDS 30

#define BUFFERSIZES (MAXPROGRAMSTEPS*10)+5

struct FullProgram theProgram;
extern unsigned char cobsBuffer[BUFFERSIZES];
extern unsigned int cobsBufferLength;
unsigned char barray[BUFFERSIZES];

extern rtc_time_t local_time;
extern unsigned char isRTCInitialized;
extern unsigned char isTimeCallAnswered;
extern errorFlags_t ErrorsRegister;
int validationCounter;
unsigned int milliSecondCounter;


void SetCurrentStep(){
  unsigned int i;
  for(i=0;i<theProgram.NumSteps;i++) {
    if(theProgram.correctedSeconds <= theProgram.Steps[i].ElapsedDurationAtEnd) {
      theProgram.CurrentStep=i;
      return;
    }
  }
  // If you get here you must have gone beyond the end of a LINEAR
  // program, so indicate this and don't start it.
  theProgram.CurrentStep=theProgram.NumSteps;
}

unsigned char IsStartTimeZero(){
  // Just need to look at year and act if it is before, say 2010.
  if(theProgram.startTime.year<10)
    return 1;
  else
    return 0;
}

// if start time==0 at all parts, then starttime should be changed to current time.
void UpdateProgram(){
  int i;
  unsigned long elapsedSec;
  elapsedSec=0;
  theProgram.startEpochTime = time_date_to_epoch(&theProgram.startTime);
  for(i=0;i<theProgram.NumSteps;i++) {
    theProgram.Steps[i].ElapsedDurationAtEnd = elapsedSec + theProgram.Steps[i].DurationSeconds;
    elapsedSec += theProgram.Steps[i].DurationSeconds;
  }
  theProgram.totalProgramDuration = elapsedSec;
}

void AddProgramStep(unsigned int islighton, unsigned int freq, unsigned int pw, unsigned char trigger, unsigned long int duration){
  if(theProgram.NumSteps>=MAXPROGRAMSTEPS) {
    ErrorsRegister.bits.TooManyStepsError=1;
    return;
  }
  theProgram.Steps[theProgram.NumSteps].StepNumber=theProgram.NumSteps+1;
  theProgram.Steps[theProgram.NumSteps].IsLightOn=islighton;
  theProgram.Steps[theProgram.NumSteps].Frequency=freq;
  theProgram.Steps[theProgram.NumSteps].PulseWidth=pw;
  theProgram.Steps[theProgram.NumSteps].ActiveTriggers=trigger;
  theProgram.Steps[theProgram.NumSteps].DurationSeconds=duration;
  theProgram.NumSteps++;
}

void ConfigureSimpleProgram(unsigned long int secondsOn, unsigned long int secondsOff){
  ClearProgram();
  theProgram.programType = LOOPING;
  AddProgramStep(1,40,80,0,secondsOn);
  AddProgramStep(0,40,80,0,secondsOff);
  theProgram.startTime.seconds = 0;
  theProgram.startTime.minutes = 0;
  theProgram.startTime.hours =  0;
  theProgram.startTime.monthday =1;
  theProgram.startTime.month = 1;
  theProgram.startTime.year = 1;
  UpdateProgram();
  theProgram.programStatus = LOADED;
}

void LoadProgram(){
  ClearProgram();
  LoadProgram_24LC256();
  UpdateProgram();
  theProgram.programStatus=LOADED;
  milliSecondCounter=0;
}

void ValidateProgramTime() {
  if (isRTCInitialized == 0) return;
  isTimeCallAnswered=2;
  ReadTimeFromRTC(&local_time);
  if(isTimeCallAnswered==0)
    return;
  theProgram.ElapsedSecondsFromStart = time_date_to_epoch(&local_time) - theProgram.startEpochTime;
  if(theProgram.programType==LINEAR) {
    theProgram.correctedSeconds = theProgram.ElapsedSecondsFromStart;
  }
  else if (theProgram.programType == LOOPING) {
    theProgram.correctedSeconds = theProgram.ElapsedSecondsFromStart % theProgram.totalProgramDuration;
    }
  else if (theProgram.programType == CIRCADIAN) {
    theProgram.correctedSeconds = (unsigned long)(local_time.hours) * 3600L;
    theProgram.correctedSeconds +=  (unsigned long)(local_time.minutes) * 60L;
    theProgram.correctedSeconds+= (unsigned long)(local_time.seconds);
  }
}

void StopProgram(){
  theProgram.programStatus = LOADED;
  SetOptoState(0x00);
}

void StartProgram() {
  if(theProgram.programStatus != STAGED) return;
  
  theProgram.correctedSeconds=0;
  theProgram.ElapsedSecondsFromStart=0;
  theProgram.uninterruptedLoops=0;
  theProgram.CurrentStep=0;
  ValidateProgramTime();
  SetCurrentStep();
  if(theProgram.CurrentStep == theProgram.NumSteps) {  // Beyond end of LINEAR program
    StopProgram();
    return;
  }
  // Need to set the first program step going.
  SetOptoParameters(theProgram.Steps[theProgram.CurrentStep].Frequency, theProgram.Steps[theProgram.CurrentStep].PulseWidth);
  if (theProgram.Steps[theProgram.CurrentStep].IsLightOn == 0) {
    SetOptoState(0x00);
  }
  else {
    SetOptoState(0x0F);
  }
  validationCounter = 0;
  theProgram.programStatus = RUNNING;
}

void CheckProgramForStart(){
  if(theProgram.programStatus != STAGED) return;
  if(time_date_to_epoch(&local_time) >= theProgram.startEpochTime) {
    StartProgram();
   }
}

void StageProgram(){
 if(theProgram.NumSteps==0) return;

  if(isRTCInitialized==0 && theProgram.programType == CIRCADIAN)
    theProgram.programType = LOOPING;
    
  UpdateProgram();
  if(theProgram.programType==CIRCADIAN && theProgram.totalProgramDuration!=86400L)
    theProgram.programType=LOOPING;

  if(IsStartTimeZero()) {
    theProgram.startTime.seconds = local_time.seconds;
    theProgram.startTime.minutes = local_time.minutes;
    theProgram.startTime.hours =  local_time.hours;
    theProgram.startTime.weekday = local_time.weekday;
    theProgram.startTime.monthday = local_time.monthday;
    theProgram.startTime.month = local_time.month;
    theProgram.startTime.year = local_time.year;
  }

  if(theProgram.programType == CIRCADIAN){
    theProgram.startTime.seconds = 0;
    theProgram.startTime.minutes = 0;
    theProgram.startTime.hours =  0;
  }
  // Need this here even though update program was previously set
  // because the start time will have changed. No need to
  // recaculate all elapsed times so i didn't call UpdateProgram() again.
  theProgram.startEpochTime = time_date_to_epoch(&theProgram.startTime);
  theProgram.programStatus = STAGED;
  SetOptoState(0x00);
}

void ClearProgram(){
  StopProgram();
  theProgram.NumSteps=0;
  theProgram.programType = LOOPING;
  theProgram.CurrentStep=0;
  theProgram.uninterruptedLoops=0;
  theProgram.programStatus = NOTLOADED;
  theProgram.startTime.seconds = 0;
  theProgram.startTime.minutes = 0;
  theProgram.startTime.hours =  0;
  theProgram.startTime.monthday = 1;
  theProgram.startTime.month = 1;
  theProgram.startTime.year = 1;
  theProgram.startEpochTime = time_date_to_epoch(&theProgram.startTime);
  theProgram.totalProgramDuration=0;
  theProgram.correctedSeconds=0;
  theProgram.ElapsedSecondsFromStart=0;
}

void ProcessProgramValidationStep(){
  validationCounter=0;
  ValidateProgramTime();
  SetCurrentStep();
  if(theProgram.CurrentStep == theProgram.NumSteps) {  // Beyond end of LINEAR program
    StopProgram();
    return;
  }
  SetOptoParameters(theProgram.Steps[theProgram.CurrentStep].Frequency, theProgram.Steps[theProgram.CurrentStep].PulseWidth);
  if (theProgram.Steps[theProgram.CurrentStep].IsLightOn == 0) {
    SetOptoState(0x00);
  }
  else {
    SetOptoState(0x0F);
  }
}

// With the inclusion of an external trigger(s) on sensor ports 1 and 2
// this function is called every 1ms and will revert to 1sec if normal program
// is used.
void ProcessProgramStep(){
  unsigned char ison=0;
  milliSecondCounter++;
  // This function will assume the current program counters are correct.
  // If the above is true, then the type of program doesn't matter.
  if(theProgram.programStatus == STAGED){
    CheckProgramForStart();
  }
  if(theProgram.programStatus != RUNNING)
    return;

  if(milliSecondCounter>=1000){
    // Should only reach here every second.
    milliSecondCounter=0;
  
    if(++validationCounter > VALIDATIONSECONDS) {
      ProcessProgramValidationStep();
      if(theProgram.programStatus != RUNNING)
        return;
    }
    
    theProgram.ElapsedSecondsFromStart++;
    theProgram.correctedSeconds++;
    if(theProgram.correctedSeconds >= theProgram.Steps[theProgram.CurrentStep].ElapsedDurationAtEnd) {
      theProgram.CurrentStep++;
      if(theProgram.CurrentStep==theProgram.NumSteps) {
        theProgram.uninterruptedLoops++;
        if(theProgram.programType == LINEAR){
         StopProgram();
          return;
        }
        else {
          theProgram.CurrentStep=0;
          theProgram.correctedSeconds=0;
        }
      }
      SetOptoParameters(theProgram.Steps[theProgram.CurrentStep].Frequency,theProgram.Steps[theProgram.CurrentStep].PulseWidth);
      if(theProgram.Steps[theProgram.CurrentStep].IsLightOn==0) {
        SetOptoState(0x00);
      }
      else {
        SetOptoState(0x0F);
      }
    }
  }

  // This is at the end to override things that come previously
  if(theProgram.Steps[theProgram.CurrentStep].ActiveTriggers >0){
    if(theProgram.Steps[theProgram.CurrentStep].ActiveTriggers & 0x01)
      if(TRIGGER_PORT==1)
        ison=1;
    if(SUPRESSOR_PORT==1)
      ison=0;
    if(ison)
      SetOptoState(0x0F);
    else
      SetOptoState(0x00);
  }
}

void SendProgramData(){
  unsigned int i,bindex=0;
  barray[bindex++]=0xFE; //Address packet to UART Master
  if(theProgram.NumSteps==0){
     barray[bindex++]=(unsigned char)(1);
     barray[bindex++]=(unsigned char)(1);
     barray[bindex++]=(unsigned char)(1);
     barray[bindex++]=(unsigned char)(1);
     barray[bindex++]=(unsigned char)(1);
     barray[bindex++]=(unsigned char)(1);
     barray[bindex++]=(unsigned char)(1);
     barray[bindex++]=(unsigned char)(1);
     barray[bindex++]=(unsigned char)(1);
     barray[bindex++]=(unsigned char)(1);
  }
  else {
    for(i=0;i<theProgram.NumSteps;i++){
      barray[bindex++]=(unsigned char)(theProgram.Steps[i].IsLightOn);
      barray[bindex++]=(unsigned char)(theProgram.Steps[i].Frequency>>8);
      barray[bindex++]=(unsigned char)(theProgram.Steps[i].Frequency);
      barray[bindex++]=(unsigned char)(theProgram.Steps[i].PulseWidth>>8);
      barray[bindex++]=(unsigned char)(theProgram.Steps[i].PulseWidth);
      barray[bindex++]=(unsigned char)(theProgram.Steps[i].ActiveTriggers);
      barray[bindex++]=(unsigned char)(theProgram.Steps[i].DurationSeconds>>24);
      barray[bindex++]=(unsigned char)(theProgram.Steps[i].DurationSeconds>>16);
      barray[bindex++]=(unsigned char)(theProgram.Steps[i].DurationSeconds>>8);
      barray[bindex++]=(unsigned char)(theProgram.Steps[i].DurationSeconds);
    }
  }
  if(bindex >0) {
    cobsBufferLength=encodeCOBS(barray,bindex,cobsBuffer);
    cobsBuffer[cobsBufferLength++]=0x00;
    SendByteArray(cobsBuffer,cobsBufferLength);
  }
}

void SendProgramStatus(){
  barray[0]=0xFE; //Address packet to UART Master
  switch(theProgram.programStatus){
    case NOTLOADED:
      barray[1]=0x01;
      break;
    case LOADED:
      barray[1]=0x02;
      break;
    case STAGED:
      barray[1]=0x03;
      break;
    case RUNNING:
      barray[1]=0x04;
      break;
  }
  switch(theProgram.programType){
    case LINEAR:
      barray[2]=0x01;
      break;
  case LOOPING:
      barray[2]=0x02;
      break;
  case CIRCADIAN:
      barray[2]=0x03;
      break;
  }
  isTimeCallAnswered=2;
  ReadTimeFromRTC(&local_time);
  // If isRTCPresent is 0 it indicated a I2C timeout so its not there.
  // This change is probably not needed, but it is included here for clarity.
  if(isTimeCallAnswered==0) {
    local_time.seconds = 0;
    local_time.minutes =0;
    local_time.hours = 0;
    local_time.monthday = 1;
    local_time.month = 1;
    local_time.year = 1;
  }

  barray[3] = theProgram.startTime.year;
  barray[4] = theProgram.startTime.month;
  barray[5] = theProgram.startTime.monthday;
  barray[6] = theProgram.startTime.hours;
  barray[7] = theProgram.startTime.minutes;
  barray[8] = theProgram.startTime.seconds;

  barray[9] =local_time.year;
  barray[10] =local_time.month;
  barray[11] =local_time.monthday;
  barray[12] =local_time.hours;
  barray[13] =local_time.minutes;
  barray[14] =local_time.seconds;

  barray[15] =(unsigned char)(theProgram.ElapsedSecondsFromStart>>24);
  barray[16] =(unsigned char)(theProgram.ElapsedSecondsFromStart>>16);
  barray[17] =(unsigned char)(theProgram.ElapsedSecondsFromStart>>8);
  barray[18] =(unsigned char)theProgram.ElapsedSecondsFromStart;

  barray[19] =(unsigned char)(theProgram.correctedSeconds>>24);
  barray[20] =(unsigned char)(theProgram.correctedSeconds>>16);
  barray[21] =(unsigned char)(theProgram.correctedSeconds>>8);
  barray[22] =(unsigned char)(theProgram.correctedSeconds);

  barray[23] =(unsigned char)(theProgram.totalProgramDuration>>24);
  barray[24] =(unsigned char)(theProgram.totalProgramDuration>>16);
  barray[25] =(unsigned char)(theProgram.totalProgramDuration>>8);
  barray[26] =(unsigned char)(theProgram.totalProgramDuration);

  barray[27] =(unsigned char)(theProgram.NumSteps>>8);
  barray[28] =(unsigned char)(theProgram.NumSteps);

  barray[29] =(unsigned char)(theProgram.CurrentStep>>8);
  barray[30] =(unsigned char)(theProgram.CurrentStep);

  barray[31] =(unsigned char)(theProgram.uninterruptedLoops>>24);
  barray[32] =(unsigned char)(theProgram.uninterruptedLoops>>16);
  barray[33] =(unsigned char)(theProgram.uninterruptedLoops>>8);
  barray[34] =(unsigned char)(theProgram.uninterruptedLoops);

  cobsBufferLength=encodeCOBS(barray,35,cobsBuffer);
  cobsBuffer[cobsBufferLength++]=0x00;
  SendByteArray(cobsBuffer,cobsBufferLength);
}

void LoadProgramFromUART(unsigned char* buffer, unsigned int len){
  unsigned char nSteps,i;
  unsigned int lights,freq,pw,pindex;
  unsigned long int dur;
  unsigned char triggers;

  if(((len-9) % 10) != 0) // Nine bytes before steps, which are each 10.
    return;

  ClearProgram();

  if(buffer[2]==0x01)
    theProgram.programType=LINEAR;
  else if(buffer[2]==0x02)
    theProgram.programType=LOOPING;
  else if(buffer[2]==0x03)
    theProgram.programType=CIRCADIAN;
  else
    theProgram.programType=LOOPING;
      
     
  theProgram.startTime.year=buffer[3];
  theProgram.startTime.month=buffer[4];
  theProgram.startTime.monthday=buffer[5];
  theProgram.startTime.hours=buffer[6];
  theProgram.startTime.minutes=buffer[7];
  theProgram.startTime.seconds=buffer[8];

  nSteps = (int)((len-9)/10);
  pindex = 9;
  for(i=0;i<nSteps;i++){
    lights = buffer[pindex];
    freq = (unsigned int)(buffer[pindex+1]<<8) + (unsigned int)(buffer[pindex+2]); 
    pw = (unsigned int)(buffer[pindex+3]<<8) + (unsigned int)(buffer[pindex+4]);
    triggers = buffer[pindex+5];
    dur = (unsigned int)(buffer[pindex+6]<<24) + (unsigned int)(buffer[pindex+7]<<16)+
          (unsigned int)(buffer[pindex+8]<<8) + (unsigned int)(buffer[pindex+9]);
    AddProgramStep(lights, freq,  pw, triggers, dur);
    pindex+=10;
  }
  UpdateProgram();
  theProgram.programStatus=LOADED;
}

void AddStepsFromUART(unsigned char* buffer, unsigned int len){

}