/*
 * sensor_manager.c
 *
 *  Created on: Jun 28, 2026
 *      Author: Mujaid
 */
#include "sensor_manager.h"
#include "bmp280.h"
#include "sw420.h"
#include "mq2.h"

HAL_StatusTypeDef SensorManager_Init(void)
{
	if(BMP280_Init() != HAL_OK)
	        return HAL_ERROR;

	if(MQ2_Init() != HAL_OK)
	        return HAL_ERROR;

	if(SW420_Init() != HAL_OK)
	        return HAL_ERROR;

	return HAL_OK;
}

HAL_StatusTypeDef SensorManager_Read(SensorData_t *data)
{
    data->temperature = BMP280_ReadTemperature();

    data->pressure = BMP280_ReadPressure();

    data->gas = MQ2_Read();

    data->vibration = SW420_Read();

    return HAL_OK;
}
