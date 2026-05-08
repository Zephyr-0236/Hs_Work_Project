/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
#include "tim.h"
#include "usart.h"
#include "led.h"
#include "rs485.h"

/* USER CODE BEGIN 0 */
uint8_t Timer1_Bt;
uint8_t Timer3_Bt;
uint16_t Timer14_Bt;

uint16_t TimerCounter[TIMER_COUNTER_NUM];
uint8_t TimerStatus[TIMER_COUNTER_NUM];
/* USER CODE END 0 */

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim14;

/* TIM1 init function */
void MX_TIM1_Init(void)				// 500ms 触发一次 中断函数进行计数
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 63999;
  htim1.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim1.Init.Period = 499;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */
	HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 2, 0); // 优先级可调整	// 注意：G0 只有 0-3 的抢占优先级，没有子优先级
	HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn); // 开启NVIC中断通道
  
	//启动 TIM1 中断
	HAL_TIM_Base_Start_IT(&htim1);
  /* USER CODE END TIM1_Init 2 */

}
/* TIM3 init function */
void MX_TIM3_Init(void)				// 1ms 触发一次 中断函数进行 LED灯的相关控制
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 63;
  htim3.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
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
	HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0); // 优先级可调整	// 注意：G0 只有 0-3 的抢占优先级，没有子优先级
	HAL_NVIC_EnableIRQ(TIM3_IRQn); // 开启NVIC中断通道

	//启动 TIM3 中断
	HAL_TIM_Base_Start_IT(&htim3);
  /* USER CODE END TIM3_Init 2 */

}
/* TIM14 init function */
void MX_TIM14_Init(void)			// 500ms 触发一次 中断函数进行计数
{
  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  
  __HAL_RCC_TIM14_CLK_ENABLE(); // 使能TIM14时钟

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 63999;
  htim14.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim14.Init.Period = 499;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim14, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim14, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */
  
	// 3. 配置中断并启动
    HAL_NVIC_SetPriority(TIM14_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM14_IRQn);

    HAL_TIM_Base_Start_IT(&htim14);
  /* USER CODE END TIM14_Init 2 */
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspInit 0 */

  /* USER CODE END TIM1_MspInit 0 */
    /* TIM1 clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();

    /* TIM1 interrupt Init */
    HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
  /* USER CODE BEGIN TIM1_MspInit 1 */

  /* USER CODE END TIM1_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspInit 0 */

  /* USER CODE END TIM3_MspInit 0 */
    /* TIM3 clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();

    /* TIM3 interrupt Init */
    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
  /* USER CODE BEGIN TIM3_MspInit 1 */

  /* USER CODE END TIM3_MspInit 1 */
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspDeInit 0 */

  /* USER CODE END TIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM1_CLK_DISABLE();

    /* TIM1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
  /* USER CODE BEGIN TIM1_MspDeInit 1 */

  /* USER CODE END TIM1_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspDeInit 0 */

  /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();

    /* TIM3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
  /* USER CODE BEGIN TIM3_MspDeInit 1 */

  /* USER CODE END TIM3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
  * @brief  TIM1 更新中断服务函数
  */
void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
    if (__HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_UPDATE) != RESET)    // 判断更新中断标志
    {
        if (__HAL_TIM_GET_IT_SOURCE(&htim1, TIM_IT_UPDATE) != RESET) // 判断更新中断是否使能
        {
            __HAL_TIM_CLEAR_IT(&htim1, TIM_IT_UPDATE);   // 清除中断标志位
            
            if (Timer1_Bt < 255)   // 防止数据越界
                Timer1_Bt++;
        }
    }
}

/**
  * @brief  TIM3 更新中断服务函数
  */
void TIM3_IRQHandler(void)
{
    // 检查TIM3的更新中断标志
    if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE) != RESET)
    {
        // 清除更新中断标志
        if (__HAL_TIM_GET_IT_SOURCE(&htim3, TIM_IT_UPDATE) != RESET)
        {
            __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
            
            // 调用定时器计数服务函数
            L2_Timer_CounterServer();
            
            // 如果不在工厂模式退出状态，执行LED应用函数
            if (modbus.factory_exit == 0)
            {
                L3_LED_APP();
            }
        }
    }
}

/**
  * @brief  TIM14 更新中断服务函数
  */
void TIM14_IRQHandler(void)
{
    if (__HAL_TIM_GET_FLAG(&htim14, TIM_FLAG_UPDATE) != RESET)    // 判断更新中断标志
    {
        if (__HAL_TIM_GET_IT_SOURCE(&htim14, TIM_IT_UPDATE) != RESET) // 判断更新中断是否使能
        {
            __HAL_TIM_CLEAR_IT(&htim14, TIM_IT_UPDATE);   // 清除中断标志位
            
            if (Timer14_Bt < 65535)   // 防止数据越界
                Timer14_Bt++;
        }
    }
}

void L2_Timer_CounterInit(void)
{
	int8_t i;
	for(i=0;i<TIMER_COUNTER_NUM;i++)
	{
		TimerStatus[i]=TIMER_COUNTER_STOP;
	}
	
	for(i=0;i<TIMER_COUNTER_NUM;i++)
	{
		TimerCounter[i]=0;
	}
}


void L2_Timer_CounterServer(void)
{
	int8_t i;
	
	//timer counter1
	for(i=0;i<TIMER_COUNTER_NUM;i++)
	{
		if(TimerCounter[i]> 60000)
		{
			//溢出保护
			TimerStatus[i]=TIMER_COUNTER_STOP;
		}
	}
	
	
	for(i=0;i<TIMER_COUNTER_NUM;i++)
	{
		if(TimerStatus[i] == TIMER_COUNTER_RUNNING)
		{	
			TimerCounter[i]++;
		}
		else if(TimerStatus[i] == TIMER_COUNTER_STOP)
		{
			TimerCounter[i]=0;
		}
	}
	
}

void L2_Timer_CounterSet(uint8_t CounterNum,uint8_t CounterStatus)
{
	if(CounterNum < TIMER_COUNTER_NUM)
	{
		TimerStatus[CounterNum] = CounterStatus;
	}

}	

uint16_t L2_Timer_CounterValue(uint8_t CounterNum)
{
	if(CounterNum<TIMER_COUNTER_NUM)
		return TimerCounter[CounterNum];
	else 
		return 0;
}

///////////////// test start
// 初始化后启动定时器
void Start_TIM14(void)
{
    // 启动定时器中断模式
    if (HAL_TIM_Base_Start_IT(&htim14) != HAL_OK)
    {
        Error_Handler();
    }
}

// 停止定时器
void Stop_TIM14(void)
{
    HAL_TIM_Base_Stop_IT(&htim14);
}

// 修改定时器周期
void Change_TIM14_Period(uint32_t period)
{
    __HAL_TIM_SET_AUTORELOAD(&htim14, period);
}

// 获取当前计数值
uint32_t Get_TIM14_Counter(void)
{
    return __HAL_TIM_GET_COUNTER(&htim14);
}

//////////////////// test end


/* USER CODE END 1 */
