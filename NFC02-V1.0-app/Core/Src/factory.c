#include "factory.h"
#include "RS485.h"
#include "usart.h"
#include "verify.h"
#include "led.h"
#include "nfc.h"
#include "usart.h"
#include "eeprom.h"

#include <string.h>
#include <stdlib.h>

#include "phbalReg.h"
#include "phhalHw.h"
#include "phpalSli15693.h"
#include "phalSli.h"
#include "phalI15693.h"
#include "phalSli.h"
#include "RegCtl_SpiHw.h"
#include "phkeyStore.h"
#include "phalI15693_Sw.h"
#include "delay.h"
#include "NFC.H"
#include "LED.h"
#include "RS485.h"
#include "usart.h"
#include "iwdg.h"
#include "tim.h"

uint8_t NFC_Factory_status = 0;

// 通过校验的 测试数据 帧头帧尾校验 + 校验和 双重校验 表示 握手成功
uint8_t Test_Verify_data[9] = {0xAA, 0x55, 0x12, 0x23, 0x34, 0x45, 0xAD, 0x55, 0xAA};

void Factory_App(void)
{
	LED_PWR_OFF();
	LED_COM_OFF();
	LED_MODE_OFF();
	
	modbus.factory_exit = 1;
	
	if(modbus.reflag == 1)
	{
		modbus.reflag = 0;
		
		if(verify_checksum(modbus.rcbuf, 7) && validate_packet_frame(modbus.rcbuf, 9))			// 校验通过表示握手成功
		{
			USART1_SendBuff(modbus.rcbuf, 9);
			
			USART1_R_BUFF_CLEAR();
			
			while(modbus.factory_exit)
				NFC_Factory_APP();
		}
		
	}
	
	
}

void NFC_Factory_APP(void)
{
	phStatus_t status = 65535;
	
	// NFC 状态机主循环，根据当前状态执行对应操作
	switch(NFC_Factory_status)
	{
		// 当NFC状态为空闲时，初始化NFC设备并设置状态为初始化
		case 0x00:					// 空闲状态
			L2_NFC_Device_IDLE();				// 执行 NFC 设备空闲态操作（如低功耗模式）
			Tag_Exisit = 0;
			
			NFC_Factory_status++;	// 状态跳转至初始化
			break;
		
		case 0x01:					// 初始化状态		
			// 根据当前 NFC 模式（15693 或 14443）调用对应初始化函数
			if(RC663_MODE == RC663_15693)
				status = L2_15693_Init();			// ISO 15693 模式初始化
			else if(RC663_MODE == RC663_14443)
				status = L2_14443_Init();			// ISO 14443 模式初始化
			IWDG_FeedDog();
			
			Tag_Exisit = 0;
			
			// 初始化成功则跳转至“开启场”状态，失败则跳转至错误状态
			if(status == PH_ERR_SUCCESS)
				NFC_Factory_status++;
			else
				NFC_Factory_status = 0x50;
			break;
			
		// 当NFC状态为开启场时，根据RC663模式开启NFC场
		case 0x02:				// 开启射频场状态
			status = 65535;
		
			if(RC663_MODE == RC663_15693)
				status = L2_15693_OpenField();	// 开启 15693 射频场
			else if(RC663_MODE == RC663_14443)
				status = L2_14443_OpenField();	// 开启 14443 射频场
			
			IWDG_FeedDog();
			
			if(status == PH_ERR_SUCCESS)
				NFC_Factory_status++;	// 开启成功
			else
			{
				NFC_Factory_status = 0x50;			// 失败则跳转错误状态
			}
			break;
		
		// 当NFC状态为配置标签时，根据RC663模式配置标签
		case 0x03:			// 配置标签参数状态
			status = 65535;
			// 根据模式配置标签参数（如通信速率、协议选项）
			if(RC663_MODE == RC663_15693)
				status = L2_15693_ConfigLable();	// 配置 15693 标签
			else if(RC663_MODE == RC663_14443)
				status = L2_14443_ConfigLable();// 配置 14443 标签

			if(status == PH_ERR_SUCCESS)		// 配置成功
				NFC_Factory_status++;	// 跳转至激活标签
			else
			{
				NFC_Factory_status = 0x50;		// 失败则跳转错误状态
			}
			break;
		
		// 当NFC状态为激活标签时，根据RC663模式激活标签
		case 0x04:			// 激活标签状态
			status = 65535;
			// 尝试激活标签（建立通信）
			if(RC663_MODE == RC663_15693)
				status = L2_15693_ActivateLable();// 激活 15693 标签
			else if(RC663_MODE == RC663_14443)
				status = L2_14443_ActivateLable();// 激活 14443 标签
			
			IWDG_FeedDog();

			// 检查激活是否成功，并根据结果设置下一步状态
			if(status == PH_ERR_SUCCESS)		// 激活成功
			{
				NFC_Factory_status++;											// 跳转至获取标签ID
				
				Tag_Exisit = 1;
			}
			else
			{
				NFC_Factory_status = 0x50;										// 失败则跳转错误状态
				
				Tag_Exisit = 0;
			}
			break;
		
		 // 当NFC状态为获取标签ID时，等待一段时间后获取标签ID
		case 0x05:			// 获取标签UID状态
			status = 65535;
			// 等待10ms后读取标签UID
			memset(pUid, 0, sizeof(pUid));
			memset(pUidOut, 0, sizeof(pUidOut));
			
			HAL_Delay(10);
			
			if(RC663_MODE == RC663_15693)
				status = L2_15693_GetLableID();	// 读取 15693 标签UID
			else if(RC663_MODE == RC663_14443)
				status = L2_14443_GetLableID();	// 读取 14443 标签UID
			
			// 检查获取ID是否成功，并根据结果设置下一步状态
			if(status == PH_ERR_SUCCESS)
				NFC_Factory_status++;		// 跳转至新标签处理
			else
				NFC_Factory_status = 0x50;	// 失败则跳转错误
			break;
			
		// 当NFC状态为新标签时，保存标签UID并发送消息
		case 0x06:				// 新标签处理状态
			status = 65535;
			
			IWDG_FeedDog();
			
			// 保存标签UID到缓冲区（根据模式区分UID长度）
			if(MODE_IOLINK_or_RS485 == SELECT_IOLINK) {
				
			} else {
				//新增
				if(modbus.readID == 1)
				{
					modbus.readID = 0;
					
					if(RC663_MODE == RC663_14443)
					{
						uint8_t Test_data[11] = {modbus.myadd, 0x01, pUidOut[0], pUidOut[1], pUidOut[2], pUidOut[3], pUidOut[4], pUidOut[5], pUidOut[6]};
						
						uint16_t Test_Crc = ModbusCRC(Test_data, 9);
						
						Test_data[9] = Test_Crc/256;
						Test_data[10] = Test_Crc;
						
						USART1_SendBuff(Test_data, sizeof(Test_data));
					}
					else
					{
						uint8_t Test_data[12] = {modbus.myadd, 0x01, pUid[0], pUid[1], pUid[2], pUid[3], pUid[4], pUid[5], pUid[6], pUid[7]};
						
						uint16_t Test_Crc = ModbusCRC(Test_data, 10);
						Test_data[10] = Test_Crc/256;
						Test_data[11] = Test_Crc;
						
						USART1_SendBuff(Test_data, sizeof(Test_data));
					}
				}
				
			}
			
			NFC_Factory_status ++;
			break;
			
		// 当NFC状态为等待逗号指令时，定期检查标签状态
		case 0x07:				// 等待外部指令状态
			status = 65535;
			//此处可以加入一些延时来调整负载率
			HAL_Delay(10);
			// 持续轮询检测标签是否在位
			if(RC663_MODE == RC663_14443)
			{
				uint8_t Testdata[10] = {0};
				status = PH_ERR_SUCCESS;
				
				// 持续轮询检测标签是否在位	// 二次检测 14443A标签是否存在
				status = NTAG213_ReadPage(0x01, Testdata);
			}
			else
			{
				status = L2_15693_ActivateLable();					// 二次检测 15693标签是否存在
			}

			// 根据激活结果设置下一步状态
			if(status != PH_ERR_SUCCESS)	// 标签丢失或通信失败
				NFC_Factory_status = 0x50;
			else
			{
				IWDG_FeedDog();
				
				Factory_Process_Modbus();					// 处理Modbus指令（如读写标签数据）
			}
			break;
			
		// 当NFC状态为标签错误时，设置诊断错误并重置NFC设备
		case 0x50:			// 标签错误处理状态
			L2_NFC_DeviceReset();				// 复位NFC硬件模块
			NFC_Factory_status = 0x00;
			break;
		
		// 当NFC状态为默认时，设置诊断错误并重置NFC设备
		default:								// 未知状态处理
			L2_NFC_DeviceReset();				// 复位NFC硬件
			NFC_Factory_status = 0x00;
			break;
	}
	
	return ;
}

void Factory_Process_Modbus(void)
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
		if(Timer14_flag == 1)
			Timer14Cout[0] = Timer14_Bt;			// 有数据时需要更新 计数基数
		
		if(modbus.rcbuf[0] == EEPROM_ReadByte(0xEC))  //检查地址是否时自己的地址
		{
			switch(modbus.rcbuf[1])   //分析modbus功能码
			{
				case 0x01:
				{
					NFC_Factory_status = 0x04;  //L3_NFC_APP回到标签检测，再次读取并发送ID
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
					
					if (block >= BLOCK_COUNT)
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
				case 0x06:
				{
					block = modbus.rcbuf[3];	// 读写块 地址的取值范围  14443A 的范围 是0x04 ~ 0x27； 15693 的范围 是 0x00 ~ 0x20；
					
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
				
				case 0x07:
				{
					LED_PWR_ON();
					LED_COM_ON();
					LED_MODE_ON();
					
					USART1_SendBuff(modbus.rcbuf,8);
					break;
				}
					
				
				case 0x08:
				{
					LED_PWR_OFF();
					LED_COM_OFF();
					LED_MODE_OFF();
					
					USART1_SendBuff(modbus.rcbuf,8);
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
				
				case 0x11:			// 后续用来实现 查询 设备型号  eeprom 地址中的第20-40地址表示 设备型号 的存放地址
				{
					uint8_t value = 0; value = value;
					uint8_t SN_Data_Write[8] = {0};
					uint8_t SN_Data_Read[12] = {0};
					uint16_t Crc = 0;
					
					// modbus.rcbuf 中存储的是 对应的 SN 码  2025 w 35
					SN_Data_Write[0] = 0x11; SN_Data_Write[1] = 0xE9; SN_Data_Write[2] = 0x35; SN_Data_Write[3] = 110;
					
					for(i = 20; i<28; i++)
					{
						EEPROM_WriteByte(i, SN_Data_Write[i-20]);
					}
					
					SN_Data_Read[0] = modbus.rcbuf[0]; 		 SN_Data_Read[1] = modbus.rcbuf[1];
					for(i = 20; i<28; i++)
					{
						SN_Data_Read[i-18] = EEPROM_ReadByte(i);
					}
					Crc = ModbusCRC(SN_Data_Read, 10);
					SN_Data_Read[10] = Crc / 256; SN_Data_Read[11] = Crc;
					
					USART1_SendBuff(SN_Data_Read, 12);
				}
					
					break;
				
				case 0x12:			// 后续用来实现 查询 SN存放指令  eeprom 地址中的前20个表示 SN 的存放地址
				{
					uint8_t value = 0; value = value;
					uint8_t i = 0;
					uint8_t SN_Data_Read[30] = {0};
					uint16_t Crc = 0;
					
					SN_Data_Read[0] = modbus.rcbuf[0]; 		 SN_Data_Read[1] = modbus.rcbuf[1];
					for(i = 0; i<16 ; i++)
					{
						SN_Data_Read[i+2] = EEPROM_ReadByte(i);
					}
					
					Crc = ModbusCRC(SN_Data_Read, 18);
					SN_Data_Read[18] = Crc / 256; SN_Data_Read[19] = Crc;
					
					USART1_SendBuff(SN_Data_Read, 20);
				}
					break;
				
				case 0x13:			// 后续用来实现 查询 软件版本号 eeprom 地址中的第40-60地址表示 软件版本号 的存放地址
				{
					uint8_t value = 0; value = value;
					uint8_t SN_Data_Write[8] = {0};
					uint8_t SN_Data_Read[12] = {0};
					uint16_t Crc = 0;
					
					SN_Data_Write[0] = 0x07; SN_Data_Write[1] = 0xE9; SN_Data_Write[2] = 0x35; SN_Data_Write[3] = 110;
					for(i = 40; i<48; i++)
					{
						EEPROM_WriteByte(i, SN_Data_Write[i-40]);
					}
					
					SN_Data_Read[0] = modbus.rcbuf[0]; 		 SN_Data_Read[1] = modbus.rcbuf[1];
					for(i = 40; i<48; i++)
					{
						SN_Data_Read[i-38] = EEPROM_ReadByte(i);
					}
					
					Crc = ModbusCRC(SN_Data_Read, 10);
					SN_Data_Read[10] = Crc / 256; SN_Data_Read[11] = Crc;
					
					USART1_SendBuff(SN_Data_Read, 12);
				}
					
					break;
				
				case 0x14:			// 后续用来实现 写入 硬件版本号 eeprom 地址中的第60-80地址表示 硬件版本号 的存放地址
				{
					uint8_t value = 0; value = value;
					uint8_t SN_Data_Write[8] = {0};
					uint8_t SN_Data_Read[12] = {0};
					uint16_t Crc = 0;
					
					SN_Data_Write[0] = 0x14; SN_Data_Write[1] = 0xE9; SN_Data_Write[2] = 0x35; SN_Data_Write[3] = 110;
					for(i = 60; i<68; i++)
					{
						EEPROM_WriteByte(i, SN_Data_Write[i-60]);
					}
					
					SN_Data_Read[0] = modbus.rcbuf[0]; 		 SN_Data_Read[1] = modbus.rcbuf[1];
					for(i = 60; i<68; i++)
					{
						SN_Data_Read[i-58] = EEPROM_ReadByte(i);
					}
					
					Crc = ModbusCRC(SN_Data_Read, 10);
					SN_Data_Read[10] = Crc / 256; SN_Data_Read[11] = Crc;
					
					USART1_SendBuff(SN_Data_Read, 12);
				}
					
					break;
				case 0x15:				// 写入 SN号 功能码
				{
					uint8_t value = 0; value = value;
					uint8_t i = 0;
					uint8_t SN_Data_Read[30] = {0};
					uint16_t Crc = 0;
					
					// modbus.rcbuf 中存储的是 对应的 SN 码
					for(i = 0; i<16; i++)
					{
						EEPROM_WriteByte(i, modbus.rcbuf[2+i]);
					}
					
					SN_Data_Read[0] = modbus.rcbuf[0]; 		 SN_Data_Read[1] = modbus.rcbuf[1];
					for(i = 0; i<16 ; i++)
					{
						SN_Data_Read[i+2] = EEPROM_ReadByte(i);
					}
					
					Crc = ModbusCRC(SN_Data_Read, 18);
					SN_Data_Read[18] = Crc / 256; SN_Data_Read[19] = Crc;
					
					USART1_SendBuff(SN_Data_Read, 20);
					
				}
					break;
				
				case 0x20:			// 退出工厂模式
						modbus.factory_exit = 0;
						Timer1_Bt = 200;
						Tag_Exisit = 0;
						LED_PWR_ON();
						
					break;
				case 0x36:			// 修改RFID模式
				{
					if(modbus.rcbuf[0] == EEPROM_ReadByte(0xEC))		// 对比地址是否是 该设备的地址
					{
						RC663_MODE = RC663_14443;
						
						EEPROM_WriteByte(0xA1, RC663_14443);
						
						Timer14_flag = 1;								// 修改Tim14标志位 从而开启超时机制
						
						L3_NFC_SetStatus(NFC_STATUS_IDLE);
						
						USART1_SendBuff(modbus.rcbuf, 8);
					}
				}
					break;
				case 0x37:			// 修改RFID模式
				{
					if(modbus.rcbuf[0] == EEPROM_ReadByte(0xEC))		// 对比地址是否是 该设备的地址
					{
						RC663_MODE = RC663_15693;
						
						EEPROM_WriteByte(0xA1, RC663_15693);
						
						L3_NFC_SetStatus(NFC_STATUS_IDLE);
						
						USART1_SendBuff(modbus.rcbuf, 8);
					}
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
					
					break;
			}
		}
	}
	
	USART1_R_BUFF_CLEAR();
	modbus.recount = 0;//接收计数清零
	modbus.reflag = 0; //接收标志清零
}
