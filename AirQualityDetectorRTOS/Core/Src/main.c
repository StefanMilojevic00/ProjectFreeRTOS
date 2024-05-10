/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "Variables.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DangerousPPM 2000 // to be adjusted
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc3;

CRC_HandleTypeDef hcrc;

UART_HandleTypeDef huart1;

/* Definitions for MainTask */
osThreadId_t MainTaskHandle;
const osThreadAttr_t MainTask_attributes = {
  .name = "MainTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TerminalTask */
osThreadId_t TerminalTaskHandle;
const osThreadAttr_t TerminalTask_attributes = {
  .name = "TerminalTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ButtonTask */
osThreadId_t ButtonTaskHandle;
const osThreadAttr_t ButtonTask_attributes = {
  .name = "ButtonTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SingleLEDTask */
osThreadId_t SingleLEDTaskHandle;
const osThreadAttr_t SingleLEDTask_attributes = {
  .name = "SingleLEDTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ButtonTimer */
osTimerId_t ButtonTimerHandle;
const osTimerAttr_t ButtonTimer_attributes = {
  .name = "ButtonTimer"
};
/* Definitions for RegimeMutex */
osMutexId_t RegimeMutexHandle;
const osMutexAttr_t RegimeMutex_attributes = {
  .name = "RegimeMutex"
};
/* Definitions for LED_Blink_Mutex */
osMutexId_t LED_Blink_MutexHandle;
const osMutexAttr_t LED_Blink_Mutex_attributes = {
  .name = "LED_Blink_Mutex"
};
/* Definitions for UARTMutex */
osMutexId_t UARTMutexHandle;
const osMutexAttr_t UARTMutex_attributes = {
  .name = "UARTMutex"
};
/* Definitions for LED_FSM_Mutex */
osMutexId_t LED_FSM_MutexHandle;
const osMutexAttr_t LED_FSM_Mutex_attributes = {
  .name = "LED_FSM_Mutex"
};
/* Definitions for ButtonCountMutex */
osMutexId_t ButtonCountMutexHandle;
const osMutexAttr_t ButtonCountMutex_attributes = {
  .name = "ButtonCountMutex"
};
/* Definitions for SystemWorkStateMutex */
osMutexId_t SystemWorkStateMutexHandle;
const osMutexAttr_t SystemWorkStateMutex_attributes = {
  .name = "SystemWorkStateMutex"
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC3_Init(void);
static void MX_CRC_Init(void);
static void MX_USART1_UART_Init(void);
void StartMainTask(void *argument);
void StartTerminalTask(void *argument);
void StartButtonTask(void *argument);
void StartSingleLEDTask(void *argument);
void ButtonTimerCallback(void *argument);

/* USER CODE BEGIN PFP */

// Button variables //
volatile uint8_t button_press_counter = 0;
// END Button variables //

// Read parameters variables //
volatile bool quality_status = true;
// END Read parameters variables //

// For UART commands //
uint16_t cmd_find = 0;
const uint16_t len_of_array = 20;
// END For UART commands //





//Messages for sending
static char IdleMSG[] = "System waits for configuration";
static char PPM_MSG[] = "PPM = ";
static char AlertMSG[] = "Dangerous leves of PPM in the room";
static char RoomClearedMSG[] = "Room cleared, it is now safe to return";
static char S1WorkStateMSG[] = "S1 work regime selected";
static char S3WorkStateMSG[] = "S3 work regime selected";
static char S5WorkStateMSG[] = "S5 work regime selected";
static char FAN_ON[] = "FAN started";
static char FAN_OFF[] = "FAN stoped";
static char ErrorMSG[] = "Unknown command, type HELP to see all commands";

// Structs //


char* cmdStrings[] = {

		"IDLE\r",			 // cmd_0
		"WORK_S1\r",	 	 // cmd_1
		"WORK_S3\r",		 // cmd_2
		"WORK_S5\r",		 // cmd_3

		"READ_MQ_SENSOR\r",	 // cmd_4

		"SEND_MQ_DATA\r",	 // cmd_5

		"FAN_ON\r",		     // cmd_6
		"FAN_OFF\r",		 // cmd_7

		"ALARM_ON\r",	   	 // cmd_8
		"ALARM_OFF\r",  	 // cmd_9

		"LED_ON\r",		     // cmd_10
		"LED_OFF\r",		 // cmd_11

		"SEGMENT_0\r",		 // cmd_12
		"SEGMENT_1\r",		 // cmd_13
		"SEGMENT_2\r",	   	 // cmd_14
		"SEGMENT_3\r",		 // cmd_15
		"SEGMENT_4\r",		 // cmd_16
		"SEGMENT_5\r",		 // cmd_17
		"SEGMENT_6\r", 		 // cmd_18

		"HELP\r", 	 		 // cmd_19

};

// FSMs //
ProgramStateFSM progState = P_IDLE_START;
ProgramChangeStateFSM  progChangeState = PC_IDLE;
CountingTasterFSM countState = C_IDLE;
LED_StatusFSM ledState = LED_OFF;
UART_commandsFSM uartCmdState;

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_ADC3_Init();
  MX_CRC_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  DevicesInit(); // Function that initializes all of systems components
//  LED_Drive(true);
//  osTimerStop(ButtonTimerHandle);
//  osTimerStart(LEDTimerHandle, 3000);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
  /* Create the mutex(es) */
  /* creation of RegimeMutex */
  RegimeMutexHandle = osMutexNew(&RegimeMutex_attributes);

  /* creation of LED_Blink_Mutex */
  LED_Blink_MutexHandle = osMutexNew(&LED_Blink_Mutex_attributes);

  /* creation of UARTMutex */
  UARTMutexHandle = osMutexNew(&UARTMutex_attributes);

  /* creation of LED_FSM_Mutex */
  LED_FSM_MutexHandle = osMutexNew(&LED_FSM_Mutex_attributes);

  /* creation of ButtonCountMutex */
  ButtonCountMutexHandle = osMutexNew(&ButtonCountMutex_attributes);

  /* creation of SystemWorkStateMutex */
  SystemWorkStateMutexHandle = osMutexNew(&SystemWorkStateMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of ButtonTimer */
  ButtonTimerHandle = osTimerNew(ButtonTimerCallback, osTimerOnce, NULL, &ButtonTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of MainTask */
  MainTaskHandle = osThreadNew(StartMainTask, NULL, &MainTask_attributes);

  /* creation of TerminalTask */
  TerminalTaskHandle = osThreadNew(StartTerminalTask, NULL, &TerminalTask_attributes);

  /* creation of ButtonTask */
  ButtonTaskHandle = osThreadNew(StartButtonTask, NULL, &ButtonTask_attributes);

  /* creation of SingleLEDTask */
  SingleLEDTaskHandle = osThreadNew(StartSingleLEDTask, NULL, &SingleLEDTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

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
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(IndicatorYellow2_GPIO_Port, IndicatorYellow2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOI, Led_out_Pin|Buzzer_out_Pin|IndicatorBlue2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_BL_CTRL_GPIO_Port, LCD_BL_CTRL_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_DISP_GPIO_Port, LCD_DISP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, DCMI_PWR_EN_Pin|IndicatorBlue3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(IndicatorGreen_GPIO_Port, IndicatorGreen_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, IndicatorBlue1_Pin|IndicatorYellow1_Pin|EXT_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LCD_B0_Pin */
  GPIO_InitStruct.Pin = LCD_B0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(LCD_B0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_HS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_HS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_HS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : QSPI_D2_Pin */
  GPIO_InitStruct.Pin = QSPI_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(QSPI_D2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_TXD1_Pin RMII_TXD0_Pin RMII_TX_EN_Pin */
  GPIO_InitStruct.Pin = RMII_TXD1_Pin|RMII_TXD0_Pin|RMII_TX_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : FMC_NBL1_Pin FMC_NBL0_Pin FMC_D5_Pin FMC_D6_Pin
                           FMC_D8_Pin FMC_D11_Pin FMC_D4_Pin FMC_D7_Pin
                           FMC_D9_Pin FMC_D12_Pin FMC_D10_Pin */
  GPIO_InitStruct.Pin = FMC_NBL1_Pin|FMC_NBL0_Pin|FMC_D5_Pin|FMC_D6_Pin
                          |FMC_D8_Pin|FMC_D11_Pin|FMC_D4_Pin|FMC_D7_Pin
                          |FMC_D9_Pin|FMC_D12_Pin|FMC_D10_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_SCL_D15_Pin ARDUINO_SDA_D14_Pin */
  GPIO_InitStruct.Pin = ARDUINO_SCL_D15_Pin|ARDUINO_SDA_D14_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : IndicatorYellow2_Pin */
  GPIO_InitStruct.Pin = IndicatorYellow2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IndicatorYellow2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPDIF_RX0_Pin */
  GPIO_InitStruct.Pin = SPDIF_RX0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF8_SPDIFRX;
  HAL_GPIO_Init(SPDIF_RX0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SDMMC_CK_Pin SDMMC_D3_Pin SDMMC_D2_Pin PC9
                           PC8 */
  GPIO_InitStruct.Pin = SDMMC_CK_Pin|SDMMC_D3_Pin|SDMMC_D2_Pin|GPIO_PIN_9
                          |GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_PWM_D9_Pin */
  GPIO_InitStruct.Pin = ARDUINO_PWM_D9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(ARDUINO_PWM_D9_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DCMI_D6_Pin DCMI_D7_Pin */
  GPIO_InitStruct.Pin = DCMI_D6_Pin|DCMI_D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : QSPI_NCS_Pin */
  GPIO_InitStruct.Pin = QSPI_NCS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
  HAL_GPIO_Init(QSPI_NCS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : FMC_SDNCAS_Pin FMC_SDCLK_Pin FMC_A11_Pin FMC_A10_Pin
                           FMC_BA1_Pin FMC_BA0_Pin */
  GPIO_InitStruct.Pin = FMC_SDNCAS_Pin|FMC_SDCLK_Pin|FMC_A11_Pin|FMC_A10_Pin
                          |FMC_BA1_Pin|FMC_BA0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_B1_Pin LCD_B2_Pin LCD_B3_Pin LCD_G4_Pin
                           LCD_G1_Pin LCD_G3_Pin LCD_G0_Pin LCD_G2_Pin
                           LCD_R7_Pin LCD_R5_Pin LCD_R6_Pin LCD_R4_Pin
                           LCD_R3_Pin LCD_R1_Pin LCD_R2_Pin */
  GPIO_InitStruct.Pin = LCD_B1_Pin|LCD_B2_Pin|LCD_B3_Pin|LCD_G4_Pin
                          |LCD_G1_Pin|LCD_G3_Pin|LCD_G0_Pin|LCD_G2_Pin
                          |LCD_R7_Pin|LCD_R5_Pin|LCD_R6_Pin|LCD_R4_Pin
                          |LCD_R3_Pin|LCD_R1_Pin|LCD_R2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_VBUS_Pin */
  GPIO_InitStruct.Pin = OTG_FS_VBUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_VBUS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Audio_INT_Pin */
  GPIO_InitStruct.Pin = Audio_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Audio_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : FMC_D2_Pin FMC_D3_Pin FMC_D1_Pin FMC_D15_Pin
                           FMC_D0_Pin FMC_D14_Pin FMC_D13_Pin */
  GPIO_InitStruct.Pin = FMC_D2_Pin|FMC_D3_Pin|FMC_D1_Pin|FMC_D15_Pin
                          |FMC_D0_Pin|FMC_D14_Pin|FMC_D13_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : OTG_FS_P_Pin OTG_FS_N_Pin OTG_FS_ID_Pin */
  GPIO_InitStruct.Pin = OTG_FS_P_Pin|OTG_FS_N_Pin|OTG_FS_ID_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SAI2_MCLKA_Pin SAI2_SCKA_Pin SAI2_FSA_Pin SAI2_SDA_Pin */
  GPIO_InitStruct.Pin = SAI2_MCLKA_Pin|SAI2_SCKA_Pin|SAI2_FSA_Pin|SAI2_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_DE_Pin LCD_B7_Pin LCD_B6_Pin LCD_B5_Pin
                           LCD_G6_Pin LCD_G7_Pin LCD_G5_Pin */
  GPIO_InitStruct.Pin = LCD_DE_Pin|LCD_B7_Pin|LCD_B6_Pin|LCD_B5_Pin
                          |LCD_G6_Pin|LCD_G7_Pin|LCD_G5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_B4_Pin */
  GPIO_InitStruct.Pin = LCD_B4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
  HAL_GPIO_Init(LCD_B4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SAI2_SDB_Pin */
  GPIO_InitStruct.Pin = SAI2_SDB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
  HAL_GPIO_Init(SAI2_SDB_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DCMI_D5_Pin */
  GPIO_InitStruct.Pin = DCMI_D5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(DCMI_D5_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Led_out_Pin Buzzer_out_Pin IndicatorBlue2_Pin */
  GPIO_InitStruct.Pin = Led_out_Pin|Buzzer_out_Pin|IndicatorBlue2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : uSD_Detect_Pin */
  GPIO_InitStruct.Pin = uSD_Detect_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(uSD_Detect_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : FMC_A0_Pin FMC_A1_Pin FMC_A2_Pin FMC_A3_Pin
                           FMC_A4_Pin FMC_A5_Pin FMC_A6_Pin FMC_A9_Pin
                           FMC_A7_Pin FMC_A8_Pin FMC_SDNRAS_Pin */
  GPIO_InitStruct.Pin = FMC_A0_Pin|FMC_A1_Pin|FMC_A2_Pin|FMC_A3_Pin
                          |FMC_A4_Pin|FMC_A5_Pin|FMC_A6_Pin|FMC_A9_Pin
                          |FMC_A7_Pin|FMC_A8_Pin|FMC_SDNRAS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_HSYNC_Pin LCD_VSYNC_Pin LCD_R0_Pin LCD_CLK_Pin */
  GPIO_InitStruct.Pin = LCD_HSYNC_Pin|LCD_VSYNC_Pin|LCD_R0_Pin|LCD_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_BL_CTRL_Pin */
  GPIO_InitStruct.Pin = LCD_BL_CTRL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_BL_CTRL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DCMI_VSYNC_Pin */
  GPIO_InitStruct.Pin = DCMI_VSYNC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(DCMI_VSYNC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SDMMC_CMD_Pin */
  GPIO_InitStruct.Pin = SDMMC_CMD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(SDMMC_CMD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TP3_Pin NC2_Pin */
  GPIO_InitStruct.Pin = TP3_Pin|NC2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_SCK_D13_Pin */
  GPIO_InitStruct.Pin = ARDUINO_SCK_D13_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(ARDUINO_SCK_D13_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_DISP_Pin */
  GPIO_InitStruct.Pin = LCD_DISP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_DISP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DCMI_PWR_EN_Pin */
  GPIO_InitStruct.Pin = DCMI_PWR_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DCMI_PWR_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DCMI_D4_Pin DCMI_D3_Pin DCMI_D0_Pin DCMI_D2_Pin
                           DCMI_D1_Pin */
  GPIO_InitStruct.Pin = DCMI_D4_Pin|DCMI_D3_Pin|DCMI_D0_Pin|DCMI_D2_Pin
                          |DCMI_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : Btn_pin_Pin */
  GPIO_InitStruct.Pin = Btn_pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(Btn_pin_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_PWM_D10_Pin */
  GPIO_InitStruct.Pin = ARDUINO_PWM_D10_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(ARDUINO_PWM_D10_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_INT_Pin */
  GPIO_InitStruct.Pin = LCD_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LCD_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_RX_D0_Pin */
  GPIO_InitStruct.Pin = ARDUINO_RX_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
  HAL_GPIO_Init(ARDUINO_RX_D0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IndicatorGreen_Pin */
  GPIO_InitStruct.Pin = IndicatorGreen_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IndicatorGreen_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : FMC_SDNME_Pin FMC_SDNE0_Pin */
  GPIO_InitStruct.Pin = FMC_SDNME_Pin|FMC_SDNE0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : IndicatorBlue1_Pin IndicatorYellow1_Pin */
  GPIO_InitStruct.Pin = IndicatorBlue1_Pin|IndicatorYellow1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : FMC_SDCKE0_Pin */
  GPIO_InitStruct.Pin = FMC_SDCKE0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(FMC_SDCKE0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_MDC_Pin RMII_RXD0_Pin RMII_RXD1_Pin */
  GPIO_InitStruct.Pin = RMII_MDC_Pin|RMII_RXD0_Pin|RMII_RXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : QSPI_D1_Pin QSPI_D3_Pin QSPI_D0_Pin */
  GPIO_InitStruct.Pin = QSPI_D1_Pin|QSPI_D3_Pin|QSPI_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : EXT_RST_Pin */
  GPIO_InitStruct.Pin = EXT_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(EXT_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RMII_RXER_Pin */
  GPIO_InitStruct.Pin = RMII_RXER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RMII_RXER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_REF_CLK_Pin RMII_MDIO_Pin RMII_CRS_DV_Pin */
  GPIO_InitStruct.Pin = RMII_REF_CLK_Pin|RMII_MDIO_Pin|RMII_CRS_DV_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : DCMI_HSYNC_Pin PA6 */
  GPIO_InitStruct.Pin = DCMI_HSYNC_Pin|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_SCL_Pin LCD_SDA_Pin */
  GPIO_InitStruct.Pin = LCD_SCL_Pin|LCD_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : ULPI_D3_Pin */
  GPIO_InitStruct.Pin = ULPI_D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(ULPI_D3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IndicatorBlue3_Pin */
  GPIO_InitStruct.Pin = IndicatorBlue3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IndicatorBlue3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_MISO_D12_Pin ARDUINO_MOSI_PWM_D11_Pin */
  GPIO_InitStruct.Pin = ARDUINO_MISO_D12_Pin|ARDUINO_MOSI_PWM_D11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartMainTask */
/**
  * @brief  Function implementing the MainTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartMainTask */
void StartMainTask(void *argument)
{
  /* USER CODE BEGIN 5 */
	//Setup code
	ProgramStateFSM progStateLocal;
	float PPMValue;
	bool sentAlarmMSG = false; // flag to transmit alarm msg only once
	bool meassuring = false;
	uint16_t delay_main_task_time = 5;

  /* Infinite loop */
  for(;;)
  {
	  osMutexAcquire(RegimeMutexHandle, osWaitForever);
	  progStateLocal = progState;
	  osMutexRelease(RegimeMutexHandle);

	  PPMValue = ReadGasSensor();

	  switch(progStateLocal)
	  {
	  	  case P_IDLE_START:

	  		  osMutexAcquire(UARTMutexHandle, osWaitForever);

	  		  HelpSendUART(len_of_array, cmdStrings);
	  		  UART_TransmitString(IdleMSG);
	  		  osMutexRelease(UARTMutexHandle);

	  		  osMutexAcquire(RegimeMutexHandle, osWaitForever);
	  		  progState = P_IDLE;
	  		  osMutexRelease(RegimeMutexHandle);

	  		  // LED Blink logic //
	  		  //osTimerStart(LEDTimerCallback,500);

	  		  osMutexAcquire(LED_FSM_MutexHandle, osWaitForever);
	  		  ledState = LED_OFF;
	  		  osMutexRelease(LED_FSM_MutexHandle);

	  		  osMutexAcquire(SystemWorkStateMutexHandle, osWaitForever);
	  		  progChangeState = PC_IDLE;
	  		  osMutexRelease(SystemWorkStateMutexHandle);

	  		  meassuring = false;
	  		  SetIndicatorLEDs(0); // resets the indicator

		  break;

	  	  case P_IDLE:

	  		  //System waits for configuration
	  		  meassuring = false;
	  		  delay_main_task_time = 5;

		  break;

	  	  case P_WORK_S1:

	  		  meassuring = true;

	  		  osMutexAcquire(UARTMutexHandle, osWaitForever);
	  		  UART_TransmitString(PPM_MSG);
	  		  UART_TransmitFloat(PPMValue);
	  		  osMutexRelease(UARTMutexHandle);
	  		  delay_main_task_time = 1000;

	  		  osMutexAcquire(SystemWorkStateMutexHandle, osWaitForever);
	  		  if(progChangeState == PC_IDLE)
	  		  {
		  		  progChangeState = PC_WORK_START;
	  		  }
	  	      osMutexRelease(SystemWorkStateMutexHandle);

		  break;

	  	  case P_WORK_S3:

	  		  meassuring = true;
	  		  osMutexAcquire(UARTMutexHandle, osWaitForever);
	  		  UART_TransmitString(PPM_MSG);
	  		  UART_TransmitFloat(PPMValue);
	  		  osMutexRelease(UARTMutexHandle);

	  		  osMutexAcquire(SystemWorkStateMutexHandle, osWaitForever);
	  		  if(progChangeState == PC_IDLE)
	  		  {
		  		  progChangeState = PC_WORK_START;
	  		  }
	  	      osMutexRelease(SystemWorkStateMutexHandle);
	  	      delay_main_task_time = 3000;
		  break;

	  	  case P_WORK_S5:

	  		  meassuring = true;
	  		  osMutexAcquire(UARTMutexHandle, osWaitForever);
	  		  UART_TransmitString(PPM_MSG);
	  		  UART_TransmitFloat(PPMValue);
	  		  osMutexRelease(UARTMutexHandle);

	  		  osMutexAcquire(SystemWorkStateMutexHandle, osWaitForever);
	  		  if(progChangeState == PC_IDLE)
	  		  {
		  		  progChangeState = PC_WORK_START;
	  		  }
	  	      osMutexRelease(SystemWorkStateMutexHandle);

	  	      delay_main_task_time = 5000;

		  break;
	  }
		  osMutexAcquire(SystemWorkStateMutexHandle, osWaitForever);
		  if(progChangeState == PC_WORK_START)
		  {
			  progChangeState = PC_WORK;
		  }
	      osMutexRelease(SystemWorkStateMutexHandle);

	  //Take action
	  //Setting the indicator system
	  if(meassuring == true)
	  {
		  SetIndicatorLEDs(PPMValue);
	  }

	  //Check for contamination
	  if(meassuring == true)
	  {
		  if(PPMValue > DangerousPPM)
		  {
			  osMutexAcquire(UARTMutexHandle, osWaitForever);
			  UART_TransmitString(AlertMSG);
			  osMutexRelease(UARTMutexHandle);

			  osMutexAcquire(LED_Blink_MutexHandle, osWaitForever);
				quality_status = false;
			  osMutexRelease(LED_Blink_MutexHandle);

			  sentAlarmMSG = true;
			  AlarmON();
		  }
		  else
		  {
			  osMutexAcquire(LED_Blink_MutexHandle, osWaitForever);
				quality_status = true;
			  osMutexRelease(LED_Blink_MutexHandle);

			  AlarmOFF();
			  if(sentAlarmMSG == true)
			  {
				  osMutexAcquire(UARTMutexHandle, osWaitForever);
				  UART_TransmitString(RoomClearedMSG);
				  osMutexRelease(UARTMutexHandle);

				  sentAlarmMSG = false; // to be avaliable for next time danger happens
			  }
		  }
	  }
	  osDelay(delay_main_task_time);
  }

  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTerminalTask */
/**
* @brief Function implementing the TerminalTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTerminalTask */
void StartTerminalTask(void *argument)
{
  /* USER CODE BEGIN StartTerminalTask */
	//setup
	ProgramStateFSM progStateLocal;
	UART_commandsFSM uartCmdState;
	uint16_t CommandIndex;
	bool CommandStatus;
	float LocalPPM;
  /* Infinite loop */
  for(;;)
  {
	  if(IsTransferComplete() == true)
	  {
		  CommandStatus = StringCompareFromUART(GetRxBuffer(), cmdStrings, len_of_array, &CommandIndex);
		  if(CommandStatus == false)
		  {
			  UART_TransmitString(ErrorMSG);
			  ClearRxBuffer();
			  osMutexAcquire(UARTMutexHandle, osWaitForever);
			  HelpSendUART(len_of_array, cmdStrings);
			  osMutexRelease(UARTMutexHandle);
			  progStateLocal = P_IDLE;
		  }
		  else
		  {
			  ClearRxBuffer();
				uartCmdState = CommandIndex;

				switch(uartCmdState)
				{
					case cmd_0:

						progStateLocal = P_IDLE_START;

						break;

					case cmd_1:

						progStateLocal = P_WORK_S1;
						UART_TransmitString(S1WorkStateMSG);

						break;

					case cmd_2:

						progStateLocal = P_WORK_S3;
						UART_TransmitString(S3WorkStateMSG);

						break;

					case cmd_3:

						progStateLocal = P_WORK_S5;
						UART_TransmitString(S5WorkStateMSG);

						break;

					case cmd_4:

						LocalPPM = ReadGasSensor();

						break;

					case cmd_5:

						UART_TransmitString(PPM_MSG);
						UART_TransmitFloat(LocalPPM);

						break;


					case cmd_6:

						UART_TransmitString(FAN_ON);
						progStateLocal = P_IDLE;

						break;

					case cmd_7:

						UART_TransmitString(FAN_OFF);
						progStateLocal = P_IDLE;

						break;


					case cmd_8:

						AlarmON();
						progStateLocal = P_IDLE;

						break;

					case cmd_9:

						AlarmOFF();
						progStateLocal = P_IDLE;

						break;

					case cmd_10:

						LED_Drive(true);
						progStateLocal = P_IDLE;

						break;

					case cmd_11:

						LED_Drive(false);
						progStateLocal = P_IDLE;

						break;


					case cmd_12:

						SetIndicatorLEDsNum(0);
						progStateLocal = P_IDLE;

						break;

					case cmd_13:

						SetIndicatorLEDsNum(1);
						progStateLocal = P_IDLE;

						break;

					case cmd_14:

						SetIndicatorLEDsNum(2);
						progStateLocal = P_IDLE;

						break;

					case cmd_15:

						SetIndicatorLEDsNum(3);
						progStateLocal = P_IDLE;

						break;


					case cmd_16:

						SetIndicatorLEDsNum(4);
						progStateLocal = P_IDLE;

						break;

					case cmd_17:

						SetIndicatorLEDsNum(5);
						progStateLocal = P_IDLE;

						break;


					case cmd_18:

						SetIndicatorLEDsNum(6);
						progStateLocal = P_IDLE;

						break;

					case cmd_19:

						HelpSendUART(len_of_array, cmdStrings);
						progStateLocal = P_IDLE;

						break;

					case cmd_20:
						// reserved

						break;
				}

				//ClearRxBuffer();

				osMutexAcquire(RegimeMutexHandle, osWaitForever);
				progState = progStateLocal;
				osMutexRelease(RegimeMutexHandle);
		  }
	  }

    osDelay(100);
  }
  /* USER CODE END StartTerminalTask */
}

/* USER CODE BEGIN Header_StartButtonTask */
/**
* @brief Function implementing the ButtonTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartButtonTask */
void StartButtonTask(void *argument)
{
  /* USER CODE BEGIN StartButtonTask */
  /* Infinite loop */
	bool read_button_flag = true;
	bool press_button_flag = false;

  for(;;)
  {
	press_button_flag = ReadSignal(&read_button_flag);
    if(press_button_flag == true)
    {
    	osTimerStart(ButtonTimerHandle, 3000);

    	osMutexAcquire(ButtonCountMutexHandle, osWaitForever);
    	button_press_counter++;
    	osMutexRelease(ButtonCountMutexHandle);

    	press_button_flag = false;
    }
    osDelay(5);
  }
  /* USER CODE END StartButtonTask */
}

/* USER CODE BEGIN Header_StartSingleLEDTask */
/**
* @brief Function implementing the SingleLEDTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSingleLEDTask */
void StartSingleLEDTask(void *argument)
{
  /* USER CODE BEGIN StartSingleLEDTask */

	LED_StatusFSM ledStateLocal;
	bool quality_status_local;
	ProgramStateFSM progStateLocal;
	bool local_task_enable = false;

	uint16_t delay_time = 10;
  /* Infinite loop */
  for(;;)
  {

	//============ Update from mutex ============//
	osMutexAcquire(LED_Blink_MutexHandle, osWaitForever);
		quality_status_local = quality_status;
	osMutexRelease(LED_Blink_MutexHandle);

	osMutexAcquire(LED_FSM_MutexHandle, osWaitForever);
		ledStateLocal = ledState;
	osMutexRelease(LED_FSM_MutexHandle);

	osMutexAcquire(RegimeMutexHandle, osWaitForever);
		progStateLocal = progState;
	osMutexRelease(RegimeMutexHandle);
	//========== END Update from mutex ==========//

	if((progStateLocal != P_IDLE_START) && (progStateLocal != P_IDLE))
	{
		//============ Checking does system first time in work state after idle state ============//
		osMutexAcquire(SystemWorkStateMutexHandle, osWaitForever);
		if(progChangeState == PC_WORK)
		{
			local_task_enable = true;
			progChangeState = PC_WORK_CHECK;
		}
		else
		{
			local_task_enable = false;
		}
		osMutexRelease(SystemWorkStateMutexHandle);
		//========= END  Checking does system first time in work state after idle state =========//

		//============ Update logic state ===================//

		if(local_task_enable == true)
		{
			if(quality_status_local == true)
			{
				ledStateLocal = LED_ON_CORRECT;
			}
			else if(quality_status_local == false)
			{
				ledStateLocal = LED_ON_INCORRECT;
			}
			local_task_enable = false;
		}
		else
		{
			switch(ledStateLocal)
			{
				case LED_OFF:
					LED_Drive(false);
					delay_time = 3000;
					if(quality_status_local == true)
					{
						ledStateLocal = LED_ON_CORRECT;
					}
					else
					{
						ledStateLocal = LED_ON_INCORRECT;
					}

					break;

				case LED_ON_CORRECT:
					LED_Drive(true);
					delay_time = 2000;
					ledStateLocal = LED_OFF;

					break;

				case LED_ON_INCORRECT:

					LED_Drive(true);
					delay_time = 100;
					ledStateLocal = LED_OFF;

					break;
			}
		}

		//========== Keep LED state for delay_time ===========//
		osMutexAcquire(LED_FSM_MutexHandle, osWaitForever);
			ledState = ledStateLocal;
		osMutexRelease(LED_FSM_MutexHandle);
		//========== END Keep LED state for delay_time ===========//

	}

	//============ Keep LED state for delay_time or task delay execute =============//
	osDelay(delay_time);
	//===================== END  Keep LED state for delay_time =====================//
  }
  /* USER CODE END StartSingleLEDTask */
}

/* ButtonTimerCallback function */
void ButtonTimerCallback(void *argument)
{
  /* USER CODE BEGIN ButtonTimerCallback */
	ProgramStateFSM progStateLocal;
	uint8_t ButtonCountLocal;

	osMutexAcquire(ButtonCountMutexHandle, osWaitForever);
	ButtonCountLocal = button_press_counter;
	osMutexRelease(ButtonCountMutexHandle);

	switch(ButtonCountLocal)
	{
		case 0:

			progStateLocal = P_IDLE_START;

			break;

		case 1:

			progStateLocal = P_WORK_S1;
			UART_TransmitString(S1WorkStateMSG);

			break;

		case 2:

			progStateLocal = P_WORK_S3;
			UART_TransmitString(S3WorkStateMSG);

			break;

		case 3:

			progStateLocal = P_WORK_S5;
			UART_TransmitString(S5WorkStateMSG);

			break;

		case 4:

			// Not specified

			break;

		case 5:

			progStateLocal = P_IDLE_START;

			break;

	}

	osMutexAcquire(ButtonCountMutexHandle, osWaitForever);
	button_press_counter = 0;
	osMutexRelease(ButtonCountMutexHandle);

    osMutexAcquire(RegimeMutexHandle, osWaitForever);
    progState = progStateLocal;
	osMutexRelease(RegimeMutexHandle);

   //osTimerStop(ButtonTimerHandle);
  /* USER CODE END ButtonTimerCallback */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM14 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */


  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM14) {
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

#ifdef  USE_FULL_ASSERT
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
