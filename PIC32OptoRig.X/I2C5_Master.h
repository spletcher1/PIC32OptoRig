#ifndef __SP_I2C_PROP_H
#define __SP_I2C_PROP_H

void ConfigureI2C5(void);
I2C_RESULT Read8FromI2C5(unsigned char slaveaddress, unsigned char dataaddress, unsigned char *data);
I2C_RESULT Read16FromI2C5(unsigned char slaveaddress, unsigned char dataaddress, unsigned int *data);
I2C_RESULT Read32FromI2C5Backward(unsigned char slaveaddress, unsigned char dataaddress, unsigned int *data);
I2C_RESULT Write8ToI2C5(unsigned char slaveaddress, unsigned char dataaddress, unsigned char data);
I2C_RESULT WriteByte_24LC256(unsigned int address, unsigned char wData, unsigned char delay);

I2C_RESULT ReadByte_24LC256(unsigned int address, unsigned char *data);
I2C_RESULT WriteTimeToRTC(rtc_time_t *sTime);
I2C_RESULT ReadTimeFromRTC(rtc_time_t *ts);
void FreeI2C(void);

#endif

