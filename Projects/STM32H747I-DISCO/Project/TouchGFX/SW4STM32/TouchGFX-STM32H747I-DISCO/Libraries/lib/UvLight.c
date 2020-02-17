#include "UvLight.h"
#include "Debug.h"

ADC_HandleTypeDef adc_handle;
DMA_HandleTypeDef adc_dma_handle;
static ADC_ChannelConfTypeDef adc_config;

/**
  * @brief  Initialize the UV light sensor and lights
  * @param  None
  * @retval None
  */
void uvLightInitialization(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

    /* Enable ADC clock */
    UV_LIGHT_SENSOR_ADC_ENABLE_CLOCK();

    /* Enable DMA clock */
    UV_LIGHT_SENSOR_DMA_ENABLE_CLOCK();

	/* Enable clocks */
	UV_LIGHT_1_PIN_ENABLE_CLOCK();
	UV_LIGHT_2_PIN_ENABLE_CLOCK();
	UV_LIGHT_SENSOR_ADC_PIN_ENABLE_CLOCK();

	/* ADC Periph interface clock configuration */
	__HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_CLKP);

	/* Configure GPIO pins */
	GPIO_InitStruct.Pin = UV_LIGHT_1_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(UV_LIGHT_1_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = UV_LIGHT_2_PIN;
	HAL_GPIO_Init(UV_LIGHT_2_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UV_LIGHT_SENSOR_ADC_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(UV_LIGHT_SENSOR_ADC_PORT, &GPIO_InitStruct);

    /* Switch off UV Lights */
    uvLight1Off();
    uvLight2Off();

    /* Configure ADC */
	adc_handle.Instance = UV_LIGHT_SENSOR_ADC;
	adc_handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;						/* Asynchronous clock mode, input ADC clock divided by 2 */
	adc_handle.Init.Resolution = ADC_RESOLUTION_16B;							/* 16-bit resolution for converted data */
	adc_handle.Init.ScanConvMode = DISABLE;										/* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
	adc_handle.Init.ContinuousConvMode = DISABLE;								/* Continuous mode disabled to have only 1 conversion at each conversion trig */
	adc_handle.Init.DiscontinuousConvMode = DISABLE;							/* Parameter discarded because sequencer is disabled */
	adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;		/* Parameter discarded because trigger of conversion by software start (no external event) */
	adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;						/* Software start to trigger the 1st conversion manually, without external event */
	adc_handle.Init.NbrOfConversion = 1;										/* Parameter discarded because sequencer is disabled */
	adc_handle.Init.NbrOfDiscConversion = 1;
	adc_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;							/* EOC flag picked-up to indicate conversion end */
	adc_handle.Init.LowPowerAutoWait = DISABLE;                         		/* Auto-delayed conversion feature disabled */
	adc_handle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
	adc_handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;        					/* DR register is overwritten with the last conversion result in case of overrun */
	adc_handle.Init.OversamplingMode = DISABLE;                         		/* No oversampling */
	/* Initialize ADC handle */
	if (HAL_ADC_Init(&adc_handle) != HAL_OK)
	{
		debug(DEBUG_GROUP_UV_LIGHT, DEBUG_LEVEL_ERROR, "Uv light ADC initialization error\r\n");
	}

	adc_config.Channel = UV_LIGHT_SENSOR_ADC_CHANNEL;
	adc_config.Rank = ADC_REGULAR_RANK_1;
	adc_config.SamplingTime = ADC_SAMPLETIME_32CYCLES_5;
	adc_config.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
	adc_config.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */

	if (HAL_ADC_ConfigChannel(&adc_handle, &adc_config) != HAL_OK)
	{
		debug(DEBUG_GROUP_UV_LIGHT, DEBUG_LEVEL_ERROR, "Uv light ADC configuration error\r\n");
	}

	/* Run the ADC calibration in single-ended mode */
	if (HAL_ADCEx_Calibration_Start(&adc_handle, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
	{
		debug(DEBUG_GROUP_UV_LIGHT, DEBUG_LEVEL_ERROR, "Uv light ADC calibration error\r\n");
	}
}

/**
  * @brief  Turn on the first UV lamp
  * @param  None
  * @retval None
  */
void uvLight1On(void)
{
	HAL_GPIO_WritePin(UV_LIGHT_1_GPIO_PORT, UV_LIGHT_1_PIN, GPIO_PIN_SET);
}

/**
  * @brief  Turn off the first UV lamp
  * @param  None
  * @retval None
  */
void uvLight1Off(void)
{
	HAL_GPIO_WritePin(UV_LIGHT_1_GPIO_PORT, UV_LIGHT_1_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  Is the first UV lamp on?
  * @param  None
  * @retval 0: UV lamp off, 1: UV lamp on
  */
uint8_t isUvLight1On(void)
{
	return ((UV_LIGHT_1_GPIO_PORT->ODR & UV_LIGHT_1_PIN) == (uint32_t)GPIO_PIN_SET);
}

/**
  * @brief  Turn on the second UV lamp
  * @param  None
  * @retval None
  */
void uvLight2On(void)
{
	HAL_GPIO_WritePin(UV_LIGHT_2_GPIO_PORT, UV_LIGHT_2_PIN, GPIO_PIN_SET);
}

/**
  * @brief  Turn off the second UV lamp
  * @param  None
  * @retval None
  */
void uvLight2Off(void)
{
	HAL_GPIO_WritePin(UV_LIGHT_2_GPIO_PORT, UV_LIGHT_2_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  Is the second UV lamp on?
  * @param  None
  * @retval 0: UV lamp off, 1: UV lamp on
  */
uint8_t isUvLight2On(void)
{
	return ((UV_LIGHT_2_GPIO_PORT->ODR & UV_LIGHT_2_PIN) == (uint32_t)GPIO_PIN_SET);
}

/**
  * @brief  Get an ADC sample of the UV light sensor
  * @param  None
  * @retval Sample value
  */
uint32_t uvLightSensorGetSample(void)
{
	if(HAL_ADC_Start(&adc_handle) == HAL_OK)
	{
		if(HAL_ADC_PollForConversion(&adc_handle, 10)== HAL_OK)
		{
			return (HAL_ADC_GetValue(&adc_handle));
		}
	}
	return 0;
}
