/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim14;

/* USER CODE BEGIN Private defines */
#define TIMER_COUNTER_NUM 4

#define TIMER_COUNTER_NFC    0
#define TIMER_COUNTER_IOLINK 1
#define TIMER_COUNTER_EEPROM 2
#define TIMER_COUNTER_LED    3

#define TIMER_COUNTER_RUNNING 1
#define TIMER_COUNTER_STOP    0

#define TIMER_COUNTER_10MS  10     //1counter = 1ms

extern uint8_t Timer1_Bt;
extern uint8_t Timer3_Bt;
extern uint16_t Timer14_Bt;

/* USER CODE END Private defines */

void MX_TIM1_Init(void);
void MX_TIM3_Init(void);
void MX_TIM14_Init(void);

/* USER CODE BEGIN Prototypes */


///////////////////////////////////
void L2_Clock_Timer_Init(void); 
void ConfigSystemClockToHSE(void);
void L2_Timer_CounterInit(void); 
void ConfigSystemClockToHSI(void);
void Timer1_Init(void);
void Timer2_Init(void);
void Timer3_Init(u16 arr,u16 psc);
void L2_Timer_CounterSet(uint8_t CounterNum,uint8_t CounterStatus);
uint16_t L2_Timer_CounterValue(uint8_t CounterNum);
void L2_Timer_CounterServer(void);
void L2_Timer_CounterInit(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

