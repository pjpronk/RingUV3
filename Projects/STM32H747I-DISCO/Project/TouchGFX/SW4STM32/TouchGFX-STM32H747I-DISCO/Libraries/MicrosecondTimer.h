#ifndef MICROSECONDTIMER_H_
#define MICROSECONDTIMER_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "Config.h"

void microsecondTimerInitialization(void);
uint32_t microsecondGetTimePassed(uint32_t microseconds_start);
void microsecondDelay(uint32_t microseconds);
uint32_t microsecondGetTime(void);

#ifdef __cplusplus
}
#endif

#endif /* MICROSECONDTIMER_H_ */
