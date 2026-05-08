#include "led.h"

static uint8_t u8_L2_LED_NFC_MODE;
static uint8_t u8_L2_LED_COMM_MODE;

void L2_LED_Init(void)
{
    // LED相关引脚 PA01, PA02, PB10
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 使能 GPIOA和GPIOB 时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

    // 配置 PA01
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;      // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;              // 无上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;    // 50MHz
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 配置 PA02
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;      // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;              // 无上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;    // 50MHz
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	// 配置 PB10
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;      // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;              // 无上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;    // 50MHz
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
}

void L2_LED_NFC_Mode(uint8_t LEDmode)
{
   u8_L2_LED_NFC_MODE = LEDmode;
}

void L2_LED_COMM_Mode(uint8_t LEDmode)
{
   u8_L2_LED_COMM_MODE = LEDmode;
}


void L3_LED_APP(void)
{
	static uint16_t LedNfcCounter=0;
	static uint16_t LedCommCounter=0;

	switch(u8_L2_LED_NFC_MODE)
	{
		case LED_ON:
//			LED_MODE_ON();
			LED_COM_ON();
//			LED_PWR_ON();
			break;
		
		case LED_OFF:
//			LED_MODE_OFF();
			LED_COM_OFF();
//			LED_PWR_OFF();
			break;
		
		case LED_0S5:
			LedNfcCounter++;
			if(LedNfcCounter == LED_COUNTER_0S5 )
			{
//				LED_MODE_ON();
				LED_COM_ON();
//				LED_PWR_ON();
			}
			if(LedNfcCounter >=(2 * LED_COUNTER_0S5))
			{
//				LED_MODE_OFF();
				LED_COM_OFF();
//				LED_PWR_OFF();
				LedNfcCounter=0;
			}
			break;
			
		case LED_2S0:
			LedNfcCounter++;
			if(LedNfcCounter == LED_COUNTER_2S0 )
			{
//				LED_MODE_ON();
				LED_COM_ON();
//				LED_PWR_ON();
			}
			if(LedNfcCounter >=(2 * LED_COUNTER_2S0))
			{
//				LED_MODE_OFF();
				LED_COM_OFF();
//				LED_PWR_OFF();
				LedNfcCounter =0;
			}
			break;
			
		default:
			break;
	}
	
	//COMM LED
	switch(u8_L2_LED_COMM_MODE)
	{
		case LED_ON:
//			LED_COM_ON();
			break;
		
		case LED_OFF:
//			LED_COM_OFF();
			break;
		
		case LED_0S5:
			LedCommCounter++;
			if(LedCommCounter == LED_COUNTER_0S5 )
			{
//			   LED_COM_ON();
			}
			if(LedCommCounter >= ( 2 * LED_COUNTER_0S5))
			{
//				LED_COM_OFF();
				LedCommCounter =0;
			}
			break;
			
		case LED_2S0:
			LedCommCounter++;
			if(LedCommCounter == LED_COUNTER_2S0 )
			{
//			   LED_COM_ON();
			}
			if(LedCommCounter >= ( 2 * LED_COUNTER_2S0))
			{
//				LED_COM_OFF();
				LedCommCounter =0;
			}
			break;
			
		default:
			break;
	}
}

void LED_MODE_ON(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
}

void LED_MODE_OFF(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
}

void LED_PWR_ON(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
}

void LED_PWR_OFF(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
}

void LED_COM_ON(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
}

void LED_COM_OFF(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
}
