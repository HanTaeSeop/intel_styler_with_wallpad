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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "clcd.h"
#include "Model_Mode.h"
#include "Model_Mortor_Mode.h"
#include "Model_Sub_Mode.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
#define ARR_CNT 10
#define CMD_SIZE 55
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
uint8_t rx2char;
volatile unsigned char rx2Flag = 0;
volatile char rx2Data[50];
volatile unsigned char btFlag = 0;
uint8_t btchar;
char btData[55];

volatile int tim3Flag1Sec;
volatile unsigned int tim3Cnt;
int updateTimeFlag = 0;
typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
  char date[4];
} DATETIME;
DATETIME dateTime = {0, 0, 0, 12, 0, 0, " "};

typedef struct {
	int temp;
	int humi;
	int water;
} SENSORSTATUS;
SENSORSTATUS sensorStatus = {0, 0, 0};

model_mode currentState;
volatile int key1,key3,key4;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void bluetooth_Event();
void clock_calc(DATETIME *dateTime);
int checkButtonState(int buttonIndex, GPIO_TypeDef* btnPort, uint16_t btnPin);
void buttonTrigger();
void btn1Event();
void btn3Event();
void btn4Event();
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
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2, &rx2char,1);
  HAL_UART_Receive_IT(&huart6, &btchar,1);
  LCD_init(&hi2c1);
  if(HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
  {
	Error_Handler();
  }
  Model_setMode(CLOCK);
  Model_setMotorMode(STOP);
  Model_setSubMode(STOP_S);


  updateTimeFlag = 1;
  printf("start main()\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  currentState = Model_getMode();

	  if(rx2Flag)
	  {
			printf("recv2 : %s\r\n",rx2Data);
			rx2Flag =0;

	  }
	  if(btFlag)
	  {
			btFlag =0;
			bluetooth_Event();
	  }

	  btn1Event();
	  if(currentState == MAN)
	  {
		  btn3Event();
		  btn4Event();
	  }
	 // printf("key1 : %d\r\n",key1);

//	  if(tim3Flag1Sec)
//	  {
//		  tim3Flag1Sec = 0;
//		  if (!(tim3Cnt % 5))
//		  {
//			  printf("tim3Cnt\r\n");
//		  }
//	  }





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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  hi2c1.Init.ClockSpeed = 10000;
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
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 8200-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 10000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

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
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 9600;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

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

  /*Configure GPIO pins : BTN1_Pin BTN2_Pin BTN3_Pin BTN4_Pin
                           BTN5_Pin BTN6_Pin */
  GPIO_InitStruct.Pin = BTN1_Pin|BTN2_Pin|BTN3_Pin|BTN4_Pin
                          |BTN5_Pin|BTN6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//void MX_GPIO_LED_ON(int pin)
//{
//	HAL_GPIO_WritePin(LD2_GPIO_Port, pin, GPIO_PIN_SET);
//}
//void MX_GPIO_LED_OFF(int pin)
//{
//	HAL_GPIO_WritePin(LD2_GPIO_Port, pin, GPIO_PIN_RESET);
//}
//void MX_GPIO_LAMP_ON(int pin)
//{
//	HAL_GPIO_WritePin(TEST_LED_PIN_GPIO_Port, pin, GPIO_PIN_SET);
//}
//void MX_GPIO_LAMP_OFF(int pin)
//{
//	HAL_GPIO_WritePin(TEST_LED_PIN_GPIO_Port, pin, GPIO_PIN_RESET);
//}
void bluetooth_Event()
{

  int i=0;
  char * pToken;
  char * pArray[ARR_CNT]={0};
  char recvBuf[CMD_SIZE]={0};
  char sendBuf[CMD_SIZE]={0};

  strcpy(recvBuf,btData);
  printf("btData : %s\r\n",btData);


  pToken = strtok(recvBuf,"[@]");
  while(pToken != NULL)
  {
    pArray[i] =  pToken;
    if(++i >= ARR_CNT)
      break;
    pToken = strtok(NULL,"[@]");
  }
  if (!strncmp(pArray[1]," New conn",strlen(" New conn"))) // New Connected
  {
	  //printf("start main() dt\r\n");
	  LCD_writeCmdData(0x01);
	  LCD_writeStringXY(0, 0, "hello user!");
      updateTimeFlag = 1;
 //     HAL_Delay(1000);
      return ;
  }
  else if(!strcmp(pArray[0],"GETTIME"))
  {  //GETTIME
      dateTime.year = (pArray[1][0]-0x30) * 10 + pArray[1][1]-0x30 ;
      dateTime.month =  (pArray[1][3]-0x30) * 10 + pArray[1][4]-0x30 ;
      dateTime.day =  (pArray[1][6]-0x30) * 10 + pArray[1][7]-0x30 ;
      dateTime.hour = (pArray[1][9]-0x30) * 10 + pArray[1][10]-0x30 ;
      dateTime.min =  (pArray[1][12]-0x30) * 10 + pArray[1][13]-0x30 ;
      dateTime.sec =  (pArray[1][15]-0x30) * 10 + pArray[1][16]-0x30 ;
      strncpy(dateTime.date,&pArray[1][18],4);
      return;
  }
  else if(!strcmp(pArray[1],"SENSOR"))
  {
	  sensorStatus.humi = atoi(pArray[2]);
	  sensorStatus.temp = atoi(pArray[3]);
	  sensorStatus.water = atoi(pArray[4]);
	  return;
  }

  else if(!strncmp(pArray[1]," Already log",sizeof(" Already log")))
  {
      return;
  }
  else
      return;

  sprintf(sendBuf,"[%s]%s@%s\n",pArray[0],pArray[1],pArray[2]);
  HAL_UART_Transmit(&huart6, (uint8_t *)sendBuf, strlen(sendBuf), 0xFFFF);

}
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART6 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
    	static int i=0;
    	rx2Data[i] = rx2char;
    	if((rx2Data[i] == '\r')||(rx2Data[i] == '\n'))
    	{
    		rx2Data[i] = '\0';
    		rx2Flag = 1;
    		i = 0;
    	}
    	else
    	{
    		i++;
    	}
    	HAL_UART_Receive_IT(&huart2, &rx2char,1);
    }
    if(huart->Instance == USART6)
    {
    	static int i=0;
    	btData[i] = btchar;
    	//printf("%c\r\n",btData[i]);

    	if((btData[i] == '\n') || btData[i] == '\r')
    	{
    		btData[i] = '\0';
    		btFlag = 1;
    		i = 0;
    	}
    	else
    	{
    		i++;
    	}
    	HAL_UART_Receive_IT(&huart6, &btchar,1);
    }
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	tim3Cnt++;
	tim3Flag1Sec = 1;
	clock_calc(&dateTime);
}
void clock_calc(DATETIME *dateTime)
{

  dateTime->sec++;          // increment second

  if(dateTime->sec >= 60)                              // if second = 60, second = 0
  {
      dateTime->sec = 0;
      dateTime->min++;

      if(dateTime->min >= 60)                          // if minute = 60, minute = 0
      {
          dateTime->min = 0;
          dateTime->hour++;                               // increment hour
          if(dateTime->hour == 24)
          {
            dateTime->hour = 0;
            updateTimeFlag = 1;
          }
       }
    }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin)
	{
	case BTN1_Pin:
		key1++;

		break;
	case BTN2_Pin:
		//btn2Event();
		break;
	case BTN3_Pin:
		  key3++;
		  break;
	case BTN4_Pin:
		  key4++;
		  break;
	case BTN5_Pin:
			break;
	case BTN6_Pin:
			break;
	default:
		break;
	}
}
void btn1Event()
{
	uint8_t mode =Model_getMode();
	char buff[30];
	char bdff[30];
	if (key1 > 3)
	{
		key1=0;
	}
	switch(mode)
	{
		case CLOCK :
			sprintf(bdff, "%02d.%02d  %02d:%02d:%02d", dateTime.month, dateTime.day, dateTime.hour, dateTime.min, dateTime.sec );
			sprintf(buff, "  CLOCK   %s", dateTime.date);
			if (updateTimeFlag)
			{
				HAL_UART_Transmit(&huart6, (uint8_t *)"[GETTIME]\n", strlen("[GETTIME]\n"), 0xFFFF);
				updateTimeFlag = 0;
			}
			//LCD_writeCmdData(0x01);
			LCD_writeStringXY(0, 0, buff);
			LCD_writeStringXY(1, 0, bdff);
			if(key1 == 1)
			{
				LCD_writeCmdData(0x01);
				mode = STATE;
				Model_setMode(mode);
				//HAL_UART_Transmit(&huart6, (uint8_t *)"[ALLMSG]GET@SENSOR\n", strlen("[ALLMSG]GET@SENSOR\n"), 0xFFFF);
			}
			break;

		case STATE :
			//LCD_writeCmdData(0x01);

			if(sensorStatus.water < 500){
				sprintf(bdff, "H:%02d T:%02d W:%03d", sensorStatus.humi, sensorStatus.temp, sensorStatus.water );
			}
			else{
				sprintf(bdff, "H:%02d T:%02d W:%s", sensorStatus.humi, sensorStatus.temp, "FULL" );
			}


			LCD_writeStringXY(0, 0, "  state");
			LCD_writeStringXY(1, 0, bdff);
			if(key1 == 2)
			{
				LCD_writeCmdData(0x01);
				mode = MAN;
				Model_setMode(mode);
			}
			break;

		case MAN :
			//LCD_writeCmdData(0x01);

			LCD_writeStringXY(0, 0, "  manual");
			if(key1 == 3)
			{
				LCD_writeCmdData(0x01);
				mode = CLOCK;
				Model_setMode(mode);
				key1 = 0;
			}
			break;
	}
}
void btn3Event()
{
	//uint8_t mode = Model_getMode();
	uint8_t state = Model_getMotorMode();

	if (key3 > 2)
		{
			key3=0;
		}
	switch(state)
	{
		case STOP :

			LCD_writeStringXY(1, 0, "FAN:X");
			if(key3 == 1)
			{

				HAL_UART_Transmit(&huart6, (uint8_t *)"[ALLMSG]FAN@ON\n", strlen("[ALLMSG]FAN@ON\n"), 0xFFFF);
				state = RUN;
				Model_setMotorMode(state);
			}

			break;
		case RUN  :
			LCD_writeStringXY(1, 0, "FAN:O ");
			if(key3 == 2)
			{
				HAL_UART_Transmit(&huart6, (uint8_t *)"[ALLMSG]FAN@OFF\n", strlen("[ALLMSG]FAN@OFF\n"), 0xFFFF);
				state = STOP;
				Model_setMotorMode(state);
				key3 = 0;
			}
			break;
	}

}
void btn4Event()
{
	uint8_t state_s = Model_getSubMode();

	if (key4 > 2)
			{
				key4=0;
			}

	switch(state_s)
	{
		case STOP_S :

			LCD_writeStringXY(1, 7, "DOOR:X");
			if(key4 == 1)
			{

				HAL_UART_Transmit(&huart6, (uint8_t *)"[ALLMSG]MOTOR@OPEN\n", strlen("[ALLMSG]MOTOR@OPEN\n"), 0xFFFF);
				state_s = RUN_S;
				Model_setSubMode(state_s);
			}

			break;
		case RUN_S  :
			LCD_writeStringXY(1, 7, "DOOR:O ");
			if(key4 == 2)
			{
				HAL_UART_Transmit(&huart6, (uint8_t *)"[ALLMSG]MOTOR@CLOSE\n", strlen("[ALLMSG]MOTOR@CLOSE\n"), 0xFFFF);
				state_s = STOP_S;
				Model_setSubMode(state_s);
				key4 = 0;
			}
			break;
	}
}
/* USER CODE END 4 */

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
