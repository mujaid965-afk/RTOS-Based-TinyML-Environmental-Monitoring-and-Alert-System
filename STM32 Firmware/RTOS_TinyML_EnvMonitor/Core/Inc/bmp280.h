/*
 * bmp280.h
 *
 *  Created on: Jun 27, 2026
 *      Author: Mujaid
 */

#ifndef INC_BMP280_H_
#define INC_BMP280_H_

#include "main.h"

#define BMP280_ADDR  (0x76 << 1) //I2C Address (7-bit address for HAL)

//Registers
#define BMP280_CHIP_ID_REG  0xD0
#define BMP280_RESET_REG	0xE0
#define	BMP280_CTRL_HUM		0xF2
#define	BMP280_STATUS		0xF3
#define	BMP280_CTRL_MEAS	0xF4
#define BMP280_CONFIG		0xF5

#define BMP280_CALIB_START 0x88
#define BMP280_CALIB_LENGTH 24

#define BMP280_TEMP_MSB		0xFA
#define BMP280_TEMP_LSB		0xFB
#define BMP280_TEMP_XLSB 	0xFC

uint8_t BMP280_ReadRegister(uint8_t reg);
HAL_StatusTypeDef BMP280_WriteRegister(uint8_t reg, uint8_t value);

uint8_t BMP280_ReadChipID(void);

typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

} BMP280_CalibData_t;

HAL_StatusTypeDef BMP280_ReadCalibration(BMP280_CalibData_t *calib);

HAL_StatusTypeDef BMP280_Init(void);

uint32_t BMP280_ReadRawTemperature(void);








#endif /* INC_BMP280_H_ */
