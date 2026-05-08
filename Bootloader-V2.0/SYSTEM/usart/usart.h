#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
#define USART_REC_LEN  			1024*2 	//定义最大接收字节数 41K
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收

#define RS485_TX_ENABLE GPIO_SetBits(GPIOA,GPIO_Pin_11)   	//使能485控制端(启动发送)
#define RS485_RX_ENABLE GPIO_ResetBits(GPIOA,GPIO_Pin_11)	//失能485控制端(改为接收)

typedef struct {
    volatile uint8_t data_ready;  // 接收完成标志
    volatile uint16_t packet_num; // 当前包序号
    uint8_t rcbuf[2000];          // 接收缓冲
    volatile uint16_t rx_count;   // 当前接收字节数
} Modbus_t;

extern Modbus_t modbus;

void uart_init(u32 bound);
void USART1_R_BUFF_CLEAR(void);
void Flash_Erase(uint32_t startAddr, uint32_t length);
void Flash_Write(uint32_t startAddr, uint8_t *data, uint32_t length);
void *my_memmem(const void *haystack, size_t haystack_len,const void *needle, size_t needle_len);
uint8_t calculate_checksum(const uint8_t *data, size_t len);
int verify_checksum(const uint8_t *data, size_t len);

//串口的 modbus 协议发送数据
void USART1_SendByte(uint8_t data);
void USART1_SendString(uint8_t *str);
void USART1_SendBuffer(uint8_t *buffer, uint16_t length);

#endif


