#ifndef __NFC_H_
#define __NFC_H_

#include "main.h"
/////////////////

// NFC 选择通信模式
#define MODE_IOLINK_or_RS485  	SELECT_RS485
#define SELECT_RS485 			0
#define SELECT_IOLINK 			1

//缓存区大小
#define BUFFER_SIZE 256

//NFC_STATUS BIT
#define NFC_STATUS_IDLE               0x00
#define NFC_STATUS_INIT               0x01
#define NFC_STATUS_OPEN_FIELD         0x02
#define NFC_STATUS_CONFIG_LABEL       0x03
#define NFC_STATUS_ACTIVATE_LABEL     0x04
#define NFC_STATUS_GET_LABEL_ID       0x05
#define NFC_STATUS_NEW_LABEL          0x06
#define NFC_STATUS_WAIT_COMMA         0x07
#define NFC_STATUS_READ_LABEL         0x08
#define NFC_STATUS_WRITE_LABEL        0x09
#define NFC_STATUS_LABEL_ERROR        0x50
#define NFC_STATUS_WRITE_DATA         0x10
#define NFC_STATUS_WRITE_ERROR        0x52

#define ISO15693_BLOCK_SIZE           4

#define ON_1  0x01
#define OFF_0 0x00

#define RC663_15693  0x00
#define RC663_14443  0x01

// 后续可能使用情况
#define SELECT_AGREEMENT RC663_14443


#define  CHECK_SUCCESS(status)   {if ((status) != PH_ERR_SUCCESS) {return ( status );}}


extern uint8_t UART_TxData_Buf[100];
extern uint8_t UART_PDInputLength;
extern uint8_t PDout;
extern uint8_t NFC_Read_Data[4];      //4字节块读取数据

extern uint8_t RC663_MODE;

extern uint8_t pUid[8];
extern uint8_t pUidOut[20];

extern uint8_t Tag_Exisit;

uint16_t L2_15693_Init(void);
uint16_t L2_14443_Init(void);
void L2_NFC_DeviceReset(void);            
//void L3_NFC_GetUID(uint8_t *pUID_APP);
uint8_t L2_NFC_Sever(void);
void L3_NFC_APP(void) ;
void L3_NFC_WRITE(uint8_t bBlockNo, uint8_t * pTxBuffer);
void L3_NFC_READ(uint8_t bBlockNo);

uint8_t L2_NFC_ReadLableIDbuff(uint8_t IDnum);
void UART_data(void);
uint8_t UART_CheckSum(uint8_t *data, uint8_t length);
void L3_NFC_SetStatus(uint8_t status);

uint8_t L2_NFC_ReadPara(uint8_t bBlockNo, uint8_t *pData);

void L2_NFC_Device_IDLE(void);

uint16_t L2_15693_OpenField(void);
uint16_t L2_15693_ConfigLable(void);
uint16_t L2_15693_ActivateLable(void);
uint16_t L2_15693_GetLableID(void);

uint16_t L2_14443_OpenField(void);
uint16_t L2_14443_ConfigLable(void);
uint16_t L2_14443_ActivateLable(void);
uint16_t L2_14443_GetLableID(void);


void EditAddr(void);
uint16_t NTAG213_ReadPage(uint8_t pageAddr, uint8_t *pPageBuffer);
uint16_t L2_14443_WritePara_Block(uint8_t pageAddr, uint8_t *pTxBuffer);

#endif
