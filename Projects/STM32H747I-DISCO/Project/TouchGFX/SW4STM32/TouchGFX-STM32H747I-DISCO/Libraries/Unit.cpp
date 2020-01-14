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
static void unitInstallationRun(void);
static void unitControlRun(void);
static void unitDisinfectionRun(void);
static void unitSaveOutcome(UnitOutcome result, UnitError error);

//Tom:ADD00
static bool fail = false;
static bool success = true;


//Tom:ADD1
#include <touchgfx/hal/BoardConfiguration.hpp>
extern "C" long ini_getl(const TCHAR *Section, const TCHAR *Key, long DefValue, const TCHAR *Filename);
extern "C" void LCD_SetBrightness(int value);

void BackLightSetting()
{
    //Get Backlight Setting from INI FILE
    //DEVICE and LCD_BACKLIGHT_PERCENT
    uint32_t value = (int)ini_getl(DEVICE_SECTION_NAME, LCD_BACKLIGHT_PERCENT_KEY_NAME, 0L, SETTINGS_INI_FILE);
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
    uint32_t value = (int)ini_getl(DEVICE_SECTION_NAME, DEVICE_VOLUME_PERCENTAGE_KEY_NAME, 100L, SETTINGS_INI_FILE);

    if ((value < 0) || (value > 100))
    {
        value = 100;
    }

    AUDIOPLAYER_SetVolume( value );
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
    uint32_t value = (int)ini_getl(DEVICE_SECTION_NAME, UV_LAMP_WEARDOWN_MAX_KEY_NAME, 0L, SETTINGS_INI_FILE);

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


//Tom:ADD33
//This routine will log the LampWeardown value in procent
//It will allso log the all time low as well.
//
void unitUVLampsWeardownProcentLog(uint32_t LampId, float fValue)
{
    uint32_t lValue;
    uint32_t lValueMin;


    lValue =    (uint32_t)( (fValue * (float)100) );
    if (LampId == 1)
    {


        ini_putl(LOGGING_SECTION_NAME, UV_LAMP_1_WEARDOWN_CURRENT_KEY_NAME, lValue, SETTINGS_INI_FILE);

        lValueMin = ini_getl(LOGGING_SECTION_NAME, UV_LAMP_1_WEARDOWN_MIN_KEY_NAME, 1000, SETTINGS_INI_FILE);
        if (lValueMin > lValue)
        {
            /* New all time low = store this value */
            ini_putl(LOGGING_SECTION_NAME, UV_LAMP_1_WEARDOWN_MIN_KEY_NAME, lValue, SETTINGS_INI_FILE);
        }

    }
    if (LampId == 2)
    {
        ini_putl(LOGGING_SECTION_NAME, UV_LAMP_2_WEARDOWN_CURRENT_KEY_NAME, lValue, SETTINGS_INI_FILE);

        lValueMin = ini_getl(LOGGING_SECTION_NAME, UV_LAMP_2_WEARDOWN_MIN_KEY_NAME, 1000, SETTINGS_INI_FILE);
        if (lValueMin > lValue)
        {
            /* New all time low = store this value */
            ini_putl(LOGGING_SECTION_NAME, UV_LAMP_2_WEARDOWN_MIN_KEY_NAME, lValue, SETTINGS_INI_FILE);
        }
    }
}



//Tom:ADD34
//This routine will set the all time low weardown to 9999 so the next time te new value will be set.
//
void unitUVLampsWeardownProcentLogReset()
{
    ini_putl(LOGGING_SECTION_NAME, UV_LAMP_1_WEARDOWN_MIN_KEY_NAME, 9999, SETTINGS_INI_FILE);
    ini_putl(LOGGING_SECTION_NAME, UV_LAMP_2_WEARDOWN_MIN_KEY_NAME, 9999, SETTINGS_INI_FILE);
}



//Tom:ADD33
void unitUVLampUsedAddOne( int32_t lamp_id)
{
    uint32_t times;

    if (lamp_id == 1)
    {
        times = (int)ini_getl(DEVICE_SECTION_NAME, UV_LAMP_1_USED_KEY_NAME, 0L, SETTINGS_INI_FILE);
        times++;
        ini_putl(DEVICE_SECTION_NAME, UV_LAMP_1_USED_KEY_NAME, times, SETTINGS_INI_FILE);

    }
    if (lamp_id == 2)
    {
        times = (int)ini_getl(DEVICE_SECTION_NAME, UV_LAMP_2_USED_KEY_NAME, 0L, SETTINGS_INI_FILE);
        times++;
        ini_putl(DEVICE_SECTION_NAME, UV_LAMP_2_USED_KEY_NAME, times, SETTINGS_INI_FILE);
    }
}



//Tom:ADD8
// ******************
// This routine will set the unit in the First Step of the disinfection run.
void unitDisinfectionRun_GotoIdle()
{
    unit_disinfection_state = UNIT_DISINFECTION_STATE_START;
}




//Tom:ADD11
void unitDisinfectionRun_GotoMaintanceRequired()
{
    //Exactly the same as LIGHT_X_ERROR:
    unit_disinfection_state = UNIT_DISINFECTION_STATE_MAINTANCE_REQUIRED;

    debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "unitDisinfectionRun_GotoMaintanceRequired\r\n");

    /* RGB red on */
    rgbRedLedOn();

    /* Display screen J */
    playAviFile(VIDEO_SCREEN_J_1, true, NULL); //Repeat
    /* Set the out-of-service error */
    ini_putl(DEVICE_SECTION_NAME, OUT_OF_SERVICE_ERROR_KEY_NAME, UNIT_ERROR_001, SETTINGS_INI_FILE);
    /* Wait */
    while(1);
}

//Tom:ADD13
void unitDisinfectionRun_ResetSolenoidFailCounter()
{
    debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "unitDisinfectionRun_ResetSolenoidFailCounter\r\n");

    ini_putl(DEVICE_SECTION_NAME, SOLENOID_FAIL_COUNTER, 0, SETTINGS_INI_FILE);
}

//Tom:ADD15
bool unitDisinfectionRun_AddSolenoidFailCounter()
{
    debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "unitDisinfectionRun_AddSolenoidFailCounter\r\n");
    uint32_t failcounter = ini_getl(DEVICE_SECTION_NAME, SOLENOID_FAIL_COUNTER, 0, SETTINGS_INI_FILE);

    failcounter++;

    ini_putl(DEVICE_SECTION_NAME, SOLENOID_FAIL_COUNTER, failcounter, SETTINGS_INI_FILE);

    if (failcounter > VARIABLE_X)
    {
        return fail;
    }
    return success;
}


//Tom:ADD20
bool unitDisinfectionRun_IsLampSetBroken( uint8_t lampset )
{
    debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "unitDisinfectionRun_IsLampSetBroken\r\n");
    uint32_t uv_lamp_calibration_value;

    uv_lamp_calibration_value = 0;  // Default is fail.
    if (lampset == 1)
    {
        uv_lamp_calibration_value = ini_getl(UV_SENSOR_SECTION_NAME, UV_LAMP_1_RUN_SUCCESS_KEY_NAME, 0, SETTINGS_INI_FILE);
    }
    if (lampset == 2)
    {
        uv_lamp_calibration_value = ini_getl(UV_SENSOR_SECTION_NAME, UV_LAMP_2_RUN_SUCCESS_KEY_NAME, 0, SETTINGS_INI_FILE);
    }

    if (uv_lamp_calibration_value == 1)
    {
        return true;
    }
    return false;
}


//Tom:ADD21
bool unitDisinfectionRun_IsOnlyOneBroken(void)
{
    debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "unitDisinfectionRun_IsOnlyOneBroken\r\n");

    bool stateLamp1 = unitDisinfectionRun_IsLampSetBroken(1);
    bool stateLamp2 = unitDisinfectionRun_IsLampSetBroken(2);

    if ((stateLamp1 == success) || (stateLamp2 == success))
    {
        return success;
    }
    return fail;
}

uint32_t unitDisinfectionRun_GetLampFailCount(uint8_t lampset)
{
    debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "unitDisinfectionRun_GetLampFailCount\r\n");

    uint32_t count = 0;

    if (lampset == 1)
    {
        count = ini_getl(UV_SENSOR_SECTION_NAME, UV_LAMP_1_FAIL_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE);
    }
    if (lampset == 2)
    {
        count = ini_getl(UV_SENSOR_SECTION_NAME, UV_LAMP_2_FAIL_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE);
    }

    return count;
}



uint32_t unitDisinfectionRun_ResetLampFailCount(uint8_t lampset)
{
    debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "unitDisinfectionRun_ResetLampFailCount\r\n");

    uint32_t count = 0;

    if (lampset == 1)
    {
        ini_putl(UV_SENSOR_SECTION_NAME, UV_LAMP_1_FAIL_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE);

    }
    if (lampset == 2)
    {
        ini_putl(UV_SENSOR_SECTION_NAME, UV_LAMP_2_FAIL_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE);
    }

    return count;
}

uint32_t unitDisinfectionRun_AddLampFailCount(uint8_t lampset)
{
    debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "unitDisinfectionRun_AddLampFailCount\r\n");
    uint32_t count = 0;

    if (lampset == 1)
    {
        count = ini_getl(UV_SENSOR_SECTION_NAME, UV_LAMP_1_FAIL_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE);
        count++;
        ini_putl(UV_SENSOR_SECTION_NAME, UV_LAMP_1_FAIL_COUNT_KEY_NAME, count, SETTINGS_INI_FILE);

    }
    if (lampset == 2)
    {
        count = ini_getl(UV_SENSOR_SECTION_NAME, UV_LAMP_2_FAIL_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE);
        count++;
        ini_putl(UV_SENSOR_SECTION_NAME, UV_LAMP_2_FAIL_COUNT_KEY_NAME, count, SETTINGS_INI_FILE);
    }

    return count;
}


uint32_t unitDisinfectionRun_FlipLastUsedLampset()
{
    uint8_t last_active_uv_lamp = (uint8_t)ini_getl(DEVICE_SECTION_NAME, LAST_ACTIVE_UV_LAMP_KEY_NAME, 0, SETTINGS_INI_FILE);

    if (last_active_uv_lamp == 1)
    {
        /* Save the new value */
        ini_putl(DEVICE_SECTION_NAME, LAST_ACTIVE_UV_LAMP_KEY_NAME, 2, SETTINGS_INI_FILE);

    }
    else
    {
        /* Save the new value */
        ini_putl(DEVICE_SECTION_NAME, LAST_ACTIVE_UV_LAMP_KEY_NAME, 1, SETTINGS_INI_FILE);
    }

}


uint32_t unitDisinfectionRun_GetLampSetToUse()
{
    uint32_t fail_count = 0;


    // Get the last used lmapset, and flip.
    uint8_t last_active_uv_lamp = (uint8_t)ini_getl(DEVICE_SECTION_NAME, LAST_ACTIVE_UV_LAMP_KEY_NAME, 0, SETTINGS_INI_FILE);

    if (last_active_uv_lamp == 1)
    {
        // Check lampset 2 ready to use.

        fail_count = unitDisinfectionRun_GetLampFailCount(2);

        if (fail_count >= VARIABLE_Y)  //Lampset 2 is NOT ok!
        {
            //Back to lampset 1

            fail_count = unitDisinfectionRun_GetLampFailCount(1);

            if (fail_count >= VARIABLE_Y)  //Lampset 1 is NOT ok!
            {
                return 0; // FAIL!
            }

            return 1;
        }
        return 2;
    }

    // Ckeck lmapset 1 if ready to use

    fail_count = unitDisinfectionRun_GetLampFailCount(1);

    if (fail_count >= VARIABLE_Y)  //Lampset 1 is NOT ok!
    {

        fail_count = unitDisinfectionRun_GetLampFailCount(2);

        if (fail_count >= VARIABLE_Y)  //Lampset 2 is NOT ok!
        {
            return 0; // FAIL!
        }

        //Back to lampset 2
        return 2;
    }
    return 1;
}



void unitDisinfectionRun_SaveTime(char key[], uint32_t value)
{
    debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "unitDisinfectionRun_SaveTime\r\n");


    ini_putl(UV_SENSOR_SECTION_NAME, key, value, SETTINGS_INI_FILE);
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
    LoggingTimeStampNow("TomTime");

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
static void unitDisinfectionRun(void)
{
    static uint8_t lid_open_error_counter = 0;		//Number of times a lid open error has been detected
    static uint8_t uv_light_error_counter = 0;		//Number of times a UV light error has been detected
    static uint32_t uv_lamp_timer = 0;				//Timer for the UV lamps
    static uint32_t uv_lamp_sensor_value = 0;		//UV sensor value
    static uint32_t uv_lamp_calibration_value = 0;	//Calibration value for lamp 1 and lamp 2
    static uint8_t last_active_uv_lamp = 0;			//Last active UV lamp set
    static uint8_t active_uv_lamp = 0;				//Current active UV lamp set
    uint8_t run_count = 0;							//Number of runs



    switch(unit_disinfection_state)
    {
        case UNIT_DISINFECTION_STATE_START:
            debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "UNIT_DISINFECTION_STATE_START\r\n");
            unitSetVolume();

            //Play STARTUP video
            playAviFile(VIDEO_SCREEN_A, true, AUDIO_STARTUP);

            //If lid is closed we have to open the lid.
            if(isLidClosed() == 1){
                lidActivateSolenoid();
            }

            //Go into lid open state
            unit_disinfection_state = UNIT_DISINFECTION_STATE_OPEN_LID;

            break;

        case UNIT_DISINFECTION_STATE_OPEN_LID:
            while(isLidClosed() == 0){
                playAviFile(VIDEO_SCREEN_F_1, false, NULL);
            }
            break;


        case UNIT_DISINFECTION_STATE_DISINFECTION:
            debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "UNIT_DISINFECTION_STATE_DISINFECTION\r\n");


            /* RGB off */
            rgbAllLedsOff();

            /* Which UV lamp was activated on the last run? */
            last_active_uv_lamp = (uint8_t)ini_getl(DEVICE_SECTION_NAME, LAST_ACTIVE_UV_LAMP_KEY_NAME, 0, SETTINGS_INI_FILE);

            active_uv_lamp = unitDisinfectionRun_GetLampSetToUse();

            if (active_uv_lamp == 0)
            {
                // ERROR No lamp to use!!

                unit_disinfection_state = UNIT_DISINFECTION_STATE_UV_LIGHT_X_ERROR;
                break;
            }

            /* Last active UV lamp was 1 */
            if(active_uv_lamp == 1)
            {
                /* Switch on lamp set 1 */
                uvLight1On();
                unitUVLampUsedAddOne(1);
                active_uv_lamp = 1;
                /* Reset the successful run setting */
                ini_putl(DEVICE_SECTION_NAME, UV_LAMP_1_RUN_SUCCESS_KEY_NAME, 0, SETTINGS_INI_FILE);
            }
                /* Last active UV lamp was  */
            else
            {
                /* Switch on lamp set 2 */
                uvLight2On();
                unitUVLampUsedAddOne(2);
                active_uv_lamp = 2;
                /* Reset the successful run setting */
                ini_putl(DEVICE_SECTION_NAME, UV_LAMP_2_RUN_SUCCESS_KEY_NAME, 0, SETTINGS_INI_FILE);
            }
            /* Save the new value */
            ini_putl(DEVICE_SECTION_NAME, LAST_ACTIVE_UV_LAMP_KEY_NAME, active_uv_lamp, SETTINGS_INI_FILE);


            /* Display screen G */
            playAviFile(VIDEO_SCREEN_G, false, NULL);
            /* Reset sample buffer and counter */
            memset(uv_sensor_samples, 0, sizeof(uv_sensor_samples));
            uv_sensor_sample_counter = 0;
            /* Ignore first sample */
            uvLightSensorGetSample();
            /* Wait for timer to end */
            while(millisecondGetTimePassed(uv_lamp_timer) <= UV_LAMP_FUNCTIONALITY_CHECK_TIME)
            {
                uv_sensor_samples[uv_sensor_sample_counter++] = (uint16_t)uvLightSensorGetSample();
                osDelay(UV_LAMP_FUNCTIONALITY_CHECK_TIME/NUMBER_OF_UV_SENSOR_SAMPLES);
                /* Lid opened unauthorized */
                if(isLidClosed() == 0)
                {

                    //Tom:ADD35
                    uvLight1Off();
                    uvLight2Off();
                    unit_disinfection_state = UNIT_DISINFECTION_STATE_LID_OPENED_UNAUTHORIZED_ERROR;
                    goto DISIN_BREAK;
                }
            }
            /* Calculate the average sensor value for the active UV lamp */
            uv_lamp_sensor_value = 0;
            for(uint16_t i = 0; i < uv_sensor_sample_counter; i++)
                uv_lamp_sensor_value += uv_sensor_samples[i];
            uv_lamp_sensor_value /= uv_sensor_sample_counter;
            debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "UV lamp sensor value: %ld\r\n", uv_lamp_sensor_value);
            /* Turn off the UV lamps */
            uvLight1Off();
            uvLight2Off();
            /* Wait for the video and audio to end */
            while((isAudioPlaying() == 1) || (isVideoPlaying() == 1));
            /* Start the functionality test of the second UV lamp if the lid is still closed */
        DISIN_BREAK:
            if(isLidClosed() == 1)
                unit_disinfection_state = UNIT_DISINFECTION_STATE_OPEN_LID;
            else
                unit_disinfection_state = UNIT_DISINFECTION_STATE_LID_OPENED_UNAUTHORIZED_ERROR;
            break;

        case UNIT_DISINFECTION_STATE_LID_OPENED_UNAUTHORIZED_ERROR:
            debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "UNIT_DISINFECTION_STATE_LID_OPENED_UNAUTHORIZED_ERROR\r\n");
            /* Turn off the both UV lamps */
            uvLight1Off();
            uvLight2Off();

            //Tom:ADD36
            /* Save the outcome */
            unitSaveOutcome(UNIT_OUTCOME_UNSUCCESSFUL, UNIT_ERROR_003);
            /* RGB red on */
            rgbRedLedOn();
            /* Display screen I */
            playAviFile(VIDEO_SCREEN_I, true, AUDIO_WARNING);

            /* Wait for lid to close */

            //Tom:ADD3
            {
                uint32_t wait_close_lid_timer = millisecondGetTime();
                while(millisecondGetTimePassed(wait_close_lid_timer) <= VARIABLE_S)
                {
                    if(isLidClosed() == 1)
                    {
                        //Tom: Closed within the VARIABLE_S time.

                        //Because UNIT_DISINFECTION_STATE_IS_LID_CLOSED State ALWAYS flips, I will flip it here too.    :-)  :-)
                        unitDisinfectionRun_FlipLastUsedLampset();

                        unit_disinfection_state = UNIT_DISINFECTION_STATE_IS_LID_CLOSED;
                        break;
                    }
                }
            }



            //Tom: REMOVED:  while(isLidClosed() == 0);
            /* RGB red off */
            rgbAllLedsOff();
            /* Restart process at UNIT_DISINFECTION_STATE_IS_LID_CLOSED */
            unitDisinfectionRun_GotoIdle();

            break;



        case UNIT_DISINFECTION_STATE_UV_LIGHT_X_ERROR:
            debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "UNIT_DISINFECTION_STATE_UV_LIGHT_X_ERROR\r\n");

            /* RGB red on */
            rgbRedLedOn();
            /* Display screen J */
            playAviFile(VIDEO_SCREEN_J_1, true, NULL); //Repeat

            /* Set the out-of-service error */
            ini_putl(DEVICE_SECTION_NAME, OUT_OF_SERVICE_ERROR_KEY_NAME, UNIT_ERROR_001, SETTINGS_INI_FILE);

            /* Wait */
            while(1);
            break;

        case UNIT_DISINFECTION_STATE_SUCCESSFUL:
            debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "UNIT_DISINFECTION_STATE_SUCCESSFUL\r\n");

            ini_putl(DEVICE_SECTION_NAME, (active_uv_lamp == 1) ? UV_LAMP_1_RUN_SUCCESS_KEY_NAME : UV_LAMP_2_RUN_SUCCESS_KEY_NAME, 1, SETTINGS_INI_FILE);


            /* RGB green on */
            rgbGreenLedOn();
            /* Display screen H */
            playAviFile(VIDEO_SCREEN_H, false, AUDIO_POSITIVE);
            /* Wait for the video and audio to end */
            while((isAudioPlaying() == 1) || (isVideoPlaying() == 1));

            /* RGB green off */
            rgbAllLedsOff();

            //LoggingTimeStampNow(LOGGING_LAST_DISINF_SUCCESS_KEY_NAME);


            unit_disinfection_state = UNIT_DISINFECTION_STATE_CHECK_RUNS;
            break;

        case UNIT_DISINFECTION_STATE_CHECK_RUNS:
            debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "UNIT_DISINFECTION_STATE_CHECK_RUNS\r\n");

            run_count = (uint8_t)ini_getl(DEVICE_SECTION_NAME, RUN_COUNT_KEY_NAME, VARIABLE_V, SETTINGS_INI_FILE);
            run_count++;
            ini_putl(DEVICE_SECTION_NAME, RUN_COUNT_KEY_NAME, run_count, SETTINGS_INI_FILE);




            unit_disinfection_state = UNIT_DISINFECTION_STATE_START;  //Call it IDLE

            break;

        default:
            break;
    }
}

/**
  * @brief  Save the outcome of a run into the outcome file
  * @param  result: result of the run (UnitOutcome)
  * 		error: error detected during the run (UnitError)
  * @retval None
  */
static void unitSaveOutcome(UnitOutcome result, UnitError error) {
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
    if (f_stat("0://RESULT", &file_info) != FR_OK)
        f_mkdir("0://RESULT");

    char file_name[32] = "0://RESULT/2019-01.TXT";
    snprintf(file_name, 32, "0://RESULT/%.4d-%.2d.TXT", (date.year + 2000), date.month);
    /* Try to open the file */
    if (f_open(&outcome_file, file_name, FA_WRITE | FA_OPEN_APPEND) != FR_OK) {
        debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "Failed to open outcome file\r\n");
        return;
    }

    uint16_t length = snprintf(file_buffer, sizeof(file_buffer), "%s,%.2d:%.2d:%.2d-%.2d:%.2d:%.2d,%.3d\r\n",
                               (char *) OUTCOME_STRINGS[result], time.hours, time.minutes, time.seconds, date.day,
                               date.month, date.year, error);
    uint32_t number_of_bytes_written = 0;
    /* Write the result to the file and close it afterwards */
    if (f_write(&outcome_file, file_buffer, length, (UINT * ) & number_of_bytes_written) != FR_OK) {
        debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "Failed to write to outcome file\r\n");
    }
    f_close(&outcome_file);
    /* Free the shared resource */
    xSemaphoreGive(StorageSemaphore);
}
