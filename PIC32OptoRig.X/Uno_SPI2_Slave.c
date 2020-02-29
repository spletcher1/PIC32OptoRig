
#include "GlobalIncludes.h"
#include <stdio.h>

//BYTE SPIWrite(BYTE data);
#define ClockSPI() SPIWrite(0xFF);
#define SPIRead() SPIWrite(0xFF);
#define SPIGet() SPI2BUF  // Short macro to pull the input byte from the buffer.

//#define SPI_BAUD 0 // with Fpb=40MHz this give 20MHz transfer speed
#define SPI_BAUD 15 // with Fpb=40MHz this give 1.25MHz transfer speed

void ConfigureSPI2_Slave(void)
{
    int tmp;
	// Set as slave mode, 32 bit, use SDO pin, Idle low, transition from idle to active, no interrupts
        IEC0CLR=0x03800000; // disable all interrupts
	SPI2CONbits.ON=0;  // Turn off while configuring
        tmp = SPI2BUF; // Clear the receive buffer.
        SPI2CONbits.ENHBUF = 0; // Set to use enhanced buffer mode.
        SPI2STATbits.SPIROV=0;	// Clear the buffer overflow bit.

	SPI2CONbits.SIDL= 0; // Stop in idle mode?
	SPI2CONbits.DISSDO = 0; // If set, then disable SDO pin for use as GPIO. Used for receive only mode.
	SPI2CONbits.MODE32 = 1; // Use 32 bit mode? (If this is set, 32bit is used regardless of MODE16 bit.)
	SPI2CONbits.MODE16 = 0; // Use 16 bit mode? (If neither MODE32 nor MODE16 are set, then use 8 bit mode.)
	SPI2CONbits.MSTEN = 0; //1=Master mode; 0= Slave mode.
        SPI2CONbits.SMP = 0; // In Master mode: 1= input sampled at end of data output, 0 = input sampled at middle.
	SPI2CONbits.CKE = 0; // 1 = data changes from active to idle. 0= Opposite.
	SPI2CONbits.SSEN = 1; // 1= SSx pint used for slave mode; 0 = Not used for slave mode, controlled as GPIO.
	SPI2CONbits.CKP = 0; // 1= Idle stat for clock is high, active is low. 0 = Idle is low, active is high.
        

	/********************************
	// These bits may not be available on 64pin MX460256H
	SPI2CONbits.STXISEL1 = 0;  // Set when transmit interrupt is triggered if active.
	SPI2CONbits.STXISEL0 = 0;  // 00=Set when buffer is NOT full. 10 = Set when buffer is empty by 1/2 or more.
							  // 01 = Set when buffer is completely empty. 00 = set after last shift and transmit complete.
	SPI2CONbits.RTXISEL1 = 0; // Set when receive interrupt is triggered, if active.
	SPI2CONbits.RTXISEL0 = 0; // 00=Set when buffer is full. 10 = Set when buffer is full by 1/2 or more.
							  // 01 = Set when buffer is NOT empty. 00 = set after word in receive buffer is read (buffer is empty).
	*********************************/

	SPI2BRG= SPI_BAUD;
	

	//	Need a second slave?
	//	SLAVEPINTRIS2 = 0;
	//	SLAVEPIN2=1;

	SPI2CONbits.ON=1; // Let 'er rip
}

