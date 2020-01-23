#include "Unit.h"
#include "Storage.h"
#include "Rgb.h"
#include "UvLight.h"
#include "Lid.h"
#include "Audio.h"
#include "MillisecondTimer.h"
#include "RealTimeClock.h"
#include "Debug.h"
#include <time.h>


#include "minIni.h"
#include "ff.h"
#include <gui/video_player_screen/VideoPlayerView.hpp>

#include <iostream>

osMessageQId UnitEvent = 0;
static osThreadId UnitThreadId = 0;


static UnitDisinfectionState unit_disinfection_state = UNIT_DISINFECTION_STATE_START;
static uint16_t uv_sensor_samples[NUMBER_OF_UV_SENSOR_SAMPLES] = {0};
static uint16_t uv_sensor_sample_counter = 0;

static FIL outcome_file;

static void unitThread(void const * argument);
static void unitDisinfectionRun(void);
static void EnsureOpenLid(void);
static void writeDebug(char debug_info[], bool writeIni);


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


void LoggingTimeStampNow(const char *keyname)
{
    char str[80];
    struct tm  ts;
    uint32_t time;

    time_t  timestamp = rtcGetUnixTime();
    ts = *localtime(&timestamp);

    strftime(str, sizeof(str), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

    ini_puts(LOGGING_SECTION_NAME, keyname, str, SETTINGS_INI_FILE);

}


//Tom:ADD31
//This routine will read the max weardown allowed from the Init file.
//It will read a uint32_t value 0 = 0%   80 wquals 80%
//
//Return is a float  0.8   equals 80%
float unitUVLampsWeardownProcent()
{
    //uint32_t value = (int)ini_getl(DEVICE_SECTION_NAME, UV_LAMP_WEARDOWN_MAX_KEY_NAME, 0L, SETTINGS_INI_FILE);

	uint32_t value = 100;
    if (value == 0)
    {
        return VARIABLE_T;
    }

    /*
    if (value >= 100)
    {
        return VARIABLE_T;
    }
*/
    return ((float)value)/100;

}



/**
  * @brief  Initialize the UV smart unit
  * @param  None
  * @retval None
  */
void unitInitialization(void)
{
    debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "unitInitialization\r\n");

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

    rtcInitialization();
    unitSetup();

    for(;;)
    {
        unitDisinfectionRun();
    }
}


/**
  * @brief  Handle the unit disinfection
  * @param  None
  * @retval None
  */
static void unitDisinfectionRun(void) {
    static uint32_t uv_lamp_timer = 0;                //Timer for the UV lamps
    static uint32_t uv_lamp_sensor_value = 0;        //UV sensor value
    static uint8_t last_active_uv_lamp = 0;            //Last active UV lamp set
    static uint8_t active_uv_lamp = 0;                //Current active UV lamp set



    switch (unit_disinfection_state) {
        case UNIT_DISINFECTION_STATE_START: {
            unitSetVolume();
            //Play STARTUP video and turn on WHITE lights
            rgbWhiteLedOn();
            playAviFile(VIDEO_SCREEN_A, false, NULL);
            /* Wait for the video and audio to end */
            while ((isAudioPlaying() == 1) || (isVideoPlaying() == 1));

            //If lid is closed we have to open the lid.
            if (isLidClosed() == 1) {
                lidActivateSolenoid();
                osDelay(2000);
                lidDeactivateSolenoid();
                /* Ensures that lid has opened */
                EnsureOpenLid();
            }

            /* Go to check if lid is actually open state */
            unit_disinfection_state = UNIT_DISINFECTION_STATE_OPEN_LID;

            break;
        }

        case UNIT_DISINFECTION_STATE_OPEN_LID: {

            playAviFile(VIDEO_SCREEN_F_1, true, NULL); //Play on repeat

            //While lid is not closed, wait
            while (isLidClosed() == 0) {};

            unit_disinfection_state = UNIT_DISINFECTION_STATE_DISINFECTION;

            break;
        }

        case UNIT_DISINFECTION_STATE_DISINFECTION: {
            debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "UNIT_DISINFECTION_STATE_DISINFECTION\r\n");
            rgbAllLedsOff();

            /* Which UV lamp was activated on the last run? */
            last_active_uv_lamp = (uint8_t) ini_getl(DEVICE_SECTION_NAME, LAST_ACTIVE_UV_LAMP_KEY_NAME, 1,
                                                     SETTINGS_INI_FILE);

            uint32_t lamp_1_fail_count = ini_getl(UV_SENSOR_SECTION_NAME, UV_LAMP_1_FAIL_COUNT_KEY_NAME, 0,
                                                           SETTINGS_INI_FILE);
            uint32_t lamp_2_fail_count = ini_getl(UV_SENSOR_SECTION_NAME, UV_LAMP_2_FAIL_COUNT_KEY_NAME, 0,
                                                           SETTINGS_INI_FILE);

            writeDebug("Lamp_1_fail_count", false);
            char lamp_1_count[] = {0};
            snprintf(lamp_1_count, 64,"%lu", lamp_1_fail_count);
            writeDebug(lamp_1_count, false);

            writeDebug("Lamp_2_fail_count", false);
            char lamp_2_count[] = {0};
            snprintf(lamp_2_count, 64,"%lu", lamp_2_fail_count);
            writeDebug(lamp_2_count, false);



            /* Lamp 1 has failed */
            if (lamp_1_fail_count == 1) {
                /* Switch on lamp set 2 */
                uvLight2On();
                active_uv_lamp = 2;
                writeDebug("Disinfection started on light 2", false);
            }
            /* Lamp 2 has failed */
            else if (lamp_2_fail_count == 1) {
                /* Switch on lamp set 1 */
                uvLight1On();
                active_uv_lamp = 1;
                writeDebug("Disinfection started on light 1", false);
            }
            /* Both lamps work*/
            else{
                if(last_active_uv_lamp == 1){
                    uvLight2On();
                    active_uv_lamp = 2;
                    writeDebug("Disinfection started on light 2", false);
                } else if (last_active_uv_lamp == 2){
                    uvLight1On();
                    active_uv_lamp = 1;
                    writeDebug("Disinfection started on light 1", false);
                }
            }


            /* Display screen G */
            playAviFile(VIDEO_SCREEN_G, false, NULL);
            /* Reset sample buffer and counter */
            memset(uv_sensor_samples, 0, sizeof(uv_sensor_samples));
            uv_sensor_sample_counter = 0;
            /* Ignore first sample */
            uvLightSensorGetSample();

            /* Start the timer */
            uv_lamp_timer = millisecondGetTime();

            /* Wait for timer to end */
            while (millisecondGetTimePassed(uv_lamp_timer) <= UV_LAMP_FUNCTIONALITY_CHECK_TIME) {
                uv_sensor_samples[uv_sensor_sample_counter++] = (uint16_t) uvLightSensorGetSample();
                osDelay(UV_LAMP_FUNCTIONALITY_CHECK_TIME / NUMBER_OF_UV_SENSOR_SAMPLES);
                /* Lid opened unauthorized */
                if (isLidClosed() == 0) {
                    uvLight1Off();
                    uvLight2Off();
                    unit_disinfection_state = UNIT_DISINFECTION_STATE_LID_OPENED_UNAUTHORIZED_ERROR;

                    /* goto used to escape nested while loop and case at the same time */
                    goto DISINFECTION_BREAK;
                }
            }
            /* Calculate the average sensor value for the active UV lamp */
            uv_lamp_sensor_value = 0;
            for (uint16_t i = 0; i < uv_sensor_sample_counter; i++)
                uv_lamp_sensor_value += uv_sensor_samples[i];
            uv_lamp_sensor_value /= uv_sensor_sample_counter;
            /* Turn off the UV lamps */
            uvLight1Off();
            uvLight2Off();
            /* Wait for the video and audio to end */
            while ((isAudioPlaying() == 1) || (isVideoPlaying() == 1));

            if (isLidClosed() == 1) {
                         lidActivateSolenoid();
                         osDelay(2000);
                         lidDeactivateSolenoid();
                         EnsureOpenLid();
                         unit_disinfection_state = UNIT_DISINFECTION_STATE_CHECK_RUNS;
                         break;
                     }

            DISINFECTION_BREAK:
            break;
        }

        case UNIT_DISINFECTION_STATE_LID_OPENED_UNAUTHORIZED_ERROR: {
            /* Turn off both UV lamps */
            uvLight1Off();
            uvLight2Off();

            writeDebug("Disinfection opened unauthorized\n", false);
            /* RGB red on */
            rgbRedLedOn();
            /* Display screen I */
            playAviFile(VIDEO_SCREEN_I, true, NULL);

            /* Wait for lid to close */
            uint32_t
			wait_close_lid_timer = millisecondGetTime();
            while (millisecondGetTimePassed(wait_close_lid_timer) <= VARIABLE_S) {
                if (isLidClosed() == 1) {
                    //Tom: Closed within the VARIABLE_S time.
                    unit_disinfection_state = UNIT_DISINFECTION_STATE_DISINFECTION;
                    break;
                }
            }

            /* RGB red off */
            rgbAllLedsOff();

            writeDebug("Run failed because of open lid\n", false);
            unit_disinfection_state = UNIT_DISINFECTION_STATE_OPEN_LID;
            break;
        }

        case UNIT_DISINFECTION_STATE_CHECK_RUNS: {

            ini_putl(DEVICE_SECTION_NAME, LAST_ACTIVE_UV_LAMP_KEY_NAME, active_uv_lamp, SETTINGS_INI_FILE);

            /* Check if cleaning was unsuccessful and active_lamp is broken */
            if (uv_lamp_sensor_value == 0) {
                /* Depends on which lamp is active and has failed*/
                ini_putl(UV_SENSOR_SECTION_NAME,
                         (active_uv_lamp == 1) ? UV_LAMP_1_FAIL_COUNT_KEY_NAME : UV_LAMP_2_FAIL_COUNT_KEY_NAME,
                         1, SETTINGS_INI_FILE);
                /* Run has failed so go to unsuccessful state */
                unit_disinfection_state = UNIT_DISINFECTION_STATE_UNSUCCESSFUL;
            } else {
                unit_disinfection_state = UNIT_DISINFECTION_STATE_SUCCESSFUL;
                ini_putl(UV_SENSOR_SECTION_NAME,
                         (active_uv_lamp == 1) ? UV_CALIBRATION_1_KEY_NAME : UV_CALIBRATION_2_KEY_NAME,
                         uv_lamp_sensor_value, SETTINGS_INI_FILE);
            }

            break;
        }


        case UNIT_DISINFECTION_STATE_SUCCESSFUL: {
            writeDebug("Successful disinfection", true);
            rgbGreenLedOn();
            playAviFile(VIDEO_SCREEN_H, false, NULL);

            /* Wait for the video and audio to end */
            while ((isAudioPlaying() == 1) || (isVideoPlaying() == 1));

            rgbAllLedsOff();
            unit_disinfection_state = UNIT_DISINFECTION_STATE_OPEN_LID;
            break;
        }


        case UNIT_DISINFECTION_STATE_UNSUCCESSFUL: {
            writeDebug("Unsuccessful disinfection", true);
            rgbRedLedOn();
            playAviFile(VIDEO_SCREEN_I, true, NULL);

            while(isLidClosed() == 0){};

            rgbAllLedsOff();
            unit_disinfection_state = UNIT_DISINFECTION_STATE_DISINFECTION;
            break;
        }

        default:
            break;
    }
}


/**
  * @brief  Check if lid has actually opened and give an error if this is not the case
  * @param  None
  * @retval None
  */
static void EnsureOpenLid(void){
    if(isLidClosed() == 1){
        rgbWhiteLedBlink();
        playAviFile(VIDEO_SCREEN_L, true, NULL);
        while(isLidClosed() != 0);
    }
}


/**
  * @brief  Save debug information to a file.
  * @retval None
  */
static void writeDebug(char debug_info[], bool writeIni) {
    /* Wait 10000 ticks for the shared resource to become free */
    if (xSemaphoreTake(StorageSemaphore, 10000) != pdTRUE)
        return;

    FILINFO file_info;
    /* Get the date and time */
    RtcDate date;
    RtcTime time;

    rtcGetDate(&date);
    rtcGetTime(&time);
    /* Create directory if it doesn't exist */
    if (f_stat("0://DEBUG", &file_info) != FR_OK)
        f_mkdir("0://DEBUG");

    char file_name[32] = "0://DEBUG/debug_report";
    snprintf(file_name, 32, "0://DEBUG/%.4d-%.2d.TXT", (date.year + 2000), date.month);
    /* Try to open the file */
    if (f_open(&outcome_file, file_name, FA_WRITE | FA_OPEN_APPEND) != FR_OK) {
        debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "Failed to open outcome file\r\n");
        return;
    }

    char result[400] = {0};
    strcat(result, debug_info);
    strcat(result, "\n");

    if(writeIni){

    	 strcat(result, "{");

        char active_l[] = {0};
        char uv_1_fail[] = {0};
        char uv_2_fail[] = {0};
        char uv_1_calibration[] = {0};
        char uv_2_calibration[] = {0};

        snprintf(active_l, 64,"%lu", ini_getl(DEVICE_SECTION_NAME, LAST_ACTIVE_UV_LAMP_KEY_NAME, 0, SETTINGS_INI_FILE));
        snprintf(uv_1_fail, 64,"%lu", ini_getl(UV_SENSOR_SECTION_NAME, UV_LAMP_1_FAIL_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE));
        snprintf(uv_2_fail, 64,"%lu", ini_getl(UV_SENSOR_SECTION_NAME, UV_LAMP_2_FAIL_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE));
        snprintf(uv_1_calibration, 64,"%lu", ini_getl(UV_SENSOR_SECTION_NAME, UV_CALIBRATION_1_KEY_NAME, 0, SETTINGS_INI_FILE));
        snprintf(uv_2_calibration, 64,"%lu", ini_getl(UV_SENSOR_SECTION_NAME, UV_CALIBRATION_2_KEY_NAME, 0, SETTINGS_INI_FILE));

        strcat(result, "\n");
    	strcat(result, LAST_ACTIVE_UV_LAMP_KEY_NAME);
    	strcat(result, " = ");
        strcat(result, active_l);

        strcat(result, "\n");
        strcat(result, UV_LAMP_1_FAIL_COUNT_KEY_NAME);
        strcat(result, " = ");
        strcat(result, uv_1_fail);

        strcat(result, "\n");
        strcat(result, UV_LAMP_2_FAIL_COUNT_KEY_NAME);
        strcat(result, " = ");
        strcat(result, uv_2_fail);

        strcat(result, "\n");
        strcat(result, UV_CALIBRATION_1_KEY_NAME);
        strcat(result, " = ");
        strcat(result, uv_1_calibration);

        strcat(result, "\n");
        strcat(result, UV_CALIBRATION_2_KEY_NAME);
        strcat(result, " = ");
        strcat(result, uv_2_calibration);

        strcat(result, "\n}\n\n");

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

