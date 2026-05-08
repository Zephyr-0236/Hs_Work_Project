#include "led.h"

//LED IO初始化
void LED_Init(void)
{
	//COMMM-LED DRIVER PB05
	GPIO_InitTypeDef GPIO_InitStructure = {0};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//打开GPIOB口时钟
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_5;//PB05
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;//输出速度
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化gpio
	
	//因为 GPIOB 04 引脚的默认功能不是 GPIO 口的功能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); /*使能SW-DP 禁用JTAG-DP*/
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4;//PB04
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;//输出速度
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化gpio
}

void LED4_ON(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_4);
}

void LED5_ON(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_5);
}

void LED4_OFF(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_4);
}

void LED5_OFF(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_5);
}
