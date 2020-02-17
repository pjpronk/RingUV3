#include "Rgb.h"
#include "cmsis_os.h"
#include "MillisecondTimer.h"
#include "Debug.h"

osMessageQId RgbEvent = 0;
static osThreadId RgbThreadId = 0;
static uint8_t blink_white_led = 0;
static uint8_t white_led_on = 0;

static void rgbWhiteLedBlinkOn(void);
static void rgbWhiteLedBlinkOff(void);
static void Rgb_Thread(void const * argument);

/**
  * @brief  Initialize the RGB hardware
  * @param  None
  * @retval None
  */
void rgbInitialization(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Enable GPIO clocks */
	RGB_LED_R_PIN_ENABLE_CLOCK();
	RGB_LED_G_PIN_ENABLE_CLOCK();
	RGB_LED_B_PIN_ENABLE_CLOCK();

	/* Configure GPIO pins */
	GPIO_InitStruct.Pin = RGB_LED_R_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(RGB_LED_R_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = RGB_LED_G_PIN;
	HAL_GPIO_Init(RGB_LED_G_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = RGB_LED_B_PIN;
	HAL_GPIO_Init(RGB_LED_B_GPIO_PORT, &GPIO_InitStruct);

	/* Switch off the RGB LED */
	rgbAllLedsOff();

	/* Create Rgb Queue */
	osMessageQDef(RGB_Queue, 10, uint16_t);
	RgbEvent = osMessageCreate (osMessageQ(RGB_Queue), NULL);
	/* Create Rgb task */
	osThreadDef(osRgb_Thread, Rgb_Thread, osPriorityLow, 0, 512);
	RgbThreadId = osThreadCreate (osThread(osRgb_Thread), NULL);
}

/**
  * @brief  Red LED on
  * @param  None
  * @retval None
  */
void rgbRedLedOn(void)
{
	blink_white_led = 0;

	/* Green and blue off */
	HAL_GPIO_WritePin(RGB_LED_G_GPIO_PORT, RGB_LED_G_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RGB_LED_B_GPIO_PORT, RGB_LED_B_PIN, GPIO_PIN_RESET);
	/* Red on */
	HAL_GPIO_WritePin(RGB_LED_R_GPIO_PORT, RGB_LED_R_PIN, GPIO_PIN_SET);
}

/**
  * @brief  Is the red LED on?
  * @param  None
  * @retval 0: red LED is off, 1: red LED is on
  */
uint8_t rgbIsRedLedOn(void)
{
	return (HAL_GPIO_ReadPin(RGB_LED_R_GPIO_PORT, RGB_LED_R_PIN) == GPIO_PIN_SET);
}

/**
  * @brief  Green LED on
  * @param  None
  * @retval None
  */
void rgbGreenLedOn(void)
{
	blink_white_led = 0;

	/* Red and blue off */
	HAL_GPIO_WritePin(RGB_LED_R_GPIO_PORT, RGB_LED_R_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RGB_LED_B_GPIO_PORT, RGB_LED_B_PIN, GPIO_PIN_RESET);
	/* Green on */
	HAL_GPIO_WritePin(RGB_LED_G_GPIO_PORT, RGB_LED_G_PIN, GPIO_PIN_SET);
}

/**
  * @brief  Is the green LED on?
  * @param  None
  * @retval 0: green LED is off, 1: green LED is on
  */
uint8_t rgbIsGreenLedOn(void)
{
	return (HAL_GPIO_ReadPin(RGB_LED_G_GPIO_PORT, RGB_LED_G_PIN) == GPIO_PIN_SET);
}

/**
  * @brief  Blue LED on
  * @param  None
  * @retval None
  */
void rgbBlueLedOn(void)
{
	blink_white_led = 0;

	/* Red and green off */
	HAL_GPIO_WritePin(RGB_LED_R_GPIO_PORT, RGB_LED_R_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RGB_LED_G_GPIO_PORT, RGB_LED_G_PIN, GPIO_PIN_RESET);
	/* Blue on */
	HAL_GPIO_WritePin(RGB_LED_B_GPIO_PORT, RGB_LED_B_PIN, GPIO_PIN_SET);
}

void rgbOrangeLedOn(void)
{
	blink_white_led = 0;

	/* Red and green on */
	HAL_GPIO_WritePin(RGB_LED_R_GPIO_PORT, RGB_LED_R_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RGB_LED_G_GPIO_PORT, RGB_LED_G_PIN, GPIO_PIN_SET);
	/* Blue off */
	HAL_GPIO_WritePin(RGB_LED_B_GPIO_PORT, RGB_LED_B_PIN, GPIO_PIN_RESET);
}


/**
  * @brief  Is the blue LED on?
  * @param  None
  * @retval 0: blue LED is off, 1: blue LED is on
  */
uint8_t rgbIsBlueLedOn(void)
{
	return (HAL_GPIO_ReadPin(RGB_LED_B_GPIO_PORT, RGB_LED_B_PIN) == GPIO_PIN_SET);
}

/**
  * @brief  All LEDs on
  * @param  None
  * @retval None
  */
void rgbWhiteLedOn(void)
{
	/* All on */
	HAL_GPIO_WritePin(RGB_LED_R_GPIO_PORT, RGB_LED_R_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RGB_LED_G_GPIO_PORT, RGB_LED_G_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RGB_LED_B_GPIO_PORT, RGB_LED_B_PIN, GPIO_PIN_SET);

	blink_white_led = 0;
}

/**
  * @brief  All LEDs off
  * @param  None
  * @retval None
  */
void rgbAllLedsOff(void)
{
	/* All off */
	HAL_GPIO_WritePin(RGB_LED_R_GPIO_PORT, RGB_LED_R_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RGB_LED_G_GPIO_PORT, RGB_LED_G_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RGB_LED_B_GPIO_PORT, RGB_LED_B_PIN, GPIO_PIN_RESET);

	blink_white_led = 0;
}

/**
  * @brief  Blink all LEDs
  * @param  None
  * @retval None
  */
void rgbWhiteLedBlink(void)
{
	blink_white_led = 1;
}

/**
  * @brief  Blink all LEDs on state
  * @param  None
  * @retval None
  */
static void rgbWhiteLedBlinkOn(void)
{
	HAL_GPIO_WritePin(RGB_LED_R_GPIO_PORT, RGB_LED_R_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RGB_LED_G_GPIO_PORT, RGB_LED_G_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RGB_LED_B_GPIO_PORT, RGB_LED_B_PIN, GPIO_PIN_SET);

	white_led_on = 1;
}

/**
  * @brief  Blink all LEDs off state
  * @param  None
  * @retval None
  */
static void rgbWhiteLedBlinkOff(void)
{
	HAL_GPIO_WritePin(RGB_LED_R_GPIO_PORT, RGB_LED_R_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RGB_LED_G_GPIO_PORT, RGB_LED_G_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RGB_LED_B_GPIO_PORT, RGB_LED_B_PIN, GPIO_PIN_RESET);

	white_led_on = 0;
}

/**
  * @brief  Rgb task that handles the blinking of the LEDs
  * @param  argument: pointer that is passed to the thread function as start argument
  * @retval None
  */
static void Rgb_Thread(void const * argument)
{
	uint32_t blink_timer = 0;
	osEvent event;

	for(;;)
	{
		event = osMessageGet(RgbEvent, 0);

		if(blink_white_led == 1)
		{
			if(millisecondGetTimePassed(blink_timer) >= 500)
			{
				if(white_led_on == 0)
					rgbWhiteLedBlinkOn();
				else
					rgbWhiteLedBlinkOff();
				blink_timer = millisecondGetTime();
			}
		}
	}
}

