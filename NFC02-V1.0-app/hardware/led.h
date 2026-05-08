#ifndef _LED_H
#define _LED_H


#include "stm32g0xx_hal.h"

//LED MODE
#define LED_ON     0x00
#define LED_OFF    0x01
#define LED_0S5    0x02
#define LED_2S0    0x03

//LED TIMER COUNTER
#define LED_COUNTER_0S5      500 //TIM2 1ms
#define LED_COUNTER_2S0     2000 //TIM2 1ms


void L2_LED_Init(void);
void L2_LED_NFC_Mode(uint8_t LEDmode);
void L2_LED_COMM_Mode(uint8_t LEDmode);
void L3_LED_APP(void);


void LED_MODE_ON(void);
void LED_MODE_OFF(void);

void LED_PWR_ON(void);
void LED_PWR_OFF(void);

void LED_COM_ON(void);
void LED_COM_OFF(void);

#endif
