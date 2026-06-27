/*
 * bmp280.c
 *
 *  Created on: Jun 27, 2026
 *      Author: Mujaid
 */

#include "bmp280.h"

extern I2C_HandleTypeDef hi2c1;

uint8_t BMP280_ReadRegister(uint8_t reg)
{
	uint8_t value;

	HAL_I2C_Mem_Read(&hi2c1,
	                     BMP280_ADDR,
	                     reg,
	                     I2C_MEMADD_SIZE_8BIT,
	                     &value,
	                     1,
	                     HAL_MAX_DELAY);
	return value;
}

HAL_StatusTypeDef BMP280_WriteRegister(uint8_t reg,
                                       uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c1,
                             BMP280_ADDR,
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             &value,
                             1,
                             HAL_MAX_DELAY);
}

uint8_t BMP280_ReadChipID(void)
{
    return BMP280_ReadRegister(BMP280_CHIP_ID_REG);
}


HAL_StatusTypeDef BMP280_ReadCalibration(BMP280_CalibData_t *calib)
{
	uint8_t buffer[24];
	HAL_StatusTypeDef status;

	status = HAL_I2C_Mem_Read(&hi2c1,
	                              BMP280_ADDR,
	                              BMP280_CALIB_START,
	                              I2C_MEMADD_SIZE_8BIT,
	                              buffer,
	                              BMP280_CALIB_LENGTH,
	                              HAL_MAX_DELAY);
	if(status != HAL_OK)
	{
		return status;
	}

	calib->dig_T1 = (uint16_t)(buffer[1] << 8 | buffer[0]);
	calib->dig_T2 = (int16_t)(buffer[3] << 8 | buffer[2]);
	calib->dig_T3 = (int16_t)(buffer[5] << 8 | buffer[4]);

	calib->dig_P1 = (uint16_t)(buffer[7] << 8 | buffer[6]);
	calib->dig_P2 = (int16_t)(buffer[9] << 8 | buffer[8]);
	calib->dig_P3 = (int16_t)(buffer[11] << 8 | buffer[10]);
	calib->dig_P4 = (int16_t)(buffer[13] << 8 | buffer[12]);
	calib->dig_P5 = (int16_t)(buffer[15] << 8 | buffer[14]);
	calib->dig_P6 = (int16_t)(buffer[17] << 8 | buffer[16]);
	calib->dig_P7 = (int16_t)(buffer[19] << 8 | buffer[18]);
	calib->dig_P8 = (int16_t)(buffer[21] << 8 | buffer[20]);
	calib->dig_P9 = (int16_t)(buffer[23] << 8 | buffer[22]);

	return HAL_OK;
}

HAL_StatusTypeDef BMP280_Init(void)
{
    HAL_StatusTypeDef status;

    status = BMP280_WriteRegister(BMP280_CTRL_MEAS, 0x27);

    if(status != HAL_OK)
        return status;

    status = BMP280_WriteRegister(BMP280_CONFIG, 0xA0);

    if(status != HAL_OK)
        return status;

    return HAL_OK;
}

uint32_t BMP280_ReadRawTemperature(void)
{
    uint8_t buffer[3];

    HAL_I2C_Mem_Read(&hi2c1,
                     BMP280_ADDR,
                     BMP280_TEMP_MSB,
                     I2C_MEMADD_SIZE_8BIT,
                     buffer,
                     3,
                     HAL_MAX_DELAY);

    uint32_t adc_T;

    adc_T =
        ((uint32_t)buffer[0] << 12) |
        ((uint32_t)buffer[1] << 4)  |
        ((uint32_t)buffer[2] >> 4);

    return adc_T;
}



