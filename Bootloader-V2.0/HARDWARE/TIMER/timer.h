#ifndef __TIMER_H
#define __TIMER_H

#include "sys.h"

extern u8 Timer_Bt;
extern u8 Timer1_Bt;

void TIM3_Int_Init(u16 arr,u16 psc);

#endif
