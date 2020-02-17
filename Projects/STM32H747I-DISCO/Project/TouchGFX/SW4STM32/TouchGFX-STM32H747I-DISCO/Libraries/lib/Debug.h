#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <errno.h>
#include <sys/unistd.h>
#include "stdbool.h"

typedef enum
{
  DEBUG_DISABLED,
  DEBUG_ENABLED,
}DebugState;

typedef enum
{
  DEBUG_LEVEL_ERROR,
  DEBUG_LEVEL_WARNING,
  DEBUG_LEVEL_INFORMATION,
  DEBUG_LEVEL_DEBUG
}DebugLevel;

/* Enable/disable debug groups */
#define DEBUG_GROUP_MAIN				DEBUG_ENABLED
#define DEBUG_GROUP_UV_LIGHT			DEBUG_ENABLED
#define DEBUG_GROUP_RTC					DEBUG_ENABLED

/* Debug level, increase or decrease the debug level (DebugLevel) to allow more or less debug information */
#define DEBUG_LEVEL 		DEBUG_LEVEL_INFORMATION

#define DEBUG_NONE			0
#define DEBUG_SERIAL		1
#define DEBUG_SEMIHOSTING	2

#define DEBUG_OUTPUT		DEBUG_SERIAL

/*
* @brief  Print a debug message
* @param  debug: debug group that should be enabled (DEBUG_ENABLED) to allow it to print the message
*         level: level of debugging importance
*         *fmt: pointer to buffer
*         ...: variadic argument list containing the arguments required for printing
* @retval None
*/
#if (DEBUG_OUTPUT == DEBUG_SERIAL)
#define debug(debug, level, fmt, ...) do { if(debug == DEBUG_ENABLED && level <= DEBUG_LEVEL) printf(fmt, ##__VA_ARGS__); } while(0)
#elif (DEBUG_OUTPUT == DEBUG_SEMIHOSTING)
#define debug(debug, level, fmt, ...) do { printf(fmt, ##__VA_ARGS__); } while(0)
#else
#define debug(debug, level, fmt, ...)
#endif

void writeDebug(char debug_info[], bool writeIni);

#if (DEBUG_OUTPUT == DEBUG_SERIAL)
void debugSerialInitilization(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H_ */
