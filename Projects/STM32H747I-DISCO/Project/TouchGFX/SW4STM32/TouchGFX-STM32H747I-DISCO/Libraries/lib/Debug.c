#include "Config.h"
#include "Debug.h"
#include "stdbool.h"
#include "RealTimeClock.h"
#include "Storage.h"
#include "../Unit.h"

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


/**
  * @brief  Save debug information to a file.
  * @retval None
  */
void writeDebug(char debug_info[], bool writeIni) {
    /* Wait 10000 ticks for the shared resource to become free */
    if (xSemaphoreTake(StorageSemaphore, 10000) != pdTRUE)
        return;

    FILINFO file_info;
    FIL outcome_file;
    FIL ini_file;

    char read_buffer[512] = {0};
    char result[512] = {0};

     //Relative time change from now till now
     long long timestamp = rtcGetUnixTime() - RTC_UNIX_START_TIME;

     int days = round(timestamp / (86400));
     int hours = round(timestamp/ 3600 % 24);
     int minutes = round(timestamp / 60 % 60);
     int seconds =  round(timestamp % 60);

     snprintf(result, sizeof(result), "%i days, %02dHH:%02dMM:%02dSS ", days, hours, minutes, seconds);

    strcat(result, debug_info);
    strcat(result, "\n");

    uint32_t number_of_bytes_read = 0;

    if (writeIni) {
        if (f_open(&ini_file, "0://SETTINGS.INI", FA_READ|FA_OPEN_ALWAYS) == FR_OK) {
        	strcat(result, "{");
        	f_read(&ini_file, read_buffer, 512, (UINT * ) & number_of_bytes_read);
        	strcat(result, read_buffer);
            f_close(&ini_file);
            strcat(result, "}\n\n");
        }
    }

    int months_running = floor(timestamp / (86400) / 30);

    /* Create directory if it doesn't exist */
    if (f_stat("0://DEBUG", &file_info) != FR_OK) {
        f_mkdir("0://DEBUG");
    }

    char file_name[64] = "0://DEBUG/debug_report";
    snprintf(file_name, 64, "0://DEBUG/%.2d.TXT", months_running);
    /* Try to open the file */
    if (f_open(&outcome_file, file_name, FA_WRITE | FA_OPEN_APPEND) != FR_OK) {
        debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "Failed to open outcome file\r\n");
        return;
    }

    uint16_t length = snprintf(file_buffer, sizeof(file_buffer), result);
    uint32_t number_of_bytes_written = 0;

    /* Write the result to the file and close it afterwards */
    if (f_write(&outcome_file, file_buffer, length, (UINT * ) & number_of_bytes_written) != FR_OK) {
        debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "Failed to write to outcome file\r\n");
    }


    f_close(&outcome_file);
    /* Free the shared resource */
    xSemaphoreGive(StorageSemaphore);
}
#endif
