#ifndef RGB_H_
#define RGB_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "Config.h"

void rgbInitialization(void);
void rgbOrangeLedOn(void);
void rgbRedLedOn(void);
uint8_t rgbIsRedLedOn(void);
void rgbGreenLedOn(void);
uint8_t rgbIsGreenLedOn(void);
void rgbBlueLedOn(void);
uint8_t rgbIsBlueLedOn(void);
void rgbWhiteLedOn(void);
void rgbAllLedsOff(void);
void rgbWhiteLedBlink(void);

#ifdef __cplusplus
}
#endif

#endif /* RGB_H_ */
