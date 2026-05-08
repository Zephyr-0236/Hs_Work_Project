/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN Private defines */
#define USART1_RX_BUF_SIZE 200
#define USART1_TX_BUF_SIZE 200
#define USART3_RX_BUF_SIZE 256
#define USART3_TX_BUF_SIZE 256


/* USER CODE END Private defines */

void MX_USART1_UART_Init(uint32_t baudrate);
void MX_USART3_UART_Init(uint32_t baudrate);

/* USER CODE BEGIN Prototypes */

void USART1_SendByte(uint8_t data);
void USART1_SendStr(uint8_t *str);
void USART1_SendBuff(uint8_t *buffer, uint16_t length);
void USART1_R_BUFF_CLEAR(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

