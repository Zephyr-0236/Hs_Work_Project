#include "verify.h"

/**
 * @brief 计算数据的8位校验和 (求和取模)
 * 
 * @param data 指向需要计算校验和的数据缓冲区的指针
 * @param len 数据长度（字节数）
 * @return uint8_t 计算得到的8位校验和
 */
uint8_t calculate_checksum(const uint8_t *data, uint8_t len)
{
    uint16_t sum = 0;  // 使用16位变量防止溢出
	uint8_t i = 0;
    
    // 累加所有字节
    for (i = 0; i < len; i++) {
        sum += data[i];
    }
    
    // 返回低8位（自动截断）
    return (uint8_t)sum;
}

/**
 * @brief 验证数据的8位校验和是否正确
 * 
 * @param data 指向需要验证的数据缓冲区的指针（包含校验和字节）
 * @param len 数据长度（包括校验和字节）
 * @return int 验证结果：0-成功，-1-失败
 */
int verify_checksum(const uint8_t *data, uint8_t len)
{
	uint8_t calculated_checksum = 0;
    // 确保数据长度有效
    if (len < 1) {
        return -1;
    }
    
    // 计算除最后一个字节外所有数据的校验和
    calculated_checksum = calculate_checksum(data, len - 1);
    
    // 与接收到的校验和比较
    if (calculated_checksum == data[len - 1]) {
        return 1;  // 校验成功
    } else {
        return -1; // 校验失败
    }
}

uint8_t validate_packet_frame(const uint8_t* packet, uint8_t length)
{
	uint8_t flag = 0;
	
	if(packet[0] == 0xAA && packet[length - 1] == 0xAA && packet[1] == 0x55 && packet[length - 2] == 0x55)
		flag = 1;
	
	return flag;
}

uint8_t Search_HandShakeData(const uint8_t *parent, uint8_t parent_len)
{
	uint8_t Text[9] = {0xAA, 0x55, 0x12, 0x23, 0x34, 0x45, 0xAD, 0x55, 0xAA};
	uint8_t TextLength = 9;
	uint8_t flag = 99;
	uint8_t i = 0, j = 0;
	
	// 遍历母串，寻找子串
    for(i = 0; i <= parent_len - TextLength; i++)
    {
        // 假设当前位置匹配，开始逐字节比对
        for(j = 0; j < TextLength; j++)
        {
            // 如果有一个字节不匹配，退出内层循环
            if(parent[i + j] != Text[j])
            {
                break;
            }
        }
        
        // 如果所有字节都匹配，记录位置并退出
        if(j == TextLength)
        {
            flag = i;  // 找到子串，记录起始索引
            break;
        }
    }
	
	return flag;
}	
