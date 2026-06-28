/*
 * sensor_manager.c
 *
 *  Created on: Jun 28, 2026
 *      Author: Mujaid
 */
#include "sensor_manager.h"
#include "bmp280.h"

HAL_StatusTypeDef SensorManager_Init(void)
{
    return BMP280_Init();
}

HAL_StatusTypeDef SensorManager_Read(SensorData_t *data)
{
    data->temperature = BMP280_ReadTemperature();

    /* Pressure will be added after we complete the driver */

    return HAL_OK;
}
