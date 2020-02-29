/* 
 * File:   SystemFunctions.h
 * Author: scott
 *
 * Created on March 16, 2013, 10:07 PM
 */

#ifndef SYSTEMFUNCTIONS_H
#define	SYSTEMFUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

void Startup();
void DelayMs(WORD ms);
void Delay10us(DWORD dwCount);

#ifdef	__cplusplus
}
#endif

#endif	/* SYSTEMFUNCTIONS_H */

