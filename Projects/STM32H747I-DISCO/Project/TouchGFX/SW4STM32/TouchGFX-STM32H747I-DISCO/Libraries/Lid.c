#include "Lid.h"
#include "MillisecondTimer.h"
#include "cmsis_os.h"

osMessageQId LidEvent = 0;
static osThreadId LidThreadId = 0;
static LidInput lid_inputs[2];

static void debounceLidInput(LidInput* lid_input);
static void Lid_Thread(void const * argument);

/**
  * @brief  Initialize the lid sensors and solenoid
  * @param  None
  * @retval None
  */
void lidInitialization(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Enable GPIO clocks */
	LID_SOLENOID_PIN_ENABLE_CLOCK();
	LID_CLIP_SENSOR_PIN_ENABLE_CLOCK();
	LID_HINGE_SENSOR_PIN_ENABLE_CLOCK();

	/* Configure GPIO pins */
	GPIO_InitStruct.Pin = LID_SOLENOID_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LID_SOLENOID_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LID_CLIP_SENSOR_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LID_CLIP_SENSOR_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LID_HINGE_SENSOR_PIN;
	HAL_GPIO_Init(LID_HINGE_SENSOR_GPIO_PORT, &GPIO_InitStruct);

	/* Deactivate the solenoid */
	lidDeactivateSolenoid();

	/* Create Lid Queue */
	osMessageQDef(LID_Queue, 10, uint16_t);
	LidEvent = osMessageCreate (osMessageQ(LID_Queue), NULL);
	/* Create Lid task */
	osThreadDef(osLid_Thread, Lid_Thread, osPriorityLow, 0, 512);
	LidThreadId = osThreadCreate (osThread(osLid_Thread), NULL);
}

/**
  * @brief  Activate the solenoid to release the lid
  * @param  None
  * @retval None
  */
void lidActivateSolenoid(void)
{
	HAL_GPIO_WritePin(LID_SOLENOID_GPIO_PORT, LID_SOLENOID_PIN, GPIO_PIN_SET);
}

/**
  * @brief  Deactivate the solenoid
  * @param  None
  * @retval None
  */
void lidDeactivateSolenoid(void)
{
	HAL_GPIO_WritePin(LID_SOLENOID_GPIO_PORT, LID_SOLENOID_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  Is the lid closed? Both the clip sensor and hinge sensor should be in the correct position to detect a closed lid
  * @param  None
  * @retval 0: lid is not closed, 1: lid is closed
  */
uint8_t isLidClosed(void)
{
	return ((lid_inputs[0].output == LID_INPUT_STATE_NOT_ACTIVE) && (lid_inputs[1].output == LID_INPUT_STATE_ACTIVE));
}

/**
  * @brief  Debounce the lid input
  * @param  *lid_input: pointer to lid input structure
  * @retval None
  */
static void debounceLidInput(LidInput* lid_input)
{
	lid_input->change = 0;

	if(lid_input->input == 1)
	{
		/* Reset the not active timer */
		lid_input->not_active_timer = millisecondGetTime();
		/* Check if the input remains pressed during the debounce time */
		if((millisecondGetTimePassed(lid_input->active_timer) >= ACTIVE_DEBOUNCE_TIME) && (lid_input->output == 0))
		{
			lid_input->output = 1;
			lid_input->change = 1;
			lid_input->state = LID_INPUT_STATE_ACTIVE;
		}
	}
	else
	{
		/* Reset the active timer */
		lid_input->active_timer = millisecondGetTime();
		/* Check if the input remains released during the debounce time */
		if((millisecondGetTimePassed(lid_input->not_active_timer) >= NOT_ACTIVE_DEBOUNCE_TIME) && (lid_input->output == 1))
		{
			lid_input->output = 0;
			lid_input->change = 1;
			lid_input->state = LID_INPUT_STATE_NOT_ACTIVE;
		}
	}
}

/**
  * @brief  Lid task that continuously debounces the lid inputs
  * @param  argument: pointer that is passed to the thread function as start argument
  * @retval None
  */
static void Lid_Thread(void const * argument)
{
	for(;;)
	{
		/* Read inputs */
		lid_inputs[0].input = (HAL_GPIO_ReadPin(LID_CLIP_SENSOR_GPIO_PORT, LID_CLIP_SENSOR_PIN) == GPIO_PIN_SET);
		lid_inputs[1].input = (HAL_GPIO_ReadPin(LID_HINGE_SENSOR_GPIO_PORT, LID_HINGE_SENSOR_PIN) == GPIO_PIN_SET);
		/* Debounce inputs */
		debounceLidInput(&lid_inputs[0]);
		debounceLidInput(&lid_inputs[1]);
	}
}
