#ifndef _RS485_H
#define _RS485_H

#include "main.h"
#include "usart.h"

#define RS485_TX_ENABLE   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET)   // 发送模式
#define RS485_RX_ENABLE   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET) // 接收模式

#define MODBUS_SLAVE_ADDR      0x01
#define BLOCK_COUNT            0x28
#define BLOCK_COUNT_MAX		   0x2E
#define MODBUS_REG_COUNT       0x56

// 接收状态结构体
typedef struct 
{
	uint8_t  myadd;        //本设备地址
	uint8_t  rcbuf[200];   //modbus接受缓冲区
	uint8_t  recount;      //modbus端口接收到的数据个数
	uint8_t  reflag;       //1:一帧数据接受完成标志位 0:未完成
	uint8_t  sendbuf[200]; //modbus发送缓冲区
	uint8_t  readID;
	uint8_t  factory_exit;
	uint8_t Data_Eeprom[520];
}Modbus_t;

extern Modbus_t modbus;

void L2_RS485_Init(void);
uint16_t ModbusCRC(uint8_t *pdata, uint16_t len);
void L2_Process_Modbus(void);
void Device_Addr_Init(uint8_t num);

#endif
