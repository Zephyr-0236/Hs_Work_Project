#ifndef __VERIFY_H_
#define __VERIFY_H_

#include "stm32g0xx_hal.h"

uint8_t calculate_checksum(const uint8_t *data, uint8_t len);
int verify_checksum(const uint8_t *data, uint8_t len);
uint8_t validate_packet_frame(const uint8_t* packet, uint8_t length);
uint8_t Search_HandShakeData(const uint8_t *parent, uint8_t parent_len);

#endif
