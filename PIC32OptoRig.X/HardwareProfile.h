/*********************************************************************
 *
 *	UNO32 board specific hardware definitions
 *
 *	UNO32 has PIC32MX320F128
 *********************************************************************
 *
 * Author               Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Scott Pletcher       02/16/13    First code for UNO32.
 ********************************************************************/
#ifndef __HARDWARE_PROFILE_H
#define __HARDWARE_PROFILE_H
#include <p32xxxx.h>

#define controllerID 0x01
#define CURRENTVERSION "2.5.0"

#define MAXPROGRAMSTEPS 35


#define HEARTBEAT_LAT  _RE3
#define HEARTBEAT_TRIS  _TRISE3
#define FLIP_HEARTBEAT_LED() PORTEINV = 0x0008
#define HB_ON() _RE3=1
#define HB_OFF() _RE3=0

// The following lines apply to all of the boards.

#define LEDSTRING1_ON() _LATA1=1
#define LEDSTRING2_ON() LATA.LATA0=1
#define LEDSTRING1_OFF() LATA.LATA1=0
#define LEDSTRING2_OFF() LATA.LATA0=0

#define LEDSTRING4_ON() LATA.LATA3=1
#define LEDSTRING3_ON() LATA.LATA4=1
#define LEDSTRING4_OFF() LATA.LATA3=0
#define LEDSTRING3_OFF() LATA.LATA4=0

#define LEDSTRING2_PORT PORTA.RA0
#define LEDSTRING1_PORT PORTA.RA1
#define LEDSTRING4_PORT PORTA.RA4
#define LEDSTRING3_PORT PORTA.RA3

#define USERBUTTON1_TRIS _TRISE6
#define USERBUTTON1_PORT _RE6

#define USERBUTTON2_TRIS _TRISE5
#define USERBUTTON2_PORT _RE5

#define BUTTON1_DEBOUNCE_COUNTS 100
#define BUTTON2_DEBOUNCE_COUNTS 100

#define TRIGGER1_TRIS _TRISD0
#define SUPRESSOR1_TRIS _TRISD1

#define TRIGGER1_PORT _RD0
#define SUPRESSOR1_PORT _RD1

#define TRIGGER1_LAT _LATD0
#define SUPRESSOR1_LAT _LATD1

/*********************************************************************
* GetSystemClock() returns system clock frequency.
*
* GetPeripheralClock() returns peripheral clock frequency.
*
********************************************************************/

#define GetSystemClock() 			(80000000ul)

/*********************************************************************
* Macro: #define	GetPeripheralClock() 
*
* Overview: This macro returns the peripheral clock frequency 
*			used in Hertz.
*			* value for PIC32 is <PRE>(GetSystemClock()/(1<<OSCCONbits.PBDIV)) </PRE>
*
* GetInstructionClock() is required for the new Microchip Libraries 2010-10-19
********************************************************************/
#define	GetPeripheralClock()		(GetSystemClock()/(1<<OSCCONbits.PBDIV))
#define GetInstructionClock() (GetSystemClock())
/*********************************************************************/





#endif // __HARDWARE_PROFILE_H
