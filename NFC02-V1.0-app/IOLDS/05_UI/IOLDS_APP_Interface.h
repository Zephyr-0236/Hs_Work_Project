#ifndef _IOLDS_APP_INTERFACE_H_
#define _IOLDS_APP_INTERFACE_H_

#include "main.h"

/*
#ifndef uint8_t
typedef u8 uint8_t;
#endif

#ifndef uint16_t
typedef u16 uint16_t;
#endif

#ifndef uint32_t
typedef u32 uint32_t;
#endif

#ifndef uint64_t
typedef u64 uint64_t;
#endif
*/

#define u8    uint8_t
#define u16   uint16_t
#define u32   uint32_t
#define u64   uint64_t

#define bReturnStatus uint8_t



/*函数声明**********************************/

bReturnStatus IOLDS_APP_MainInitial(void);
bReturnStatus IOLDS_APP_SetDevicePara(void);
bReturnStatus IOLDS_APP_ReadDevicePara(void);
bReturnStatus IOLDS_APP_ReadDeviceMode(void);
bReturnStatus IOLDS_APP_ReadPDoutBuf(u8 *pData);
bReturnStatus IOLDS_APP_SetPDinBuf(u8 *pData);

bReturnStatus IOLDS_APP_CreateISDU(u16 index,  u8 subindex, u8 *data,  u16 dataLength);
bReturnStatus IOLDS_APP_ReadISDU(u16 index,  u8 subindex, u8 *data,  u16 dataLength);
bReturnStatus IOLDS_APP_SetISDU(u16 index,  u8 subindex, u8 *data,  u16 dataLength);

/************************************函数声明*/




#endif
