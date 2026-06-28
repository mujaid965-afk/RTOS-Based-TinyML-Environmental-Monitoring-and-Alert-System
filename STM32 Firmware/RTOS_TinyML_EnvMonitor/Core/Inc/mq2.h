/*
 * mq2.h
 *
 *  Created on: Jun 28, 2026
 *      Author: Mujaid
 */

#ifndef INC_MQ2_H_
#define INC_MQ2_H_

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef MQ2_Init(void);

uint16_t MQ2_Read(void);




#endif /* INC_MQ2_H_ */
