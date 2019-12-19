#include <stdio.h>
#include "ff.h"
#include "RealTimeClock.h"
#include "Debug.h"



#include "minIni.h"

static RTC_HandleTypeDef rtc_handle;

static void rtcDateTimeConfiguration(void);

/**
  * @brief  Initialize the internal Real Time Clock (RTC)
  * @param  None
  * @retval None
  */
void rtcInitialization(void)
{
	RCC_OscInitTypeDef rcc_osc_init_struct;
	RCC_PeriphCLKInitTypeDef periph_clk_init_struct;

	/* Enable write access */
	HAL_PWR_EnableBkUpAccess();

	/* Configure the RTC clock source */
	rcc_osc_init_struct.OscillatorType =  RCC_OSCILLATORTYPE_LSE;
	rcc_osc_init_struct.PLL.PLLState = RCC_PLL_NONE;
	rcc_osc_init_struct.LSEState = RCC_LSE_ON;
	if(HAL_RCC_OscConfig(&rcc_osc_init_struct) != HAL_OK)
	{
		debug(DEBUG_GROUP_RTC, DEBUG_LEVEL_ERROR, "RTC RCC initialization error\r\n");
	}

	periph_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	periph_clk_init_struct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	if(HAL_RCCEx_PeriphCLKConfig(&periph_clk_init_struct) != HAL_OK)
	{
		debug(DEBUG_GROUP_RTC, DEBUG_LEVEL_ERROR, "RTC RCC configuration error\r\n");
	}

	/* Enable RTC Clock */
	__HAL_RCC_RTC_ENABLE();

	/* Configure RTC */
	rtc_handle.Instance = RTC;
	rtc_handle.Init.HourFormat = RTC_HOURFORMAT_24;
	rtc_handle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
	rtc_handle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
	rtc_handle.Init.OutPut = RTC_OUTPUT_DISABLE;
	rtc_handle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	rtc_handle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	__HAL_RTC_RESET_HANDLE_STATE(&rtc_handle);
	if (HAL_RTC_Init(&rtc_handle) != HAL_OK)
	{
		debug(DEBUG_GROUP_RTC, DEBUG_LEVEL_ERROR, "RTC initialization error\r\n");
	}

	/* Read the Back Up Register 1 Data */
	if (HAL_RTCEx_BKUPRead(&rtc_handle, RTC_BKP_DR1) != RTC_KEY)
	{
		rtcDateTimeConfiguration();
	}
	else
	{
		/* Check if the Power On Reset flag is set */
		if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
		{
			//Do stuff
		}
		/* Check if Pin Reset flag is set */
		if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
		{
			//Do stuff
		}
		/* Clear source Reset Flag */
		__HAL_RCC_CLEAR_RESET_FLAGS();
	}
	/* Check if the date and time should be set */
	//Tom::ADD29    Default waarde nu 0 gebruikt.
	if(ini_getl(DATE_TIME_SECTION_NAME, DATE_READ_KEY_NAME, 0, SETTINGS_INI_FILE) == 1)
	{

		//TreADD:42
		ini_putl(DATE_TIME_SECTION_NAME, DATE_READ_KEY_NAME, 0, SETTINGS_INI_FILE);

		RtcDate date;
		RtcTime time;
		/* Read the date and time from the settings file */
		date.year = (uint8_t)(ini_getl(DATE_TIME_SECTION_NAME, YEAR_KEY_NAME, 19, SETTINGS_INI_FILE) % 2000);
		date.month = (uint8_t)(ini_getl(DATE_TIME_SECTION_NAME, MONTH_KEY_NAME, 1, SETTINGS_INI_FILE) % 13);
		date.day = (uint8_t)(ini_getl(DATE_TIME_SECTION_NAME, DAY_KEY_NAME, 1, SETTINGS_INI_FILE) % 32);
		time.hours = (uint8_t)(ini_getl(DATE_TIME_SECTION_NAME, HOUR_KEY_NAME, 0, SETTINGS_INI_FILE) % 24);
		time.minutes = (uint8_t)(ini_getl(DATE_TIME_SECTION_NAME, MINUTE_KEY_NAME, 0, SETTINGS_INI_FILE) % 60);
		time.seconds = 0;

		rtcSetDate(&date);
		rtcSetTime(&time);
		debug(DEBUG_GROUP_RTC, DEBUG_LEVEL_ERROR, "RTC date/time has been set to: %d/%d/%d %d:%d\r\n", date.day, date.month, date.year, time.hours, time.minutes);

		RtcDate dater;
		RtcTime timer;

		rtcGetDate(&dater);
		rtcGetTime(&timer);

		debug(DEBUG_GROUP_RTC, DEBUG_LEVEL_ERROR, "RTC date/time read: %d/%d/%d %d:%d:%d\r\n", dater.day, dater.month, dater.year, timer.hours, timer.minutes, timer.seconds);

	}
}

/**
  * @brief  Configure the date and time of the RTC
  * @param  None
  * @retval None
  */
void rtcDateTimeConfiguration(void)
{
	RTC_DateTypeDef date;
	RTC_TimeTypeDef time;

	/* Set date: Monday January 1th 2018 */
	date.Year = 0x18;
	date.Month = RTC_MONTH_JANUARY;
	date.Date = 0x01;
	date.WeekDay = RTC_WEEKDAY_MONDAY;

	if(HAL_RTC_SetDate(&rtc_handle, &date, RTC_FORMAT_BCD) != HAL_OK)
	{
		debug(DEBUG_GROUP_RTC, DEBUG_LEVEL_ERROR, "RTC set date error\r\n");
	}
	/* Set time: 00:00:00 */
	time.Hours = 0x00;
	time.Minutes = 0x00;
	time.Seconds = 0x00;
	time.TimeFormat = RTC_HOURFORMAT12_AM;
	time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
	time.StoreOperation = RTC_STOREOPERATION_RESET;

	if(HAL_RTC_SetTime(&rtc_handle, &time, RTC_FORMAT_BCD) != HAL_OK)
	{
		debug(DEBUG_GROUP_RTC, DEBUG_LEVEL_ERROR, "RTC set time error\r\n");
	}
	/* Write the RTC key to the first backup drive to indicate the RTC has been configured */
	HAL_RTCEx_BKUPWrite(&rtc_handle, RTC_BKP_DR1, RTC_KEY);
}

/**
  * @brief  Get the time passed since a start time
  * @param  seconds_start: start time used as reference
  * @retval Time passed in seconds
  */
uint32_t rtcGetTimePassed(uint32_t seconds_start)
{
	return (rtcGetUnixTime() - seconds_start);
}

/**
  * @brief  Delay in seconds
  * @param  seconds: number of seconds to delay
  * @retval None
  */
void rtcDelay(uint32_t seconds)
{
	uint32_t start_time = rtcGetUnixTime();

	while(rtcGetTimePassed(start_time) < seconds);
}


//Tom:ADD23
/**
  * @brief  Get time in seconds since 1970 (UNIX time)
  * @param  None
  * @retval String readable date and time string.
  */

TCHAR * rtcGetUnixTimeString(TCHAR *str, int32_t len)
{
	RTC_DateTypeDef date;
	RTC_TimeTypeDef time;
	//char str[20];

	//  2019-11-26 10-53-22
	//  1234567890123456789 0x0   dus 20 lang.


	/* The order of calling the date and time is very important! */
	/* The get time should be called before the get date! */
	HAL_RTC_GetTime(&rtc_handle, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&rtc_handle, &date, RTC_FORMAT_BIN);

	uint16_t year = (uint16_t)(date.Year + 2000);

	if(year < RTC_UNIX_START_YEAR)
	{
		strcpy( str, "FAIL2GETDATE");
	}

	str = itoa(year,str,10);

	snprintf(str, len, "%04d-%02d-%02d %02d:%02d:%02d", year, date.Month, date.Date, time.Hours, time.Minutes, time.Seconds );

	return str;
}



/**
  * @brief  Get time in seconds since 1970 (UNIX time)
  * @param  None
  * @retval Seconds since 1970 (UNIX time)
  */
uint32_t rtcGetUnixTime(void)
{
	RTC_DateTypeDef date;
	RTC_TimeTypeDef time;

	/* The order of calling the date and time is very important! */
	/* The get time should be called before the get date! */
	HAL_RTC_GetTime(&rtc_handle, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&rtc_handle, &date, RTC_FORMAT_BIN);

	uint32_t days = 0;
	uint16_t year = (uint16_t)(date.Year + 2000);

	if(year < RTC_UNIX_START_YEAR)
		return 0;
	/* Days in previous years */
	for(uint16_t i = RTC_UNIX_START_YEAR; i < year; i++)
		days += RTC_DAYS_IN_YEAR(i);
	/* Days in current year */
	for (uint8_t i = 1; i < date.Month; i++)
		days += RTC_DAYS_IN_MONTH[RTC_LEAP_YEAR(year)][i - 1];
	/* Day starts with 1 */
	days += date.Date - 1;
	uint32_t seconds = (days * RTC_SECONDS_PER_DAY);
	seconds += (time.Hours * RTC_SECONDS_PER_HOUR);
	seconds += (time.Minutes * RTC_SECONDS_PER_MINUTE);
	seconds += time.Seconds;

	return seconds;
}

/**
  * @brief  Set the RTC time
  * @param  *rtc_time: RTC time structure
  * @retval None
  */
void rtcSetTime(RtcTime* rtc_time)
{
	RTC_TimeTypeDef time;
	/* Set time */
	time.Hours = rtc_time->hours;
	time.Minutes = rtc_time->minutes;
	time.Seconds = rtc_time->seconds;
	time.TimeFormat = RTC_HOURFORMAT12_AM;
	time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
	time.StoreOperation = RTC_STOREOPERATION_RESET;

	if(HAL_RTC_SetTime(&rtc_handle, &time, RTC_FORMAT_BIN) != HAL_OK)
	{
		debug(DEBUG_GROUP_RTC, DEBUG_LEVEL_ERROR, "RTC set time error\r\n");
	}
}

/**
  * @brief  Get the RTC time
  * @param  *rtc_time: RTC time structure
  * @retval None
  */
void rtcGetTime(RtcTime* rtc_time)
{
	RTC_DateTypeDef date;
	RTC_TimeTypeDef time;
	/* The order of calling the date and time is very important! */
	/* The get time should be called before the get date! */
	HAL_RTC_GetTime(&rtc_handle, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&rtc_handle, &date, RTC_FORMAT_BIN);

	rtc_time->hours = time.Hours;
	rtc_time->minutes = time.Minutes;
	rtc_time->seconds = time.Seconds;
}

/**
  * @brief  Set the RTC date
  * @param  *rtc_date: RTC date structure
  * @retval None
  */
void rtcSetDate(RtcDate* rtc_date)
{
	RTC_DateTypeDef date;
	/* Set date */
	date.Year = rtc_date->year;
	date.Month = rtc_date->month;
	date.Date = rtc_date->day;
	date.WeekDay = RTC_WEEKDAY_MONDAY;

	if(HAL_RTC_SetDate(&rtc_handle, &date, RTC_FORMAT_BIN) != HAL_OK)
	{
		debug(DEBUG_GROUP_RTC, DEBUG_LEVEL_ERROR, "RTC set date error\r\n");
	}
}

/**
  * @brief  Get the RTC date
  * @param  *rtc_date: RTC date structure
  * @retval None
  */
void rtcGetDate(RtcDate* rtc_date)
{
	RTC_DateTypeDef date;
	RTC_TimeTypeDef time;
	/* The order of calling the date and time is very important! */
	/* The get time should be called before the get date! */
	HAL_RTC_GetTime(&rtc_handle, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&rtc_handle, &date, RTC_FORMAT_BIN);

	rtc_date->year = date.Year;
	rtc_date->month = date.Month;
	rtc_date->day = date.Date;
}



