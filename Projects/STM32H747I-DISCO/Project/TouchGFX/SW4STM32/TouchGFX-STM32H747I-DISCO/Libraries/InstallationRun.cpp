#include "InstallationRun.h"
#include "Unit.h"
#include <gui/video_player_screen/VideoPlayerView.hpp>


UnitInstallationState unit_installation_state = UNIT_INSTALLATION_STATE_START;

static uint8_t uv_light_error_counter = 0;	//Number of times a UV light error has been detected
static int uv_lamp_1_sensor_value = 0;	//UV sensor value for lamp 1
static int uv_lamp_2_sensor_value = 0;	//UV sensor value for lamp 2


/**
  * @brief  case: INSTALLATION_STATE_START
  */
UnitInstallationState installation_state_start(void) {
    writeDebug("Installation run started.", true);

    /* Set and clear certain INI values */
    ini_putl(DEVICE_SECTION_NAME, UV_LAMP_1_FAIL_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE);
    ini_putl(DEVICE_SECTION_NAME, UV_LAMP_2_FAIL_COUNT_KEY_NAME, 0, SETTINGS_INI_FILE);


    playAviFile(VIDEO_SCREEN_C, true, AUDIO_ATTENTION); //Play on repeat

    /* While lid is not closed, wait */
    while (isLidClosed() == 0) {};

    return UNIT_INSTALLATION_STATE_CHECK_UV_LAMPS;
}

/**
  * @brief  case: INSTALLATION_STATE_CHECK_UV_LAMPS
  */
UnitInstallationState installation_state_check_uv_lamps(void) {
    /* RGB off */
    rgbAllLedsOff();
    /* Display screen M */
    playAviFile(VIDEO_SCREEN_M, false, NULL);

    uv_lamp_1_sensor_value = RunLampCycle(UV_LAMP_1);

    if(uv_lamp_1_sensor_value == -1){
        return UNIT_INSTALLATION_STATE_LID_OPENED_UNAUTHORIZED_ERROR;
    }

    uv_lamp_2_sensor_value = RunLampCycle(UV_LAMP_2);

    if(uv_lamp_2_sensor_value == -1){
        return UNIT_INSTALLATION_STATE_LID_OPENED_UNAUTHORIZED_ERROR;
    }

    /* Wait for the video and audio to end */
    while ((isAudioPlaying() == 1) || (isVideoPlaying() == 1));

    return UNIT_INSTALLATION_STATE_CHECK_RUNS;
};


/**
  * @brief  case: INSTALLATION_STATE_LID_OPENED_UNAUTHORIZED
  */
UnitInstallationState installation_state_lid_opened_unauthorized_error(void){
    writeDebug("Run interrupted due to lid opening.", false);

    /* Turn off both UV lamps */
    uvLight1Off();
    uvLight2Off();

    /* RGB red on */
    rgbRedLedOn();
    /* Display screen E */
    playAviFile(VIDEO_SCREEN_E, true, AUDIO_WARNING); //Repeat


    /* Wait for lid to close */
    while (isLidClosed() == 0);
    /* RGB red off */
    rgbAllLedsOff();

    return UNIT_INSTALLATION_STATE_CHECK_UV_LAMPS;
}

/**
  * @brief  case: INSTALLATION_STATE_CHECK_RUNS
  */
UnitInstallationState installation_state_check_runs(void) {
    OpenLid();
    /* Log the sensor values for both runs. */
    ini_putl(UV_SENSOR_SECTION_NAME, UV_SENSOR_VALUE_1_KEY_NAME, uv_lamp_1_sensor_value, SETTINGS_INI_FILE);
    ini_putl(UV_SENSOR_SECTION_NAME, UV_SENSOR_VALUE_2_KEY_NAME, uv_lamp_2_sensor_value, SETTINGS_INI_FILE);

    /* Check if the minimum threshold is reached */
    if ((uv_lamp_1_sensor_value >= MINIMUM_SENSOR_THRESHOLD_UV_LAMP_1) &&
        (uv_lamp_2_sensor_value >= MINIMUM_SENSOR_THRESHOLD_UV_LAMP_2)) {
        writeDebug("Calibration successful.", true);
        return UNIT_INSTALLATION_STATE_SUCCESSFUL;
    }
    else {
        writeDebug("Calibration failed.", true);
        return UNIT_INSTALLATION_STATE_UNSUCCESSFUL;
    }
};

/**
  * @brief  case: INSTALLATION_STATE_SUCCESSFUL
  */
UnitInstallationState installation_state_successful(void) {
    uv_light_error_counter = 0;

    /* Save the calibration values */
    ini_putl(UV_SENSOR_SECTION_NAME, UV_CALIBRATION_1_KEY_NAME, uv_lamp_1_sensor_value, SETTINGS_INI_FILE);
    ini_putl(UV_SENSOR_SECTION_NAME, UV_CALIBRATION_2_KEY_NAME, uv_lamp_2_sensor_value, SETTINGS_INI_FILE);

    /* RGB green on */
    rgbGreenLedOn();
    playAviFile(VIDEO_SCREEN_D, false, AUDIO_POSITIVE);

    /* Wait for the video and audio to end */
    while ((isAudioPlaying() == 1) || (isVideoPlaying() == 1));

    rgbAllLedsOff();

    return UNIT_INSTALLATION_STATE_START;
};

/**
  * @brief  case: INSTALLATION_STATE_UNSUCCESSFUL
  */
UnitInstallationState installation_state_unsuccessful(void){
    rgbRedLedOn();

    uv_light_error_counter++;

    if(uv_light_error_counter >= VARIABLE_Y){
        DisplayBlockingError(UNIT_ERROR_001);
    }

    playAviFile(VIDEO_SCREEN_E, true, AUDIO_WARNING);

    /* Wait till lid has been closed. */
    while (isLidClosed() == 0){};

    /* Try again to verify results. */
    return UNIT_INSTALLATION_STATE_CHECK_UV_LAMPS;
};



/**
  * @brief  Base function for installation state.
  *         Cycles through the different states the Unit can take during installation
  */
UnitRun unitInstallationRun(void){


    switch (unit_installation_state) {
        case UNIT_INSTALLATION_STATE_START:
            unit_installation_state = installation_state_start();
            break;

        case UNIT_INSTALLATION_STATE_CHECK_UV_LAMPS:
            unit_installation_state = installation_state_check_uv_lamps();
            break;

        case UNIT_INSTALLATION_STATE_LID_OPENED_UNAUTHORIZED_ERROR:
            unit_installation_state = installation_state_lid_opened_unauthorized_error();
            break;

        case UNIT_INSTALLATION_STATE_CHECK_RUNS:
            unit_installation_state = installation_state_check_runs();
            break;

        case UNIT_INSTALLATION_STATE_SUCCESSFUL:
            unit_installation_state = installation_state_successful();
            /* Exit point */
            return UNIT_DISINFECTION_RUN;

        case UNIT_INSTALLATION_STATE_UNSUCCESSFUL:
            unit_installation_state = installation_state_unsuccessful();
            break;

        case UNIT_INSTALLATION_STATE_BREAK:
            unit_installation_state = UNIT_INSTALLATION_STATE_START;
            return UNIT_DISINFECTION_RUN;
    }
    return UNIT_INSTALLATION_RUN;
}
