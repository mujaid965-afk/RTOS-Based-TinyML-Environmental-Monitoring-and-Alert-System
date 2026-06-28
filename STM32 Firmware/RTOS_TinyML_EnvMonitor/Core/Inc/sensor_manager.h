/*
 * sensor_manager.h
 *
 *  Created on: Jun 28, 2026
 *      Author: Mujaid
 */

#ifndef INC_SENSOR_MANAGER_H_
#define INC_SENSOR_MANAGER_H_

#include "stm32f4xx_hal.h"
#include "project_data.h"

HAL_StatusTypeDef SensorManager_Init(void);

HAL_StatusTypeDef SensorManager_Read(SensorData_t *data);




#endif /* INC_SENSOR_MANAGER_H_ */
