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

#define MAXPROGRAMSTEPS 100

#define HEARTBEAT_PORT  _RE4
#define HEARTBEAT_TRIS  _TRISE4
#define FLIP_HEARTBEAT_LED() PORTEINV = 0x0010
#define HB_ON() _RE4=1
#define HB_OFF() _RE4=0

#define SIGNALLED_PORT  _RE5
#define SIGNALLED_TRIS  _TRISE5
#define FLIP_SIGNALLED_LED() PORTEINV = 0x0020
#define SIGNALLED_ON() _RE5=1
#define SIGNALLED_OFF() _RE5=0

// The following lines apply to all of the boards.

#define LEDSTRING1_ON() _LATE1=1
#define LEDSTRING2_ON() _LATE0=1
#define LEDSTRING3_ON() _LATE3=1
#define LEDSTRING4_ON() _LATE4=1


#define LEDSTRING1_OFF() _LATE1=0
#define LEDSTRING2_OFF() _LATE0=0
#define LEDSTRING3_OFF() _LATE3=0
#define LEDSTRING4_OFF() _LATE4=0



#define LEDSTRING1_PORT _RE1
#define LEDSTRING2_PORT _RE0
#define LEDSTRING3_PORT _RE3
#define LEDSTRING4_PORT _RE4

#define LEDSTRING1_TRIS _TRISE1
#define LEDSTRING2_TRIS _TRISE0
#define LEDSTRING3_TRIS _TRISE3
#define LEDSTRING4_TRIS _TRISE4


#define USERBUTTON1_TRIS _TRISB4
#define USERBUTTON1_PORT _RB4

#define USERBUTTON2_TRIS _TRISB3
#define USERBUTTON2_PORT _RB3

#define BUTTON1_DEBOUNCE_COUNTS 100
#define BUTTON2_DEBOUNCE_COUNTS 100

#define TRIGGER_TRIS _TRISD6
#define SUPRESSOR_TRIS _TRISD5
#define INDICATOR_TRIS _TRISD4

#define TRIGGER_PORT _RD6
#define SUPRESSOR_PORT _RD5
#define INDICATOR_PORT _RD4

#define TRIGGER_LAT _LATD6
#define SUPRESSOR_LAT _LATD5
#define INDICATOR_LAT _LATD4

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
