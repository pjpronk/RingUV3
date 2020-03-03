#include "DisinfectionRun.h"
#include "Unit.h"
#include <gui/video_player_screen/VideoPlayerView.hpp>


static UnitDisinfectionState unit_disinfection_state = UNIT_DISINFECTION_STATE_START;
static int active_uv_lamp;
static int uv_lamp_sensor_value;

UvLamp SelectUVLamp() {
    /* Get last active uv lamp from INI file */
    int last_active_uv_lamp = (uint8_t) ini_getl(DEVICE_SECTION_NAME, LAST_ACTIVE_UV_LAMP_KEY_NAME, 1,
                                                 SETTINGS_INI_FILE);

    uint32_t lamp_1_fail_count = ini_getl(DEVICE_SECTION_NAME, UV_LAMP_1_FAIL_COUNT_KEY_NAME, 0,
                                          SETTINGS_INI_FILE);
    uint32_t lamp_2_fail_count = ini_getl(DEVICE_SECTION_NAME, UV_LAMP_2_FAIL_COUNT_KEY_NAME, 0,
                                          SETTINGS_INI_FILE);


    if((lamp_1_fail_count > VARIABLE_Y) && (lamp_2_fail_count > VARIABLE_Y)){
        /* Something is definitely wrong with the machine. */
        DisplayBlockingError(UNIT_ERROR_001);
    }


    /* Lamp 1 has failed */
    if (lamp_1_fail_count > VARIABLE_Y) {
        /* Switch on lamp set 2 */
        return UV_LAMP_2;
    }

    /* Lamp 2 has failed */
    if (lamp_2_fail_count > VARIABLE_Y) {
        /* Switch on lamp set 1 */
        return UV_LAMP_1;
    }


    /* Both lamps not excluded yet.*/
    if(last_active_uv_lamp == UV_LAMP_1){
        return UV_LAMP_2;
    } else if (last_active_uv_lamp == UV_LAMP_2) {
    	return UV_LAMP_1;
    }


    /* If no lamp can be selected, we use the first lamp. */
    return UV_LAMP_1;
};





/**
  * @brief  case: UNIT_DISINFECTION_STATE_START
  */
UnitDisinfectionState disinfection_state_start() {

    int time_since_installation = rtcGetUnixTime() - RTC_UNIX_START_TIME;
    int runs_completed = ini_getl(DEVICE_SECTION_NAME, RUN_COUNT_KEY_NAME, 1, SETTINGS_INI_FILE);


    /* If 18 months have passed since installation we go into error mode. */
    if (time_since_installation >= VARIABLE_R) {
        DisplayBlockingError(UNIT_ERROR_003);
    }

    /* If 12 months have passed since the installation run we ensure the user gets a warning when using the device */
    if (time_since_installation >= VARIABLE_Q) {
        playAviFile(VIDEO_SCREEN_F_2, true, NULL);
    }
        /* Every 50 runs we want the device to be cleaned. */
    else if (runs_completed % 50 == 0) {
        playAviFile(VIDEO_SCREEN_K, true, AUDIO_ATTENTION);
    } else {
        playAviFile(VIDEO_SCREEN_F_1, true, NULL);
    }

    while (isLidClosed() == 0) {};

    /* Update the last run timestamp before every run */
    ini_putl(DEVICE_SECTION_NAME, LAST_RUN_TIMESTAMP_KEY_NAME, rtcGetUnixTime(), SETTINGS_INI_FILE);

    return UNIT_DISINFECTION_STATE_DISINFECTION;
}

/**
  * @brief  case: UNIT_DISINFECTION_STATE_DISINFECTION
  */
UnitDisinfectionState disinfection_state_disinfection() {

    rgbAllLedsOff();

    active_uv_lamp = SelectUVLamp();

    /* Display screen G */
    playAviFile(VIDEO_SCREEN_G, false, NULL);

    uv_lamp_sensor_value = RunLampCycle((UvLamp) active_uv_lamp);

    //Double break to escape while loop and case
    if (uv_lamp_sensor_value < 0) {
        return UNIT_DISINFECTION_STATE_LID_OPENED_UNAUTHORIZED_ERROR;
    }

    /* Wait for the video and audio to end */
    while ((isAudioPlaying() == 1) || (isVideoPlaying() == 1));

    OpenLid();

    return UNIT_DISINFECTION_STATE_CHECK_RUNS;

}

/**
  * @brief  case: UNIT_DISINFECTION_STATE_LID_OPENED_UNAUTHORIZED_ERROR
  */
UnitDisinfectionState disinfection_state_lid_opened_unauthorized_error(){

    uvLight1Off();
    uvLight2Off();

    writeDebug("The lid was opened during disinfection.", false);
    rgbRedLedOn();

    /* Display screen I */
    playAviFile(VIDEO_SCREEN_I, true, AUDIO_WARNING);

    /* Wait for lid to close within VARIABLE_S time */
    uint32_t wait_close_lid_timer = millisecondGetTime();
    while (millisecondGetTimePassed(wait_close_lid_timer) <= VARIABLE_S) {
        if (isLidClosed() == 1) {
            return UNIT_DISINFECTION_STATE_DISINFECTION;
        }
    }

    writeDebug("Disinfection failed because lid was opened during disinfection.", false);
    rgbAllLedsOff();

    return UNIT_DISINFECTION_STATE_START;
}

/**
  * @brief  case: UNIT_DISINFECTION_STATE_CHECK_RUNS
  * Read carefully.
  *
  */
UnitDisinfectionState disinfection_state_check_runs(){

    const char* UV_CALIBRATION_KEY_NAME = (active_uv_lamp == 1) ? UV_CALIBRATION_1_KEY_NAME : UV_CALIBRATION_2_KEY_NAME;
    const char* UV_SENSOR_VALUE_KEY_NAME = (active_uv_lamp == 1) ? UV_SENSOR_VALUE_1_KEY_NAME : UV_SENSOR_VALUE_2_KEY_NAME;
    const char* UV_LAMP_FAIL_COUNT_KEY_NAME = (active_uv_lamp == 1) ? UV_LAMP_1_FAIL_COUNT_KEY_NAME : UV_LAMP_2_FAIL_COUNT_KEY_NAME;

    int uv_lamp_calibration_value = ini_getl(UV_SENSOR_SECTION_NAME, UV_CALIBRATION_KEY_NAME, 1000, SETTINGS_INI_FILE);
    
    /* Set sensor values of this run. */
    ini_putl(UV_SENSOR_SECTION_NAME, UV_SENSOR_VALUE_KEY_NAME, uv_lamp_sensor_value, SETTINGS_INI_FILE);

    float max_wear_down_percent = UV_LAMP_MAX_WEAR_DOWN_PERCENT;
    float actual_wear_down_percent = (float) uv_lamp_sensor_value / (float) uv_lamp_calibration_value;

    /* Log last active lamp value. */
    ini_putl(DEVICE_SECTION_NAME, LAST_ACTIVE_UV_LAMP_KEY_NAME, active_uv_lamp, SETTINGS_INI_FILE);

    /* Increase the runs completed by 1 */
    int runs_completed = ini_getl(DEVICE_SECTION_NAME, RUN_COUNT_KEY_NAME, 1, SETTINGS_INI_FILE);
    ini_putl(DEVICE_SECTION_NAME, RUN_COUNT_KEY_NAME, runs_completed + 1, SETTINGS_INI_FILE);


    if (actual_wear_down_percent > max_wear_down_percent) {
        /* Lamp set is functioning correctly. */
        logResult(true, active_uv_lamp, uv_lamp_sensor_value, uv_lamp_calibration_value);

        return UNIT_DISINFECTION_STATE_SUCCESSFUL;

    } else {
        /* Lamp set is functioning correctly. */
        int failed_runs = ini_getl(DEVICE_SECTION_NAME, UV_LAMP_FAIL_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE);
        ini_putl(DEVICE_SECTION_NAME, UV_LAMP_FAIL_COUNT_KEY_NAME, failed_runs + 1, SETTINGS_INI_FILE);

        logResult(false, active_uv_lamp, uv_lamp_sensor_value, uv_lamp_calibration_value);

        return UNIT_DISINFECTION_STATE_UNSUCCESSFUL;
    }

}

/**
  * @brief  case: UNIT_DISINFECTION_STATE_SUCCESSFUL
  */
UnitDisinfectionState disinfection_state_successful(){
    writeDebug("Disinfection run completed successfully.", true);
    rgbGreenLedOn();

    playAviFile(VIDEO_SCREEN_H, false, AUDIO_POSITIVE);

    /* Wait for the video and audio to end */
    while ((isAudioPlaying() == 1) || (isVideoPlaying() == 1));

    rgbAllLedsOff();
    return UNIT_DISINFECTION_STATE_START;
}

/**
  * @brief  case: UNIT_DISINFECTION_STATE_UNSUCCESSFUL
  */
UnitDisinfectionState disinfection_state_unsuccessful(){
    writeDebug("Disinfection run completed unsuccessfully.", true);
    rgbRedLedOn();

    playAviFile(VIDEO_SCREEN_I, true, AUDIO_WARNING);

    while(isLidClosed() == 0){};

    rgbAllLedsOff();
    return UNIT_DISINFECTION_STATE_DISINFECTION;
}


/**
  * @brief  Base function for disinfection state.
  *         Cycles through the different states the Unit can take during disinfection.
  */
UnitRun unitDisinfectionRun(){

    switch (unit_disinfection_state) {
        case UNIT_DISINFECTION_STATE_START:
            unit_disinfection_state = disinfection_state_start();
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

        default:
            unit_disinfection_state = UNIT_DISINFECTION_STATE_START;
            break;

    }
    return UNIT_DISINFECTION_RUN;
}




