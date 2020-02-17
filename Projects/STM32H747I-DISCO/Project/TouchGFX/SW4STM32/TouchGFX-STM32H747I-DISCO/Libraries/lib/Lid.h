#ifndef LID_H_
#define LID_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "Config.h"

typedef enum {
	LID_INPUT_STATE_NOT_ACTIVE,
	LID_INPUT_STATE_ACTIVE,
}LidInputState;

typedef struct {
	uint8_t input;				//Lid input
	uint8_t change;				//State changed
	uint32_t active_timer;		//Active (upper flank) timer
	uint32_t not_active_timer;	//Not active (lower flank) timer
	uint8_t output;				//Debounced input
	LidInputState state;		//State of the input
}LidInput;

#define ACTIVE_DEBOUNCE_TIME		50	//Minimum active (upper flank) debounce time in ms
#define NOT_ACTIVE_DEBOUNCE_TIME	50	//Minimum not active (lower flank) debounce time in ms

void lidInitialization(void);
void lidActivateSolenoid(void);
void lidDeactivateSolenoid(void);
uint8_t isLidClosed(void);

#ifdef __cplusplus
}
#endif

#endif /* LID_H_ */
