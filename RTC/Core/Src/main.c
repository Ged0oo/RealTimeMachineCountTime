/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "rtc.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include "RCC_interface.h"
#include "GPIO_interface.h"
#include "LCD_interface.h"
#include "KEYPAD_interface.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

uint8_t keypadVal = NOTPRESSED;

extern RCC_Config_t RCC_obj;
extern chr_lcd_4bit_t lcd_1;
extern chr_lcd_4bit_t keypad_1;

char time[10];
char date[10];

volatile int TotalHours=0;
volatile int CurrentSec=0;
volatile int CurrentMin=0;
volatile int TotalDays=0;

volatile uint8_t IntialMinuts = 0x30;
volatile uint8_t IntialSeconds = 0x20;
volatile uint8_t IntialHours = 0;

#define Counter_1	10
#define Counter_2	15

uint16_t ArrayCounter[2][2] = {0};
uint16_t ArrayTimers[2] = {0};
uint16_t ArrayPosition[2] = {Counter_1, Counter_2};

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

void set_time (void)
{
	  RTC_TimeTypeDef sTime;
	  RTC_DateTypeDef sDate;
    /**Initialize RTC and set the Time and Date
    */
  sTime.Hours = IntialHours;
  sTime.Minutes = IntialMinuts;
  sTime.Seconds = IntialSeconds;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
  }
  /* USER CODE BEGIN RTC_Init 3 */

  /* USER CODE END RTC_Init 3 */

  sDate.WeekDay = RTC_WEEKDAY_TUESDAY;
  sDate.Month = RTC_MONTH_AUGUST;
  sDate.Date = 0x12;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
  }
  /* USER CODE BEGIN RTC_Init 4 */

  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);  // backup register

  /* USER CODE END RTC_Init 4 */
}


volatile int TotalCounts=0,FlagDetect=0;

void LcdUpdate()
{
	for(int i=0 ; i<2 ; i++)
	{
		LCD_WriteNumber_Position(ArrayTimers[i], 1, ArrayPosition[i]);
	}
}

void MinDetect()
{
	for(int i=0 ; i<2 ; i++)
	{
		if((ArrayCounter[i][1] -1 == CurrentSec) && FlagDetect==0)
		{
			ArrayTimers[i]--;
			FlagDetect=1;
			LcdUpdate();
		}
	}

	if((ArrayCounter[0][1] == CurrentSec) || (ArrayCounter[1][1] == CurrentSec) || CurrentSec==1)
		FlagDetect=0;

	if(CurrentSec == 59 && FlagDetect==0)
	{
		TotalCounts++;
		FlagDetect=1;
		LCD_WriteNumber_Position(TotalCounts,1,1);
	}

	LCD_WriteNumber_Position(CurrentMin,1,5);
	LCD_WriteNumber_Position(CurrentSec,2,5);

	/* LCD Prnt */
}

void get_time(void)
{
  RTC_DateTypeDef gDate;
  RTC_TimeTypeDef gTime;

   //Get the RTC current Time
  HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);

  // Get the RTC current Date
  HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);

  CurrentSec = gTime.Seconds;
  CurrentMin = gTime.Minutes;

   //Display time Format: hh:mm:ss
  sprintf((char*)time,"%02d:%02d:%02d",gTime.Hours, gTime.Minutes, gTime.Seconds);

  // Display date Format: mm-dd-yy
  sprintf((char*)date,"%02d-%02d-%2d",gDate.Date, gDate.Month, 2000 + gDate.Year);
}

uint16_t GetKeypadValue()
{
	uint16_t retVal = 0;
	keypadVal = NOTPRESSED;

	while(keypadVal != '*')
	{
		keypadVal = read_keypad(&keypad_1);
		if(keypadVal>='0' && keypadVal<='9')
		{
			lcd_4bit_send_char_data(&lcd_1, keypadVal);
			retVal = ((keypadVal - '0') + (retVal * 10));
			keypadVal = NOTPRESSED;
		}
	}

	keypadVal = NOTPRESSED;
	return retVal;
}


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

	MRCC_voidPeripheralClockEnable(RCC_APB2_BUS , RCC_GPIOA_CLOCK);
	MRCC_voidPeripheralClockEnable(RCC_APB2_BUS , RCC_GPIOB_CLOCK);

	lcd_4bit_intialize(&lcd_1);

	LCD_Clear(&lcd_1);
	lcd_4bit_send_string_pos(&lcd_1, 1, 2, "1st Mins :");
	ArrayCounter[0][0] = GetKeypadValue();

	LCD_Clear(&lcd_1);
	lcd_4bit_send_string_pos(&lcd_1, 1, 2, "1st Secs :");
	ArrayCounter[0][1] = GetKeypadValue();

	LCD_Clear(&lcd_1);
	lcd_4bit_send_string_pos(&lcd_1, 1, 2, "2nd Mins :");
	ArrayCounter[1][0] = GetKeypadValue();

	LCD_Clear(&lcd_1);
	lcd_4bit_send_string_pos(&lcd_1, 1, 2, "2nd Secs :");
	ArrayCounter[1][1] = GetKeypadValue();


	LCD_Clear(&lcd_1);
	lcd_4bit_send_string_pos(&lcd_1, 1, 2, "Intial Timer 1:");
	ArrayTimers[0] = GetKeypadValue();

	LCD_Clear(&lcd_1);
	lcd_4bit_send_string_pos(&lcd_1, 1, 2, "Intial Timer 2:");
	ArrayTimers[1] = GetKeypadValue();


	LCD_Clear(&lcd_1);
  set_time();
  LcdUpdate();
  LCD_WriteNumber_Position(TotalCounts,1,1);
  uint8_t k=22;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	 get_time();
	 MinDetect();
	 HAL_Delay(100);

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV128;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
