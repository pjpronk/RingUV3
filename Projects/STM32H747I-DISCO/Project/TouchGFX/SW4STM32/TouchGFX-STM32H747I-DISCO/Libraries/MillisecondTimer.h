#ifndef MILLISECONDTIMER_H_
#define MILLISECONDTIMER_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "Config.h"

void millisecondTimerInitialization(void);
uint32_t millisecondGetTimePassed(uint32_t milliseconds_start);
void millisecondDelay(uint32_t milliseconds);
uint32_t millisecondGetTime(void);

#ifdef __cplusplus
}
#endif

#endif /* MILLISECONDTIMER_H_ */
