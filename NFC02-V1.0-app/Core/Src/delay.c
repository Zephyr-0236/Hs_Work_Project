#include "delay.h"

static uint32_t fac_us = 0;  // us 延时倍数
static uint32_t fac_ms = 0;  // ms 延时倍数

/**
 * @brief  初始化延时函数
 * @note   必须在 SystemClock_Config() 之后调用
 *         使用 HCLK 作为 SysTick 时钟源（HAL 默认设置）
 */
void L2_delay_Init(void)
{
    uint32_t sysclk_hz = SystemCoreClock;  // 应该是 64000000
	
	(void)fac_ms;

    // 计算每微秒和每毫秒对应的 tick 数
    fac_us = sysclk_hz / 1000000U;        // 例如：64MHz → 64
    fac_ms = fac_us * 1000U;               // 64000

    // 可选：确保 fac_us 至少为 1
    if (fac_us == 0) {
        fac_us = 1;
        fac_ms = 1000;
    }
}

/**
 * @brief  微秒级阻塞延时（不干扰 HAL_Delay）
 * @param  nus 要延时的微秒数
 */
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD; // 获取当前 HAL 配置的重载值 (通常是 1ms 的 ticks)

    ticks = nus * fac_us;            // 需要等待的 ticks 数
    told = SysTick->VAL;             // 刚进入时的计数器值

    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow; // SysTick 是向下计数的
            }
            else
            {
                // 发生了重装载（跨越了 1ms 的边界）
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks) break; // 时间够了，退出
        }
    }
}

/**
 * @brief  毫秒级阻塞延时
 * 直接调用 HAL 库的函数即可，没必要重复造轮子且破坏系统
 */
void delay_ms(uint16_t nms)
{
    HAL_Delay(nms); 
}
