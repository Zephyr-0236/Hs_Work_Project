/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "crc.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "rs485.h"
#include "nfc.h"
#include "delay.h"
#include "led.h"
#include "RegCtl_SpiHw.h"
#include "verify.h"
#include "eeprom.h"
#include "factory.h"

//#include "factory.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t Timer1_flag = 3;
uint16_t Timer14Cout[2] = {0};
uint8_t Timer14_flag = 0;

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
	SCB->VTOR = FLASH_BASE | 0x10000;

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_IWDG_Init();
	MX_TIM1_Init();
	MX_TIM3_Init();
	MX_TIM14_Init();
	MX_USART1_UART_Init(9600);
	/* USER CODE BEGIN 2 */
	L2_delay_Init();
	L2_LED_Init();
	
	Soft_I2C_Init();
	
	// 等待所有 初始化结束
	__enable_irq(); // 最后开启总中断
	
	LED_PWR_ON();
	LED_COM_OFF();
	LED_MODE_OFF();
	
	// 从机地址读取
    modbus.myadd = EEPROM_ReadByte(0xEC);
	
	// 0xA1 地址存储 RFID 的模式 是 15693 还是 14443A
	if(EEPROM_ReadByte(0xA1) != RC663_15693 && EEPROM_ReadByte(0xA1) != RC663_14443)
	{
		EEPROM_WriteByte(0xA1, RC663_15693);
		RC663_MODE = RC663_15693;
	}
	else
	{
		RC663_MODE = EEPROM_ReadByte(0xA1);
	}
	
	IWDG_FeedDog();
	
	// 用于定时器的开始计时
	L2_Timer_CounterInit();
	
//	L2_LED_NFC_Mode(LED_OFF);		// 添加以后灯的亮灭出现 紊乱
	
	/* USER CODE END 2 */

	/* Infinite loop */
	while(1)
	{
		
		if(modbus.reflag == 1 && Timer1_Bt < Timer1_flag)
		{
			if(!Search_HandShakeData(modbus.rcbuf, modbus.recount))
			{
				USART1_SendBuff(Test_Verify_data, sizeof(Test_Verify_data));
				
				LED_PWR_OFF();
				LED_COM_OFF();
				LED_MODE_OFF();
				
//				Timer14Cout[0] = Timer14_Bt;
				modbus.factory_exit = 1;
				
				IWDG_FeedDog();
				
				while(modbus.factory_exit)
				{
					Timer14Cout[1] = Timer14_Bt;
					if(Timer14Cout[1] > Timer14Cout[0] && Timer14Cout[1] - Timer14Cout[0] > 10 && Timer14_flag)		// 对应 两者之间相差 5s（10 对应 5s，因为500ms 触发一次中断） 时 可以退出 工厂模式，如果修改标签以后 超时5s退出工厂模式；
					{
						modbus.factory_exit = 0;
						Timer1_Bt = 200;
						LED_PWR_ON();
					}
					
					if(Tag_Exisit == 0)
					{
						EditAddr();
					}
					NFC_Factory_APP();
				}
			}
			
			IWDG_FeedDog();
			if(modbus.recount < sizeof(Test_Verify_data))
			{
				
			}
			else
			{
				modbus.reflag = 0;
				USART1_R_BUFF_CLEAR();
			}
			
		}
		
		if(Timer1_Bt >= Timer1_flag)
		{
			if(Tag_Exisit == 0)
			{
				EditAddr();
			}
			modbus.factory_exit = 0;
			L3_NFC_APP();
		}
		
		IWDG_FeedDog();
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
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
	RCC_OscInitStruct.PLL.PLLN = 12;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV3;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
