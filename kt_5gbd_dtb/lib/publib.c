#include <stdio.h>
#include <time.h>
#include "publib.h"

uint64_t get_timestamp_ms(void) 
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        return (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
    }
    return 0;
}

//获取系统开始运行时长信息, 放入第一个参数中.
int getCurrentTime(struct timeval *tv, struct timezone *tz)
{
#if ARM
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / 1000;
#else
	gettimeofday(tv, NULL);
#endif
	return 0;
}

/* 判断是否超时,单位是秒,如果timeout,return 0 */
int BeTimeOut(struct timeval *stv, int s)
{
	struct timeval curtv;
	if (stv == NULL || s < 1)
		return -1;
	if (getCurrentTime(&curtv, NULL))
		return -1;
	if ((curtv.tv_sec - stv->tv_sec > s)
		|| ((curtv.tv_usec >= stv->tv_usec) && (curtv.tv_sec - stv->tv_sec == s))) {
		stv->tv_sec = curtv.tv_sec;
		stv->tv_usec = curtv.tv_usec;
		return 0;
	}
	return 1;
}

int BeTimeOutN(struct timeval stv, int s)
{
	struct timeval curtv;
	if (s < 1)
		return -1;
	if (getCurrentTime(&curtv, NULL))
		return -1;
	if ((curtv.tv_sec - stv.tv_sec > s)
		|| ((curtv.tv_usec >= stv.tv_usec) && (curtv.tv_sec - stv.tv_sec == s))) {
		return 0;
	}
	return 1;
}

int BeTimeOutM(struct timeval *stv, int ms)
{
	struct timeval curtv;
	unsigned long ms1, ms2, ms_run;

	if (stv == NULL || ms < 1)
		return -1;
	if (getCurrentTime(&curtv, NULL))
		return -1;

	ms1 = stv->tv_usec / 1000;
	ms2 = (curtv.tv_sec - stv->tv_sec) * 1000 + curtv.tv_usec / 1000;
	ms_run = ms2 - ms1;

	if (ms_run >= ms) {
		stv->tv_sec = curtv.tv_sec;
		stv->tv_usec = curtv.tv_usec;
		return 0;
	}
	return 1;
}

int BeTimeOutMN(struct timeval *stv, int ms)
{
	struct timeval curtv;
	unsigned long ms1, ms2, ms_run;

	if (stv == NULL || ms < 1)
		return -1;

	if (getCurrentTime(&curtv, NULL))
		return -1;

	ms1 = stv->tv_usec / 1000;
	ms2 = (curtv.tv_sec - stv->tv_sec) * 1000 + curtv.tv_usec / 1000;
	ms_run = ms2 - ms1;

	if (ms_run >= ms) {
		return 0;
	}
	return 1;
}

//计算时间差函数
uint32_t calcTimeOff(struct timeval end, struct timeval begin)
{
	struct timeval tv;
	uint32_t rtt;

	tv.tv_sec = end.tv_sec - begin.tv_sec;
	tv.tv_usec = end.tv_usec - begin.tv_usec;
	if (tv.tv_usec < 0) {
		tv.tv_sec--;
		tv.tv_usec += 1000000;
	}

	rtt = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	return rtt;
}


char *GetSysTimeStr(int isWithDate)
{
	static char tmpBuf[4][32];
	static uint8_t pos;
	char *p;
	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	pos = pos%4;
	p = &tmpBuf[pos++][0];

	if(isWithDate) {
		sprintf(p, "%04d-%02d-%02d %02d:%02d:%02d",
				timeinfo->tm_year+1900,timeinfo->tm_mon+1,timeinfo->tm_mday,
				timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	} else {
		sprintf(p, "%02d:%02d:%02d",
				timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	}

	return p;
}

uint16_t checkSum_8(uint8_t *buf, uint16_t len) //buf为数组，len为数组长度
{
    uint8_t i;
    uint16_t ret = 0;
    for(i=0; i<len; i++)
    {
        ret += *(buf++);
    }
//    ret = ~ret;
    return ret;
}


void get_system_time(Time *t)
{
    time_t rawtime;
	struct tm *timeinfo;

    if (t == NULL) {
        return;
    }

	time (&rawtime);
	timeinfo = localtime(&rawtime);

    t->usYear = timeinfo->tm_year + 1900;
    t->ucMonth = timeinfo->tm_mon + 1;
    t->ucDay = timeinfo->tm_mday;
    t->ucHour = timeinfo->tm_hour;
    t->ucMinute = timeinfo->tm_min;
    t->ucSecond = timeinfo->tm_sec;
}