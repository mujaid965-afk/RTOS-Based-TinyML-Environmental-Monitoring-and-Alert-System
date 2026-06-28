/*
 * sw420.h
 *
 *  Created on: Jun 28, 2026
 *      Author: Mujaid
 */

#ifndef INC_SW420_H_
#define INC_SW420_H_

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef SW420_Init(void);

uint8_t SW420_Read(void);



#endif /* INC_SW420_H_ */
