#include <stddef.h>
#include "GlobalIncludes.h"

#include <time.h>

rtc_time_t local_time;
rtc_time_t setTime;

unsigned char isRTCInitialized;
errorFlags_t ErrorsRegister;

// Must be called after I2C1Init

void InitRTC() {
    I2C_RESULT tmp;
    //SetRTC();
    //DelayMs(1000);
    tmp=ReadTimeFromRTC(&local_time);    
   
    // If isRTCPresent is 0 it indicated a I2C timeout so its not there.
    // This change is probably not needed, but it is included here for clarity.
    if (tmp==I2C_SUCCESS)
        isRTCInitialized = 1;
    else {
        ErrorsRegister.bits.RTCError = 1;
        isRTCInitialized = 0;
        local_time.seconds = 0;
        local_time.minutes = 0;
        local_time.hours = 0;
        local_time.monthday = 1;
        local_time.month = 1;
        local_time.year = 1;
    }
    setTime.seconds = 0;
    setTime.minutes = 0;
    setTime.hours = 0;
    setTime.weekday = 0;
    setTime.monthday = 1;
    setTime.month = 1;
    setTime.year = 1;   
}

void SetRTCFromUART(unsigned char* buffer, unsigned int len) {
    if (len != 8) {
        ErrorsRegister.bits.RTCError = 1;
        return;
    }
    setTime.seconds = buffer[7];
    setTime.minutes = buffer[6];
    setTime.hours = buffer[5];
    setTime.monthday = buffer[4];
    setTime.month = buffer[3];
    setTime.year = buffer[2];

    if (setTime.year > 30) {
        ErrorsRegister.bits.RTCError = 1;
        return;
    }
    if (setTime.month > 12 || setTime.month < 1) {
        ErrorsRegister.bits.RTCError = 1;
        return;
    }
    if (setTime.monthday > 31 || setTime.monthday < 1) {
        ErrorsRegister.bits.RTCError = 1;
        return;
    }
    if (setTime.hours > 24 || setTime.monthday < 0) {
        ErrorsRegister.bits.RTCError = 1;
        return;
    }
    if (setTime.minutes > 60 || setTime.minutes < 0) {
        ErrorsRegister.bits.RTCError = 1;
        return;
    }
    if (setTime.seconds > 60 || setTime.seconds < 0) {
        ErrorsRegister.bits.RTCError = 1;
        return;
    }
   
    WriteTimeToRTC(&setTime);
    
}

void SetRTC() {
    setTime.seconds = 0;
    setTime.minutes = 36;
    setTime.hours = 22;
    setTime.monthday = 22;
    setTime.month = 11;
    setTime.year = 19;
    WriteTimeToRTC(&setTime);
    setTime.seconds = 0;
    setTime.minutes = 0;
    setTime.hours = 0;
    setTime.monthday = 1;
    setTime.month = 1;
    setTime.year = 1;
}

// Currently not used as the time is only
// checked against RTC every so often during 
// the program run.
void StepRTC() {
    if (isRTCInitialized) {
        ReadTimeFromRTC(&local_time);
    } else {
        local_time.seconds = 0;
        local_time.minutes = 0;
        local_time.hours = 0;
        local_time.monthday = 1;
        local_time.month = 1;
        local_time.year = 1;
    }
}

long int GetSecondsFromMidnight() {
    long int secs;
    if (isRTCInitialized) {
        secs = (local_time.hours * 60) + local_time.minutes;
        secs += ((secs * 60) + local_time.seconds);
        return secs;
    } else {
        return 0;
    }
}

void SendTimeInformation() {
    I2C_RESULT tmp;
    tmp=ReadTimeFromRTC(&local_time);
    // If isRTCPresent is 0 it indicated a I2C timeout so its not there.
    // This change is probably not needed, but it is included here for clarity.
    if (tmp != I2C_SUCCESS) {
        ErrorsRegister.bits.RTCError = 1;
        local_time.seconds = 0;
        local_time.minutes = 0;
        local_time.hours = 0;
        local_time.monthday = 1;
        local_time.month = 1;
        local_time.year = 1;
    }
    SendByte(local_time.year);
    SendByte(local_time.month);
    SendByte(local_time.monthday);
    SendByte(local_time.hours);
    SendByte(local_time.minutes);
    SendByte(local_time.seconds);
}

long time_date_to_epoch(rtc_time_t *ts) {
    struct tm tmVar ={0};    
    long epoch;

    tmVar.tm_sec = ts->seconds;
    tmVar.tm_min = ts->minutes;
    tmVar.tm_hour = ts->hours;
    tmVar.tm_mday = ts->monthday;
    tmVar.tm_mon = ts->month-1;
    tmVar.tm_year = ts->year + 2000 - 1900;   
  
    
    epoch = mktime(&tmVar);
    return epoch;
}

unsigned char Bcd2Dec(uint8_t n) {
    return n - 6 * (n / 16);
}

unsigned char Dec2Bcd(unsigned char n) {
    return n + 6 * (n / 10);
}

unsigned char Bcd2DecOther(unsigned char c) {
    unsigned char dec;

    dec = (c >> 4) & 0x07;
    dec = (dec << 3) + (dec << 1) + (c & 0x0F);

    return dec;
}

