#include "IOLDS_LL.h"
#include "IOLDS_LL_StateJump.h"

bReturnStatus LL_ChangeState(u8 state);

void LL_StateInit0(void){
    //初始化LL层
}

// 暂时没用到
//void LL_StateWake1(void){
//    u8 txbuf;
//    if(IOLDS_STATUS_OK == PL_ReadTxBuf(&txbuf,1)){
//        if(txbuf == 0xFF){
//            LL_ChangeState(LL_MACHINE_STATE_WAKE1);
//        }
//    }
//    
//}

void LL_StateStartup2(void){
    static u8 wakeCounter=0;
    if(++wakeCounter > 3){//100us周期，主站在唤醒后500us开始通信握手，400us开始配置通讯
        //重新配置Uart波特率到默认波特率
        //做好通讯准备

        LL_ChangeState(LL_MACHINE_STATE_STARTUP2);
        wakeCounter = 0;
    } 
}

void LL_StateResponse3(void){
    static u8 wakeCounter=0;
    if(++wakeCounter > 3){//100us周期，主站在唤醒后500us开始通信握手，400us开始配置通讯
        //重新配置Uart波特率到默认波特率
        //做好通讯准备

        LL_ChangeState(LL_MACHINE_STATE_STARTUP2);
        wakeCounter = 0;
    } 
}

bReturnStatus LL_StateRun(void){
    
    switch (LL_MachineState)
    {
    case LL_MACHINE_STATE_INIT0:
        LL_StateInit0();
        break;

    case LL_MACHINE_STATE_WAKE1:
        /* code */
        break;

    case LL_MACHINE_STATE_STARTUP2:
        /* code */
        break;

    case LL_MACHINE_STATE_RESPONSE3:
        /* code */
        break;

    case LL_MACHINE_STATE_PREOP4:
        /* code */
        break;

    case LL_MACHINE_STATE_OP5:
        /* code */
        break;

    

    default:
//        machineState=LL_MACHINE_STATE_INIT0;
		LL_MachineState = LL_MACHINE_STATE_INIT0;
        return IOLDS_STATUS_ERR;
    }

    return IOLDS_STATUS_OK;
}

/**
  * @brief  改变系统状态
  * @param  state: 目标状态
  * @retval 状态改变结果: 0-成功, 1-失败
  */
uint8_t LL_ChangeState(uint8_t state)
{
    static uint8_t current_state = 0; // 当前状态 
	current_state = current_state;
    
    // 状态验证
    if (state > MAX_VALID_STATE) // MAX_VALID_STATE需要根据实际情况定义
    {
        return 1; // 无效状态
    }
    
    
    return 0; // 成功
}
