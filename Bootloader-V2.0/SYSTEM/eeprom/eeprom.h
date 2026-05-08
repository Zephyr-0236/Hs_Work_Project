#ifndef __EEPROM_H
#define __EEPROM_H

#include "stm32f10x.h"

// =============== I2C 引脚定义 ===============
#define I2C_GPIO_PORT   GPIOB
#define I2C_SCL_PIN     GPIO_Pin_8
#define I2C_SDA_PIN     GPIO_Pin_9

#define SCL_High()  GPIO_SetBits(I2C_GPIO_PORT, I2C_SCL_PIN)
#define SCL_Low()   GPIO_ResetBits(I2C_GPIO_PORT, I2C_SCL_PIN)
#define SDA_High()  GPIO_SetBits(I2C_GPIO_PORT, I2C_SDA_PIN)
#define SDA_Low()   GPIO_ResetBits(I2C_GPIO_PORT, I2C_SDA_PIN)
#define SDA_Read()  GPIO_ReadInputDataBit(I2C_GPIO_PORT, I2C_SDA_PIN)

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

uint8_t EEPROM_GetDeviceAddr(uint16_t memAddr, uint8_t rw);
void     EEPROM_WriteByte(uint16_t memAddr, uint8_t data);
uint8_t  EEPROM_ReadByte(uint16_t memAddr);
void     EEPROM_PageWrite(uint16_t memAddr, uint8_t *buf, uint8_t len);
void     EEPROM_ReadBuffer(uint16_t memAddr, uint8_t *buf, uint16_t len);

void EEPROM_Test(void);

#endif
