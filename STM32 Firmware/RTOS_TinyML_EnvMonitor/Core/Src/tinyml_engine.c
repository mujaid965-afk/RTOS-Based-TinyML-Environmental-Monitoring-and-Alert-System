/*
 * tinyml_engine.c
 *
 *  Created on: Jun 28, 2026
 *      Author: Mujaid
 */
#include "tinyml_engine.h"
#include <string.h>

void TinyML_RunInference(const SensorData_t *sensor,
                         InferenceData_t *result)
{
	if(sensor->gas > 1000)
	{
		result->anomaly = 1;
		strcpy(result->reason, "HIGH GAS");
	}
	else if(sensor->temperature > 35.0f)
	{
		result->anomaly = 1;
		strcpy(result->reason, "HIGH TEMP");
	}
	else if(sensor->vibration == 1)
	{
		result->anomaly = 1;
		strcpy(result->reason, "VIBRATION");
	}
	else
	{
		result->anomaly = 0;
		strcpy(result->reason, "NORMAL");
	}
}
