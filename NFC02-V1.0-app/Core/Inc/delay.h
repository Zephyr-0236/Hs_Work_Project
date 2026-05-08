#ifndef __DELAY_H
#define __DELAY_H

#include "main.h"
#include <stdint.h>

// 初始化延时函数 (会自动根据 SystemCoreClock 计算)
void L2_delay_Init(void);

// 毫秒延时 (阻塞式)
void delay_ms(uint16_t nms);

// 微秒延时 (阻塞式)
void delay_us(uint32_t nus);

#endif
