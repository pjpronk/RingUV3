#include "MillisecondTimer.h"

/**
  * @brief  Initialize the millisecond timer
  * @param  None
  * @retval None
  */
void millisecondTimerInitialization(void)
{
	/* Configure the Systick interrupt time (1ms) */
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
	/* Configure the Systick */
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
  * @brief  Get the time passed since a start time
  * @param  milliseconds_start: start time used as reference
  * @retval Time passed in milliseconds
  */
uint32_t millisecondGetTimePassed(uint32_t milliseconds_start)
{
	return (millisecondGetTime() - milliseconds_start);
}

/**
  * @brief  Delay in milliseconds
  * @param  milliseconds: number of milliseconds to delay
  * @retval None
  */
void millisecondDelay(uint32_t milliseconds)
{
	uint32_t start_time = millisecondGetTime();

	while(millisecondGetTimePassed(start_time) < milliseconds);
}

/**
  * @brief  Get time in milliseconds since start
  * @param  None
  * @retval Milliseconds since start
  */
uint32_t millisecondGetTime(void)
{
	return (uint32_t)HAL_GetTick();
}

