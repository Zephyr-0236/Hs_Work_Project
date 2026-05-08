#include "nfc.h"
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
#include "tim.h"
#include "LED.h"
#include "phpali14443p4.h"
#include "RS485.h"
#include "phpalI14443p4a.h"
#include "phpalI14443p3a.h"
#include "usart.h"
#include "iwdg.h"
#include "RegCtl_SpiHw.h"
#include "ph_Status.h"
#include "eeprom.h"

//////////////////
// 标签是否存在的 标志位
uint8_t Tag_Exisit = 0;

void *pHal;
//u8  u8_NFC_Write_Data[4];//4字节写入数据
uint8_t  L2_NFC_LableUID[PHPAL_SLI15693_UID_LENGTH];
phhalHw_Rc663_DataParams_t      halReader;
phpalSli15693_Sw_DataParams_t   palSli;
phalI15693_Sw_DataParams_t      al15693;

uint8_t L3_NFC_Status;
uint8_t NFC_Read_Data[4];      //4字节块读取数据

//两种标签情况选择
//uint8_t RC663_MODE = RC663_15693;
uint8_t RC663_MODE = RC663_14443;

//通用变量
uint8_t bIndex;

//uint8_t bHalBufferReader[256];
uint8_t bHalBufferReader[BUFFER_SIZE];
uint8_t bHalBufferWrite[BUFFER_SIZE];

//PAL层使用变量
uint8_t  bOption, bFlags, bAfi;
uint8_t  pMask[PHPAL_SLI15693_UID_LENGTH], bMaskLength;
uint8_t  bDsfid, pUid[PHPAL_SLI15693_UID_LENGTH];
uint8_t  bRxLength, bMoreCardsAvaliable;
uint8_t  bBlockNo;
uint16_t wNoOfBlocks;
uint8_t  bUidLength;
//uint8_t  pData[PHAL_SLI_BLOCK_SIZE * 28];
uint8_t  pData[PHAL_SLI_BLOCK_SIZE * 1];
uint16_t wDataLength=12;

phbalReg_Stub_DataParams_t balReader;
phKeyStore_Rc663_DataParams_t Rc663keyStore;

//AL层使用的变量
uint8_t pOriginalData[PHAL_SLI_BLOCK_SIZE];
uint8_t ** ppData = (uint8_t**)&pData;


/*14443协议变量*/
uint8_t L2_14443_LableUID[10]; // UID长度可能为4、7、10字节
phpalI14443p4_Sw_DataParams_t   dataParams; 
phpalI14443p3a_Sw_DataParams_t  dataParamsP3a;
phpalI14443p4a_Sw_DataParams_t  dataParamsP4a;

//uint8_t L3_NFC_Status;
uint8_t NFC_Read_Data[4];
uint8_t pUidOut[20];

// 通用变量
uint8_t bIndex;

// PAL层变量
uint8_t pAts[255];
uint8_t bFsdi = 8;      // FSDI值（PCD帧大小）
uint8_t bCid = 0;        // 卡标识符
uint8_t bDri = 0;        // 接收分频
uint8_t bDsi = 0;        // 发送分频

// 协议参数
uint8_t bSak;
uint8_t bLenUidOut;

/*14443协议变量*/


uint8_t L2_NFC_ReadLableIDbuff(uint8_t IDnum)
{
	if( IDnum < PHPAL_SLI15693_UID_LENGTH)
	{
	    return L2_NFC_LableUID[IDnum];
	}
	return 0xFF;//溢出，返回错误
}

void dataParams_14443_Init(void)
{
	dataParams.wId = 0x0100;              // Layer ID 未定  
	dataParams.pHalDataParams = NULL;     // 不需要底层参数，设置为NULL  
	dataParams.bStateNow = 0;             // 初始状态  
	dataParams.bCidEnabled = 0;           // 初始时不启用CID  
	dataParams.bCid = 0;                  // 初始CID值  
	dataParams.bNadEnabled = 0;           // 初始时不启用NAD  
	dataParams.bNad = 0;                  // 初始NAD值  
	dataParams.bFwi = 10;                 // 设置FWI值  
	dataParams.bFsdi = 8;                 // 设置PCD帧大小  
	dataParams.bFsci = 8;                	// 设置PICC帧大小  
	dataParams.bPcbBlockNum = 0;          // 初始块号  
	dataParams.bMaxRetryCount = 3;        // 设置最大重试次数
}

uint16_t L2_15693_Init(void)
{
	phStatus_t status;
	//NFC PDOWN PA15
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	__HAL_RCC_GPIOA_CLK_ENABLE();  // 使能 GPIOA 时钟

	GPIO_InitStruct.Pin = GPIO_PIN_15;        // PA15
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // 推挽输出
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速 (50MHz)
	GPIO_InitStruct.Pull = GPIO_NOPULL;      // 无上下拉

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  // 初始化 GPIO
	
	RegCtl_SpiHwInit();
	L2_NFC_DeviceReset();//复位NFC驱动
	
	//初始化BAL层
	status = phbalReg_Stub_Init(&balReader, sizeof(phbalReg_Stub_DataParams_t));
	CHECK_SUCCESS(status);// 不成功返回错误指示，PH_ERR_SUCCESS=操作成功 
	
	HAL_Delay(2);//原40
	
	//初始化663硬件抽象层HAL
	status = phhalHw_Rc663_Init(&halReader, sizeof(phhalHw_Rc663_DataParams_t), &balReader, NULL,\
		bHalBufferReader, sizeof(bHalBufferReader), bHalBufferWrite, sizeof(bHalBufferWrite));
				
	CHECK_SUCCESS(status);// 不成功返回错误指示，PH_ERR_SUCCESS=操作成功 
	
	//设置为使用SPI接口
	halReader.bBalConnectionType = PHHAL_HW_BAL_CONNECTION_SPI;
	pHal = &halReader;
	
	//初始化协议抽象层
	//使用Sli15693协议
	status = phpalSli15693_Sw_Init(&palSli, sizeof(palSli), &halReader);
	CHECK_SUCCESS(status);// 不成功返回错误指示，PH_ERR_SUCCESS=操作成功 
	
	//初始化“应用层”AL:
	//使用ISO15693的应用
	status = phalI15693_Sw_Init(&al15693, sizeof(al15693), &palSli);
	
	CHECK_SUCCESS(status);// 不成功返回错误指示，PH_ERR_SUCCESS=操作成功 

	
	return PH_ERR_SUCCESS;
}

// 暂时不确定 对应的重启引脚是 那个
/**
 * @brief 对 NFC 设备进行复位操作（通过控制 GPIOA15 引脚）
 * 
 * 此函数通过拉高再拉低控制引脚（通常连接到 NFC 模块的 RST 引脚）
 * 实现 NFC 芯片的硬件复位。复位脉冲宽度为 10 微秒。
 */
void L2_NFC_DeviceReset(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);		// 设置引脚为高电平
    delay_us(10);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);		// 设置引脚为低电平
}

/**
 * @brief 设置 NFC 模块进入 IDLE 状态（关闭天线控制 GPIO）
 */
void L2_NFC_Device_IDLE(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
}


/**
 * @brief 打开 NFC 天线场，准备进行标签激活
 * 
 * @return phStatus_t 状态码，PH_ERR_SUCCESS 表示成功
 */
uint16_t L2_15693_OpenField(void)
{
	phStatus_t status;
	
	//激活卡的参数设置
	// 设置激活参数	
	bOption = PHPAL_SLI15693_ACTIVATE_ADDRESSED;
	bFlags =  PHPAL_SLI15693_FLAG_NBSLOTS | PHPAL_SLI15693_FLAG_DATA_RATE;
	bAfi = 0xFF; //设置为0xff，支持所有应用
	bMaskLength = 0;
	
	// 打开射频场
//	status = phhalHw_FieldReset(&halReader);     	//打开场
	status = phhalHw_FieldOn(&halReader);           //打开场
	CHECK_SUCCESS(status);//这个地方有时候返回值不对
	
	return status;
}	

/**
 * @brief 配置标签协议为 ISO15693
 * 
 * @return phStatus_t 状态码，PH_ERR_SUCCESS 表示成功
 */
uint16_t L2_15693_ConfigLable(void)
{
	phStatus_t status;
	
	//配置HAL支持Sli15693协议的标签
	// 设置协议为 ISO15693
	status = phhalHw_ApplyProtocolSettings(&halReader, PHHAL_HW_CARDTYPE_ISO15693);
	CHECK_SUCCESS(status);
	
	return status;
}	

/**
 * @brief 激活 ISO15693 标签
 * 
 * @return phStatus_t 状态码，PH_ERR_SUCCESS 表示成功
 */
uint16_t L2_15693_ActivateLable(void)
{
	phStatus_t status = 65535;
	
	// 激活卡片
	status = phpalSli15693_ActivateCard(&palSli, bOption, bFlags,bAfi, pMask, \
										bMaskLength, &bDsfid, pUid, &bMoreCardsAvaliable);
	
	CHECK_SUCCESS(status);
	
	return status;
}


/**
 * @brief 获取标签 UID（序列号）
 * 
 * @return phStatus_t 状态码，PH_ERR_SUCCESS 表示成功
 */
uint16_t L2_15693_GetLableID(void)
{
	phStatus_t status = 65535;
	
	// 读取标签序列号
	status = phpalSli15693_GetSerialNo(&palSli, pUid, &bRxLength);
	CHECK_SUCCESS(status);
	
	return status;
}

/**
 * @brief 向指定块号写入参数数据（4 字节）
 * 
 * @param bBlockNo 要写入的块编号
 * @param pTxBuffer 指向写入数据（4字节）的指针
 * 
 * @return phStatus_t 状态码，PH_ERR_SUCCESS 表示成功
 */
uint16_t L2_15693_WritePara(uint8_t bBlockNo, uint8_t * pTxBuffer)
{
	phStatus_t status;
	if (pTxBuffer == NULL)  
	{
		return PH_ERR_INVALID_PARAMETER;
	}
	
	// 写单块数据
	status = phalI15693_WriteSingleBlock(&al15693, PHAL_I15693_OPTION_ON, bBlockNo, pTxBuffer, 4);
	
	CHECK_SUCCESS(status);
	return status;
}

/**
 * @brief 从指定块号读取参数数据（4 字节）
 * 
 * @param bBlockNo 要读取的块编号
 * @param pData 读取到的数据输出指针（接收 4 字节）
 * 
 * @return uint8_t 读取状态码，PH_ERR_SUCCESS 表示成功，错误码参考 NXP HAL
 */
uint8_t L2_NFC_ReadPara(uint8_t bBlockNo, uint8_t *pData)
{	
	int i;
	static uint8_t rxBuffer[4];
	static uint8_t  *pRxBuffer= rxBuffer; 
	uint8_t **ppRxBuffer = &pRxBuffer;
	uint16_t rxLength = sizeof(rxBuffer)-1;
	
	phStatus_t status;
	
	status = phalI15693_ReadSingleBlock(&al15693, PHAL_I15693_OPTION_ON, bBlockNo, ppRxBuffer, &rxLength);
	
	CHECK_SUCCESS(status);
	if (pData != NULL)  
	{
		for(i=0; i<4; i++)
		{
			pData[i] = pRxBuffer[i + 1];
		}
	}  
	else  
	{
		return  PH_ERR_INVALID_PARAMETER;
	}
	
	return status;
}

/* 14443协议 */
uint16_t L2_14443_Init(void)
{
    phStatus_t status;
	
	//NFC PDOWN PA15
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE();  // 使能 GPIOA 时钟

	GPIO_InitStruct.Pin = GPIO_PIN_15;        // PA15
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // 推挽输出
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速 (50MHz)
	GPIO_InitStruct.Pull = GPIO_NOPULL;      // 无上下拉

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  // 初始化 GPIO

    RegCtl_SpiHwInit();
    L2_NFC_DeviceReset(); // 复位NFC驱动

    // 初始化BAL层
    status = phbalReg_Stub_Init(&balReader, sizeof(phbalReg_Stub_DataParams_t));
    CHECK_SUCCESS(status);

    HAL_Delay(2); // 原40

    // 初始化663硬件抽象层HAL
    status = phhalHw_Rc663_Init(
        &halReader,
        sizeof(phhalHw_Rc663_DataParams_t),
        &balReader,
        NULL,
        bHalBufferReader,
        sizeof(bHalBufferReader),
        bHalBufferWrite,
        sizeof(bHalBufferWrite));

    CHECK_SUCCESS(status);
    
    // 设置为使用SPI接口
    halReader.bBalConnectionType = PHHAL_HW_BAL_CONNECTION_SPI;
    pHal = &halReader;

    // 初始化14443协议
    status = phpalI14443p3a_Sw_Init(&dataParamsP3a, sizeof(dataParamsP3a), &halReader);
    CHECK_SUCCESS(status);

    return PH_ERR_SUCCESS;
}

/**
 * @brief 打开 ISO14443 天线场
 * @retval 操作状态，PH_ERR_SUCCESS 表示成功
 */
uint16_t L2_14443_OpenField(void)
{
    phStatus_t status;

    // 打开场
    status = phhalHw_FieldOn(&halReader);
    CHECK_SUCCESS(status);

    return status;
}

/**
 * @brief 配置 ISO14443 协议参数和初始化标签通信协议
 * @retval 操作状态
 */
uint16_t L2_14443_ConfigLable(void)
{
    phStatus_t status;

    // 配置HAL支持14443协议的标签
	// 配置 HAL 层协议支持为 ISO14443A
    status = phhalHw_ApplyProtocolSettings(&halReader, PHHAL_HW_CARDTYPE_ISO14443A);
    CHECK_SUCCESS(status);
	
    return status;
}

/**
 * @brief 激活 ISO14443A 类型标签（卡）
 * @retval 操作状态
 */
uint16_t L2_14443_ActivateLable(void)
{
    phStatus_t status;
    uint8_t lenUidOut;
    uint8_t sak;
	uint8_t bMoreCardsAvailable = 0;

    // 激活卡片
    status = phpalI14443p3a_ActivateCard(
        &dataParamsP3a,
        NULL,                // pUidIn
        0,                   // bLenUidIn
        pUidOut,             // pUidOut
        &lenUidOut,          // pLenUidOut
        &sak,                // pSak
        &bMoreCardsAvailable // pMoreCardsAvailable
    );
	
    CHECK_SUCCESS(status);

    return status;
}

/**
 * @brief 获取 ISO14443A 卡片 UID
 * @retval 操作状态
 */
uint16_t L2_14443_GetLableID(void)
{
    phStatus_t status;
    uint8_t lenUidOut;

    // 获取卡片UID
    status = phpalI14443p3a_GetSerialNo(
        &dataParamsP3a,
        pUidOut,
        &lenUidOut
    );
	
    CHECK_SUCCESS(status);

    return status;
}

/**
 * @brief 向 ISO14443 标签写数据（16 字节）
 * @param bBlockNo 块地址
 * @param pTxBuffer 要写入的数据
 * @retval 操作状态
 */
uint16_t L2_14443_WritePara(uint8_t bBlockNo, uint8_t *pTxBuffer)
{
    phStatus_t status;
	uint8_t cmdBuffer[21];  // 5字节命令头 + 16字节数据
    uint8_t *pRxBuffer;
    uint16_t rxLen;

    if (pTxBuffer == NULL)
    {
        return PH_ERR_INVALID_PARAMETER;
    }
		

	cmdBuffer[0] = 0xA2;                  // MIFARE Write命令
	cmdBuffer[1] = bBlockNo;
	memcpy(&cmdBuffer[2], pTxBuffer, 16); // 拷贝完整的16字节数据
	
    // 发送数据
    status = phpalI14443p4_Exchange(
        &dataParams,
        0,
        cmdBuffer,
        sizeof(cmdBuffer),
        NULL,
        NULL 
    );
		
	if (status == PH_ERR_SUCCESS && rxLen >= 2) 
	{
        if (pRxBuffer[rxLen-2] != 0x90 || pRxBuffer[rxLen-1] != 0x00) 
		{
            status = PH_ERR_PROTOCOL_ERROR;
        }
    }
	
    CHECK_SUCCESS(status);

    return status;
}

/**
 * @brief 读取 ISO14443 标签中指定块的内容（16 字节）
 * @param bBlockNo 块地址
 * @param pData 接收数据缓存（16字节）
 * @retval 操作状态
 */
uint8_t L2_14443_ReadPara(uint8_t bBlockNo, uint8_t *pData)
{
//	int i;
    phStatus_t status;
	uint8_t cmdBuffer[5];
    //static uint8_t rxBuffer[4];
	uint8_t rxBuffer[18];  // 16字节数据 + 2字节状态
    uint8_t *pRxBuffer = rxBuffer;
    uint16_t rxLength = sizeof(rxBuffer);

    if (pData == NULL)
    {
        return PH_ERR_INVALID_PARAMETER;
    }
		
	cmdBuffer[0] = 0x30; // MIFARE Read命令
	cmdBuffer[1] = bBlockNo;
	
    // 接收数据
    status = phpalI14443p4_Exchange(
        &dataParams,
        0,
        cmdBuffer,
        sizeof(cmdBuffer),
        &pRxBuffer,
        &rxLength
    );
	
    //CHECK_SUCCESS(status);
		
	if (status == PH_ERR_SUCCESS) 
	{
		if (rxLength == 18 && pRxBuffer[16] == 0x90 && pRxBuffer[17] == 0x00) 
		{
			memcpy(pData, pRxBuffer, 16); // 提取有效数据
			/*
			for (i = 0; i < rxLength; i++)
			   pData[i] = pRxBuffer[i];
			*/
		} 
		else 
		{
			status = PH_ERR_PROTOCOL_ERROR;
		}
	}

    // 复制数据
    /*for (i = 0; i < rxLength; i++)
        pData[i] = pRxBuffer[i];
	*/

    return status;
}

/**
 * @brief 设置当前 NFC 状态
 * @param status NFC 状态值
 */
void L3_NFC_SetStatus(uint8_t status)
{
    L3_NFC_Status = status;
}

void L3_NFC_APP(void) //check nfc server status bit
{
	phStatus_t status = 65535;
	uint8_t len;
	
	// NFC 状态机主循环，根据当前状态执行对应操作
	switch(L3_NFC_Status)
	{
		// 当NFC状态为空闲时，初始化NFC设备并设置状态为初始化
		case NFC_STATUS_IDLE:					// 空闲状态
			L2_NFC_Device_IDLE();				// 执行 NFC 设备空闲态操作（如低功耗模式）
			L3_NFC_SetStatus(NFC_STATUS_INIT);	// 状态跳转至初始化
			break;
		
		// 当NFC状态为初始化时，关闭NFC模式的LED灯，并根据RC663模式进行初始化
		case NFC_STATUS_INIT:					// 初始化状态
			L2_LED_NFC_Mode(LED_OFF);			// 关闭 NFC 指示灯
			
			// 根据当前 NFC 模式（15693 或 14443）调用对应初始化函数
			if(RC663_MODE == RC663_15693)
				status = L2_15693_Init();			// ISO 15693 模式初始化 L2_15693_Init() == 之前的 L2_NFC_Init()
			else if(RC663_MODE == RC663_14443)
				status = L2_14443_Init();			// ISO 14443 模式初始化
			
			// 初始化成功则跳转至“开启场”状态，失败则跳转至错误状态
			if(status == PH_ERR_SUCCESS)
				L3_NFC_SetStatus(NFC_STATUS_OPEN_FIELD);
			else
				L3_NFC_SetStatus(NFC_STATUS_LABEL_ERROR);
			break;
		
		// 当NFC状态为开启场时，根据RC663模式开启NFC场
		case NFC_STATUS_OPEN_FIELD:				// 开启射频场状态
			status = 65535;
			if(RC663_MODE == RC663_15693)
				status = L2_15693_OpenField();	// 开启 15693 射频场
			else if(RC663_MODE == RC663_14443)
				status = L2_14443_OpenField();	// 开启 14443 射频场
			
			if(status == PH_ERR_SUCCESS)
				L3_NFC_SetStatus(NFC_STATUS_CONFIG_LABEL);	// 开启成功
			else
			{
				L3_NFC_SetStatus(NFC_STATUS_LABEL_ERROR);	// 失败则跳转错误状态
			}
			break;
		
		// 当NFC状态为配置标签时，根据RC663模式配置标签
		case NFC_STATUS_CONFIG_LABEL:			// 配置标签参数状态
			status = 65535;
			IWDG_FeedDog();
			// 根据模式配置标签参数（如通信速率、协议选项）
			if(RC663_MODE == RC663_15693)
				status = L2_15693_ConfigLable();	// 配置 15693 标签
			else if(RC663_MODE == RC663_14443)
				status = L2_14443_ConfigLable();	// 配置 14443 标签
			
			if(status == PH_ERR_SUCCESS)		// 配置成功
				L3_NFC_SetStatus(NFC_STATUS_ACTIVATE_LABEL);	// 跳转至激活标签
			else
				L3_NFC_SetStatus(NFC_STATUS_LABEL_ERROR);		// 失败则跳转错误状态
			break;
		
		// 当NFC状态为激活标签时，根据RC663模式激活标签
		case NFC_STATUS_ACTIVATE_LABEL:			// 激活标签状态
			status = 65535;
			// 尝试激活标签（建立通信）
			if(RC663_MODE == RC663_15693)
				status = L2_15693_ActivateLable();// 激活 15693 标签
			else if(RC663_MODE == RC663_14443)
				status = L2_14443_ActivateLable();// 激活 14443 标签

			// 检查激活是否成功，并根据结果设置下一步状态
			if(status == PH_ERR_SUCCESS)		// 激活成功
			{
				L3_NFC_SetStatus(NFC_STATUS_GET_LABEL_ID);						// 跳转至获取标签ID
				L2_Timer_CounterSet(TIMER_COUNTER_NFC,TIMER_COUNTER_RUNNING);	// 启动定时器
				L2_LED_NFC_Mode(LED_0S5);//打开NFC led							// NFC 指示灯闪烁（0.5秒周期）
				
				Tag_Exisit = 1;
			}
			else
			{
				L3_NFC_SetStatus(NFC_STATUS_LABEL_ERROR);			// 失败则跳转错误状态
				
				Tag_Exisit = 0;
			}
			break;
		
		 // 当NFC状态为获取标签ID时，等待一段时间后获取标签ID
		case NFC_STATUS_GET_LABEL_ID:			// 获取标签UID状态
			status = 65535;
			// 等待10ms后读取标签UID
			memset(pUid, 0, sizeof(pUid));
			memset(pUidOut, 0, sizeof(pUidOut));
			HAL_Delay(10);
		
			L2_Timer_CounterSet(TIMER_COUNTER_NFC,TIMER_COUNTER_STOP);		// 停止计时器
		
			if(RC663_MODE == RC663_15693)
				status = L2_15693_GetLableID();	// 读取 15693 标签UID
			else if(RC663_MODE == RC663_14443)
				status = L2_14443_GetLableID();	// 读取 14443 标签UID
			
			// 检查获取ID是否成功，并根据结果设置下一步状态
			if(status == PH_ERR_SUCCESS)
				L3_NFC_SetStatus(NFC_STATUS_NEW_LABEL);		// 跳转至新标签处理
			else
				L3_NFC_SetStatus(NFC_STATUS_LABEL_ERROR);	// 失败则跳转错误
			break;
			
		// 当NFC状态为新标签时，保存标签UID并发送消息
		case NFC_STATUS_NEW_LABEL:				// 新标签处理状态
			status = 65535;
			// 保存标签UID到缓冲区（根据模式区分UID长度）
			if(RC663_MODE == RC663_15693) {
				for(len=0;len<PHPAL_SLI15693_UID_LENGTH;len++)
				  L2_NFC_LableUID[len]=pUid[len];		// 保存 15693 UID（通常8字节）
			}
			else if(RC663_MODE == RC663_14443) {
				for(len=0;len<10;len++)
					L2_NFC_LableUID[len]=pUidOut[len];	// 保存 14443 UID（可能4/7/10字节）
			}
			
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
			
			L3_NFC_SetStatus(NFC_STATUS_WAIT_COMMA);//等待isdu指令	// 跳转至等待指令状态
			L2_LED_NFC_Mode(LED_0S5);								// 维持指示灯闪烁
			break;
			
		// 当NFC状态为等待逗号指令时，定期检查标签状态
		case NFC_STATUS_WAIT_COMMA:				// 等待外部指令状态
			status = 65535;
			// 持续轮询检测标签是否在位
			if(RC663_MODE == RC663_15693)
				status = L2_15693_ActivateLable();	//监控Lable是否一直都在 	// 检测15693标签是否在线
			else if(RC663_MODE == RC663_14443)
			{
				uint8_t Testdata[10] = {0};
				
				// 持续轮询检测标签是否在位	// 二次检测 14443A标签是否存在
				status = NTAG213_ReadPage(0x01, Testdata);
			}

			// 根据激活结果设置下一步状态
			if(status != PH_ERR_SUCCESS)	// 标签丢失或通信失败
				L3_NFC_SetStatus(NFC_STATUS_LABEL_ERROR);
			else
			{
				IWDG_FeedDog();
				
				L2_Process_Modbus();					// 处理Modbus指令（如读写标签数据）
			}
			break;
			
		// 当NFC状态为标签错误时，设置诊断错误并重置NFC设备
		case NFC_STATUS_LABEL_ERROR:			// 标签错误处理状态
			L2_NFC_DeviceReset();				// 复位NFC硬件模块
			L3_NFC_SetStatus(NFC_STATUS_INIT);	// 状态机重置为初始化状态
			L2_LED_NFC_Mode(LED_OFF);			// 关闭NFC指示灯
			break;
		
		// 当NFC状态为默认时，设置诊断错误并重置NFC设备
		default:								// 未知状态处理
			L3_NFC_SetStatus(NFC_STATUS_INIT);	// 重置为初始化状态
			L2_NFC_DeviceReset();				// 复位NFC硬件
			L2_LED_NFC_Mode(LED_OFF);			// 关闭指示灯
			break;
	}
	
	return ;
}

/**
 * @brief L3层统一NFC写块操作（支持15693与14443协议）
 * 
 * @param bBlockNo 要写入的块编号
 * @param pTxBuffer 指向要写入的数据缓冲区（长度需符合协议要求：15693为4字节，14443为16字节）
 * 
 * @note 根据当前RC663工作模式自动选择协议（15693 或 14443）调用底层写入函数
 *       若写入成功则发送写入完成消息；失败则组装错误码并发送错误消息
 */
void L3_NFC_WRITE(uint8_t bBlockNo, uint8_t * pTxBuffer)
{
	phStatus_t status;  
	if(RC663_MODE == RC663_15693)
		status = L2_15693_WritePara(bBlockNo, pTxBuffer);
	else if(RC663_MODE == RC663_14443)
		status = L2_14443_WritePara(bBlockNo, pTxBuffer);
	
	if(status == PH_ERR_SUCCESS)
		return ;
	else
		return ;
}

/**
 * @brief L3层统一NFC读块操作（支持15693与14443协议）
 * 
 * @param bBlockNo 要读取的块编号
 * 
 * @note 根据当前RC663工作模式自动选择协议（15693 或 14443）调用底层读取函数
 *       读取结果存入全局变量 NFC_Read_Data
 *       成功发送读完成消息；失败发送错误消息
 */
void L3_NFC_READ(uint8_t bBlockNo)
{
	phStatus_t status;
	if(RC663_MODE == RC663_15693)
		status = L2_NFC_ReadPara(bBlockNo, NFC_Read_Data);
	else if(RC663_MODE == RC663_14443)
		status = L2_14443_ReadPara(bBlockNo, NFC_Read_Data);
	
	if(status == PH_ERR_SUCCESS)
	{
//		L2_IOLM_SendMessage(IOLM_MESSAGE_NEW_BLOCK_R);
	}
	else
	{ 
//		L2_RF_Hx_IOL_Errcode_Assem(RF_LABEL_LEAVE_R);
//		L2_IOLM_SendMessage(IOLM_MESSAGE_NEW_ERR);
	}
}

//////////////////////////////////////////////////////////////////
void EditAddr(void)
{
	if(modbus.reflag == 1)
	{
		uint16_t crc = 0, rcrc = 0;
		if(modbus.recount < 2)
		{
			return ;
		}
		crc = ModbusCRC(modbus.rcbuf, modbus.recount-2);
		rcrc = modbus.rcbuf[modbus.recount-2]*256+modbus.rcbuf[modbus.recount-1]; //计算读取的CRC校验位
		
		IWDG_FeedDog();
		
		if(crc == rcrc) //CRC检验成功 开始分析包
		{
			if(Timer14_flag == 1)
				Timer14Cout[0] = Timer14_Bt;			// 有数据时需要更新 计数基数
			
			switch(modbus.rcbuf[1])
			{
				case 0x02:
				{
					modbus.myadd = EEPROM_ReadByte(0xEC);
					
					// 没有实现 eeprom 的读写情况
					USART1_SendByte(modbus.myadd);
					
					USART1_R_BUFF_CLEAR();
					modbus.recount = 0;//接收计数清零
					modbus.reflag = 0; //接收标志清零
				}
					break;
				case 0x05:
				{
					if(modbus.rcbuf[0] == EEPROM_ReadByte(0xEC))		// 对比地址是否是 该设备的地址
					{
						if(modbus.rcbuf[2] > 0 && modbus.rcbuf[2] <= 255)
						{
							EEPROM_WriteByte(0xEC, modbus.rcbuf[2]);
							modbus.myadd = EEPROM_ReadByte(0xEC);
							
							USART1_SendBuff(modbus.rcbuf,5);
						}
						else
						{
							USART1_SendByte(0xAA);
						}
					}
					
					USART1_R_BUFF_CLEAR();
					modbus.recount = 0;//接收计数清零
					modbus.reflag = 0; //接收标志清零
				}
					break;
				case 0x07:
				{
					LED_PWR_ON();
					LED_COM_ON();
					LED_MODE_ON();
					
					USART1_SendBuff(modbus.rcbuf,8);
					
					USART1_R_BUFF_CLEAR();
					modbus.recount = 0;//接收计数清零
					modbus.reflag = 0; //接收标志清零
					break;
				}
				case 0x08:
				{
					LED_PWR_OFF();
					LED_COM_OFF();
					LED_MODE_OFF();
					
					USART1_SendBuff(modbus.rcbuf,8);
					
					USART1_R_BUFF_CLEAR();
					modbus.recount = 0;//接收计数清零
					modbus.reflag = 0; //接收标志清零
					break;
				}
				case 0x20:			// 退出工厂模式
				{
					if(modbus.rcbuf[0] == EEPROM_ReadByte(0xEC))		// 对比地址是否是 该设备的地址
					{
						modbus.factory_exit = 0;
						Timer1_Bt = 200;
					}
				}
					break;
				
				case 0x36:
				{
					if(modbus.rcbuf[0] == EEPROM_ReadByte(0xEC))		// 对比地址是否是 该设备的地址
					{
						RC663_MODE = RC663_14443;
						
						EEPROM_WriteByte(0xA1, RC663_14443);
						
						Timer14_flag = 1;								// 修改Tim14标志位 从而开启超时机制
						
						L3_NFC_SetStatus(NFC_STATUS_IDLE);
						
						USART1_SendBuff(modbus.rcbuf, 8);
						
						USART1_R_BUFF_CLEAR();
						modbus.recount = 0;//接收计数清零
						modbus.reflag = 0; //接收标志清零
					}
				}
					break;
				case 0x37:
				{
					if(modbus.rcbuf[0] == EEPROM_ReadByte(0xEC))		// 对比地址是否是 该设备的地址
					{
						RC663_MODE = RC663_15693;
						
						EEPROM_WriteByte(0xA1, RC663_15693);
						
						L3_NFC_SetStatus(NFC_STATUS_IDLE);
						
						USART1_SendBuff(modbus.rcbuf, 8);
						
						USART1_R_BUFF_CLEAR();
						modbus.recount = 0;//接收计数清零
						modbus.reflag = 0; //接收标志清零
					}
				}
					break;
				case 0x38:
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
						
						USART1_R_BUFF_CLEAR();
						modbus.recount = 0;//接收计数清零
						modbus.reflag = 0; //接收标志清零
					}
				}
					break;
				default:
					break;
				
			}
			
			USART1_R_BUFF_CLEAR();
			modbus.recount = 0;//接收计数清零
			modbus.reflag = 0; //接收标志清零
		}
	}
}

uint16_t NTAG213_ReadPage(uint8_t pageAddr, uint8_t *pPageBuffer)
{
    phStatus_t status;
    uint8_t cmdBuffer[2];
    uint8_t *pRxBuffer;
    uint16_t rxLen;

    if (pPageBuffer == NULL) {
        return PH_ERR_INVALID_PARAMETER;
    }

    cmdBuffer[0] = 0x30;       // READ 命令
    cmdBuffer[1] = pageAddr;   // 起始页地址

    // 发送命令 (NTAG213是ISO14443-3A)
    status = phpalI14443p3a_Exchange(
        &dataParamsP3a,
        0,
        cmdBuffer,
        sizeof(cmdBuffer),
        &pRxBuffer,
        &rxLen
    );
	
	if (status == PH_ERR_SUCCESS && rxLen == 16) {
        // 取返回的前 4 字节（对应 pageAddr 页）
        memcpy(pPageBuffer, pRxBuffer, 4);
    } else {
        status = PH_ERR_PROTOCOL_ERROR;
    }

    return status;
}

uint16_t L2_14443_WritePara_Block(uint8_t pageAddr, uint8_t *pTxBuffer)
{
    phStatus_t status;
    uint8_t cmdBuffer[6];   // A2命令 + PageAddr + 4字节数据
    uint8_t *pRxBuffer;
    uint16_t rxLen;

    if (pTxBuffer == NULL) {
        return PH_ERR_INVALID_PARAMETER;
    }

    cmdBuffer[0] = 0xA2;         // WRITE 命令 (NTAG/Mifare Ultralight)
    cmdBuffer[1] = pageAddr;     // 页地址
    memcpy(&cmdBuffer[2], pTxBuffer, 4); // 4字节数据
	
	// 稍微延时，确保场稳定
    HAL_Delay(2);
    phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_TXWAIT_US, 128);
    phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_TIMEOUT_VALUE_US, 2000);

    // 发送数据 (注意用 14443-3 层的 API)
    status = phpalI14443p3a_Exchange(
        &dataParamsP3a,
        0,
        cmdBuffer,
        sizeof(cmdBuffer),
        &pRxBuffer,
        &rxLen
    );
	
    if (status == PH_ERR_SUCCESS)
    {
        if (rxLen == 1 && pRxBuffer[0] == 0x0A)
        {
            // ACK 成功
            HAL_Delay(10); // EEPROM 写入时间
        }
        else
        {
            // 卡返回 NAK
            status = PH_ERR_PROTOCOL_ERROR;
        }
    }

    return status;
}

