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
#include <ph_Status.h>
#include <RegCtl_SpiHw.h>
#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include "spi.h"
#include "delay.h"

#if CONFIG_ENABLE_DRIVER_SSP==1
/* statistics of all the interrupts */
volatile uint32_t interruptRxStat0 = 0;
volatile uint32_t interruptOverRunStat0 = 0;
volatile uint32_t interruptRxTimeoutStat0 = 0;

volatile uint32_t interruptRxStat1 = 0;
volatile uint32_t interruptOverRunStat1 = 0;
volatile uint32_t interruptRxTimeoutStat1 = 0;
#endif



//void SPI1_Init(void);
//void Reset_RC663_device(void);
void _SSP0_SetReg(uint8_t *buf);
void _SSP0_GetReg(uint8_t *buf);
phStatus_t RegCtl_SpiHwGetReg(uint8_t address, uint8_t *reg_data);
phStatus_t RegCtl_SpiHwSetReg(uint8_t address, uint8_t reg_data);
phStatus_t RegCtl_SpiHwModReg(uint8_t address, uint8_t mask, uint8_t set);
phStatus_t RegCtl_SpiHwSetMultiData(uint8_t *buf, uint32_t Len);
phStatus_t RegCtl_SpiHwGetMultiData(uint8_t *txbuf, uint8_t *rxbuf, uint32_t Len);

/*void SPI1_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	//RCC_APB2PeriphClockCmd(SPI_CS_GPIO_CLK, ENABLE);//PB口时钟打开
	//RCC_APB2PeriphClockCmd(SPI_MOSI_GPIO_CLK, ENABLE);
	//RCC_APB2PeriphClockCmd(SPI_SCLK_GPIO_CLK, ENABLE);
	//RCC_APB2PeriphClockCmd(SPI_MISO_GPIO_CLK, ENABLE);
	//RCC_APB2PeriphClockCmd(RF_RST_GPIO_CLK, ENABLE);
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);//PA,PB口时钟打开
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//PA,PB口时钟打开
	GPIO_InitStructure.GPIO_Pin = SPI_CS_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = SPI_CS_GPIO_MODE;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(SPI_CS_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = SPI_MOSI_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = SPI_MOSI_GPIO_MODE;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(SPI_MOSI_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = SPI_SCLK_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = SPI_SCLK_GPIO_MODE;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(SPI_SCLK_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = SPI_MISO_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = SPI_MISO_GPIO_MODE;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(SPI_MISO_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = RF_RST_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = RF_RST_GPIO_MODE;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(RF_RST_GPIO, &GPIO_InitStructure);

	SPI_MOSI_GPIO_H();
	SPI_SCLK_GPIO_H();
}
*/
//复位脚为PA0
/*void Reset_RC663_device(void)
{
	uint32_t volatile i;
	// RSET signal low 
	RF_RST_GPIO_L();

	// delay of ~1,2 ms 
	for (i = 0x2000; i > 0; i --);

	// RSET signal high to reset the RC663 IC 
	RF_RST_GPIO_H();

	// delay of ~1,2 ms 
	for (i = 0x2000; i > 0; i --);

	// RSET signal low 
	RF_RST_GPIO_L();

	// delay of ~1,2 ms 
	for (i = 0x2000; i > 0; i --);
}*/

/*********************************************************************/
/*
 * @brief 		Initialise the spi SSP0 for master mode
 * @param[in]	None
 *
 * @return 		None
 **********************************************************************/
void RegCtl_SpiHwInit(void)
{
	L2_SPI2_Init();			
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
	//SPIx_ReadWriteByte(address);
	//SPIx_ReadWriteByte(value);
void _SSP0_SetReg(uint8_t *buf)
{
	uint8_t address,value;
	address = *buf;	//地址
	value = *buf++;		//数据
	
	SPI2_WriteByte(address);
	SPI2_WriteByte(value);
	
	
	
	/*SPI_SCLK_GPIO_L();
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
   	SPI_SCLK_GPIO_H();*/
		
}

void _SSP0_GetReg(uint8_t *buf)
{
		 uint8_t ucAddr;
		// uint8_t ucResult=0;
		 
		
		 ucAddr = *(buf++);	//地址
	   SPI2_WriteByte(ucAddr);
	   *buf =SPI2_ReadByte();
	
   /*  SPI_SCLK_GPIO_L();
		 SPI_CS_GPIO_L();
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

			 if (GPIO_ReadInputDataBit(SPI_MISO_GPIO,SPI_MISO_GPIO_PIN))
			 {
				 ucResult|=1;
			 }
			 SPI_SCLK_GPIO_L();
     }
		 */
    //ucResult=SPIx_ReadWriteByte(ucAddr); 		 
	  
	  //SPI_CS_GPIO_H();
	  //SPI_SCLK_GPIO_H();
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
	//SPI_CS_GPIO_L();
  SPI_SSOutputCmd(SPI1,ENABLE);
	// receive 2 bytes: dummy byte + valid byte
	_SSP0_GetReg(buff);
  
	// Deassert the SSEL pin 
	//SPI_CS_GPIO_H();
  SPI_SSOutputCmd(SPI1,DISABLE);
	// 2nd valid databyte
	reg_data[1] = buff[1];

	// return success
	return PH_ERR_SUCCESS;
}
/*
phStatus_t RegCtl_SpiHwGetReg(uint8_t address, uint8_t *reg_data)
{
  uint8_t buff[2];
  buff[0] = address;
 	SPI_CS_GPIO_L();
	//_SSP0_GetReg(buff);
	buff[0]=SPIx_ReadWriteByte(buff[0]);
	//SPIx_ReadWriteByte(buff[1]);
	SPI_CS_GPIO_H();
	reg_data[1] = buff[0];
	return PH_ERR_SUCCESS;
}
*/
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
	//SPI_CS_GPIO_L();
	SPI_SSOutputCmd(SPI1,ENABLE);
	// send SPI frame
	_SSP0_SetReg(buff);

	// Deassert the SSEL pin
	//SPI_CS_GPIO_H();
	SPI_SSOutputCmd(SPI1,DISABLE);
	// return success
	return PH_ERR_SUCCESS;
}
/*
phStatus_t RegCtl_SpiHwSetReg(uint8_t address, uint8_t reg_data)
{
  uint8_t buff[2];
  buff[0] = address;
	buff[1] = reg_data;
	SPI_CS_GPIO_L();
	//_SSP0_SetReg(buff);
	SPIx_ReadWriteByte(buff[0]);
	SPI_CS_GPIO_H();
	return PH_ERR_SUCCESS;
}
*/


