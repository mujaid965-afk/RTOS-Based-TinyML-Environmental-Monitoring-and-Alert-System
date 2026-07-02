/*
 * ai_engine.h
 *
 *  Created on: Jun 30, 2026
 *      Author: Mujaid
 */

#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    float temperature;
    float pressure;
    float gas;
    float vibration;
} AI_Input_t;

typedef struct
{
    float probability;
    bool anomaly;
} AI_Output_t;

bool AI_Init(void);

bool AI_RunInference(AI_Input_t *input,
                     AI_Output_t *output);

#endif
