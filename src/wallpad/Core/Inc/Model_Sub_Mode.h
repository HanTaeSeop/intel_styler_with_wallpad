

#ifndef MODEL_SUB_MODE_H_
#define MODEL_SUB_MODE_H_
#include <stdint.h>
#include "stm32f4xx_hal.h"


typedef enum {
    STOP_S ,  // 기본 값 0
    RUN_S      // 자동으로 1      // 자동으로 2
} model_sub_mode;
uint8_t Model_getSubMode();

void Model_setSubMode(uint8_t data);
#endif /* MODEL_MORTOR_MODE_H_ */
