#ifndef _IOLDS_APP_H_
#define _IOLDS_APP_H_

#include "main.h"  //HAL 接口
#include "IOLDS_APP_Interface.h"
#include "IOLDS_PL_Interface.h"
#include "IOLDS_PL.h" 
#define ISDU_DATANUM_MAX     8    //ISDU最长数据种类
#define ISDU_INDEX_ADDRESS_MAX     0xFFFF
#define ISDU_DATALENGTH_MAX  256  //ISDU数据总长
#define PD_LENGTH_MAX        32   //PD最高长度
#define ISDU_ADDRESS_MAX 256



/*结构体定义*****************************************/

typedef struct
{
    u8 data[ISDU_DATALENGTH_MAX];
    u8 DataUsedCounter;//最高8,0表示没有使用
    u8 DataType[ISDU_DATANUM_MAX];
} IOLDS_APP_ISDUTypeDef;

typedef struct
{
    u8 Mode;
    u8 *PDin;
    u8 *PDout;
    u8 bUpdatingPDin;
    u8 bUpdatingPDout;
    IOLDS_APP_ISDUTypeDef *ISDU[ISDU_ADDRESS_MAX];  
    
} IOLDS_APP_DeviceStateTypeDef;





/*********************************************结构体定义*/


#endif
