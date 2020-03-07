#ifndef OPTOFUNCTIONS_H
#define	OPTOFUNCTIONS_H

void SetOptoParameters(unsigned int hz, unsigned int dc);
void ConfigureOpto(void);
void GetOptoStatus(unsigned char* status);
void inline ProcessOptoStep();
void inline SetOptoState(unsigned char os);

#endif	/* OPTOFUNCTIONS_H */

