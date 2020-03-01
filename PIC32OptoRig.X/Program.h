#ifndef PROGRAM_H
#define PROGRAM_H

#include "GlobalIncludes.h"

void ConfigureSimpleProgram(unsigned long int hson, unsigned long int hsoff);
void ProcessProgramStep();
void StageProgram();
void StopProgram();
void SendProgramData();
void SendProgramStatus();
void ClearProgram();
void CheckProgramForStart();
void LoadProgram();
void AddProgramStep(unsigned int islighton, unsigned int freq, unsigned int pw,unsigned char triggers, unsigned long int duration);
void LoadProgramFromUART(unsigned char* buffer, unsigned int len);
void AddStepsFromUART(unsigned char* buffer, unsigned int len);
void UpdateProgram(void);

typedef enum
{
  LINEAR,
  LOOPING,
  CIRCADIAN,
} program_type;

typedef enum
{
  NOTLOADED,
  LOADED,
  STAGED,
  RUNNING
} program_status;



struct ProgramStep {
  unsigned int StepNumber;
  unsigned int IsLightOn;
  unsigned int Frequency;
  unsigned int PulseWidth;
  unsigned char ActiveTriggers;
  unsigned long int DurationSeconds;
  unsigned long int ElapsedDurationAtEnd;
};

struct FullProgram{
  program_type programType;
  rtc_time_t startTime;
  unsigned long startEpochTime;
  unsigned long int ElapsedSecondsFromStart;
  unsigned long int correctedSeconds; // This is the number of seconds since the beginning of the latest program cycle.
  unsigned long totalProgramDuration;
  unsigned long uninterruptedLoops;
  program_status programStatus;
  unsigned int NumSteps;
  unsigned int CurrentStep;
  struct ProgramStep Steps[MAXPROGRAMSTEPS];
};



#endif