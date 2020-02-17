#include "Unit.h"
#include "InstallationRun.h"
#include "DisinfectionRun.h"
#include <iostream>
#include <gui/video_player_screen/VideoPlayerView.hpp>

osMessageQId UnitEvent = 0;
static osThreadId UnitThreadId = 0;


static UnitRun run_mode = UNIT_DISINFECTION_RUN;
static UnitInstallationState unit_installation_state = UNIT_INSTALLATION_STATE_START;

static void unitThread(void const * argument);


//Tom:ADD1
#include <touchgfx/hal/BoardConfiguration.hpp>
extern "C" long ini_getl(const TCHAR *Section, const TCHAR *Key, long DefValue, const TCHAR *Filename);
extern "C" void LCD_SetBrightness(int value);

void BackLightSetting()
{
    //Get Backlight Setting from INI FILE
    //DEVICE and LCD_BACKLIGHT_PERCENT
    //uint32_t value = (int)ini_getl(DEVICE_SECTION_NAME, LCD_BACKLIGHT_PERCENT_KEY_NAME, 0L, SETTINGS_INI_FILE);
	uint32_t value = 100;
	if (value > 0)
    {
        LCD_SetBrightness(value);
    }
    else
    {
        LCD_SetBrightness(100);
    }

}

//Tom:ADD41
void unitSetVolume()
{
    //uint32_t value = (int)ini_getl(DEVICE_SECTION_NAME, DEVICE_VOLUME_PERCENTAGE_KEY_NAME, 100L, SETTINGS_INI_FILE);

	uint32_t value = 100;
    if ((value < 0) || (value > 100))
    {
        value = 100;
    }

    AUDIOPLAYER_SetVolume( 0 );
}

void unitSetup()
{
    BackLightSetting();
    unitSetVolume();
}



/**
  * @brief  Initialize the UV smart unit
  * @param  None
  * @retval None
  */
void unitInitialization(void)
{
    rgbInitialization();
    lidInitialization();
    uvLightInitialization();


    /* Create unit queue */
    osMessageQDef(Unit_Queue, 10, uint16_t);
    UnitEvent = osMessageCreate (osMessageQ(Unit_Queue), NULL);
    /* Create unit task */
    osThreadDef(osUnitThread, unitThread, osPriorityLow, 0, 1024);
    UnitThreadId = osThreadCreate (osThread(osUnitThread), NULL);



}


/**
  * @brief  Unit task that handles the state machines for the installation, control and disinfection.
  * @param  argument: pointer that is passed to the thread function as start argument.
  * @retval None
  */
static void unitThread(void const * argument)
{
    osEvent event;

    /* Check if calibration values are on memory. */
    int uv_lamp_1_sensor_value = ini_getl(UV_SENSOR_SECTION_NAME, UV_CALIBRATION_1_KEY_NAME, 0, SETTINGS_INI_FILE);
    int uv_lamp_2_sensor_value = ini_getl(UV_SENSOR_SECTION_NAME, UV_CALIBRATION_2_KEY_NAME, 0, SETTINGS_INI_FILE);

    /* Go into installation run if unit is not calibrated. */
    //TODO REEVALUATE THIS STATEMENT
    if((uv_lamp_1_sensor_value <= MINIMUM_SENSOR_THRESHOLD_UV_LAMP_1) ||
       (uv_lamp_2_sensor_value <= MINIMUM_SENSOR_THRESHOLD_UV_LAMP_2)) {
        run_mode = UNIT_INSTALLATION_RUN;
    }

    /* Play startup video and wait for it to end */
    playAviFile(VIDEO_SCREEN_A, false , NULL);
    while ((isAudioPlaying() == 1) || (isVideoPlaying() == 1)){};

    rtcInitialization();
    unitSetup();

    for(;;)
    {
        if(run_mode == UNIT_INSTALLATION_RUN){
            run_mode = unitInstallationRun();
        } else {
            run_mode = unitDisinfectionRun();
        }
    }
}



/**
  * @brief  Open lid and makes sure it has actually opened.
  * @param  None
  * @retval None
  */
void OpenLid(void){

    lidActivateSolenoid();
    osDelay(2000);
    lidDeactivateSolenoid();

    if(isLidClosed() == 1){
        rgbWhiteLedBlink();
        playAviFile(VIDEO_SCREEN_L, true, NULL);
        while(isLidClosed() != 0);
    }
}

/**
  * @brief  Display error and put unit out of order.
  * @param  None
  * @retval None
  */
void DisplayBlockingError(int error_code){

    /* Log error codes in debug and ini file and block unit */
    if(error_code == UNIT_ERROR_001){
        ini_putl(DEVICE_SECTION_NAME, OUT_OF_SERVICE_ERROR_KEY_NAME, UNIT_ERROR_001, SETTINGS_INI_FILE);
        writeDebug("Unit failed: ERROR 001", true);
        playAviFile(VIDEO_SCREEN_J_1, true, NULL);
    } else if (error_code == UNIT_ERROR_002) {
        ini_putl(DEVICE_SECTION_NAME, OUT_OF_SERVICE_ERROR_KEY_NAME, UNIT_ERROR_002, SETTINGS_INI_FILE);
        writeDebug("Unit failed: ERROR 002", true);
        playAviFile(VIDEO_SCREEN_J_2, true, NULL);
    } else if (error_code == UNIT_ERROR_003){
        ini_putl(DEVICE_SECTION_NAME, OUT_OF_SERVICE_ERROR_KEY_NAME, UNIT_ERROR_003, SETTINGS_INI_FILE);
        writeDebug("Unit failed: ERROR 003", true);
        playAviFile(VIDEO_SCREEN_J_3, true, NULL);
    } else {
        /* If error code not valid do not block machine. */
        return;
    }

    while(1){}; /* Block program. */
}


/**
  * @brief  Display error and put unit out of order.
  * @param  UvLamp Uv lamp to run cycle for
  * @retval -1: Lid opened unauthorized, #: calibration value lampset
  */
int RunLampCycle(UvLamp lamp_set){

    if(lamp_set == 1){
        writeDebug("Ran cycle on lamp-set 1", false);
        uvLight1On();
    } else if(lamp_set == 2) {
        writeDebug("Ran cycle on lamp-set 2", false);
        uvLight2On();
    } else {
        return -1;
    }

    uint16_t uv_sensor_samples[NUMBER_OF_UV_SENSOR_SAMPLES] = {0};
    uint16_t uv_sensor_sample_counter = 0;

    /* Start the timer */
    uint32_t uv_lamp_timer = millisecondGetTime();

    uv_sensor_sample_counter = 0;
    /* Ignore first sample */
    uvLightSensorGetSample();
    /* Wait for timer to end */
    while (millisecondGetTimePassed(uv_lamp_timer) <= UV_LAMP_FUNCTIONALITY_CHECK_TIME) {
        uv_sensor_samples[uv_sensor_sample_counter++] = (uint16_t) uvLightSensorGetSample();
        osDelay(UV_LAMP_FUNCTIONALITY_CHECK_TIME / NUMBER_OF_UV_SENSOR_SAMPLES);
        /* Lid opened unauthorized */
        if (isLidClosed() == 0) {
            return -1;
        }
    }

    uvLight1Off();
    uvLight2Off();

    /* Calculate the average sensor value for UV lamp 1 */
    uint32_t uv_lamp_sensor_value = 0;
    for (uint16_t i = 0; i < uv_sensor_sample_counter; i++)
        uv_lamp_sensor_value += uv_sensor_samples[i];
    uv_lamp_sensor_value /= uv_sensor_sample_counter;

    return uv_lamp_sensor_value;
}



