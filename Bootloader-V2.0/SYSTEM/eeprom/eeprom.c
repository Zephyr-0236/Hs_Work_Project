#include "eeprom.h"
#include "delay.h"

// ================= I2C 基础函数 =================
void Soft_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = I2C_SCL_PIN | I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2C_GPIO_PORT, &GPIO_InitStructure);

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

// ================= EEPROM 函数 =================
// 计算设备地址（A10..A8 映射到设备地址的 A2/A1/A0）
uint8_t EEPROM_GetDeviceAddr(uint16_t memAddr, uint8_t rw)
{
    uint8_t block = (memAddr >> 8) & 0x07; // 取 A10..A8
    return EEPROM_ADDR_BASE | (block << 1) | (rw & 0x01);
}

//static void EEPROM_WaitBusy(uint16_t memAddr)
//{
//    uint8_t devAddr = EEPROM_GetDeviceAddr(memAddr, 0);

//    while (1)
//    {
//        I2C_Start();
//        I2C_SendByte(devAddr);
//        if (I2C_RecvAck() == 0) // 收到 ACK → 写完成
//        {
//            I2C_Stop();
//            break;
//        }
//        I2C_Stop();
//    }
//}

// 单字节写
void EEPROM_WriteByte(uint16_t memAddr, uint8_t data)
{
    uint8_t devAddr = EEPROM_GetDeviceAddr(memAddr, 0);

    I2C_Start();
    I2C_SendByte(devAddr);       I2C_RecvAck();
    I2C_SendByte((uint8_t)memAddr); I2C_RecvAck();  // 低 8 位地址
    I2C_SendByte(data);          I2C_RecvAck();
    I2C_Stop();

    delay_ms(10);
//	EEPROM_WaitBusy(memAddr);
}

// 页写（一次最多 16 字节，必须同一页内）
void EEPROM_PageWrite(uint16_t memAddr, uint8_t *buf, uint8_t len)
{
	uint8_t i = 0;
    uint8_t devAddr = EEPROM_GetDeviceAddr(memAddr, 0);
    if (len > 16) len = 16; // 每页最多 16 字节

    I2C_Start();
    I2C_SendByte(devAddr);       I2C_RecvAck();
    I2C_SendByte((uint8_t)memAddr); I2C_RecvAck();

    for (i = 0; i < len; i++)
    {
        I2C_SendByte(buf[i]);
        I2C_RecvAck();
    }
    I2C_Stop();

    delay_ms(10);
//	EEPROM_WaitBusy(memAddr);
}

// 单字节读
uint8_t EEPROM_ReadByte(uint16_t memAddr)
{
    uint8_t data;
    uint8_t devAddr = EEPROM_GetDeviceAddr(memAddr, 0);

    I2C_Start();
    I2C_SendByte(devAddr);       I2C_RecvAck();
    I2C_SendByte((uint8_t)memAddr); I2C_RecvAck();

    I2C_Start();
    I2C_SendByte(devAddr | 0x01); I2C_RecvAck();
    data = I2C_RecvByte(); // NACK
    I2C_Stop();

    return data;
}

// 缓冲区读
//void EEPROM_ReadBuffer(uint16_t memAddr, uint8_t *buf, uint16_t len)
//{
//	uint16_t i = 0;
//    uint8_t devAddr = EEPROM_GetDeviceAddr(memAddr, 0);

//    I2C_Start();
//    I2C_SendByte(devAddr);       I2C_RecvAck();
//    I2C_SendByte((uint8_t)memAddr); I2C_RecvAck();

//    I2C_Start();
//    I2C_SendByte(devAddr | 0x01); I2C_RecvAck();

//    for (i = 0; i < len; i++)
//    {
//        buf[i] = I2C_RecvByte(); // 最后一个 NACK
//    }
//    I2C_Stop();
//}

void EEPROM_ReadBuffer(uint16_t memAddr, uint8_t *buf, uint16_t len)
{
    uint16_t i;
    uint8_t devAddr = EEPROM_GetDeviceAddr(memAddr, 0);

    I2C_Start();
    I2C_SendByte(devAddr);          I2C_RecvAck();
    I2C_SendByte((uint8_t)memAddr); I2C_RecvAck();

    I2C_Start();
    I2C_SendByte(devAddr | 0x01);   I2C_RecvAck();

    for (i = 0; i < len; i++)
    {
        buf[i] = I2C_RecvByte();
        if (i < len - 1)
            I2C_SendAck(0);  // 继续读 → 发 ACK
        else
            I2C_SendAck(1);  // 最后一个字节 → 发 NACK
    }
    I2C_Stop();
}

void EEPROM_Test(void)
{
    uint16_t addr;
    uint8_t devAddr;

    for (addr = 0; addr < 0x800; addr += 0x100)
    {
        devAddr = EEPROM_GetDeviceAddr(addr, 0);
        printf("memAddr=0x%03X, devAddr=0x%02X\n", addr, devAddr);
    }
}
