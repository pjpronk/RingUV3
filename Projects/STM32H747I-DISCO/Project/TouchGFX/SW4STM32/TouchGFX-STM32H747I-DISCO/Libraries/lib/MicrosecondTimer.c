#include "MicrosecondTimer.h"

/**
  * @brief  Initialize the microsecond timer
  * @param  None
  * @retval None
  */
void microsecondTimerInitialization(void)
{
	/* Initialize the Data Watchpoint and Trace Registers */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
  * @brief  Get the time passed since a start time
  * @param  microseconds_start: start time used as reference
  * @retval Time passed in microseconds
  */
uint32_t microsecondGetTimePassed(uint32_t microseconds_start)
{
	return (microsecondGetTime() - microseconds_start);
}

/**
  * @brief  Delay in microseconds
  * @param  microseconds: number of microseconds to delay
  * @retval None
  */
void microsecondDelay(uint32_t microseconds)
{
	uint32_t start_time = microsecondGetTime();

	while(microsecondGetTimePassed(start_time) < microseconds);
}

/**
  * @brief  Get time in microseconds since start
  * @param  None
  * @retval Microseconds since start
  */
uint32_t microsecondGetTime(void)
{
	return (uint32_t) (DWT->CYCCNT / (SystemCoreClock / 1000000));
}

