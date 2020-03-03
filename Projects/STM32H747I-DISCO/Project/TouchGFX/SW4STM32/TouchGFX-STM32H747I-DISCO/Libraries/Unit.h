#ifndef UNIT_H_
#define UNIT_H_

#include "lib/Config.h"
#include "cmsis_os.h"
#include "lib/Storage.h"
#include "lib/Rgb.h"
#include "lib/UvLight.h"
#include "lib/Lid.h"
#include "lib/Audio.h"
#include "lib/MillisecondTimer.h"
#include "lib/Debug.h"
#include "lib/RealTimeClock.h"

#include "minIni.h"


#ifdef __cplusplus
 extern "C" {
#endif


extern osMessageQId UnitEvent;

/* Settings */
static const char UV_SENSOR_SECTION_NAME[] = 	"UV Sensor";
static const char UV_CALIBRATION_1_KEY_NAME[] = "UV calibration 1";
static const char UV_CALIBRATION_2_KEY_NAME[] = "UV calibration 2";
static const char UV_SENSOR_VALUE_1_KEY_NAME[] = "UV last run sensor 1";
static const char UV_SENSOR_VALUE_2_KEY_NAME[] = "UV last run sensor 2";


static const char DEVICE_SECTION_NAME[] = 				"Device";
static const char OUT_OF_SERVICE_ERROR_KEY_NAME[] = 	"Out of service error";
static const char LAST_RUN_TIMESTAMP_KEY_NAME[] = 		"Last run timestamp";
static const char LAST_ACTIVE_UV_LAMP_KEY_NAME[] = 		"Last active UV lamp";

static const char RUN_COUNT_KEY_NAME[] = 				"Run count";
static const char UV_LAMP_1_FAIL_COUNT_KEY_NAME[] =     "UV lamp 1 fail count";
static const char UV_LAMP_2_FAIL_COUNT_KEY_NAME[] =     "UV lamp 2 fail count";

static const char UV_LAMP_WEARDOWN_MAX_KEY_NAME[] =     "Lamps max wear down";
static const char SOLENOID_FAIL_COUNTER[] =             "Solenoid fail counter";


/* Audio */
static const char AUDIO_STARTUP[] = 	"0://AUDIO/STARTUP.WAV";
static const char AUDIO_ATTENTION[] = 	"0://AUDIO/ATTEN.WAV";
static const char AUDIO_POSITIVE[] = 	"0://AUDIO/SUCCESS.WAV";
static const char AUDIO_WARNING[] = 	"0://AUDIO/WARNING.WAV";

/* Video */
static const char VIDEO_SCREEN_A[] = 	"0://VIDEO/A.AVI";
static const char VIDEO_SCREEN_B[] = 	"0://VIDEO/B.AVI";
static const char VIDEO_SCREEN_C[] = 	"0://VIDEO/C.AVI";
static const char VIDEO_SCREEN_D[] = 	"0://VIDEO/D.AVI";
static const char VIDEO_SCREEN_E[] = 	"0://VIDEO/E.AVI";
static const char VIDEO_SCREEN_F_1[] = 	"0://VIDEO/F-1.AVI";
static const char VIDEO_SCREEN_F_2[] = 	"0://VIDEO/F-2.AVI";
static const char VIDEO_SCREEN_G[] = 	"0://VIDEO/G.AVI";
static const char VIDEO_SCREEN_H[] = 	"0://VIDEO/H.AVI";
static const char VIDEO_SCREEN_I[] = 	"0://VIDEO/I.AVI";
static const char VIDEO_SCREEN_J_1[] = 	"0://VIDEO/J-1.AVI";
static const char VIDEO_SCREEN_J_2[] = 	"0://VIDEO/J-2.AVI";
//Tom:Start
static const char VIDEO_SCREEN_J_3[] = 	"0://VIDEO/J-3.AVI";
//Tom:End
static const char VIDEO_SCREEN_K[] = 	"0://VIDEO/K.AVI";
static const char VIDEO_SCREEN_L[] = 	"0://VIDEO/L.AVI";

//Tom:Start Added VIDEO_SCREEN_M
static const char VIDEO_SCREEN_M[] = 	"0://VIDEO/M.AVI";


/* Functionality */
#define UV_LAMP_FUNCTIONALITY_CHECK_TIME 	27000	//In ms
#define NUMBER_OF_UV_SENSOR_SAMPLES 		100
#define MINIMUM_SENSOR_THRESHOLD_UV_LAMP_1	1000
#define MINIMUM_SENSOR_THRESHOLD_UV_LAMP_2	1000
#define UV_LAMP_MAX_WEAR_DOWN_PERCENT 0.3

//Tom:ADD10
#define VARIABLE_Q (31104000L)      // 12month * 30days * 24hours * 60 * 60  (Sec)
#define VARIABLE_R (46656000L)      // 18 month * 30 * 24 * 60 * 60
//Tom:ADD18


//Tom:ADD4
#define VARIABLE_S (60000)          // 10 Sec in MiliSec.
//Tom:ADD4-END

#define VARIABLE_T 0.8				//Percentage Max Lamp wear down
#define VARIABLE_U (24 * 60 * 60)	//Hours (in seconds)
#define VARIABLE_V 50				//Number of times
#define VARIABLE_W (24 * 60 * 60)	//Hours (in seconds)
#define VARIABLE_X 6				//Number of times
#define VARIABLE_Y 3				//Number of times
#define VARIABLE_Z 2				//Number of times


typedef enum {
    UV_LAMP_1 = 1,
    UV_LAMP_2 = 2
}UvLamp;

typedef enum {
    UNIT_ERROR_001,	//Can't guarantee successful disinfection. Both lamp sets are not performing accordingly.
    UNIT_ERROR_002,	//Can't open automatically
    UNIT_ERROR_003,	//Unauthorized open detected
}UnitError;

typedef enum {
    UNIT_INSTALLATION_RUN,
    UNIT_DISINFECTION_RUN,
}UnitRun;

typedef enum {
    UNIT_INSTALLATION_STATE_START,                          //0
    UNIT_INSTALLATION_STATE_OPEN_LID,	                    //1
    UNIT_INSTALLATION_STATE_CHECK_UV_LAMPS,					//2
    UNIT_INSTALLATION_STATE_LID_OPENED_UNAUTHORIZED_ERROR,	//3
    UNIT_INSTALLATION_STATE_CHECK_RUNS,						//4
    UNIT_INSTALLATION_STATE_SUCCESSFUL,						//5
    UNIT_INSTALLATION_STATE_UNSUCCESSFUL,					//6
    UNIT_INSTALLATION_STATE_BREAK,						//7
}UnitInstallationState;

typedef enum {
	UNIT_DISINFECTION_STATE_START,							//
	UNIT_DISINFECTION_STATE_DISINFECTION,					//2
	UNIT_DISINFECTION_STATE_LID_OPENED_UNAUTHORIZED_ERROR,	//3
    UNIT_DISINFECTION_STATE_CHECK_RUNS,						//4
	UNIT_DISINFECTION_STATE_SUCCESSFUL,						//5
    UNIT_DISINFECTION_STATE_UNSUCCESSFUL,					//6
	UNIT_DISINFECTION_STATE_BREAK,					//7

}UnitDisinfectionState;

void unitInitialization(void);
void DisplayBlockingError(UnitError error_code);
void OpenLid(void);
int RunLampCycle(UvLamp lamp_set);

#ifdef __cplusplus
}
#endif

#endif /* UNIT_H_ */
