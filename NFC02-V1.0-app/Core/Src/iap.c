#include "iap.h"
#include "string.h"

/* 定义 APP 入口函数指针 */
typedef void (*iapfun)(void);
iapfun jump2app;

/* FLASH 读缓存 */
uint16_t FLASH_READ_BUF[512];

uint32_t Flash_App_Addr;

/**
  * @brief 跳转到应用程序
  * @param appxaddr 应用程序起始地址
  */
void iap_load_app(uint32_t appxaddr)
{
    /* 判断栈顶地址是否合法 (0x2000 0000 ~ 0x200F FFFF 属于 SRAM 区域) */
    if (((*(__IO uint32_t*)appxaddr) & 0x2FFE0000) == 0x20000000)
    {
        __disable_irq();  // 关闭全局中断

        /* 关闭所有 NVIC 中断 */
        for (uint8_t i = 0; i < 8; i++)   // STM32F1 系列最多支持 240 个中断
        {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }

        /* 外设复位 (HAL 没有直接的 API，只能直接操作寄存器) */
        __HAL_RCC_APB1_FORCE_RESET();
        __HAL_RCC_APB1_RELEASE_RESET();
        __HAL_RCC_APB2_FORCE_RESET();
        __HAL_RCC_APB2_RELEASE_RESET();

        /* 关闭 SysTick */
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL  = 0;

        /* 取复位入口地址 */
        uint32_t app_entry = *(__IO uint32_t*)(appxaddr + 4);
        jump2app = (iapfun)app_entry;

        /* 设置主堆栈指针 MSP */
        __set_MSP(*(__IO uint32_t*)appxaddr);

        /* 设置中断向量表偏移 */
        SCB->VTOR = appxaddr;

        __enable_irq();

        /* 跳转执行 APP */
        jump2app();
    }
}

/**
  * @brief 从 FLASH 读取 APP 并跳转
  */
void Read_FlashToApp(void)
{
    HAL_FLASH_Unlock();

    /* 直接用 memcpy 读取 FLASH */
    memcpy(FLASH_READ_BUF, (uint16_t*)Flash_App_Addr, sizeof(FLASH_READ_BUF));

    HAL_FLASH_Lock();

    /* 判断是否有有效应用 (首地址不是全 FFFF) */
    if (FLASH_READ_BUF[0] != 0xFFFF)
    {
        iap_load_app(Flash_App_Addr);
    }
    else
    {
        // 没有有效应用，留在 BootLoader
        // BOOT_START = 1;  // 可根据项目需求自行处理
    }
}
