/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct{
	float temperature;
	float pressure;
	uint32_t gas_level;
	char system_status[12];	// "NORMAL", WARNING", "DANGER"
}EnvironmentalData_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* Definitions for LoggerTask */
osThreadId_t LoggerTaskHandle;
const osThreadAttr_t LoggerTask_attributes = {
  .name = "LoggerTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SensorTask */
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
  .name = "SensorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for DisplayTask */
osThreadId_t DisplayTaskHandle;
const osThreadAttr_t DisplayTask_attributes = {
  .name = "DisplayTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for SensorQueue */
osMessageQueueId_t SensorQueueHandle;
const osMessageQueueAttr_t SensorQueue_attributes = {
  .name = "SensorQueue"
};
/* USER CODE BEGIN PV */
/* USER CODE BEGIN PV */
/* Global shared variables for pipeline */
volatile float global_temp = 0.0f;
volatile float global_gas = 0.0f;

/* Mutex Handle and Attributes */
osMutexId_t I2CMutexHandle;
const osMutexAttr_t I2CMutex_attributes = {
  .name = "I2CMutex"
};

/* Alert Task Handle and Attributes */
osThreadId_t AlertTaskHandle;
const osThreadAttr_t AlertTask_attributes = {
  .name = "AlertTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh, // Alerts should be high priority!
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
void StartLoggerTask(void *argument);
void StartSensorTask(void *argument);
void StartDisplayTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE BEGIN PFP */
#define OLED_ADDR (0x3C << 1)
#define BMP280_ADDR     (0x76 << 1)
#define BMP280_REG_ID   0xD0

void OLED_WriteCommand(uint8_t command);
void OLED_Clear(void);
void OLED_Init(void);
void OLED_SetCursor(uint8_t page, uint8_t col);
void OLED_Putc(char c);
void OLED_Puts(const char* str);
uint8_t BMP280_ReadRegister(uint8_t reg_addr);
uint8_t run_inference(float temp, float gas);

/* Prototype for the missing alert task */
void StartAlertTask(void *argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  TinyML Edge Classifier generated via Scikit-Learn decision boundaries.
  * @param  temp: Current temperature reading.
  * @param  gas: Current gas PPM level.
  * @retval 0 = NORMAL, 1 = WARNING, 2 = DANGER
  */
uint8_t run_inference(float temp, float gas)
{
    // Fix: Evaluate DANGER conditions first so they don't get trapped by the WARNING check!
    if (gas > 180.0f || temp > 34.0f)
    {
        return 2; // DANGER Class code
    }
    else if (gas > 150.0f)
    {
        return 1; // WARNING Class code
    }
    else
    {
        return 0; // NORMAL Baseline
    }
}

uint8_t BMP280_ReadRegister(uint8_t reg_addr)
{
    uint8_t value = 0;
    HAL_StatusTypeDef status = HAL_ERROR;

    if (osMutexAcquire(I2CMutexHandle, 100) == osOK)
    {
        status = HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR, &reg_addr, 1, 100);
        if (status == HAL_OK)
        {
            status = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, &value, 1, 100);
        }
        osMutexRelease(I2CMutexHandle);
    }
    else
    {
        printf("[SensorTask] Mutex Timeout! Could not access I2C bus.\r\n");
        return 0x00;
    }

    if (status != HAL_OK) {
        return 0x00;
    }
    return value;
}

// 1. Write Command to Display
void OLED_WriteCommand(uint8_t command) {
    uint8_t buffer[2] = {0x00, command};

    if (osMutexAcquire(I2CMutexHandle, osWaitForever) == osOK) {
    	HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, buffer, 2, HAL_MAX_DELAY);
    	osMutexRelease(I2CMutexHandle);
    }
    // Removed the unprotected duplicate transmit line that was sitting here!
}

// 2. Clear Screen
void OLED_Clear(void) {
    uint8_t blank_row[128] = {0};
    for (uint8_t page = 0; page < 8; page++) {
        OLED_WriteCommand(0xB0 + page);
        OLED_WriteCommand(0x02);
        OLED_WriteCommand(0x10);

        // Mutex locking inside OLED_WriteCommand protects individual row configurations cleanly
        if (osMutexAcquire(I2CMutexHandle, osWaitForever) == osOK) {
            uint8_t control_byte = 0x40;
            HAL_I2C_Master_Sequential_Transmit_IT(&hi2c1, OLED_ADDR, &control_byte, 1, I2C_FIRST_FRAME);
            HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, blank_row, 128, HAL_MAX_DELAY);
            osMutexRelease(I2CMutexHandle);
        }
    }
}

// 3. Initialize Controller Hardware
void OLED_Init(void) {
    HAL_Delay(100); // Wait for power stabilizing

    OLED_WriteCommand(0xAE); // Display OFF
    OLED_WriteCommand(0xD5); // Set Clock Divide Ratio
    OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8); // Set Multiplex Ratio
    OLED_WriteCommand(0x3F); // 64MUX
    OLED_WriteCommand(0xD3); // Set Display Offset
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40); // Start Line Row 0
    OLED_WriteCommand(0xA1); // Mirror horizontally to look normal
    OLED_WriteCommand(0xC8); // COM Scan Direction
    OLED_WriteCommand(0xDA); // COM Pin Config
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81); // Contrast
    OLED_WriteCommand(0x7F);
    OLED_WriteCommand(0xA4); // Output follow RAM
    OLED_WriteCommand(0xA6); // Normal display mode
    OLED_WriteCommand(0xAF); // Display ON

    OLED_Clear(); // Wipe trash out of graphic memory
}

// Basic 5x7 ASCII Font Map
const uint8_t Font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space (0x20)
    // ... (Keep the rest of your Font5x7 array here untouched) ...
    {0x61, 0x51, 0x49, 0x45, 0x43}  // Z
};

// Set cursor coordinates
void OLED_SetCursor(uint8_t page, uint8_t col) {
    if (page > 7) page = 7;
    if (col > 131) col = 131;

    OLED_WriteCommand(0xB0 + page);

    uint8_t real_col = col + 2;
    OLED_WriteCommand(0x00 + (real_col & 0x0F));
    OLED_WriteCommand(0x10 + ((real_col >> 4) & 0x0F));
}

// Draw a single character
void OLED_Putc(char c) {
    if (c >= 'a' && c <= 'z') c = c - 32;
    if (c < 0x20 || c > 0x5A) c = ' ';

    uint8_t font_index = c - 0x20;
    uint8_t control_payload = 0x40;
    uint8_t blank_padding = 0x00;

    if (osMutexAcquire(I2CMutexHandle, osWaitForever) == osOK)
    {
        HAL_I2C_Master_Sequential_Transmit_IT(&hi2c1, OLED_ADDR, &control_payload, 1, I2C_FIRST_FRAME);
        HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, (uint8_t*)Font5x7[font_index], 5, HAL_MAX_DELAY);
        HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, &blank_padding, 1, HAL_MAX_DELAY);

        osMutexRelease(I2CMutexHandle);
    }
}

// Print a standard null-terminated string array
void OLED_Puts(const char* str) {
    while (*str) {
        OLED_Putc(*str++);
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* Create the Mutex to protect the shared 12C peripheral bus */
  I2CMutexHandle =osMutexNew(&I2CMutex_attributes);
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of SensorQueue */
  SensorQueueHandle = osMessageQueueNew (5, sizeof(uint32_t), &SensorQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of LoggerTask */
  LoggerTaskHandle = osThreadNew(StartLoggerTask, NULL, &LoggerTask_attributes);

  /* creation of SensorTask */
  SensorTaskHandle = osThreadNew(StartSensorTask, NULL, &SensorTask_attributes);

  /* creation of DisplayTask */
  DisplayTaskHandle = osThreadNew(StartDisplayTask, NULL, &DisplayTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    /* Spawn the Alert Handling Task thread */
  AlertTaskHandle = osThreadNew(StartAlertTask, NULL, &AlertTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartLoggerTask */
/**
  * @brief  Function implementing the LoggerTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartLoggerTask */
void StartLoggerTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(2000);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartSensorTask */
/**
* @brief Function implementing the SensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSensorTask */
void StartSensorTask(void *argument)
{
  /* USER CODE BEGIN StartSensorTask */
  EnvironmentalData_t sensor_packet;

  // Baseline simulated parameters
  sensor_packet.temperature = 27.5f;
  sensor_packet.pressure = 1013.25f;
  sensor_packet.gas_level = 110;
  snprintf(sensor_packet.system_status, sizeof(sensor_packet.system_status), "NORMAL");

  for(;;)
  {
    sensor_packet.temperature += 0.5f;
    if(sensor_packet.temperature > 35.0f) sensor_packet.temperature = 27.5f;

    sensor_packet.gas_level += 5;
    if(sensor_packet.gas_level >= 200)
    {
        sensor_packet.gas_level = 110;
        snprintf(sensor_packet.system_status, sizeof(sensor_packet.system_status), "NORMAL");
    }
    else if(sensor_packet.gas_level > 150)
    {
        snprintf(sensor_packet.system_status, sizeof(sensor_packet.system_status), "WARNING");
    }

    global_temp = sensor_packet.temperature;
    global_gas = (float)sensor_packet.gas_level;

    // Direct telemetry print loop up the wire to our laptop's Python script
    printf("TEMP : %.2f C\r\n", sensor_packet.temperature);
    printf("GAS  : %lu PPM\r\n", sensor_packet.gas_level);
    printf("STAT : %s\r\n", sensor_packet.system_status);
    printf("\r\n");

    osDelay(2000); // Wait 2 seconds
  }
  /* USER CODE END StartSensorTask */
}

void StartAlertTask(void *argument)
{
    uint8_t predicted_class = 0;
    uint8_t tx_packet[3];

    tx_packet[0] = 0xAA;
    tx_packet[2] = 0xBB;

    for(;;)
    {
        predicted_class = run_inference(
                                global_temp,
                                global_gas);

        tx_packet[1] = predicted_class;

        HAL_UART_Transmit(
            &huart1,
            tx_packet,
            3,
            HAL_MAX_DELAY);

        printf("[USART1] Sent Class=%d\r\n",
               predicted_class);

        if(predicted_class == 2)
        {
            HAL_GPIO_WritePin(
                GPIOA,
                GPIO_PIN_5,
                GPIO_PIN_SET);

            osDelay(100);

            HAL_GPIO_WritePin(
                GPIOA,
                GPIO_PIN_5,
                GPIO_PIN_RESET);

            osDelay(100);
        }
        else if(predicted_class == 1)
        {
            HAL_GPIO_TogglePin(
                GPIOA,
                GPIO_PIN_5);

            osDelay(400);
        }
        else
        {
            HAL_GPIO_WritePin(
                GPIOA,
                GPIO_PIN_5,
                GPIO_PIN_RESET);

            osDelay(500);
        }
    }
}
/* USER CODE BEGIN Header_StartDisplayTask */
/**
* @brief Function implementing the DisplayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void *argument)
{
  /* USER CODE BEGIN StartDisplayTask */
  EnvironmentalData_t received_packet;
  char display_buffer[20];
  OLED_Init();

  for(;;)
  {
    // The screen consumer logic yields safely while our pipeline logs to Python
    osDelay(1000);
  }
  /* USER CODE END StartDisplayTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM10 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM10)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
