#include "GlobalIncludes.h"

//Actually in PIC32MX795F512L uC the UARTs numbering is like this one:

//UART1A = UART1
//UART2A = UART3
//UART3A = UART2
//UART1B = UART4
//UART2B = UART6
//UART3B = UART5

#define COMMANDBUFFERSIZE (MAXPROGRAMSTEPS*13)+10

unsigned char commandBuffer[COMMANDBUFFERSIZE];
unsigned char cobsBuffer[COMMANDBUFFERSIZE+5];
unsigned int cobsBufferLength;
unsigned int packetSize=0;
unsigned char packetReceived=0;
extern struct FullProgram theProgram;
extern errorFlags_t ErrorsRegister;

/////////////////////////////////////////////////////////////////
// Select one to specify configuration.
// Baud Rate 19200
// Baud rate 38400
// Baud rate 115200
// #define TARGET_BAUD_RATE  921600
#define TARGET_BAUD_RATE  19200

/////////////////////////////////////////////////////////////////
void inline DisableUARTInterrupts(){
    INTEnable(INT_U1RX,INT_DISABLED);
	INTEnable(INT_U1E,INT_DISABLED);    
}

void inline EnableUARTInterrupts(){
    INTEnable(INT_U1RX,INT_ENABLED);
	INTEnable(INT_U1E,INT_ENABLED);  
    INTClearFlag(INT_U1RX);
    INTClearFlag(INT_U1TX); 
}


void inline SendByte(unsigned char c){  
    while (!UARTTransmitterIsReady(UART1));
        UARTSendDataByte(UART1, c);    
}

void SendByteArray(unsigned char* array, const int len){
    int i;
    for(i=0;i<len;i++) {
        SendByte(array[i]);        
    }
}

void SendString(unsigned char* array){
    while (*array) {
        // Transmit a byte
        SendByte(*array++);
    }
}

void SendVersionInformation(){
  SendString(CURRENTVERSION);
}


void SendAcknowledgement(){
  SendByte(0xFE); //ID=254 refers to the UART controller/master.
  SendByte(ErrorsRegister.byte);
  SendByte(0x00);
}

void SendShortInt(int a){
  unsigned char c;  
  c = a>>8;
  while (!UARTTransmitterIsReady(UART1));
       UARTSendDataByte(UART1, c);
  c = a;
  while (!UARTTransmitterIsReady(UART1));
       UARTSendDataByte(UART1, c); 
}

void SendInt(int a){
  unsigned char c; 
  c = a>>24;
  while (!UARTTransmitterIsReady(UART1));
       UARTSendDataByte(UART1, c);
  c = a>>16;
  while (!UARTTransmitterIsReady(UART1));
       UARTSendDataByte(UART1, c);
  c = a>>8;
  while (!UARTTransmitterIsReady(UART1));
       UARTSendDataByte(UART1, c);
  c = a;
  while (!UARTTransmitterIsReady(UART1));
       UARTSendDataByte(UART1, c);  
}

void ConfigureUART1Interrupts(){
	// For now we interrupt on RX
	INTSetVectorPriority(INT_UART_1_VECTOR,INT_PRIORITY_LEVEL_5);
	INTClearFlag(INT_U1RX);
    INTClearFlag(INT_U1TX);    
	INTEnable(INT_U1RX,INT_ENABLED);
	INTEnable(INT_U1E,INT_ENABLED);
}

void ConfigureUART1(void) {
    // Note: As of now, the baud rate set for the parallax RFID reader is 2400.
    // Data bits = 8; no parity; stop bits = 1;

    UARTConfigure(UART1, UART_ENABLE_HIGH_SPEED);
    UARTSetFifoMode(UART1, UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART1, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART1, GetPeripheralClock(), TARGET_BAUD_RATE);
    UARTEnable(UART1,UART_ENABLE_FLAGS(UART_ENABLE | UART_TX | UART_RX | UART_PERIPHERAL));

    ConfigureUART1Interrupts();
    packetSize = 0;       
}

// Interestingly, this interrupt seems to be called 9 times to process
// a 9 Byte status packet!  The duration of each interrupt call is about 1us, with
// O3 optimization.

// Additional timings revealed that it takes about 570us to receive the status request
// and then complete sending the response package. This seems well within the needed
// interval. This timing could easily handle even 100 DFM.
void __ISR(_UART1_VECTOR, IPL5AUTO) UART1Interrupt(void){
	int error;
	unsigned char data;
	error = UART1GetErrors();	
	if (error > 0) {
		if (error & 0x01) { //Overflow Error
			U2STAbits.OERR = 0;
			INTClearFlag(INT_U1E);
			while(DataRdyUART1())            
                data = UARTGetDataByte(UART1);
            packetSize=0;
            INTClearFlag(INT_U1RX);	           
            ErrorsRegister.bits.UARTRegisterOverflowError=1;
            return;
		}
		else if(error & 0x02) {
			U2STAbits.FERR=0;
			while(DataRdyUART1())            
                data = UARTGetDataByte(UART1);			
            packetSize=0;
			INTClearFlag(INT_U1E);
            ErrorsRegister.bits.UARTFrameError=1;
            INTClearFlag(INT_U1RX);	
            return;
		}	
        else {
            ErrorsRegister.bits.DeveloperBit2=1;
            while(DataRdyUART1())            
                data = UARTGetDataByte(UART1);            
            UART1ClearAllErrors();            
            INTClearFlag(INT_U1E);
            INTClearFlag(INT_U1RX);	
        }
	}	
    while(DataRdyUART1()) {
		data=UARTGetDataByte(UART1);          
        if(data == endChar)
            packetReceived=1;
        else{
            commandBuffer[packetSize++] = data; // Store your byte in the FIFO !.
            if(packetSize >=(COMMANDBUFFERSIZE-1)) {
                ErrorsRegister.bits.UARTBufferOverFlowError=1;
                packetSize=0; // Reset even though it will be chaotic.
            }
        }       
    }
    
    INTClearFlag(INT_U1RX);		
}

void ProcessCommandBuffer() {
  if(packetSize<5) {
    // THese small packets are not COBS encoded.
    if (commandBuffer[0] != controllerID & commandBuffer[0]!=255) {
      packetSize=0;
      return; // Ignore commands for other controllers.
     }
    
    if(commandBuffer[1]==programRequestChar) {
      SendProgramData();
    }
    else if(commandBuffer[1]==sendProgramStatusChar) {
      SendProgramStatus();
    }
    else if(commandBuffer[1]==stopProgramChar) {
      StopProgram();
      SendAcknowledgement();
    }
    else if(commandBuffer[1]==stageProgramChar) {
      StageProgram();
      SendAcknowledgement();
    }
    else if(commandBuffer[1]==clearProgramChar) {
      ClearProgram();
      SendAcknowledgement();
    }
    else if(commandBuffer[1]==saveProgramChar) {
      SaveProgram_24LC256();
      SendAcknowledgement();
    }
    else if(commandBuffer[1]==loadProgramChar) {
      LoadProgram();
      SendAcknowledgement();
    }
    else if(commandBuffer[1]==sendVersionChar) {
      SendVersionInformation();
    }
    else if(commandBuffer[1]==sendTimeChar) {
      SendTimeInformation();
    }
    else if(commandBuffer[1]==sendUpdateProgramChar) {
      UpdateProgram();
      SendAcknowledgement();
    }
    else if(commandBuffer[1]==resetErrorCodesChar) {
      ErrorsRegister.byte=0;
      SendAcknowledgement();
    }
    else if(commandBuffer[1]==sendErrorCodesChar) {
      SendAcknowledgement();
    }
    else {
      ErrorsRegister.bits.CommandNotFoundError=1;
      SendAcknowledgement();
    }
  }
  else {
    // The second bye of the encoded array is always the ID.
    // So don't decode unless needed.
    if (commandBuffer[1] != controllerID & commandBuffer[1]!=255) {
      packetSize=0;
      return; // Ignore commands for other controllers.
    }
    cobsBufferLength=decodeCOBS(commandBuffer,packetSize,cobsBuffer);
    if(cobsBuffer[1] == receiveProgramChar){
      LoadProgramFromUART(cobsBuffer, cobsBufferLength);
      SendAcknowledgement();
    }
    else if(cobsBuffer[1]==receiveAdditionalStepsChar){
      //AddStepsFromUART(cobsBuffer, cobsBufferLength);
      SendAcknowledgement();
    }
    else if(cobsBuffer[1]==setRTCTimeChar){
      SetRTCFromUART(cobsBuffer,cobsBufferLength);
      SendAcknowledgement();
    }
  }
  
  packetSize=0; // Clear the command buffer. 
}


unsigned int encodeCOBS(unsigned char* buffer,unsigned int bytesToEncode, unsigned char* encodedBuffer)
    {
        unsigned int read_index  = 0;
        unsigned write_index = 1;
        unsigned int code_index  = 0;
        unsigned char coded = 1;

        while (read_index < bytesToEncode)
        {

            if (buffer[read_index] == 0)
            {
                encodedBuffer[code_index] = coded;
                coded = 1;
                code_index = write_index++;
                read_index++;
            }
            else
            {
                encodedBuffer[write_index++] = buffer[read_index++];
                coded++;
                if (coded == 0xFF)
                {
                    encodedBuffer[code_index] = coded;
                    coded = 1;
                    code_index = write_index++;
                }
            }
        }
        encodedBuffer[code_index] = coded;
        return write_index;
    }

unsigned int decodeCOBS(volatile unsigned char* encodedBuffer,unsigned int bytesToEncode, unsigned char* decodedBuffer)
    {
        unsigned int read_index  = 0;
        unsigned int write_index = 0;
        unsigned char coded = 0;
        unsigned char i =  0;

        if (bytesToEncode == 0)
            return 0;

        while (read_index < bytesToEncode)
        {
            coded = encodedBuffer[read_index];
            if (read_index + coded > bytesToEncode && coded != 1)
            {
                return 0;
            }
            read_index++;
            for (i = 1; i < coded; i++)
            {
                decodedBuffer[write_index++] = encodedBuffer[read_index++];
            }
            if (coded != 0xFF && read_index != bytesToEncode)
            {
                decodedBuffer[write_index++] = '\0';
            }
        }
        return write_index;
    }
