
#ifndef __SP_UART1_H
#define __SP_UART1_H

#define endChar 0x00
#define programRequestChar 0x01
#define stageProgramChar 0x02
#define stopProgramChar 0x03
#define clearProgramChar 0x04
#define saveProgramChar 0x05
#define loadProgramChar 0x06
#define sendVersionChar 0x07
#define sendTimeChar 0x08
#define sendProgramStatusChar 0x09
#define receiveProgramChar 0x0A
#define receiveAdditionalStepsChar 0x0B
#define sendUpdateProgramChar 0x0C
#define sendErrorCodesChar 0x0D
#define resetErrorCodesChar 0x0E
#define setRTCTimeChar 0x0F

#define acknowledgeChar 0xFF


typedef union {
    unsigned char byte;
    struct ErrorBits {
        unsigned UARTFrameError : 1;
        unsigned UARTBufferOverFlowError : 1;
        unsigned UARTRegisterOverflowError : 1;
        unsigned CommandNotFoundError : 1;
        unsigned TooManyStepsError : 1;
        unsigned I2CError : 1;
        unsigned RTCError : 1;
        unsigned DeveloperBit2 : 1;
    } bits;
} errorFlags_t;


void ConfigureUART1(void);
void SendString(unsigned char* array);
void ProcessPacket();
void SendByte(unsigned char n);
void SendByteArray(unsigned char* array, int len);
unsigned int encodeCOBS(unsigned char* buffer,unsigned int bytesToEncode, unsigned char* encodedBuffer);
unsigned int decodeCOBS(volatile unsigned char* encodedBuffer,unsigned int bytesToEncode, unsigned char* decodedBuffer);
void ProcessCommandBuffer();
void SendAcknowledgement();
#endif
