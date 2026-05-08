#ifndef _IOLD_LL_STATEJUMP_H
#define _IOLD_LL_STATEJUMP_H


#include "IOLDS_APP_Interface.h"
#include "IOLDS_PL_Interface.h"

#define LL_MACHINE_STATE_INIT0       0
#define LL_MACHINE_STATE_WAKE1       1
#define LL_MACHINE_STATE_STARTUP2    2
#define LL_MACHINE_STATE_RESPONSE3   3
#define LL_MACHINE_STATE_PREOP4      4
#define LL_MACHINE_STATE_OP5         5

#define MAX_VALID_STATE				 5

u8 LL_MachineState;

bReturnStatus LL_StateRun(void);
bReturnStatus LL_ChangeState(u8 state);
u8            LL_ReadState(void);

#endif
