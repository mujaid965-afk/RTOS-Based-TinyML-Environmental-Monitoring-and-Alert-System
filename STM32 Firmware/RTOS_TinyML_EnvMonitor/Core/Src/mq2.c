/*
 * mq2.c
 *
 *  Created on: Jun 28, 2026
 *      Author: Mujaid
 */

#include "mq2.h"

extern ADC_HandleTypeDef hadc1;

HAL_StatusTypeDef MQ2_Init(void)
{
    return HAL_OK;
}

uint16_t MQ2_Read(void)
{
    HAL_ADC_Start(&hadc1);

    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);

    uint16_t value = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);

    return value;
}
