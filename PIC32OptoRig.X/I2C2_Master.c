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
// while(I2C2CONbits.SEN || I2C2CONbits.PEN || I2C2CONbits.RSEN || I2C2CONbits.RCEN || I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);

void FreeI2C() {
    int i;
    I2CEnable(I2C2, FALSE);
    TRISAbits.TRISA2 = 0;
    TRISAbits.TRISA3 = 1;

    for (i = 0; i < 50; i++) {
        PORTAINV = 0x0004;
        Delay10us(1);
    }
    I2CEnable(I2C2, TRUE);
}

void ConfigureI2C2(void) {
    int actualClock;
    INTSetVectorPriority(INT_I2C_2_VECTOR, INT_PRIORITY_LEVEL_5);
    //INTEnable(INT_I2C2M, INT_ENABLED); // Master event
    INTEnable(INT_I2C2B, INT_ENABLED); // Bus collision event
    INTClearFlag(INT_I2C2M);
    INTClearFlag(INT_I2C2B);
    I2CConfigure(I2C2, I2C_ENABLE_SLAVE_CLOCK_STRETCHING);
    actualClock = I2CSetFrequency(I2C2, GetPeripheralClock(), I2C_CLOCK_FREQ);
    if (abs(actualClock - I2C_CLOCK_FREQ) > I2C_CLOCK_FREQ / 10) {
        // Some error condition here.
    }
    I2CEnable(I2C2, TRUE);
}

void __ISR(_I2C_2_VECTOR, IPL5AUTO) I2C2InterruptServiceRoutine(void) {
    // check for MASTER and Bus events and respond accordingly
    if (IFS1bits.I2C2SIF == 1) { // Should never be here because we don't operate as slave.
        //if(IsInDarkMode==0)
        //    IO_LED6_ON();
        INTClearFlag(INT_I2C2M);
        ErrorsRegister.bits.I2CError = 1;
        return;
    }
    if (IFS1bits.I2C2BIF == 1) {
        ErrorsRegister.bits.I2CError = 1;
        FreeI2C();
        if (I2C2STATbits.BCL == 0)
            ErrorsRegister.bits.I2CError = 0;
        //if(IsInDarkMode==0)
        //    IO_LED5_ON();
        INTClearFlag(INT_I2C2B);

        return;
    }
    INTClearFlag(INT_I2C2M);
}

I2C_RESULT Read8FromI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned char *data) {
    IdleI2C2();
    // Now begin the send sequence
    StartI2C2(); // Send the start bit.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2(); // Wait until this is complete.

    // Send the slave address and memory start address	
    MasterWriteI2C2(slaveaddress);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // Make sure it is acked.	

    MasterWriteI2C2(dataaddress);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR;


    RestartI2C2(); // Send restart.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    MasterWriteI2C2(slaveaddress | 0x01); // Send slave address but last bit changed to 1 to indicate a read.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR;

    *data = MasterReadI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    NotAckI2C2();

    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    StopI2C2();
    return I2C_SUCCESS;
}

I2C_RESULT Read16FromI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned int *data) {
    // Now begin the send sequence
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    StartI2C2(); // Send the start bit.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2(); // Wait until this is complete.

    // Send the slave address and memory start address	
    MasterWriteI2C2(slaveaddress);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // Make sure it is acked.	

    MasterWriteI2C2(dataaddress);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR;


    RestartI2C2(); // Send restart.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    MasterWriteI2C2(slaveaddress | 0x01); // Send slave address but last bit changed to 1 to indicate a read.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR;

    *data = MasterReadI2C2();
    AckI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();


    *data = (*data << 8) + MasterReadI2C2();
    NotAckI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();


    StopI2C2();
    return I2C_SUCCESS;
}

I2C_RESULT Read32FromI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned int *data) {
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    // Now begin the send sequence
    StartI2C2(); // Send the start bit.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2(); // Wait until this is complete.

    // Send the slave address and memory start address	
    MasterWriteI2C2(slaveaddress);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR;

    MasterWriteI2C2(dataaddress);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR;

    RestartI2C2(); // Send restart.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    MasterWriteI2C2(slaveaddress | 0x01); // Send slave address but last bit changed to 1 to indicate a read.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    *data = MasterReadI2C2();
    AckI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    *data = (*data << 8) + MasterReadI2C2();
    AckI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    *data = (*data << 8) + MasterReadI2C2();
    AckI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    *data = (*data << 8) + MasterReadI2C2();
    NotAckI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    StopI2C2();
    return I2C_SUCCESS;
}

I2C_RESULT Read32FromI2C2Backward(unsigned char slaveaddress, unsigned char dataaddress, unsigned int *data) {
    unsigned char a, b, c, d;
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    // Now begin the send sequence
    StartI2C2(); // Send the start bit.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2(); // Wait until this is complete.

    // Send the slave address and memory start address	
    MasterWriteI2C2(slaveaddress);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(dataaddress);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    //StopI2C2(); // Send the stop condition.
    //IdleI2C2(); // Wait until done.

    RestartI2C2(); // Send restart.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    MasterWriteI2C2(slaveaddress | 0x01); // Send slave address but last bit changed to 1 to indicate a read.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    *data = 0;
    // Note that the masterread function waits until the receive bit is cleared.  Since all other relevant bits are 
    // also expected to be clear, there is no reason to check Idel before sending and acknowledgement.
    a = MasterReadI2C2();
    AckI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    b = MasterReadI2C2();
    AckI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    c = MasterReadI2C2();
    AckI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    d = MasterReadI2C2();
    NotAckI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    StopI2C2();
    *(data) = (d << 24) + (c << 16) + (b << 8) + a;
    return I2C_SUCCESS;
}


// This function is simplified for single byte addresses

I2C_RESULT Write8ToI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned char data) {
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    // Begin the send sequence
    StartI2C2(); // Send the start bit.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2(); // Wait until this is complete.

    MasterWriteI2C2(slaveaddress);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(dataaddress);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(data);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CStop(I2C2); // Send the stop condition.		
    return I2C_SUCCESS;

}

I2C_RESULT WriteByte_24LC256(unsigned int address, unsigned char wData) {
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    // Begin the send sequence
    StartI2C2(); // Send the start bit.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2(); // Wait until this is complete.

    MasterWriteI2C2(WRITEADDRESS_24LC256);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(address >> 8);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(address & 0xFF);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(wData);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CStop(I2C2); // Send the stop condition.		
    DelayMs(1);
    return I2C_SUCCESS;
}
//--------------- Reads data from 24C02 EEPROM - single location (random)

I2C_RESULT ReadByte_24LC256(unsigned int address, unsigned char *data) {
    IdleI2C2();
    // Now begin the send sequence
    StartI2C2(); // Send the start bit.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2(); // Wait until this is complete.

    // Send the slave address and memory start address	
    MasterWriteI2C2(WRITEADDRESS_24LC256);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // Make sure it is acked.	

    MasterWriteI2C2(address >> 8);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR;

    MasterWriteI2C2(address & 0xFF);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR;

    RestartI2C2(); // Send restart.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    MasterWriteI2C2(READADDRESS_24LC256); // Send slave address but last bit changed to 1 to indicate a read.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR;

    *data = MasterReadI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    NotAckI2C2();

    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    StopI2C2();
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

    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    // Begin the send sequence
    StartI2C2(); // Send the start bit.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2(); // Wait until this is complete.

    MasterWriteI2C2(WRITEADDRESS_DS3231);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(0);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(second);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(minute);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(hour);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(1);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(m_day);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(month);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    MasterWriteI2C2(year);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // If this bit is 1, then slave failed to ackknowledge, so break.	

    I2CStop(I2C2); // Send the stop condition.		
    DelayMs(1);
    return I2C_SUCCESS;
}

I2C_RESULT ReadTimeFromRTC(rtc_time_t *ts) {
    unsigned short seconds, minute, hour, m_day, month, year, tmp;

    IdleI2C2();
    // Now begin the send sequence
    StartI2C2(); // Send the start bit.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2(); // Wait until this is complete.

    // Send the slave address and memory start address	
    MasterWriteI2C2(WRITEADDRESS_DS3231);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR; // Make sure it is acked.	

    MasterWriteI2C2(0);
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR;

    RestartI2C2(); // Send restart.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();

    MasterWriteI2C2(READADDRESS_DS3231); // Send slave address but last bit changed to 1 to indicate a read.
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    if (I2C2STATbits.ACKSTAT) return I2C_ERROR;

    seconds = MasterReadI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    AckI2C2();

    minute = MasterReadI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    AckI2C2();

    hour = MasterReadI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    AckI2C2();

    tmp = MasterReadI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    AckI2C2();

    m_day = MasterReadI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    AckI2C2();

    month = MasterReadI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    AckI2C2();

    year = MasterReadI2C2();
    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    NotAckI2C2();

    if (ErrorsRegister.bits.I2CError == 1) return I2C_ERROR;
    IdleI2C2();
    StopI2C2();

    ts->seconds = Bcd2Dec(seconds);
    ts->minutes = Bcd2Dec(minute);
    ts->hours = Bcd2Dec(hour);
    ts->monthday = Bcd2Dec(m_day);
    ts->month = Bcd2Dec(month);
    ts->year = Bcd2Dec(year);

    return I2C_SUCCESS;
}