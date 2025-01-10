

#ifndef MODEL_MODE_H_
#define MODEL_MODE_H_
#include <stdint.h>
#include "stm32f4xx_hal.h"


typedef enum {
    CLOCK,  // 기본 값 0
    STATE,      // 자동으로 1
    MAN         // 자동으로 2
} model_mode;
uint8_t Model_getMode();
void Model_setMode(uint8_t data);
#endif /* MODEL_MODE_H_ */
