
void StopTransfer( void )
{
    I2C_STATUS  status;

    // Send the Stop signal
    I2CStop(I2C2);

    // Wait for the signal to complete
    do
    {
        status = I2CGetStatus(I2C2);
          if (ErrorCount > 0) {
            return;
        }

    } while ( !(status & I2C_STOP) );
}

unsigned char StartTransfer( unsigned char restart )
{
    I2C_STATUS  status;

    // Send the Start (or Restart) signal
    if(restart)
    {
        I2CRepeatStart(I2C2);
    }
    else
    {
        // Wait for the bus to be idle, then start the transfer
        while( !I2CBusIsIdle(I2C2) ){
             if (ErrorCount > 0) {
                return 0;
             }
        }

        if(I2CStart(I2C2) != I2C_SUCCESS)
        {           
            return 0;
        }
    }

    // Wait for the signal to complete
    do
    {
        status = I2CGetStatus(I2C2);
         if (ErrorCount > 0) {
                return 0;
         }
    } while ( !(status & I2C_START) );

    return 1;
}

unsigned char IsBusy(unsigned char slaveaddress){
	unsigned char result;
	StartI2C2();
	IdleI2C2();
	MasterWriteI2C2(slaveaddress);
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) result = 1;
	else result = 0;
	StopI2C2();IdleI2C2();
	return result;
}


// Return Codes:
// 0= No error
// 1 = Failed acknowledgment after write.
// 2 = Datasize too large.
char WriteCharI2C2(unsigned char slaveaddress, char data) {

	// Begin the send sequence
	StartI2C2(); // Send the start bit.
	IdleI2C2(); // Wait until this is complete.

	MasterWriteI2C2(slaveaddress << 1);
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.

	MasterWriteI2C2(data);
	IdleI2C2();
	if(I2C2STATbits.ACKSTAT) return 4; // If this bit is 1, then slave failed to ackknowledge, so break.

	I2CStop(I2C2);// Send the stop condition.
	IdleI2C2(); // Wait until done.

	return 0;

}
unsigned char ReadCharI2C2(unsigned char slaveaddress, unsigned char dataaddress, char *data) {
	char result;
	// Now begin the send sequence
	IdleI2C1();
	//while(IsPropBusy(slaveaddress));
	StartI2C1(); // Send the start bit.
	IdleI2C1(); // Wait until this is complete.

	// Send the slave address and memory start address	
	result=MasterWriteI2C1(slaveaddress);
	if(result<0) return 10-result;
	// For 24LC256, the top four bytes must be 1010 = 0xA0, then the three address bits.  The LSB is left open to indicate read/write.
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 1; // If this bit is 1, then slave failed to ackknowledge, so break.	
		
	result=MasterWriteI2C1(dataaddress);
	if(result<0) return 30-result;
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) return 3; // If this bit is 1, then slave failed to ackknowledge, so break.	
		
	StopI2C1(); // Send the stop condition.
	IdleI2C1(); // Wait until done.
	
	RestartI2C1(); // Send restart.
	IdleI2C1();
	
	result=MasterWriteI2C1(slaveaddress | 0x01); // Send slave address but last bit changed to 1 to indicate a read.
	if(result<0) return 40-result;
	IdleI2C1();
	
	*data = MasterReadI2C1();
	IdleI2C1();
	if(I2C1STATbits.ACKSTAT) {return 4;} // If this bit is 1, then slave failed to ackknowledge, so break.
	
	StopI2C1();
	IdleI2C1();
	return 0;
}

unsigned char TransmitOneByte( unsigned char data )
{
    // Wait for the transmitter to be ready
    while(!I2CTransmitterIsReady(I2C2)){
         if (ErrorCount > 0) {
                return 0;
         }
    }

    // Transmit the byte
    if(I2CSendByte(I2C2, data) == I2C_MASTER_BUS_COLLISION)
    {
        ErrorCount++;
        return 0;
    }

    // Wait for the transmission to finish
    while(!I2CTransmissionHasCompleted(I2C2)){
         if (ErrorCount > 0) {
                return 0;
         }
    }

    return 1;
}


// Below functions are for slave device.
/*
void I2C2Respond(void) {
    int dataRead, i;
    unsigned int tmp2;
    unsigned char tmp;
    static unsigned char command, argument;
    static unsigned char *pValue;
    static unsigned int dIndex;
    static unsigned int wIndex;
    if ((I2C2STATbits.R_W == 0) && (I2C2STATbits.D_A == 0)) {
        // R/W bit = 0 --> indicates data transfer is input to slave
        // D/A bit = 1 --> indicates last byte was data

        // So this means the Master sent a device address and write request
        // It should follow with a write request and data.

        // read the address data from the buffer to avoid
        // subsequent overflow.             
        dataRead = SlaveReadI2C2();
        wIndex = 0;
        // release the clock to restart I2C
        I2C2CONbits.SCLREL = 1; // release clock stretch bit
    } else if ((I2C2STATbits.R_W == 0) && (I2C2STATbits.D_A == 1)) {
        // This means the Master sent a data and write request
        if (wIndex == 0) {
            // Because wIndex is reset when the address is sent
            // then this is the first byte...the command.
            command = SlaveReadI2C2();
            wIndex++;
        } else if (wIndex==1){
            argument = SlaveReadI2C2();
            switch (command) {
                case 0x01:
                    if (argument == 0x00)
                        isInDarkMode = 0;
                    else if (argument == 0x01) {
                        isInDarkMode = 1;
                        HEARTBEAT_LAT = 0;
                        GREENLED_LAT=0;
                    }
                    break;
                case 0x02: // Need to expand this to be specific for 6 chamber setup.
                    OptoState1 = argument;
                    break;
                case 0x03:
                    break; // This is to be implemented 12-well setup.
                case 0x04:
                    SetHertz(argument);
                    break;
                case 0x05:
                    SetPulseWidth_ms(argument);
                    break;
            }
            wIndex++;
        }
        else if(wIndex==2){
            argument = SlaveReadI2C2();
            switch (command) {
                case 0x02:
                    OptoState2=argument;
                    break;            
            }
        }
        else {
            
        }
        // release the clock to restart I2C
        I2C2CONbits.SCLREL = 1; // release clock stretch bit
    } else if ((I2C2STATbits.R_W == 1) && (I2C2STATbits.D_A == 0)) {
        //if (IsInDarkMode == 0)
        //    LED1_ON();
        // So this means the Master sent an address and read request
        dataRead = SlaveReadI2C2(); // This just empties the receive buffer.
        // We need to freeze the current values at the point when the MCU
        // calls for them because the analog interrupt may change them.
        WrittenValues[15] = 0;
        for (i = 0; i < 12; i++) {
            WrittenValues[i] = CurrentValues[i];
            WrittenValues[15] += WrittenValues[i];
        }
        tmp2=isInDarkMode + (hertz << 8) + (pulseWidth_ms << 16);
        WrittenValues[12] = tmp2;        
        WrittenValues[13] = OptoState1;      
        WrittenValues[14] = OptoState2;
        WrittenValues[15] += tmp2+OptoState1+OptoState2;
        pValue = (unsigned char *) WrittenValues;
        SlaveWriteI2C2(*pValue);
        //SlaveWriteI2C2(0);
        pValue++;
        dIndex = 1;
        if (isInDarkMode == 0) FLIP_GREEN_LED();
    } else if ((I2C2STATbits.R_W == 1) && (I2C2STATbits.D_A == 1)) {
        // So this means the Master sent a data and read request

        // Send twelve data bytes and one checksum byte
        if (dIndex >= 64)
            SlaveWriteI2C2(0);
        else {
            SlaveWriteI2C2(*pValue);
            //SlaveWriteI2C2(0);
            pValue++;
            dIndex++;
        }
        I2C2CONbits.SCLREL = 1; // release clock stretch bit
        ; // After writing current values, update the next ones.
    }
}

void __ISR(_I2C_2_VECTOR, IPL4AUTO) I2C2InterruptServiceRoutine(void) {
    if (IFS1bits.I2C2MIF == 1) { // Should never be here because we don't operate as slave.
        //LED1_ON();
        mI2C2MClearIntFlag();
        return;
    }
    if (IFS1bits.I2C2BIF == 1) {
        //LED1_ON();
        mI2C2BClearIntFlag();
        return;
    }
    // If not the previous two, then I2C1SIF is thrown, which corresponds to a slave interrupt
    I2C2Respond();
    INTClearFlag(INT_I2C2S);
}


*/