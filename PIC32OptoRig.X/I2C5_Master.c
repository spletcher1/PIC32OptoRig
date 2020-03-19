#include "GlobalIncludes.h"

#define WRITEADDRESS_24LC256 0xAE // Rule is first four bits are 1010, next three don't matter, and final (least sig bit) is 0 for write and 1 for read.
#define READADDRESS_24LC256  0xAF

#define WRITEADDRESS_DS3231 0xD0 // Rule is first four bits are 1010, next three don't matter, and final (least sig bit) is 0 for write and 1 for read.
#define READADDRESS_DS3231  0xD1

// I am using a mixture of the last PLib libraries and the legacy libraries.
// The "newer" ones don't send and receive correctly, but they make configuration
// of the module and interrupts a bit more intuitive.



// Normally I2C devices run at either 100khz or 400 khz, although some can do 2MHz
// The propellor seems to only manage 100khz
#define I2C_CLOCK_FREQ 400000 // Set at 400000 for normal EEPROM and 100000 for Propeller
int ErrorCount;
I2C_RESULT theCurrentI2CResult;
extern errorFlags_t volatile ErrorsRegister;

// the IdleFunction 
// while(I2C5CONbits.SEN || I2C5CONbits.PEN || I2C5CONbits.RSEN || I2C5CONbits.RCEN || I2C5CONbits.ACKEN || I2C5STATbits.TRSTAT);

void FreeI2C() {
    int i;
    I2CEnable(I2C5, FALSE);
    TRISFbits.TRISF5 = 0;
    TRISFbits.TRISF4 = 1;

    for (i = 0; i < 50; i++) {
        PORTFINV = 0x0010;
        Delay10us(1);
    }
    I2CEnable(I2C5, TRUE);
}

void ConfigureI2C5(void) {
    int actualClock;
    INTSetVectorPriority(INT_I2C_5_VECTOR, INT_PRIORITY_LEVEL_5);
    //INTEnable(INT_I2C5M, INT_ENABLED); // Master event
    INTEnable(INT_I2C5B, INT_ENABLED); // Bus collision event
    INTClearFlag(INT_I2C5M);
    INTClearFlag(INT_I2C5B);
    I2CConfigure(I2C5, I2C_ENABLE_SLAVE_CLOCK_STRETCHING);
    actualClock = I2CSetFrequency(I2C5, GetPeripheralClock(), I2C_CLOCK_FREQ);
    if (abs(actualClock - I2C_CLOCK_FREQ) > I2C_CLOCK_FREQ / 10) {
        // Some error condition here.
    }
    I2CEnable(I2C5, TRUE);
}

void __ISR(_I2C_5_VECTOR, IPL5AUTO) I2C5InterruptServiceRoutine(void) {
    // check for MASTER and Bus events and respond accordingly
    if (IFS1bits.I2C5SIF == 1) { // Should never be here because we don't operate as slave.
        //if(IsInDarkMode==0)
        //    IO_LED6_ON();
        INTClearFlag(INT_I2C5M);
        ErrorsRegister.bits.I2CError = 1;
        return;
    }
    if (IFS1bits.I2C5BIF == 1) {
        ErrorsRegister.bits.I2CError = 1;
        FreeI2C();
        if (I2C5STATbits.BCL == 0)
            ErrorsRegister.bits.I2CError = 0;
        //if(IsInDarkMode==0)
        //    IO_LED5_ON();
        INTClearFlag(INT_I2C5B);

        return;
    }
    INTClearFlag(INT_I2C5M);
}

void IdleI2C5(void)
{
    /* Wait until I2C Bus is Inactive */
    while(I2C5CONbits.SEN || I2C5CONbits.PEN || I2C5CONbits.RSEN || I2C5CONbits.RCEN || I2C5CONbits.ACKEN || I2C5STATbits.TRSTAT);
}
unsigned char MasterReadI2C5(void)
{
    I2C5CONbits.RCEN = 1;
    while(I2C5CONbits.RCEN);
    I2C5STATbits.I2COV = 0;
    return(I2C5RCV);
}

I2C_RESULT WriteByte_24LC256(unsigned int address, unsigned char wData, unsigned char delay) {
    if(delay)
        DelayMs(5);
    IdleI2C5();
    // Begin the send sequence
    I2CStart(I2C5); // Send the start bit.   
    IdleI2C5();

    I2CSendByte(I2C5, WRITEADDRESS_24LC256);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CSendByte(I2C5, address >> 8);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CSendByte(I2C5, address & 0xFF);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CSendByte(I2C5, wData);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CStop(I2C5); // Send the stop condition.		
    DelayMs(1);
    return I2C_SUCCESS;
}
//--------------- Reads data from 24C02 EEPROM - single location (random)

I2C_RESULT ReadByte_24LC256(unsigned int address, unsigned char *data) {
    IdleI2C5();
    // Now begin the send sequence
    I2CStart(I2C5); // Send the start bit.  
    IdleI2C5();

    // Send the slave address and memory start address	
    I2CSendByte(I2C5, WRITEADDRESS_24LC256);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // Make sure it is acked.	

    I2CSendByte(I2C5, address >> 8);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR;

    I2CSendByte(I2C5, address & 0xFF);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR;

    I2CRepeatStart(I2C5);    
    IdleI2C5();

    I2CSendByte(I2C5, READADDRESS_24LC256); // Send slave address but last bit changed to 1 to indicate a read.    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR;
    
    //while(!I2CReceivedDataIsAvailable(I2C5)); 
    *data = MasterReadI2C5(); 
    I2CAcknowledgeByte(I2C5, FALSE);
    IdleI2C5();
   
    I2CStop(I2C5);
    return I2C_SUCCESS;
}

I2C_RESULT WriteTimeToRTC(rtc_time_t *sTime) {
    // convert decimal to BCD
    unsigned short second = Dec2Bcd(sTime->seconds);
    unsigned short minute = Dec2Bcd(sTime->minutes);
    unsigned short hour = Dec2Bcd(sTime->hours);
    unsigned short m_day = Dec2Bcd(sTime->monthday);
    unsigned short month = Dec2Bcd(sTime->month);
    unsigned short year = Dec2Bcd(sTime->year);
    // end conversion
   
    IdleI2C5();
    // Begin the send sequence
    I2CStart(I2C5); // Send the start bit.
    IdleI2C5();

    I2CSendByte(I2C5, WRITEADDRESS_DS3231);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CSendByte(I2C5, 0);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CSendByte(I2C5, second);   
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CSendByte(I2C5, minute);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CSendByte(I2C5, hour);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CSendByte(I2C5, 1);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CSendByte(I2C5, m_day);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CSendByte(I2C5, month);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CSendByte(I2C5, year);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CStop(I2C5); // Send the stop condition.		
    DelayMs(1);
    return I2C_SUCCESS;
}

//I2CBusIsIdle
//return ( (i2cRegisters->I2CxSTATbits.S == 0 && i2cRegisters->I2CxSTATbits.P == 0 ) ||
//             (i2cRegisters->I2CxSTATbits.S == 0 && i2cRegisters->I2CxSTATbits.P == 1 )   );





I2C_RESULT ReadTimeFromRTC(rtc_time_t *ts) {
    unsigned char seconds, minute, hour, m_day, month, year, tmp;

    while (!I2CBusIsIdle(I2C5));
    // Now begin the send sequence
    I2CStart(I2C5); // Send the start bit.    
    IdleI2C5();

    // Send the slave address and memory start address	
    I2CSendByte(I2C5, WRITEADDRESS_DS3231);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR; // Make sure it is acked.	

    I2CSendByte(I2C5, 0);    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR;

    I2CRepeatStart(I2C5);    
    IdleI2C5();

    I2CSendByte(I2C5, READADDRESS_DS3231); // Send slave address but last bit changed to 1 to indicate a read.    
    IdleI2C5();
    if (I2C5STATbits.ACKSTAT) return I2C_ERROR;
    
    //while(!I2CReceivedDataIsAvailable(I2C5)); 
    seconds = MasterReadI2C5();        
    I2CAcknowledgeByte(I2C5, TRUE);
    IdleI2C5();
//DelayMs(1);
    //while(!I2CReceivedDataIsAvailable(I2C5)); 
    minute = MasterReadI2C5();    
    I2CAcknowledgeByte(I2C5, TRUE);
    IdleI2C5();    
//DelayMs(1);
    //while(!I2CReceivedDataIsAvailable(I2C5)); 
    hour = MasterReadI2C5();        
    I2CAcknowledgeByte(I2C5, TRUE);
    IdleI2C5();    
//DelayMs(1);
    //while(!I2CReceivedDataIsAvailable(I2C5)); 
    tmp = MasterReadI2C5();            
    I2CAcknowledgeByte(I2C5, TRUE);
    IdleI2C5();
//DelayMs(1);
    //while(!I2CReceivedDataIsAvailable(I2C5)); 
    m_day = MasterReadI2C5();        
    I2CAcknowledgeByte(I2C5, TRUE);
    IdleI2C5();
//DelayMs(1);
    //while(!I2CReceivedDataIsAvailable(I2C5)); 
    month = MasterReadI2C5();    
    I2CAcknowledgeByte(I2C5, TRUE);
    IdleI2C5();    
//DelayMs(1);
    //while(!I2CReceivedDataIsAvailable(I2C5)); 
    year = MasterReadI2C5();    
    I2CAcknowledgeByte(I2C5, FALSE);
    IdleI2C5();
    
//DelayMs(1);        
    I2CStop(I2C5);
    ts->seconds = Bcd2Dec(seconds);
    ts->minutes = Bcd2Dec(minute);
    ts->hours = Bcd2Dec(hour);
    ts->monthday = Bcd2Dec(m_day);
    ts->month = Bcd2Dec(month);
    ts->year = Bcd2Dec(year);

    return I2C_SUCCESS;
}
 