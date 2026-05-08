#include "CLRC663_Interface.h"
//#include "delay.h"

/*int32_t rxdata[15];//接收缓冲区 
int32_t txdata[15];//发送缓冲区
void CLRC663_Uart_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure; 
	NVIC_InitTypeDef NVIC_InitStructure;
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA,ENABLE);
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2,ENABLE);
  //USART2_TX -> PA2 , USART2_RX ->	PA3			
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	         
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_Init(GPIOA, &GPIO_InitStructure);		   

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	        
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure); 
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  USART_ClearFlag(USART2,USART_FLAG_TC);
  USART_Cmd(USART2, ENABLE);
  //Usart2 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
}
//663初始化
/*
void CLRC663_Uart_Init(void)
{
	//GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	
	//USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
 
	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);  

   //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//

	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器USART1
  //USART 初始化设置 
	USART_InitStructure.USART_BaudRate = 115200;//设置为4800;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
  USART_Cmd(USART1, ENABLE);                    //使能串口 
}*/
//接收数据帧（读CLRC663寄存器）
void uart_rx_frame_663(uint8_t num) //接收数据帧（读CLRC663寄存器） 
{  
 	  uint8_t   i; 
	  USART_SendData(USART1, txdata[0]);
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送完成
	  USART_ClearFlag(USART1, USART_FLAG_TXE);//清除发送完成标志 
	  for(i=0;i<num;i++)                //（读取字节数），循环读取数据 
    {                                      
			 while(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == RESET);//等待接收完成 
			 rxdata[i]=USART_ReceiveData(USART1);//将接收到的数据存放到 rxdata数组，备用 
			 USART_ClearFlag(USART1, USART_FLAG_RXNE);//清除接收完成标志 
    }
}
//发送数据帧（写CLRC663寄存器）
void uart_tx_frame_663(uint8_t num)//发送数据帧（写CLRC663寄存器） 
{    
	  u8  i=1; 
    USART_SendData(USART1, txdata[0]);//发送写寄存器命令（0x80+ADDR） 
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送完成  
		USART_ClearFlag(USART1, USART_FLAG_TXE);//清除发送完成标志
    for(i=1;i<num;i++)   //根据参数 num（发送字节数），循环写数据 
    {                    
			  USART_SendData(USART1, txdata[i]);//发送数据 
        while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送完成        
        USART_ClearFlag(USART1, USART_FLAG_TXE);//清除发送完成标志  
    }                                                                 
		//USART_SendData(USART1,txdata[num]);//发送CHECKSUM            
    //while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//等待发送完成
		//USART_ClearFlag(USART1, USART_FLAG_TXE);//清除发送完成标志  
}
*/

