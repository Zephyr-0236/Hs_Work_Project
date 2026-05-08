#ifndef __EEPROM_H
#define __EEPROM_H

#include "stm32g0xx_hal.h"

// =============== I2C 引脚定义 ===============
#define I2C_GPIO_PORT   GPIOB
#define I2C_SCL_PIN     GPIO_PIN_6
#define I2C_SDA_PIN     GPIO_PIN_7

// I2C引脚操作宏定义
#define SCL_High()  HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SCL_PIN, GPIO_PIN_SET)    // SCL置高
#define SCL_Low()   HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SCL_PIN, GPIO_PIN_RESET)  // SCL置低
#define SDA_High()  HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SDA_PIN, GPIO_PIN_SET)    // SDA置高
#define SDA_Low()   HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SDA_PIN, GPIO_PIN_RESET)  // SDA置低
#define SDA_Read()  HAL_GPIO_ReadPin(I2C_GPIO_PORT, I2C_SDA_PIN)                   // 读取SDA电平

// =============== EEPROM 定义 ===============
#define EEPROM_ADDR_BASE  0xA0  // 24CW160T: 1010 + A10/A9/A8 + R/W

// =============== 接口函数 ===============
void     Soft_I2C_Init(void);
void     I2C_Start(void);
void     I2C_Stop(void);
void     I2C_SendByte(uint8_t data);
uint8_t  I2C_RecvByte(void);
uint8_t  I2C_RecvAck(void);
void     I2C_SendAck(uint8_t ack);


// =============== EEPROM 定义 ===============
#define EEPROM_ADDR_WRITE  0xA0  // 24CW160T器件地址 + 写命令: 1010 000 0
#define EEPROM_ADDR_READ   0xA1  // 24CW160T器件地址 + 读命令: 1010 000 1
#define EEPROM_PAGE_SIZE   8     // 24CW160T页大小为8字节
#define EEPROM_TOTAL_SIZE  2048  // 16Kbit = 2048字节

// =============== EEPROM 操作函数 ===============
uint8_t EEPROM_WriteByte(uint16_t addr, uint8_t data);
uint8_t EEPROM_ReadByte(uint16_t addr);
uint8_t EEPROM_WritePage(uint16_t addr, uint8_t *pData, uint8_t len);
uint8_t EEPROM_ReadBuffer(uint16_t addr, uint8_t *pData, uint16_t len);
uint8_t EEPROM_WriteBuffer(uint16_t addr, uint8_t *pData, uint16_t len);



#endif
