#include "Config.h"

uint32_t adc_buffer[ADC_RANK_NUMBERS - 1] = {0};

/**
  * @brief  Swap the bytes of a specific memory block
  * @param  *object: pointer to object (start of memory block)
  * 		size: number of bytes to swap
  * @retval None
  */
void swapBytes(void* object, size_t size)
{
	unsigned char *start, *end;

	if(!IS_BIG_ENDIAN())
	{
		for(start = (unsigned char *)object, end = start + size - 1; start < end; ++start, --end)
		{
			unsigned char swap = *start;
			*start = *end;
			*end = swap;
		}
	}
}
