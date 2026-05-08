#ifndef __CLRC663_INTERFACE_H
#define __CLRC663_INTERFACE_H

#include "main.h"


void uart_rx_frame_663(uint8_t num); //接收数据帧（读CLRC663寄存器）
void uart_tx_frame_663(uint8_t num); //发送数据帧（写CLRC663寄存器）
void CLRC663_Uart_Init(void);

#endif

