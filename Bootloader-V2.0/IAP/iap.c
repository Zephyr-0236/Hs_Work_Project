#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "stmflash.h"
#include "iap.h"

iapfun jump2app;
u16 iapbuf[1024];
u16 FLASH_READ_BUF[1050];
uint32_t Flash_App_Addr = 0x08004000;

//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u16 t;
	u16 i=0;
	u16 temp;
	u32 fwaddr=appxaddr;//当前写入的地址
	u8 *dfu=appbuf;
	
	if (appsize % 2 != 0) appsize++;
	
	for(t=0;t<appsize;t+=2)
	{						    
		temp=(u16)dfu[1]<<8;
		temp+=(u16)dfu[0];
		dfu+=2;//偏移2个字节
		iapbuf[i++]=temp;
		if(i==1024)
		{
			i=0;
			STMFLASH_Write(fwaddr,iapbuf,1024);
			fwaddr+=2048;//偏移2048  16=2*8.所以要乘以2.
		}
	}
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//将最后的一些内容字节写进去.
}

//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(uint32_t appxaddr)
{
    if(((*(vu32*)appxaddr) & 0x2FFE0000) == 0x20000000) // 栈顶是否合法
    {
        __disable_irq();   // 关全局中断

        // 关闭所有 NVIC 中断
        NVIC->ICER[0] = 0xFFFFFFFF;
        NVIC->ICER[1] = 0xFFFFFFFF;
        NVIC->ICPR[0] = 0xFFFFFFFF;
        NVIC->ICPR[1] = 0xFFFFFFFF;

        // 复位 APB1/APB2 外设
        RCC->APB1RSTR = 0xFFFFFFFF;
        RCC->APB1RSTR = 0x00000000;
        RCC->APB2RSTR = 0xFFFFFFFF;
        RCC->APB2RSTR = 0x00000000;

        // 关闭 SysTick
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL  = 0;

        // 取 APP 入口地址
        jump2app=(iapfun)*(vu32*)(appxaddr+4);		// 2. 取复位入口地址
        // 设置栈顶指针
        __set_MSP(*(__IO uint32_t*)appxaddr);

        // 建议在 APP 的 SystemInit() 里自行设置 SCB->VTOR
        SCB->VTOR = appxaddr;

        __enable_irq();
        jump2app();   // 跳转
    }
}

void Read_FlashToApp(void)
{
	if(Flash_App_Addr != 0x00)
	{
		FLASH_Unlock();
		STMFLASH_Read(Flash_App_Addr,FLASH_READ_BUF,512);
		FLASH_Lock();
		
		if(FLASH_READ_BUF[0]!=0XFFFF)
		{
			iap_load_app(Flash_App_Addr);//执行FLASH APP代码
		}
	}
	
}
