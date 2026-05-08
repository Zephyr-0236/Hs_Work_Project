#include "eeprom.h"
#include "delay.h"

// ================= I2C 基础函数 =================
void Soft_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitStructure.Pin = I2C_SCL_PIN | I2C_SDA_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;           // 开漏输出
	GPIO_InitStructure.Pull = GPIO_NOPULL;                   // HAL库需要明确指定上下拉
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;         // 高速 (50MHz)
	HAL_GPIO_Init(I2C_GPIO_PORT, &GPIO_InitStructure);
	
    SCL_High();
    SDA_High();
}

static void I2C_Delay(void)
{
    delay_us(5); // ~100kHz
}

void I2C_Start(void)
{
    SDA_High();
    SCL_High();
    I2C_Delay();
    SDA_Low();
    I2C_Delay();
    SCL_Low();
}

void I2C_Stop(void)
{
    SCL_Low();
    SDA_Low();
    I2C_Delay();
    SCL_High();
    I2C_Delay();
    SDA_High();
    I2C_Delay();
}

// 发送一个字节
void I2C_SendByte(uint8_t data)
{
	uint8_t i = 0;
	
    for (i = 0; i < 8; i++)
    {
        SCL_Low();   // 先拉低 SCL，保证数据稳定
        if (data & 0x80)
            SDA_High();
        else
            SDA_Low();

        I2C_Delay();

        SCL_High();  // 上升沿时刻采样 SDA
        I2C_Delay();
        data <<= 1;
    }
    SCL_Low(); // 确保 SCL 结束后为低电平
}

// 接收一个字节（调用者决定是否发 ACK/NACK）
uint8_t I2C_RecvByte(void)
{
    uint8_t data = 0;
	uint8_t i = 0;

    SDA_High(); // 释放 SDA，准备接收
    for (i = 0; i < 8; i++)
    {
        data <<= 1;

        SCL_Low();
        I2C_Delay();

        SCL_High();
        I2C_Delay();
        if (SDA_Read())
            data |= 0x01;
    }
    SCL_Low();
    return data;
}


uint8_t I2C_RecvAck(void)
{
    uint8_t ack;
    SDA_High();
    I2C_Delay();
    SCL_High();
    I2C_Delay();
    ack = SDA_Read();
    SCL_Low();
    return ack;
}

void I2C_SendAck(uint8_t ack)
{
    if (ack) SDA_High(); else SDA_Low();
    I2C_Delay();
    SCL_High();
    I2C_Delay();
    SCL_Low();
}

// =============== EEPROM 定义 ===============
#define EEPROM_ADDR_WRITE  0xA0  // 24CW160T器件地址 + 写命令: 1010 000 0
#define EEPROM_ADDR_READ   0xA1  // 24CW160T器件地址 + 读命令: 1010 000 1
#define EEPROM_PAGE_SIZE   8     // 24CW160T页大小为8字节
#define EEPROM_TOTAL_SIZE  2048  // 16Kbit = 2048字节

// =============== EEPROM 操作函数 ===============

/**
  * @brief  向EEPROM指定地址写入一个字节
  * @param  addr: 要写入的地址 (0-2047)
  * @param  data: 要写入的数据
  * @retval 0: 成功, 1: 失败
  */
uint8_t EEPROM_WriteByte(uint16_t addr, uint8_t data)
{
    if(addr >= EEPROM_TOTAL_SIZE) return 1;
    
    I2C_Start();
    
    // 发送器件地址 + 写命令
    I2C_SendByte(EEPROM_ADDR_WRITE);
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 1;
    }
    
    // 发送16位内存地址（高字节在前）
    I2C_SendByte((uint8_t)(addr >> 8));   // 地址高字节
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 1;
    }
    
    I2C_SendByte((uint8_t)(addr & 0xFF)); // 地址低字节
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 1;
    }
    
    // 发送数据
    I2C_SendByte(data);
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 1;
    }
    
    I2C_Stop();
    
    // 等待写入完成（重要！）
	HAL_Delay(10);
    
    return 0;
}

/**
  * @brief  从EEPROM指定地址读取一个字节
  * @param  addr: 要读取的地址 (0-2047)
  * @retval 读取到的数据
  */
uint8_t EEPROM_ReadByte(uint16_t addr)
{
    uint8_t data;
    
    if(addr >= EEPROM_TOTAL_SIZE) return 0xFF;
    
    I2C_Start();
    
    // 发送器件地址 + 写命令（设置地址指针）
    I2C_SendByte(EEPROM_ADDR_WRITE);
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 0xFF;
    }
    
    // 发送16位内存地址
    I2C_SendByte((uint8_t)(addr >> 8));   // 地址高字节
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 0xFF;
    }
    
    I2C_SendByte((uint8_t)(addr & 0xFF)); // 地址低字节
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 0xFF;
    }
    
    // 发送重复起始条件，开始读取
    I2C_Start();
    
    // 发送器件地址 + 读命令
    I2C_SendByte(EEPROM_ADDR_READ);
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 0xFF;
    }
    
    // 读取数据（发送NACK表示这是最后一个字节）
    data = I2C_RecvByte();
    I2C_SendAck(1); // 发送NACK
    
    I2C_Stop();
    
    return data;
}

/**
  * @brief  页写入函数（最多写入8字节）
  * @param  addr: 起始地址 (0-2047)
  * @param  pData: 数据缓冲区指针
  * @param  len: 要写入的字节数 (1-8)
  * @retval 0: 成功, 1: 失败
  */
uint8_t EEPROM_WritePage(uint16_t addr, uint8_t *pData, uint8_t len)
{
    uint8_t i;
    
    if(addr >= EEPROM_TOTAL_SIZE) return 1;
    if(len == 0 || len > EEPROM_PAGE_SIZE) return 1;
    // 检查是否跨越页边界
    if((addr % EEPROM_PAGE_SIZE) + len > EEPROM_PAGE_SIZE) return 1;
    
    I2C_Start();
    
    // 发送器件地址 + 写命令
    I2C_SendByte(EEPROM_ADDR_WRITE);
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 1;
    }
    
    // 发送16位内存地址
    I2C_SendByte((uint8_t)(addr >> 8));   // 地址高字节
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 1;
    }
    
    I2C_SendByte((uint8_t)(addr & 0xFF)); // 地址低字节
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 1;
    }
    
    // 发送数据
    for(i = 0; i < len; i++) {
        I2C_SendByte(pData[i]);
        if(I2C_RecvAck()) {
            I2C_Stop();
            return 1;
        }
    }
    
    I2C_Stop();
    
    // 等待写入完成
	HAL_Delay(10);
	
    return 0;
}

/**
  * @brief  连续读取函数
  * @param  addr: 起始地址 (0-2047)
  * @param  pData: 数据缓冲区指针
  * @param  len: 要读取的字节数
  * @retval 0: 成功, 1: 失败
  */
uint8_t EEPROM_ReadBuffer(uint16_t addr, uint8_t *pData, uint16_t len)
{
    uint16_t i;
    
    if(addr >= EEPROM_TOTAL_SIZE) return 1;
    if(len == 0) return 1;
    if(addr + len > EEPROM_TOTAL_SIZE) return 1;
    
    I2C_Start();
    
    // 发送器件地址 + 写命令（设置地址指针）
    I2C_SendByte(EEPROM_ADDR_WRITE);
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 1;
    }
    
    // 发送16位内存地址
    I2C_SendByte((uint8_t)(addr >> 8));   // 地址高字节
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 1;
    }
    
    I2C_SendByte((uint8_t)(addr & 0xFF)); // 地址低字节
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 1;
    }
    
    // 发送重复起始条件，开始读取
    I2C_Start();
    
    // 发送器件地址 + 读命令
    I2C_SendByte(EEPROM_ADDR_READ);
    if(I2C_RecvAck()) {
        I2C_Stop();
        return 1;
    }
    
    // 连续读取数据
    for(i = 0; i < len; i++) {
        pData[i] = I2C_RecvByte();
        // 如果是最后一个字节，发送NACK，否则发送ACK
        if(i == len - 1) {
            I2C_SendAck(1); // NACK
        } else {
            I2C_SendAck(0); // ACK
        }
    }
    
    I2C_Stop();
    
    return 0;
}

/**
  * @brief  等待EEPROM写入完成（通过应答查询）
  */
void EEPROM_WaitWriteComplete(void)
{
    uint32_t timeout = 1000; // 超时计数器
    
    do {
        I2C_Start();
        I2C_SendByte(EEPROM_ADDR_WRITE);
        if(I2C_RecvAck() == 0) {
            I2C_Stop();
            break; // 收到ACK，写入完成
        }
        I2C_Stop();
        HAL_Delay(1); // 延迟1ms再重试
    } while(timeout--);
    
    // 如果超时，可以在这里添加错误处理
    if(timeout == 0) {
        // 写入超时错误处理
    }
}

/**
  * @brief  写入任意长度数据（自动处理页边界）
  * @param  addr: 起始地址
  * @param  pData: 数据缓冲区
  * @param  len: 数据长度
  * @retval 0: 成功, 1: 失败
  */
uint8_t EEPROM_WriteBuffer(uint16_t addr, uint8_t *pData, uint16_t len)
{
    uint16_t bytes_to_write;
    uint16_t page_offset;
    uint8_t ret;
    
    if(addr >= EEPROM_TOTAL_SIZE) return 1;
    if(len == 0) return 1;
    if(addr + len > EEPROM_TOTAL_SIZE) return 1;
    
    while(len > 0) {
        // 计算当前页的剩余空间
        page_offset = addr % EEPROM_PAGE_SIZE;
        bytes_to_write = EEPROM_PAGE_SIZE - page_offset;
        
        // 如果剩余数据小于页剩余空间，则写入剩余所有数据
        if(bytes_to_write > len) {
            bytes_to_write = len;
        }
        
        // 写入一页数据
        ret = EEPROM_WritePage(addr, pData, bytes_to_write);
        if(ret != 0) {
            return ret; // 写入失败
        }
        
        // 更新地址、数据和剩余长度
        addr += bytes_to_write;
        pData += bytes_to_write;
        len -= bytes_to_write;
    }
    
    return 0;
}
