/*
 * tinyml_engine.h
 *
 *  Created on: Jun 28, 2026
 *      Author: Mujaid
 */

#ifndef INC_TINYML_ENGINE_H_
#define INC_TINYML_ENGINE_H_

#include "project_data.h"

void TinyML_RunInference(const SensorData_t *sensor,
                         InferenceData_t *result);



#endif /* INC_TINYML_ENGINE_H_ */
