#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "stmflash.h"   
#include "iap.h"	
#include "timer.h"
#include "led.h"
#include "eeprom.h"
#include "stm32f10x_flash.h"

#include <stdio.h>
#include <string.h>

uint8_t Flash_Status = 0;

u16 applenth=0;	//接收到的app代码长度.一帧数据为1024
u8	Send_Kyte_Num=0;

//FLASH_Status Flash_Erase_Status;
uint8_t value;
uint8_t Test_data[2048];
int i;
uint8_t buf[16] = {0,1,2,3,4,5,6,7,8,9,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};

uint8_t DownLoadFlag = 0;
uint32_t DownLoadEndPage = 0;

typedef struct{
	uint16_t i;
	uint16_t temp[1200];
	uint16_t *p;
	uint8_t *local_buf;
	uint16_t packet_num;
	uint32_t Download_Addr_Start;
	uint32_t Download_Addr_End;
} Flash_struct;

Flash_struct flash_data;

void LED_Init(void);

int main(void)
{
	volatile u8 Loader_Flash = 0;
	
	SCB->VTOR = FLASH_BASE | 0x0000; /* Vector Table Relocation in Internal FLASH. */
	IWDG->KR = ((uint16_t)0xAAAA);
	
  	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	
	LED_Init();
	
	TIM3_Int_Init(4999,7199);//500ms
	uart_init(115200);	 	 //串口初始化为230400
	delay_init();
	
	LED4_ON();
	LED5_ON();
	
	Soft_I2C_Init();
	
//	// test
	while(1)
	{
//		USART1_SendByte(0xAA);
//		delay_ms(1000);
		if(modbus.data_ready == 1)
		{
			USART1_SendByte(0x55);
		}
	}
	
	while(1)
	{
		if(Timer_Bt>=2 && DownLoadFlag == 0)														// 超时直接跳转 app 区域
		{
			Read_FlashToApp();
		}
		
		if(modbus.data_ready == 1)
		{
			modbus.data_ready = 0;
			
			switch(Flash_Status)
			{
				case 0:														// 握手校验状态
					if(verify_checksum(modbus.rcbuf, 14) && modbus.rcbuf[1] == 0x4C && modbus.rcbuf[2] == 0x57 && modbus.rcbuf[3] == 0x00)					// 校验数据是否完整
					{
						uint8_t Send_ACK_Buff[14] = {0x06, 0x4C, 0x57, 0x00, 0x00, 0x08, 0x00, 0x40, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00};
						
						Send_ACK_Buff[4] = modbus.rcbuf[4];
						Send_ACK_Buff[5] = modbus.rcbuf[5]; Send_ACK_Buff[6] = modbus.rcbuf[6]; Send_ACK_Buff[7] = modbus.rcbuf[7]; Send_ACK_Buff[8] = modbus.rcbuf[8];
						Send_ACK_Buff[9] = modbus.rcbuf[9]; Send_ACK_Buff[10] = modbus.rcbuf[10]; Send_ACK_Buff[11] = modbus.rcbuf[11]; Send_ACK_Buff[12] = modbus.rcbuf[12];
						Send_ACK_Buff[13] = calculate_checksum(Send_ACK_Buff, 13);
						
						flash_data.Download_Addr_Start = (uint32_t)modbus.rcbuf[5]  << 24 | (uint32_t)modbus.rcbuf[6] << 16  | (uint32_t)modbus.rcbuf[7] << 8  | modbus.rcbuf[8];
						flash_data.Download_Addr_End   = (uint32_t)modbus.rcbuf[9] << 24  | (uint32_t)modbus.rcbuf[10] << 16 | (uint32_t)modbus.rcbuf[11] << 8 | modbus.rcbuf[12];
						
						DownLoadEndPage = (flash_data.Download_Addr_End - flash_data.Download_Addr_Start)/1024;
						
						if(flash_data.Download_Addr_Start != Flash_App_Addr || flash_data.Download_Addr_End >= 0x08010000)
						{
							uint8_t Send_NACK_Buff[14] = {0x15, 0x4C, 0x57, 0x00, 0x00, 0x08, 0x00, 0x40, 0x00, 0x08, 0x01, 0x00, 0X00, 0x00};
						
							Send_NACK_Buff[4] = modbus.rcbuf[4];
							Send_NACK_Buff[5] = modbus.rcbuf[5]; Send_NACK_Buff[6] = modbus.rcbuf[6]; Send_NACK_Buff[7] = modbus.rcbuf[7]; Send_NACK_Buff[8] = modbus.rcbuf[8];
							Send_NACK_Buff[9] = modbus.rcbuf[9]; Send_NACK_Buff[10] = modbus.rcbuf[10]; Send_NACK_Buff[11] = modbus.rcbuf[11]; Send_NACK_Buff[12] = modbus.rcbuf[12];
							Send_NACK_Buff[13] = calculate_checksum(Send_NACK_Buff, 13);
							
							USART1_SendBuffer(Send_NACK_Buff, sizeof(Send_NACK_Buff));
							
							Flash_Status = 5;
							
							USART1_R_BUFF_CLEAR();
							
							break;
						}
						
						Flash_App_Addr = flash_data.Download_Addr_Start;
						
						USART1_SendBuffer(Send_ACK_Buff, sizeof(Send_ACK_Buff));
						
						Flash_Status ++;
						DownLoadFlag = 1;
						modbus.packet_num = 0;
						USART1_R_BUFF_CLEAR();
						
//						TIM_SetCounter(TIM3, 0);   // 清零计数  从而进行重新计时
//						TIM_Cmd(TIM3, ENABLE);     // 启动定时器
					}
					else if(Timer_Bt>=2)									// 校验不通过表示数据并未接收到 并且和超时 机制共同判断 是否退出 bootlaoder
					{
						// 主机超出 15 个握手帧就 对应从机的超时 500-1000ms 然后就执行退出 bootloader 操作，可以直接跳转到app区域
						
						uint8_t Send_NACK_Buff[14] = {0x15, 0x4C, 0x57, 0x00, 0x00, 0x08, 0x00, 0x40, 0x00, 0x08, 0x01, 0x00, 0X00, 0x00};
						
						Send_NACK_Buff[4] = modbus.rcbuf[4];
						Send_NACK_Buff[5] = modbus.rcbuf[5]; Send_NACK_Buff[6] = modbus.rcbuf[6]; Send_NACK_Buff[7] = modbus.rcbuf[7]; Send_NACK_Buff[8] = modbus.rcbuf[8];
						Send_NACK_Buff[9] = modbus.rcbuf[9]; Send_NACK_Buff[10] = modbus.rcbuf[10]; Send_NACK_Buff[11] = modbus.rcbuf[11]; Send_NACK_Buff[12] = modbus.rcbuf[12];
						Send_NACK_Buff[13] = calculate_checksum(Send_NACK_Buff, 13);
						
						USART1_SendBuffer(Send_NACK_Buff, sizeof(Send_NACK_Buff));
						
						DownLoadFlag = 0;
						Flash_Status = 5;
						USART1_R_BUFF_CLEAR();
					}
					break;
				case 1:														// 擦除FLASH状态			需要添加 顺序号 必须从 0 开始的判断条件 每一个状态机都对应一个标志位来表示收到数据包的个数  空闲中断中添加判断握手成功的标志位 从而进行下一下擦除 Flash 的指令的接收，后续 情况以此类推
					if(verify_checksum(modbus.rcbuf, 5))					// 通过最后两个字节的数据校验来判断 数据传输是否存在问题
					{
						uint8_t status = 0;
						uint8_t Send_ACK_Buff[5] = {0x06, 0x00, 0x00, 0x00, 0x00};
						
						Send_ACK_Buff[1] = modbus.rcbuf[1]; Send_ACK_Buff[2] = modbus.rcbuf[2]; Send_ACK_Buff[3] = modbus.rcbuf[3];
						Send_ACK_Buff[4] = calculate_checksum(Send_ACK_Buff, 4);
						
						if(modbus.rcbuf[0] == 0x10)							// 退出擦除 可以用来表示擦除已完成 跳转状态机 执行下一步数据的传输
						{
							Flash_Status ++;
							modbus.packet_num = 0;
							
							USART1_SendBuffer(Send_ACK_Buff, sizeof(Send_ACK_Buff));
							USART1_R_BUFF_CLEAR();
							
//							TIM_SetCounter(TIM3, 0);   // 清零计数 从而进行重新计时
//							TIM_Cmd(TIM3, ENABLE);     // 启动定时器
							break;
						}
						
						// 用来确认 顺序 号 是从 0 开始依次 递增的 否则就结束循环
						if(modbus.packet_num - (((uint16_t)modbus.rcbuf[2] << 8) | modbus.rcbuf[3]) != 1)
						{
							uint8_t Send_NACK_Buff[5] =  {0x15, 0x00, 0x00, 0x00, 0x00};
							
							Send_NACK_Buff[1] = modbus.rcbuf[1]; Send_NACK_Buff[2] = modbus.rcbuf[2]; Send_NACK_Buff[3] = modbus.rcbuf[3];
							Send_NACK_Buff[4] = calculate_checksum(Send_NACK_Buff, 4);
							
							USART1_SendBuffer(Send_NACK_Buff, sizeof(Send_NACK_Buff));
							
							Flash_Status = 5;
							USART1_R_BUFF_CLEAR();
							
							break;
						}
												
						// 擦除 Flash 暂时 实现的是按照 1024个字节大小进行 FLASH 的擦除 如果后续要改成 2048个字节大小的话，可以 直接使用 一对二的关系进行转化的 FLASH 的擦除
						// 由于 Flash 的擦除是按照 page 擦除的 而 stm32f103c8t6 的 page 的大小是 1024 个字节 所以进行Flash 的擦除过程中 传输的 地址的大小就是 0x400（1024）的倍数
						status = Flash_EraseSector(flash_data.Download_Addr_Start + (((uint16_t)modbus.rcbuf[2] << 8) | modbus.rcbuf[3]) * 0x400);
						
						if(status == FLASH_COMPLETE) {						//根据擦除函数的返回值来判断擦除是否成功
							USART1_SendBuffer(Send_ACK_Buff, sizeof(Send_ACK_Buff));
						}
						else 
						{
							//发送对应错误码 表示擦除失败
							uint8_t Send_NACK_Buff[5] =  {0x15, 0x00, 0x00, 0x00, 0x00};
							
							Send_NACK_Buff[1] = modbus.rcbuf[1]; Send_NACK_Buff[2] = modbus.rcbuf[2]; Send_NACK_Buff[3] = modbus.rcbuf[3];
							Send_NACK_Buff[4] = calculate_checksum(Send_NACK_Buff, 4);
							
							USART1_SendBuffer(Send_NACK_Buff, sizeof(Send_NACK_Buff));
							
							Flash_Status = 5;								//要添加对应的 退出bootloader  暂时还没有确定 具体的实现情况
							
						}
						
						USART1_R_BUFF_CLEAR();
					}
					else if(Timer_Bt>=2)									//校验不通过 也表示从机没有收到数据 // 从机超时机制，表示从机 1s 内没有收到数据从而进行相应的处理
					{
						//数据传输存在 数据丢失 或者数据传输超时
						uint8_t Send_NACK_Buff[5] =  {0x15, 0x00, 0x00, 0x00, 0x00};
						
						Send_NACK_Buff[1] = modbus.rcbuf[1]; Send_NACK_Buff[2] = modbus.rcbuf[2]; Send_NACK_Buff[3] = modbus.rcbuf[3];
						Send_NACK_Buff[4] = calculate_checksum(Send_NACK_Buff, 4);
						
						USART1_SendBuffer(Send_NACK_Buff, sizeof(Send_NACK_Buff));
						
						Flash_Status = 5;
						USART1_R_BUFF_CLEAR();
					}
					break;
				case 2:														// 数据传输状态
					if(verify_checksum(modbus.rcbuf, 1028))					// 校验数据是否完整		需要修改为 1028 因为数据传输的时候 是按照一页的大小进行使用的
					{
						uint16_t sequence_num = 0;
						uint8_t Send_ACK_Buff[4] = {0x06, 0x00, 0x00, 0x00};
						
						Send_ACK_Buff[1] = modbus.rcbuf[1]; Send_ACK_Buff[2] = modbus.rcbuf[2];
						Send_ACK_Buff[3] = calculate_checksum(Send_ACK_Buff, 3);
						
						sequence_num = ((uint16_t)modbus.rcbuf[1] << 8) | modbus.rcbuf[2];
						
						if(sequence_num == 0xFFFF)							// 退出数据传输 可以用来表示数据已经传输完成 跳转状态机 执行下一步Flash校验
						{
							Flash_Status ++;
							modbus.packet_num = 0;
							
							USART1_SendBuffer(Send_ACK_Buff, sizeof(Send_ACK_Buff));
							USART1_R_BUFF_CLEAR();
							
//							TIM_SetCounter(TIM3, 0);   // 清零计数 从而进行重新计时
//							TIM_Cmd(TIM3, ENABLE);     // 启动定时器
							break;
						}
						
						// 用来确认 数据帧顺序号 是从 0 开始依次 递增的 否则就结束循环
						if(modbus.packet_num - sequence_num != 1)
						{
							uint8_t Send_NACK_Buff[4] =  {0x15, 0x00, 0x00, 0x00};
							
							Send_NACK_Buff[1] = modbus.rcbuf[1]; Send_NACK_Buff[2] = modbus.rcbuf[2];
							Send_NACK_Buff[3] = calculate_checksum(Send_NACK_Buff, 3);
							
							USART1_SendBuffer(Send_NACK_Buff, sizeof(Send_NACK_Buff));
							
							Flash_Status = 5;
							USART1_R_BUFF_CLEAR();
							
							break;
						}
						
						// 向FLASH中写入数据 并一次性写入 1024 个字节的数据
						for (flash_data.i = 0; flash_data.i < 512; flash_data.i++)
						{
							flash_data.temp[flash_data.i] = ((uint16_t)modbus.rcbuf[flash_data.i*2+4] << 8) | modbus.rcbuf[flash_data.i*2+3];
						}
						
						Flash_Write_1(flash_data.Download_Addr_Start + (((uint16_t)modbus.rcbuf[1] << 8) | modbus.rcbuf[2]) * 0x400, flash_data.temp, 512);
						
						memset(flash_data.temp, 0, sizeof(flash_data.temp));
						
						USART1_SendBuffer(Send_ACK_Buff, sizeof(Send_ACK_Buff));
						
						USART1_R_BUFF_CLEAR();
						
//						TIM_SetCounter(TIM3, 0);   // 清零计数 从而进行重新计时
//						TIM_Cmd(TIM3, ENABLE);     // 启动定时器
					}
					else if(Timer_Bt>=2)									// 校验不通过表示数据并未接收到 并且和超时 机制共同判断 是否退出 bootlaoder
					{
						uint8_t Send_NACK_Buff[4] =  {0x15, 0x00, 0x00, 0x00};
						
						Send_NACK_Buff[1] = modbus.rcbuf[1]; Send_NACK_Buff[2] = modbus.rcbuf[2];
						Send_NACK_Buff[3] = calculate_checksum(Send_NACK_Buff, 3);
						
						USART1_SendBuffer(Send_NACK_Buff, sizeof(Send_NACK_Buff));
						
						Flash_Status = 5;									//要添加对应的 退出bootloader
						USART1_R_BUFF_CLEAR();
					}
					else{
						modbus.rcbuf[1] = modbus.rcbuf[1];
						modbus.rcbuf[2] = modbus.rcbuf[2];
						modbus.rcbuf[2] = modbus.rcbuf[2];
						modbus.rcbuf[2] = modbus.rcbuf[2];
					}
					break;
				case 3:														// flash校验状态
					if(verify_checksum(modbus.rcbuf, 4))					// 通过最后两个字节的数据校验来判断 数据传输是否存在问题
					{
						
						uint8_t Send_ACK_Buff[1050] = {0x06, 0x00, 0x00, 0x00};
						
						Send_ACK_Buff[1] = modbus.rcbuf[1]; Send_ACK_Buff[2] = modbus.rcbuf[2]; 
						
						if(modbus.rcbuf[0] == 0x0D)							// 退出 校验 表示 校验已完成
						{
							Flash_Status ++;
							modbus.packet_num = 0;
							DownLoadFlag = 0;
							
							Send_ACK_Buff[3] = calculate_checksum(Send_ACK_Buff, 3);
							USART1_SendBuffer(Send_ACK_Buff, 4);
							USART1_R_BUFF_CLEAR();
							
							TIM_SetCounter(TIM3, 0);   // 清零计数 从而进行重新计时
							TIM_Cmd(TIM3, ENABLE);     // 启动定时器
							break;
						}
						
						if(modbus.packet_num == 35)
							modbus.packet_num = modbus.packet_num;
						if(modbus.packet_num == 36)
							modbus.packet_num = modbus.packet_num;
						if(modbus.packet_num == 37)
							modbus.packet_num = modbus.packet_num;
						if(modbus.packet_num == 38)
							modbus.packet_num = modbus.packet_num;
												
						// 用来确认 数据帧顺序号 是从 0 开始依次 递增的 否则就结束循环
						if(modbus.packet_num - (((uint16_t)modbus.rcbuf[1] << 8) | modbus.rcbuf[2]) != 1)
						{
							uint8_t Send_NACK_Buff[4] =  {0x15, 0x00, 0x00, 0x00};
							
							Send_NACK_Buff[1] = modbus.rcbuf[1]; Send_NACK_Buff[2] = modbus.rcbuf[2];
							Send_NACK_Buff[3] = calculate_checksum(Send_NACK_Buff, 3);
							
							USART1_SendBuffer(Send_NACK_Buff, sizeof(Send_NACK_Buff));
							
							Flash_Status = 5;
							USART1_R_BUFF_CLEAR();
							
							break;
						}
												
						// 暂定 情况 一个 数据帧顺序号对应 1024 个字节的数据进行数据传输
						STMFLASH_Read(flash_data.Download_Addr_Start + (((uint16_t)modbus.rcbuf[1] << 8) | modbus.rcbuf[2]) * 0x400, flash_data.temp, 512);
						
						for (flash_data.i = 0; flash_data.i < 512; flash_data.i++)
						{
							Send_ACK_Buff[flash_data.i*2+3] = flash_data.temp[flash_data.i];
							Send_ACK_Buff[flash_data.i*2+4] = flash_data.temp[flash_data.i] / 256;
						}
						
						Send_ACK_Buff[1027] = calculate_checksum(Send_ACK_Buff, 1027);
						
						// 将从Flash中读到的数据结合校验 发送给 上位机
						USART1_SendBuffer(Send_ACK_Buff, 1028);
						
						if(DownLoadEndPage + 1 == (((uint16_t)modbus.rcbuf[1] << 8) | modbus.rcbuf[2]))
						{
							DownLoadFlag = 0;
							
							Send_ACK_Buff[3] = calculate_checksum(Send_ACK_Buff, 3);
							USART1_SendBuffer(Send_ACK_Buff, 4);
							USART1_R_BUFF_CLEAR();
						}
						
						USART1_R_BUFF_CLEAR();
					}
					else if(Timer_Bt>=2)									//校验不通过 也表示从机没有收到数据 // 从机超时机制，表示从机 1s 内没有收到数据从而进行相应的处理
					{
						//数据传输存在 数据丢失 或者数据传输超时  后续可以修改
						uint8_t Send_NACK_Buff[4] =  {0x15, 0x00, 0x00, 0x00};
							
						Send_NACK_Buff[1] = modbus.rcbuf[1]; Send_NACK_Buff[2] = modbus.rcbuf[2];
						Send_NACK_Buff[3] = calculate_checksum(Send_NACK_Buff, 3);
						
						USART1_SendBuffer(Send_NACK_Buff, sizeof(Send_NACK_Buff));
						
						Flash_Status = 5;
						USART1_R_BUFF_CLEAR();
					}
					break;
				case 4:														// 跳转app 状态
					Read_FlashToApp();
					break;
				case 5:														// 错误处理 状态
					Read_FlashToApp();										// 可以是复位操作 暂时都让它跳转 app 
					break;
				default :
					break;
			}
		}
	}
}
