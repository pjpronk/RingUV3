#ifndef REALTIMECLOCK_H_
#define REALTIMECLOCK_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "Config.h"

#define RTC_ASYNCH_PREDIV  	0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV	0x00FF /* LSE as RTC clock */
#define RTC_KEY				0x32F2

#define RTC_LEAP_YEAR(year)		((((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0))
#define RTC_DAYS_IN_YEAR(x)		RTC_LEAP_YEAR(x) ? 366 : 365
#define RTC_SECONDS_PER_DAY   	86400
#define RTC_SECONDS_PER_HOUR  	3600
#define RTC_SECONDS_PER_MINUTE	60

#define RTC_UNIX_START_YEAR		1970

const char DATE_TIME_SECTION_NAME[] = "Date";
const char YEAR_KEY_NAME[] = "Year";
const char MONTH_KEY_NAME[] = "Month";
const char DAY_KEY_NAME[] = "Day";
const char HOUR_KEY_NAME[] = "Hour";
const char MINUTE_KEY_NAME[] = "Minute";
const char DATE_READ_KEY_NAME[] = "Date read";

static uint8_t RTC_DAYS_IN_MONTH[2][12] = {
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},	/* Not leap year */
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}	/* Leap year */
};

typedef struct {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
}RtcTime;

typedef struct {
	uint8_t year;
	uint8_t month;
	uint8_t day;
}RtcDate;

void rtcInitialization(void);
uint32_t rtcGetTimePassed(uint32_t seconds_start);
void rtcDelay(uint32_t seconds);
uint32_t rtcGetUnixTime(void);
void rtcSetTime(RtcTime* rtc_time);
void rtcGetTime(RtcTime* rtc_time);
void rtcSetDate(RtcDate* rtc_date);
void rtcGetDate(RtcDate* rtc_date);
//Tom:ADD24
TCHAR * rtcGetUnixTimeString(TCHAR *str, int32_t len);



#ifdef __cplusplus
}
#endif

#endif /* REALTIMECLOCK_H_ */
