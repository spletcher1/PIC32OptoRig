#ifndef OPTOFUNCTIONS_H
#define	OPTOFUNCTIONS_H


void ConfigureOpto();
void SetOptoState(unsigned char os1, unsigned char os2);
void SetPulseWidth_ms(unsigned int pw);
void SetHertz(unsigned int hz);

// Definitions for legacy port
#define COL1ON() LATGSET=0x100
#define COL1OFF() LATGCLR=0x100
#define COL2ON() LATDSET=0x10
#define COL2OFF() LATDCLR=0x10

#define ROW1ON() LATDSET=0x200
#define ROW1OFF() LATDCLR=0x200
#define ROW2ON() LATDSET=0x80
#define ROW2OFF() LATDCLR=0x80
#define ROW3ON() LATDSET=0x40
#define ROW3OFF() LATDCLR=0x40
#define ROW4ON() LATDSET=0x100
#define ROW4OFF() LATDCLR=0x100
#define ROW5ON() LATGSET=0x40
#define ROW5OFF() LATGCLR=0x40
#define ROW6ON() LATGSET=0x80
#define ROW6OFF() LATGCLR=0x80

// Definitions for new port
#define CHAMBER1_ON() LATDbits.LATD0=1
#define CHAMBER2_ON() LATDbits.LATD1=1
#define CHAMBER3_ON() LATDbits.LATD2=1
#define CHAMBER4_ON() LATDbits.LATD3=1
#define CHAMBER5_ON() LATDbits.LATD4=1
#define CHAMBER6_ON() LATDbits.LATD5=1
#define CHAMBER7_ON() LATDbits.LATD6=1
#define CHAMBER8_ON() LATDbits.LATD7=1
#define CHAMBER9_ON() LATDbits.LATD8=1
#define CHAMBER10_ON() LATDbits.LATD9=1
#define CHAMBER11_ON() LATDbits.LATD10=1
#define CHAMBER12_ON() LATDbits.LATD11=1

#define CHAMBER1_OFF() LATDbits.LATD0=0
#define CHAMBER2_OFF() LATDbits.LATD1=0
#define CHAMBER3_OFF() LATDbits.LATD2=0
#define CHAMBER4_OFF() LATDbits.LATD3=0
#define CHAMBER5_OFF() LATDbits.LATD4=0
#define CHAMBER6_OFF() LATDbits.LATD5=0
#define CHAMBER7_OFF() LATDbits.LATD6=0
#define CHAMBER8_OFF() LATDbits.LATD7=0
#define CHAMBER9_OFF() LATDbits.LATD8=0
#define CHAMBER10_OFF() LATDbits.LATD9=0
#define CHAMBER11_OFF() LATDbits.LATD10=0
#define CHAMBER12_OFF() LATDbits.LATD11=0

#endif	/* OPTOFUNCTIONS_H */

