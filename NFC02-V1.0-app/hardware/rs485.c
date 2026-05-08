#include <string.h>
#include <stdlib.h>
#include "RS485.h"
#include "usart.h"
#include "nfc.h"
#include "led.h"
#include "tim.h"
#include "ph_Status.h"
#include "delay.h"
#include "eeprom.h"

uint8_t RxIndex = 8;
uint8_t TxBuffer[256];
Modbus_t modbus;
uint8_t L2_RS485_Message;

static volatile uint16_t rxIndex = 0;

void L2_RS485_Init(void)
{  
	MX_USART1_UART_Init(9600);
	RS485_RX_ENABLE;
}

// CRC计算
uint16_t ModbusCRC(uint8_t *pdata, uint16_t len)
{
	uint16_t crc;
	uint16_t i,j;
	crc = 0xFFFF;
	
    for (i = 0; i < len; i++) {
        crc ^= pdata[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
	
    return crc;
}

// 处理Modbus请求
void L2_Process_Modbus(void)
{
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t block = 0;
	uint16_t crc = 0, rcrc = 0;
	uint8_t RS485_Write_Data[4] = {0};
	i = i; j = j; RS485_Write_Data[0] = RS485_Write_Data[0];
	
	if(modbus.reflag == 0)
		return ;
	
	if(modbus.recount < 2)
		return ;
	
	crc = ModbusCRC(modbus.rcbuf, modbus.recount-2);
	rcrc = modbus.rcbuf[modbus.recount-2]*256+modbus.rcbuf[modbus.recount-1];//计算读取的CRC校验位
	
	
	if(crc == rcrc) //CRC检验成功 开始分析包
 	{
//		//读取从站地址功能
		if(modbus.rcbuf[1] == 0x02)
		{
			USART1_SendByte(modbus.myadd);
		}
		
		if(modbus.rcbuf[0] == EEPROM_ReadByte(0xEC))  //检查地址是否时自己的地址
		{
			switch(modbus.rcbuf[1])   //分析modbus功能码
			{
				case 0x01:
				{
					L3_NFC_SetStatus(NFC_STATUS_ACTIVATE_LABEL);  //L3_NFC_APP回到标签检测，再次读取并发送ID
					modbus.readID = 1;
					break;
				}
				case 0x03:
				{
					uint16_t start_reg = (modbus.rcbuf[2] << 8) | modbus.rcbuf[3];
					uint16_t reg_count = (modbus.rcbuf[4] << 8) | modbus.rcbuf[5];
					start_reg = start_reg; reg_count = reg_count;
					block = modbus.rcbuf[3];	// 读写块 地址的取值范围 0 ~ 27
					uint16_t Status = 0; Status = Status;
					
					//该条件表达式 是否需要更改
					if (block >= BLOCK_COUNT_MAX)
					{
						USART1_SendByte(0xAA);
					}
					else 
					{
						if(RC663_MODE == RC663_14443)
						{
							Status = NTAG213_ReadPage(block, modbus.sendbuf);
						}
						else
						{
							Status = L2_NFC_ReadPara(block, modbus.sendbuf);
						}
						
						if(Status == PH_ERR_SUCCESS)
						{
							uint8_t data_modbus[20] = {modbus.myadd, 0x03, 0x00, modbus.rcbuf[3], modbus.sendbuf[0], modbus.sendbuf[1], modbus.sendbuf[2], modbus.sendbuf[3]};
							uint16_t dcrc = ModbusCRC(data_modbus, 8);
							data_modbus[8] = dcrc/256;
							data_modbus[9] = dcrc;
							USART1_SendBuff(data_modbus, 10);
						}
						else 
						{
							USART1_SendBuff(modbus.sendbuf,4);
						}
						
						memset(modbus.sendbuf, 0, sizeof(modbus.sendbuf));
					}

					break;
				}
				case 0x05:
					break;
				case 0x06:
				{
					block = modbus.rcbuf[3];	// 读写块 地址的取值范围 0x04 ~ 0x27
					
					if(RC663_MODE == RC663_14443)
					{
						if (block >= BLOCK_COUNT || block < 0x04)
						{
							USART1_SendByte(0xAA);
						}
						else 
						{
							uint8_t *datap = NULL;
							uint8_t Addr = 0;
							uint16_t Status = 0; Status = Status;
							
							datap =  &modbus.rcbuf[4];
							
							Addr = block;
							Status = L2_14443_WritePara_Block(Addr, datap);
							
							HAL_Delay(5);
							memset(datap, 0, 4);
							Status = NTAG213_ReadPage(Addr, datap);
							
							// 写标签 回应的数据
							USART1_SendBuff(modbus.rcbuf,10);
						}
					}
					else
					{
						if (block >= BLOCK_COUNT)
						{
							USART1_SendByte(0xAA);
						} 
						else
						{
							uint8_t *datap = NULL;
							uint16_t Status = 0; Status = Status;
							
							datap =  &modbus.rcbuf[4];
							L3_NFC_WRITE(block, datap);
							
							memset(datap, 0, 4);
							
							Status = L2_NFC_ReadPara(block, datap);
							
							// 写标签 回应的数据
							if(Status != PH_ERR_SUCCESS)
							{
								USART1_SendByte(0xAB);
							}
							else
							{
								USART1_SendBuff(modbus.rcbuf,10);
							}
							
						}
					}
					
					break;
				}
				
				case 0x10:
				{
					uint16_t start_reg = (modbus.rcbuf[2] << 8) | modbus.rcbuf[3];
					uint16_t reg_count = (modbus.rcbuf[4] << 8) | modbus.rcbuf[5];
					uint8_t block = 0;
					uint8_t Number_bytes = modbus.rcbuf[5];
					start_reg = start_reg; reg_count = reg_count;
					block = modbus.rcbuf[3];	// 读写块 地址的取值范围 0 ~ 27
					
					if(RC663_MODE == RC663_14443)
					{
						if (block >= BLOCK_COUNT || block < 0x04)
						{
							USART1_SendByte(0xAA);
						}
						else 
						{
							uint8_t *datap = &modbus.rcbuf[6];
							
							for(uint8_t i = 0; i<Number_bytes/4; i++) {
								L2_14443_WritePara_Block(block+i, datap+i*4);
								HAL_Delay(5);
							}
							
							for(uint8_t i = 0; i<Number_bytes/4; i++) {
								NTAG213_ReadPage(block+i, datap+i*4);
							}
							// 写标签 回应的数据
							USART1_SendBuff(modbus.rcbuf,8+modbus.rcbuf[5]);
							
						}
					}
					else
					{
						if (block >= BLOCK_COUNT)
						{
							USART1_SendByte(0xAA);
						}
						else 
						{
							uint8_t *datap = &modbus.rcbuf[6];
							for(uint8_t i = 0; i<Number_bytes/4; i++) {
								L3_NFC_WRITE(block+i, datap+i*4);
							}
							
							for(uint8_t i = 0; i<Number_bytes/4; i++) {
								L2_NFC_ReadPara(block+i, datap+i*4);
							}
							
							// 写标签 回应的数据
							USART1_SendBuff(modbus.rcbuf,8+modbus.rcbuf[5]);
						}
					}
					
					break;
				}
				
				case 0x36:
				{
					RC663_MODE = RC663_14443;
					EEPROM_WriteByte(0xA1, RC663_14443);
					
					L3_NFC_SetStatus(NFC_STATUS_IDLE);
					
					USART1_SendBuff(modbus.rcbuf, 8);
				}
					break;
				case 0x37:
				{
					RC663_MODE = RC663_15693;
					EEPROM_WriteByte(0xA1, RC663_15693);
					
					L3_NFC_SetStatus(NFC_STATUS_IDLE);
					
					USART1_SendBuff(modbus.rcbuf, 8);
				}
					break;
				case 0x38:			// 读取RFID模式
				{
					uint16_t DCRC = 0;
					
					if(modbus.rcbuf[0] == EEPROM_ReadByte(0xEC))		// 对比地址是否是 该设备的地址
					{
						for(int i =0; i<6; i++)
							modbus.sendbuf[i] = modbus.rcbuf[i];
						modbus.sendbuf[4] = RC663_MODE;
						
						DCRC = ModbusCRC(modbus.sendbuf, 6);
						modbus.sendbuf[6] = DCRC/256;
						modbus.sendbuf[7] = DCRC;
						
						USART1_SendBuff(modbus.sendbuf, 8);
					}
				}
					break;
				
				default :
					TxBuffer[0] = MODBUS_SLAVE_ADDR;
					TxBuffer[1] = modbus.rcbuf[1] | 0x80;
					TxBuffer[2] = 0x01;

					break;
			}
		}
	}
	
	USART1_R_BUFF_CLEAR();
	modbus.recount = 0;//接收计数清零
	modbus.reflag = 0; //接收标志清零
}

void Device_Addr_Init(uint8_t num)
{
	modbus.myadd = num; //设备地址为 num
}
