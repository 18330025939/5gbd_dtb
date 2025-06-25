#ifndef __PUB_LIB_H
#define __PUB_LIB_H

#include <stdint.h>
#include <sys/time.h>


#define BSWAP16(x) (uint16_t)((((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00))

#define BSWAP32(x) (uint32_t)( \
    (((x) >> 24) & 0x000000FF) | \
    (((x) >> 8)  & 0x0000FF00) | \
    (((x) << 8)  & 0x00FF0000) | \
    (((x) << 24) & 0xFF000000))


typedef struct st_CustomTime
{
    uint16_t usYear;
    uint8_t  ucMonth;
    uint8_t  ucDay;
    uint8_t  ucHour;
    uint8_t  ucMinute;
    uint8_t  ucSecond;
} CustomTime;

void get_system_time(CustomTime *t);
uint64_t get_timestamp_ms(void);
int getCurrentTime(struct timeval *tv,  struct timezone *tz);
int BeTimeOut(struct timeval *stv, int s);
int BeTimeOutN(struct timeval stv, int s);
int BeTimeOutM(struct timeval *stv, int ms);
int BeTimeOutMN(struct timeval *stv, int ms);
uint32_t calcTimeOff(struct timeval end, struct timeval begin);
char *GetSysTimeStr(int isWithDate);
int _system_(const char *cmd, char *pRetMsg, int msg_len);
void db_to_bcd(uint16_t value, uint16_t* bcd_value);
int bcd_to_byte(uint8_t value);
uint8_t byte_to_bcd(uint8_t bcd);
int bcd_to_db(uint16_t value);
int dir_exists(const char *path);
int is_file_empty(const char *filename);
int file_exists(const char *path);
#endif /* __PUB_LIB_H */

