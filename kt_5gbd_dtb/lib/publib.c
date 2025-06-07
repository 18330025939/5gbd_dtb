#include <stdio.h>
#include <time.h>
#include <string.h>
#include "publib.h"

#if 0
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
#endif
void get_system_time(CustomTime *t)
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

int _system_(const char *cmd, char *pRetMsg, int msg_len)
{

	FILE *fp;
	int ret = -1;

	if (cmd == NULL)
		return -1;

	if ((fp = popen(cmd, "r")) == NULL)
		return -2;
	else {
        if (pRetMsg != NULL) {
            memset(pRetMsg, 0, msg_len);
            do{}
            while (fgets(pRetMsg, msg_len, fp) != NULL);
        }
	}

	if ((ret = pclose(fp)) == -1)
		return -3;

    if (pRetMsg != NULL) {
	    pRetMsg[strlen(pRetMsg)-1] = '\0';
    }
	return 0;
}

void db_to_bcd(uint16_t value, uint16_t* bcd_value)
{
    uint8_t high_byte = value / 100;
    uint8_t low_byte = value % 100;

    *bcd_value = (uint16_t)(((high_byte / 10) << 4) | (high_byte % 10)) << 8 |
                (uint16_t)(((low_byte / 10) << 4) | (low_byte % 10));
}

int byte_to_bcd(uint8_t value)
{
	int bcd = 0;

	bcd = (((value >> 4) & 0x0f) * 10 + (value & 0x0f));

	return bcd;
}

uint8_t bcd_to_byte(uint8_t bcd)
{
	uint8_t value = 0;

	value = (((bcd / 10) << 4) | (bcd % 10));

	return value;
}

int hex_to_bcd(uint16_t value)
{
	int bcd = 0;

	bcd = (((value >> 12) & 0x0f) * 1000 +
		   ((value >> 8) & 0x0f) * 100 +
		   ((value >> 4) & 0x0f) * 10 +
		   (value & 0x0f));
	return bcd;
}
