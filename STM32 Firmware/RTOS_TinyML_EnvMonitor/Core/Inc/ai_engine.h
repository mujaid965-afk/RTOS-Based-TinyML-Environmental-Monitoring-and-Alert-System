/*
 * ai_engine.h
 *
 *  Created on: Jun 30, 2026
 *      Author: Mujaid
 */

#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include "main.h"
#include "network.h"

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef AI_Init(void);

float AI_Run(float temperature,
             float pressure,
             float gas,
             float vibration);

#ifdef __cplusplus
}
#endif

#endif
