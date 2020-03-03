#ifndef EEPROM_H
#define EEPROM_H

unsigned char IsProgramSaved();
void Initialize_24LC256();
void WriteString_24LC256(unsigned int address, char* wData);
void ReadString_24LC256(unsigned int address, char* rData);
void LoadProgram_24LC256();
void SaveProgram_24LC256();
unsigned char IsDefaultProgramSaved_24LC256();

#endif