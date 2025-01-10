

#ifndef MODEL_MORTOR_MODE_H_
#define MODEL_MORTOR_MODE_H_
#include <stdint.h>
#include "stm32f4xx_hal.h"


typedef enum {
    STOP ,  // 기본 값 0
    RUN      // 자동으로 1      // 자동으로 2
} model_motor_mode;
uint8_t Model_getMotorMode();

void Model_setMotorMode(uint8_t data);
#endif /* MODEL_MORTOR_MODE_H_ */
