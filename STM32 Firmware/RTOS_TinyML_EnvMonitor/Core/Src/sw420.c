/*
 * sw420.c
 *
 *  Created on: Jun 28, 2026
 *      Author: Mujaid
 */

#include "sw420.h"
#include "main.h"

HAL_StatusTypeDef SW420_Init(void)
{
    return HAL_OK;
}

uint8_t SW420_Read(void)
{
    return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
}
