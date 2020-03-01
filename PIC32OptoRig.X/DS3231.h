#ifndef DS3231_H
#define DS3231_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t weekday;
    uint8_t monthday;
    uint8_t month;
    uint8_t year;
} rtc_time_t;

void GetLocalTimeString(char *st);
void SetRTC(void);
void InitRTC(void);
void StepRTC(void);
void SendTimeInformation(void);
long int GetSecondsFromMidnight(void);
long time_date_to_epoch(rtc_time_t *ts );
void SetRTCFromUART(unsigned char* buffer, unsigned int len);













#endif