#ifndef __SP_I2C_PROP_H
#define __SP_I2C_PROP_H

void ConfigureI2C2(void);
I2C_RESULT Read8FromI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned char *data);
I2C_RESULT Read16FromI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned int *data);
I2C_RESULT Read32FromI2C2Backward(unsigned char slaveaddress, unsigned char dataaddress, unsigned int *data);
I2C_RESULT Write8ToI2C2(unsigned char slaveaddress, unsigned char dataaddress, unsigned char data);

I2C_RESULT RequestMeasureSi7021I2C2(unsigned char slaveaddress, unsigned char dataaddress);
I2C_RESULT ReadHumidityFromSi7021I2C2(unsigned char slaveaddress, unsigned int *data);
void FreeI2C(void);

#endif

