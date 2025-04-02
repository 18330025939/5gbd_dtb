#ifndef __PUB_LIB_H
#define __PUB_LIB_H

#include <stdint.h>
#include <sys/time.h>


uint64_t get_timestamp_ms(void);
int getCurrentTime(struct timeval *tv,  struct timezone *tz);
int BeTimeOut(struct timeval *stv, int s);
int BeTimeOutN(struct timeval stv, int s);
int BeTimeOutM(struct timeval *stv, int ms);
int BeTimeOutMN(struct timeval *stv, int ms);
uint32_t calcTimeOff(struct timeval end, struct timeval begin);
char *GetSysTimeStr(int isWithDate);
#endif

