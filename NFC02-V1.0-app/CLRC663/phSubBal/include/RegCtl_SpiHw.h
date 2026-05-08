/*******************************************************************************
*         Copyright (c), NXP Semiconductors Gratkorn / Austria
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
********************************************************************************
*
* Filename:          RegCtl_SpiHw.h
* Processor family:  LPC11xx
*
* Description:       This file holds the functions for the SPI communication.
*                    In case of the Silica TUSA board, the SPI communication
*                    gets emulated, because it uses GPIO pins that are not
*                    designated for use of SPI communication.
*******************************************************************************/

#ifndef REGCTL_SPIHW_H_
#define REGCTL_SPIHW_H_

#include <ph_Status.h>

#define SPI_SCLK_GPIO_L()    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET)
#define	SPI_SCLK_GPIO_H()    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET)
#define SPI_CS_GPIO_L()      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET)
#define	SPI_CS_GPIO_H()      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET)
#define SPI_MOSI_GPIO_L()    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET)
#define	SPI_MOSI_GPIO_H()    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET)

#define SPI_USED

void _SSP0_SetReg(uint8_t *buf);
void _SSP0_GetReg(uint8_t *buf);
extern void Reset_RC663_device(void);
extern void RegCtl_SpiHwInit(void);
extern phStatus_t RegCtl_SpiHwGetReg(uint8_t address, uint8_t *reg_data);
extern phStatus_t RegCtl_SpiHwSetReg(uint8_t address, uint8_t reg_data);
void SPI1_Init(void);

void RC663_TestCommunication(void);
uint8_t RC663_ReadVersion(void);

#endif  /* __SSP_H__ */
/*****************************************************************************
**                            End Of File
******************************************************************************/

/*******************************************************************************
* File Name    : RegCtl_SpiHw.h
* Author       : Adapted for STM32G070 + RC663 by ChatGPT
* Description  : Header file for software SPI communication with CLRC663
*******************************************************************************/
//#ifndef __REGCTL_SPIHW_H__
//#define __REGCTL_SPIHW_H__

//#ifdef __cplusplus
//extern "C" {
//#endif

//#include <ph_Status.h>

////==========================================================
//// ? 常量定义（可根据需要调整）
////==========================================================
//#define PH_ERR_BFL_SUCCESS           0x0000
//#define PH_ERR_BFL_ERROR             0x0001

//#define SPI_SCLK_GPIO_L()    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET)
//#define	SPI_SCLK_GPIO_H()    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET)
//#define SPI_CS_GPIO_L()      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET)
//#define	SPI_CS_GPIO_H()      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET)
//#define SPI_MOSI_GPIO_L()    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET)
//#define	SPI_MOSI_GPIO_H()    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET)
//#define SPI_MISO_READ()      HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)

////-------------------------------------------------------------
//// RC663 寄存器定义示例（常用）
////-------------------------------------------------------------
//#define RC663_REG_CommandReg         0x00
//#define RC663_REG_VersionReg         0x7F

////-------------------------------------------------------------
//// 外部函数声明
////-------------------------------------------------------------

///**
// * @brief 初始化软件 SPI 引脚
// */
//void SPI0_Init(void);

///**
// * @brief 初始化 RC663 SPI 接口（置初始电平）
// */
//void RC663_SPI_Init(void);

///**
// * @brief RC663 SPI 初始化入口
// */
//void RegCtl_SpiHwInit(void);

///**
// * @brief 向 RC663 写寄存器
// * @param address 寄存器地址
// * @param reg_data 要写入的值
// */
//phStatus_t RegCtl_SpiHwSetReg(uint8_t address, uint8_t reg_data);

///**
// * @brief 从 RC663 读寄存器
// * @param address 寄存器地址
// * @param reg_data 存放读取结果的指针
// */
//phStatus_t RegCtl_SpiHwGetReg(uint8_t address, uint8_t *reg_data);

///**
// * @brief RC663 通信测试函数
// *        读取 VersionReg 寄存器验证 SPI 通信是否正常
// */
//void RC663_TestCommunication(void);

////-------------------------------------------------------------
//// 内部底层函数（可选保留）
////-------------------------------------------------------------
//void _SSP0_SetReg(uint8_t *buf);
//void _SSP0_GetReg(uint8_t *buf);

//#ifdef __cplusplus
//}
//#endif

//#endif /* __REGCTL_SPIHW_H__ */

