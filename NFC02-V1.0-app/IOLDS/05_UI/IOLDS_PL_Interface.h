#ifndef _IOLDS_PL_INTERFACE_H_
#define _IOLDS_PL_INTERFACE_H_

#include "main.h"  //HAL 接口
#include "IOLDS_PL.h"  //HAL 接口
/*数据类型**********************************/


#define bReturnStatus uint8_t

#define IOLDS_STATUS_OK     0
#define IOLDS_STATUS_ERR    1
#define IOLDS_STATUS_BUSY   2
#define IOLDS_STATUS_EMPTY  3
//#define IOLDS_STATUS_  4
//#define IOLDS_STATUS_  0


/************************************数据类型*/




/*功能定义使能**********************************************/

//#define PL_UART_WAKE_FUNCTION
#define PL_STATUS_LED_FUNCTION
#define PL_UART_ERROR_FUNCTION

/**********************************************功能定义使能*/


/*HAL库接口************************************************/
#define PL_UART_NUMBER huart3    //uart 占据
#define PL_TIMER_NUMBER htim3    // 定时器占据


#define PL_UART_TXEN_PORT GPIOB
#define PL_UART_TXEN_PIN  GPIO_PIN_1



/*IOLINK收发器支持*/
#ifdef PL_UART_WAKE_FUNCTION
#define PL_UART_WAKE_PORT GPIOX
#define PL_UART_WAKE_PIN  GPIO_PIN_X
#endif

#ifdef PL_UART_ERROR_FUNCTION
#define PL_UART_ERROR_PORT GPIOB
#define PL_UART_ERROR_PIN  GPIO_PIN_2
#endif

#ifdef PL_STATUS_LED_FUNCTION
#define PL_STATUS_LED_PORT GPIOA
#define PL_STATUS_LED_PIN  GPIO_PIN_1
#endif

/************************************************HAL库接口*/


/*HAL中断回调函数**********************************/

bReturnStatus IOLDS_PL_UartCallBack(void);
bReturnStatus IOLDS_PL_TimerCallBack(void);


/************************************HAL中断回调函数*/


#endif





