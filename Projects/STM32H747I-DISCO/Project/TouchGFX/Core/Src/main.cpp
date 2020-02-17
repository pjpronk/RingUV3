#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/hal/BoardConfiguration.hpp>
#include <touchgfx/canvas_widget_renderer/CanvasWidgetRenderer.hpp>
using namespace touchgfx;

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <bsp.h>

#include "Unit.h"
#include "lib/Config.h"
#include "lib/Audio.h"
#include "lib/Storage.h"
#include "lib/MicrosecondTimer.h"
#include "lib/MillisecondTimer.h"
#include "lib/RealTimeClock.h"
#include "lib/Debug.h"

#include <gui/video_player_screen/VideoPlayerView.hpp>

/**
 * Define the FreeRTOS task priorities and stack sizes
 */
#define configGUI_TASK_PRIORITY                 ( tskIDLE_PRIORITY + 3 )

#define configGUI_TASK_STK_SIZE                 ( 2*4048 )

extern xQueueHandle GUITaskQueue;

extern void initialise_monitor_handles(void);

/**
  * @brief  Graphical User Interface task
  * @param  *params: pointer to task parameters
  * @retval None
  */
static void GUITask(void* params)
{
	debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "GUITask\r\n");
	GUITaskQueue = xQueueGenericCreate( 5, 1, 0 );
	BspInit();
	touchgfx::HAL::getInstance()->taskEntry();
}

/**
  * @brief  Main function of the code
  * @param  None
  * @retval N/A
  */
int main(void)
{
	hw_init();
	touchgfx_init();

	microsecondTimerInitialization();
	millisecondTimerInitialization();
	storageInitialization();
	audioInitialization();

	xTaskCreate(GUITask, "GUITask",
				configGUI_TASK_STK_SIZE,
				NULL,
				configGUI_TASK_PRIORITY,
				NULL);

	unitInitialization();

	vTaskStartScheduler();

	for(;;)
	{

	}
}

/**
  * @brief  External GPIO interrupt callback function
  * @param  GPIO_Pin: External GPIO pin identifier
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == TS_INT_PIN)
	{
		/* Update GFX Library state machine according to touch acquisition ? */
	}
	else if(GPIO_Pin == SD_DETECT_PIN)
	{
		if(BSP_SD_IsDetected())
		{
			osMessagePut(StorageEvent, MSDDISK_CONNECTION_EVENT, 0);
		}
		else
		{
			osMessagePut(StorageEvent, MSDDISK_DISCONNECTION_EVENT, 0);
		}
	}
}
