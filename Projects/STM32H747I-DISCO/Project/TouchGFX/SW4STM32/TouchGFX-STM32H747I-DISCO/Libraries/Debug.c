#include "Config.h"
#include "Debug.h"

#if (DEBUG_OUTPUT == DEBUG_SERIAL)
UART_HandleTypeDef debug_handle;

/**
  * @brief  Override putchar to transmit characters over the UART
  * @param  ch: character to be transmitted
  * @retval Character
  */
int __io_putchar(int ch)
{
	HAL_UART_Transmit(&debug_handle, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}

/**
  * @brief  Override write to transmit data over the UART
  * @param  file: N/A
  * 		*ptr: pointer to data buffer
  * 		len: number of bytes to be transmitted
  * @retval Number of bytes written
  */
int _write(int file, char *ptr, int len)
{
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++){ __io_putchar( *ptr++ );}
	return len;
}

/**
  * @brief  Initialize the debug UART
  * @param  None
  * @retval None
  */
void debugSerialInitilization(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	DEBUG_UART_ENABLE_CLOCK();
	DEBUG_TX_PIN_ENABLE_CLOCK();

	/* Configure GPIO pin */
	GPIO_InitStruct.Pin = DEBUG_TX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = DEBUG_TX_AF;
	HAL_GPIO_Init(DEBUG_TX_GPIO_PORT, &GPIO_InitStruct);

	/* Configure UART handle */
	debug_handle.Instance        = DEBUG_UART;
	debug_handle.Init.BaudRate 	 = 230400;
	debug_handle.Init.WordLength = UART_WORDLENGTH_8B;
	debug_handle.Init.StopBits   = UART_STOPBITS_1;
	debug_handle.Init.Parity     = UART_PARITY_NONE;
	debug_handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	debug_handle.Init.Mode       = UART_MODE_TX;

	if(HAL_UART_Init(&debug_handle) != HAL_OK)
	{
		//Do stuff
	}
}
#endif
