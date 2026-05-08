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
* Filename:          RegCtl_SpiHw.c
* Processor family:  LPC11xx
*
* Description:       This file holds the functions for the SPI communication.
*                    In case of the Silica TUSA board, the SPI communication
*                    gets emulated, because it uses GPIO pins that are not
*                    designated for use of SPI communication.
*******************************************************************************/
#include "ph_Status.h"
#include "RegCtl_SpiHw.h"
#include "main.h"

void SPI0_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 1. 使能 GPIOB 时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // 2. 初始化 NSS (CS) - PB12 作为普通推挽输出
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;      // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;              // 无上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;    // 高速
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // 3. 初始化 SCK 和 MOSI - PB13, PB11 设置为普通推挽输出
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;      // 普通推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // 4. 初始化 MISO - PB14 设置为上拉输入
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;          // 普通输入模式
    GPIO_InitStruct.Pull = GPIO_PULLUP;              // 上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // 5. 初始状态设置
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);  	// CS高电平
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);	// SCK低电平
}

void RC663_SPI_Init(void)
{
	SPI_CS_GPIO_H();
	SPI_SCLK_GPIO_L();
	SPI_MOSI_GPIO_L();
}

/*********************************************************************/
/*
 * @brief 		Initialise the spi SSP0 for master mode
 * @param[in]	None
 *
 * @return 		None
 **********************************************************************/
void RegCtl_SpiHwInit(void)
{
	SPI0_Init();			/* Select the SSP0 for SPI interface */
	RC663_SPI_Init();
}
/*****************************************************************************
** Function name:		_SSP0_SetReg
**
** Descriptions:		Send 2 databytes [RegNr, RegVal] to the SSP0 port
**
** parameters:			buffer pointer
** Returned value:		None
**
*****************************************************************************/
void _SSP0_SetReg(uint8_t *buf)
{
	uint8_t address,value,i;
	address = *buf++;	//地址
	value = *buf;		//数据
	SPI_SCLK_GPIO_L();
	SPI_CS_GPIO_L();
	for(i=8;i>0;i--)
    {
		if(address&0x80)
		{
		  SPI_MOSI_GPIO_H();
		}
		else
		{
		  SPI_MOSI_GPIO_L();
		}
	    SPI_SCLK_GPIO_H();
        address <<= 1;
		SPI_SCLK_GPIO_L();
    }
	  for(i=8;i>0;i--)
    {
		if(value&0x80)
		{
		  SPI_MOSI_GPIO_H();
		}
		else
		{
		  SPI_MOSI_GPIO_L();
		}
	    SPI_SCLK_GPIO_H();
        value <<= 1;
	    SPI_SCLK_GPIO_L();
    }
		
   	SPI_CS_GPIO_H();
   	SPI_SCLK_GPIO_H();
}

void _SSP0_GetReg(uint8_t *buf)
{
	uint8_t i, ucAddr;
	uint8_t ucResult=0;

	SPI_SCLK_GPIO_L();
	SPI_CS_GPIO_L();
	ucAddr = *buf++;	//地址
    
  	for(i=8;i>0;i--)
	{
		if(ucAddr&0x80)
		{
			SPI_MOSI_GPIO_H();
		}
		else
		{
			SPI_MOSI_GPIO_L();
		}
		SPI_SCLK_GPIO_H();
		ucAddr <<= 1;
		SPI_SCLK_GPIO_L();
	}
	for(i=8;i>0;i--)  //接收数据
	{
		SPI_SCLK_GPIO_H();
		ucResult <<= 1;

		if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14))
		{
			ucResult|=1;
		}
		SPI_SCLK_GPIO_L();
	}
		 
    //ucResult=SPIx_ReadWriteByte(ucAddr); 		 
	*buf = ucResult;

	SPI_CS_GPIO_H();
	SPI_SCLK_GPIO_H();
}
/*********************************************************************/
/*
 * @brief 		Get a databyte from a register
 * @param[in]	address		Address of the register
 * @param[in]	*reg_data	Pointer to the databyte to be read
 *
 * @return 		- PH_ERR_BFL_SUCCESS
 *
 **********************************************************************/
phStatus_t RegCtl_SpiHwGetReg(uint8_t address, uint8_t *reg_data)
{
	uint8_t buff[2];

	// load address in tx_buff 
	buff[0] = address;

	// Assert the SSEL pin 
	SPI_CS_GPIO_L();

	// receive 2 bytes: dummy byte + valid byte
	_SSP0_GetReg(buff);

	// Deassert the SSEL pin 
	SPI_CS_GPIO_H();

	// 2nd valid databyte
	reg_data[1] = buff[1];

	// return success
	return PH_ERR_SUCCESS;
}


/*********************************************************************//**
 * @brief 		Set a databyte to a register
 * @param[in]	address		Address of the register
 * @param[in]	*reg_data	Databyte to be written
 *
 * @return 		- PH_ERR_BFL_SUCCESS
 *
 **********************************************************************/
phStatus_t RegCtl_SpiHwSetReg(uint8_t address, uint8_t reg_data)
{
	uint8_t buff[2];

	// load address and data in buffer
	buff[0] = address;
	buff[1] = reg_data;

	// Assert the SSEL pin
	SPI_CS_GPIO_L();

	// send SPI frame
	_SSP0_SetReg(buff);

	// Deassert the SSEL pin
	SPI_CS_GPIO_H();

	// return success
	return PH_ERR_SUCCESS;
}

void RC663_TestCommunication(void)
{
    uint8_t version = 0;
    uint8_t addr = 0x7F; // VersionReg

    RegCtl_SpiHwGetReg(addr, &version);
	
    version = version;
}

