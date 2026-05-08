#ifndef __FACTORY_H_
#define __FACTORY_H_

#include "stm32g0xx_hal.h"

extern uint8_t Test_Verify_data[9];

void Factory_App(void);

void NFC_Factory_APP(void);
void Factory_Process_Modbus(void);

#endif
