#ifndef UVLIGHT_H_
#define UVLIGHT_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "Config.h"

extern ADC_HandleTypeDef adc_handle;
extern DMA_HandleTypeDef adc_dma_handle;

void uvLightInitialization(void);
void uvLight1On(void);
void uvLight1Off(void);
uint8_t isUvLight1On(void);
void uvLight2On(void);
void uvLight2Off(void);
uint8_t isUvLight2On(void);
uint32_t uvLightSensorGetSample(void);

#ifdef __cplusplus
}
#endif

#endif /* UVLIGHT_H_ */
