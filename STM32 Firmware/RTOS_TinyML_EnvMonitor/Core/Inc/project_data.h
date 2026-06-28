/*
 * project_data.h
 *
 *  Created on: Jun 24, 2026
 *      Author: Mujaid
 */

#ifndef INC_PROJECT_DATA_H_
#define INC_PROJECT_DATA_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

typedef struct
{
	float temperature;
	float pressure;
	uint16_t gas;
	uint8_t vibration;

}SensorData_t;

typedef struct
{
	SensorData_t sensor;
	uint8_t anomaly;
}InferenceData_t;



#endif /* INC_PROJECT_DATA_H_ */
