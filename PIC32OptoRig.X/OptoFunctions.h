#ifndef OPTOFUNCTIONS_H
#define	OPTOFUNCTIONS_H

void SetOptoParameters(unsigned int hz, unsigned int pw);
void ConfigureOpto(void);
void inline Opto_On();
void inline Opto_Off();
void GetOptoStatus(unsigned char* status);
void ToggleOpto();
void inline ProcessOptoStep();
void inline SetOptoState(unsigned char os);
void SetPulseWidth_ms(unsigned int pw);
void SetHertz(unsigned int hz);


#endif	/* OPTOFUNCTIONS_H */

