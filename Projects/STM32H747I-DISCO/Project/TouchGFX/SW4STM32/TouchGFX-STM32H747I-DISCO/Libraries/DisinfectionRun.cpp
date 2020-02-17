#include "DisinfectionRun.h"
#include "Unit.h"
#include <gui/video_player_screen/VideoPlayerView.hpp>


static UnitDisinfectionState unit_disinfection_state = UNIT_DISINFECTION_STATE_START;
static int active_uv_lamp;
static int uv_lamp_sensor_value;



/**
  * @brief  case: UNIT_DISINFECTION_STATE_START
  */
UnitDisinfectionState disinfection_state_start(){

    int installation_run_timestamp = ini_getl(DEVICE_SECTION_NAME, INSTALLATION_TIMESTAMP_KEY_NAME, 0, SETTINGS_INI_FILE);
    int time_since_installation = rtcGetUnixTime() - installation_run_timestamp;

    /* If installation timestamp is 0, no installation run has been performed */
    if(installation_run_timestamp == 0) {
        return UNIT_DISINFECTION_STATE_BREAK;
    }

    /* If 18 months have passed since installation we go into error mode. */
    if(time_since_installation >= VARIABLE_R){
        DisplayBlockingError(UNIT_ERROR_003);
    }

    /* If 12 months have passed since the installation run we ensure the user gets a warning when using the device */
    if(time_since_installation >= VARIABLE_Q){
        playAviFile(VIDEO_SCREEN_F_2, true, NULL);
    } else {
        playAviFile(VIDEO_SCREEN_F_1, true, NULL);
    }

    while (isLidClosed() == 0) {};

    /* Update the current timestamp for every run */
    ini_putl(DEVICE_SECTION_NAME, LAST_RUN_TIMESTAMP_KEY_NAME, rtcGetUnixTime(), SETTINGS_INI_FILE);

    return UNIT_DISINFECTION_STATE_DISINFECTION;
}

/**
  * @brief  case: UNIT_DISINFECTION_STATE_DISINFECTION
  */
UnitDisinfectionState disinfection_state_disinfection(){

    rgbAllLedsOff();

    /* Get last active uv lamp from INI file */
    int last_active_uv_lamp = (uint8_t) ini_getl(DEVICE_SECTION_NAME, LAST_ACTIVE_UV_LAMP_KEY_NAME, 1,
                                             SETTINGS_INI_FILE);

    uint32_t lamp_1_fail_count = ini_getl(UV_SENSOR_SECTION_NAME, UV_LAMP_1_FAIL_COUNT_KEY_NAME, 0,
                                          SETTINGS_INI_FILE);
    uint32_t lamp_2_fail_count = ini_getl(UV_SENSOR_SECTION_NAME, UV_LAMP_2_FAIL_COUNT_KEY_NAME, 0,
                                          SETTINGS_INI_FILE);


    if(lamp_1_fail_count > 4 && lamp_2_fail_count > 4){
        /* Something is definitely wrong with the machine. */
        DisplayBlockingError(UNIT_ERROR_001);
    }


    /* Lamp 1 has failed */
    if (lamp_1_fail_count > 3) {
        /* Switch on lamp set 1 */
        active_uv_lamp = 2;
        writeDebug("Disinfection started on light 2", false);
    }
        /* Lamp 2 has failed */
    else if (lamp_2_fail_count > 3) {
        /* Switch on lamp set 1 */
        active_uv_lamp = 1;
        writeDebug("Disinfection started on light 1", false);
    }
        /* Both lamps not excluded yet.*/
    else{
        if(last_active_uv_lamp == 1){
            active_uv_lamp = 2;
            writeDebug("Disinfection started on light 2", false);
        } else if (last_active_uv_lamp == 2){
            active_uv_lamp = 1;
            writeDebug("Disinfection started on light 1", false);
        }
    }

    /* Display screen G */
    playAviFile(VIDEO_SCREEN_G, false, NULL);

    uv_lamp_sensor_value = RunLampCycle((UvLamp) active_uv_lamp);

    //Double break to escape while loop and case
    if(uv_lamp_sensor_value == -1){
        return UNIT_DISINFECTION_STATE_LID_OPENED_UNAUTHORIZED_ERROR;
    }

    /* Wait for the video and audio to end */
    while ((isAudioPlaying() == 1) || (isVideoPlaying() == 1));

    if (isLidClosed() == 1) {
        OpenLid();
    }

    return UNIT_DISINFECTION_STATE_CHECK_RUNS;

}


/**
  * @brief  case: UNIT_DISINFECTION_STATE_LID_OPENED_UNAUTHORIZED_ERROR
  */
UnitDisinfectionState disinfection_state_lid_opened_unauthorized_error(){

    /* Turn off both UV lamps */
    uvLight1Off();
    uvLight2Off();

    writeDebug("Disinfection opened unauthorized\n", false);
    /* RGB red on */
    rgbRedLedOn();
    /* Display screen I */
    playAviFile(VIDEO_SCREEN_I, true, NULL);

    /* Wait for lid to close within VARIABLE_S time */
    uint32_t wait_close_lid_timer = millisecondGetTime();
    while (millisecondGetTimePassed(wait_close_lid_timer) <= VARIABLE_S) {
        if (isLidClosed() == 1) {
            return UNIT_DISINFECTION_STATE_DISINFECTION;
        }
    }

    writeDebug("Disinfection failed because of unauthorized opening\n", false);

    /* RGB red off */
    rgbAllLedsOff();
    return UNIT_DISINFECTION_STATE_OPEN_LID;
}

/**
  * @brief  case: UNIT_DISINFECTION_STATE_CHECK_RUNS
  * Read carefully.
  *
  */
UnitDisinfectionState disinfection_state_check_runs(){


    const char* UV_CALIBRATION_KEY_NAME = (active_uv_lamp == 1) ? UV_CALIBRATION_1_KEY_NAME : UV_CALIBRATION_2_KEY_NAME;
    const char* UV_SENSOR_VALUE_KEY_NAME = (active_uv_lamp == 1) ? UV_SENSOR_VALUE_1_KEY_NAME : UV_SENSOR_VALUE_2_KEY_NAME;
    const char* UV_LAMP_SUCCESS_COUNT_KEY_NAME = (active_uv_lamp == 1) ? UV_LAMP_1_SUCCESS_COUNT_KEY_NAME : UV_LAMP_2_SUCCESS_COUNT_KEY_NAME;
    const char* UV_LAMP_FAIL_COUNT_KEY_NAME = (active_uv_lamp == 1) ? UV_LAMP_1_FAIL_COUNT_KEY_NAME : UV_LAMP_2_FAIL_COUNT_KEY_NAME;

    int uv_lamp_calibration_value = ini_getl(UV_SENSOR_SECTION_NAME, UV_CALIBRATION_KEY_NAME, 0, SETTINGS_INI_FILE);
    
    /* Set sensor values of this run. */
    ini_putl(UV_SENSOR_SECTION_NAME, UV_SENSOR_VALUE_KEY_NAME, uv_lamp_sensor_value, SETTINGS_INI_FILE);


    /* Get maximum wear down percent from file. Is 0.7 if nothing is specified. */

    float max_wear_down_percent = ini_getl(DEVICE_SECTION_NAME, UV_LAMP_WEARDOWN_MAX_KEY_NAME, 0, SETTINGS_INI_FILE);
    if(max_wear_down_percent == 0 ) max_wear_down_percent = 0.7;

    float actual_wear_down_percent = (float) uv_lamp_sensor_value / (float) uv_lamp_calibration_value;

    /* Change last active UV lamp. */
    ini_putl(DEVICE_SECTION_NAME, LAST_ACTIVE_UV_LAMP_KEY_NAME, active_uv_lamp, SETTINGS_INI_FILE);

    if (actual_wear_down_percent > max_wear_down_percent) {
        /* Lamp set is functioning correctly. */
        /* Add to the successful run to performed runs. */
        int successful_runs = ini_getl(DEVICE_SECTION_NAME, UV_LAMP_SUCCESS_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE);
        ini_putl(DEVICE_SECTION_NAME, UV_LAMP_SUCCESS_COUNT_KEY_NAME, successful_runs + 1, SETTINGS_INI_FILE);
        /* Set fail count to 0 */
        ini_putl(DEVICE_SECTION_NAME, UV_LAMP_FAIL_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE);

        return UNIT_DISINFECTION_STATE_SUCCESSFUL;
    } else {
        /* Add to the failed run to performed runs. */
        int failed_runs = ini_getl(DEVICE_SECTION_NAME, UV_LAMP_FAIL_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE);
        ini_putl(DEVICE_SECTION_NAME, UV_LAMP_FAIL_COUNT_KEY_NAME, failed_runs + 1, SETTINGS_INI_FILE);
        /* Go into unsuccessful disinfection run. */
        return UNIT_DISINFECTION_STATE_UNSUCCESSFUL;

    }

}

/**
  * @brief  case: UNIT_DISINFECTION_STATE_SUCCESSFUL
  */
UnitDisinfectionState disinfection_state_successful(){
    writeDebug("Successful disinfection", true);
    rgbGreenLedOn();
    playAviFile(VIDEO_SCREEN_H, false, NULL);

    /* Wait for the video and audio to end */
    while ((isAudioPlaying() == 1) || (isVideoPlaying() == 1));

    rgbAllLedsOff();
    return UNIT_DISINFECTION_STATE_OPEN_LID;
}

/**
  * @brief  case: UNIT_DISINFECTION_STATE_UNSUCCESSFUL
  */
UnitDisinfectionState disinfection_state_unsuccessful(){
    writeDebug("Unsuccessful disinfection", true);
    rgbRedLedOn();
    playAviFile(VIDEO_SCREEN_I, true, NULL);

    while(isLidClosed() == 0){};

    rgbAllLedsOff();
    return UNIT_DISINFECTION_STATE_DISINFECTION;
}



UnitRun unitDisinfectionRun(){

    switch (unit_disinfection_state) {
        case UNIT_DISINFECTION_STATE_START:
            unit_disinfection_state = disinfection_state_start();
            if(unit_disinfection_state == UNIT_DISINFECTION_STATE_BREAK) {
                return UNIT_INSTALLATION_RUN;
            }
            break;

        case UNIT_DISINFECTION_STATE_OPEN_LID:
            unit_disinfection_state = disinfection_state_open_lid();
            break;

        case UNIT_DISINFECTION_STATE_DISINFECTION:
            unit_disinfection_state = disinfection_state_disinfection();
            break;

        case UNIT_DISINFECTION_STATE_LID_OPENED_UNAUTHORIZED_ERROR:
            unit_disinfection_state = disinfection_state_lid_opened_unauthorized_error();
            break;

        case UNIT_DISINFECTION_STATE_CHECK_RUNS:
            unit_disinfection_state = disinfection_state_check_runs();
            break;

        case UNIT_DISINFECTION_STATE_SUCCESSFUL:
            unit_disinfection_state = disinfection_state_successful();
            break;

        case UNIT_DISINFECTION_STATE_UNSUCCESSFUL:
            unit_disinfection_state = disinfection_state_unsuccessful();
            break;

        case UNIT_DISINFECTION_STATE_BREAK:
            return UNIT_INSTALLATION_RUN;

        default:
            break;

    }
    return UNIT_DISINFECTION_RUN;
}




