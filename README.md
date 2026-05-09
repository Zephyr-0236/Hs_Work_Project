# STM32F103C8T6 Bootloader / IAP（RS485 总线）


---

## 一、项目定位与 Flash 布局

| 区域         | 地址（以 `User/IAP/iap.h` 为准）      | 说明                                                         |
| ------------ | ------------------------------------- | ------------------------------------------------------------ |
| Bootloader   | `0x08000000` 起                       | 本工程；注释中预留约 **64KB** 给 Boot，须与 **分散加载（.sct）** 一致。 |
| 应用程序 App | **`0x08004000`**（`FLASH_APP1_ADDR`） | 升级目标区；App 链接脚本起始地址须与此对齐。                 |
| Flash 末尾   | **`0x08100000`**（`FLASH_END_ADDR`）  | 1MB 片上 Flash 上界，握手时用于区间合法性判断。              |

**数据分包**：按 **`APP_FLASH_CHUNK_SIZE`（1024 字节）** 分块擦除/下载/校验，与主机侧约定一致。

**超时进 App**：主循环在 **`Timer_Bt >= 2`** 且 **`DownLoadFlag == 0`** 时调用 **`Read_FlashToApp()`**。`Timer_Bt` 在 **TIM3** 更新中断中递增（约 **500ms/次**），故 `>=2` 约 **1s** 量级（以 `User/TIM/bsp_basic_tim.c` 中 `TIM3_Int_Init` 为准）。

## 三、软件架构

### 3.1 主流程（`User/main.c`）

1. **初始化**：`RCC_Config` → `SysTick`（1ms）→ `GPIO_Config` → `USART_Config`（仅调试口）→ `CAN_Config` → `TIM_Config` → `TIM3_Int_Init` → `NVIC_Config` → `LED_Init` → `delay_init`。
2. **大循环**：
    - 超时且无下载进行时 **`Read_FlashToApp()`**。
    - **`modbus.data_ready == 1`** 时按 **`Flash_Status`** 处理**一帧 IAP 应用数据**（已由 CAN 层重组完成）。

### 3.2 IAP 状态机（`Flash_Status`）

| 宏                | 含义                                                         |
| ----------------- | ------------------------------------------------------------ |
| `BL_ST_HANDSHAKE` | 握手：校验和、魔数 `0x4C 0x57`，解析下载起止地址；非法则 NACK。 |
| `BL_ST_ERASE`     | 按序擦除扇区；`0x10` 表示擦除阶段结束并进入下一状态。        |
| `BL_ST_DOWNLOAD`  | 固件数据写入 Flash（大包长度约定如 **1028** 字节等，与主机一致）。 |
| `BL_ST_VERIFY`    | 回读校验。                                                   |
| `BL_ST_JUMP_APP`  | 跳转 App。                                                   |
| `BL_ST_ERROR`     | 异常；当前实现可跳转 App 或复位（见 `main.c`）。             |


## 二、调试与可靠性

- **HardFault**：`User/stm32f4xx_it.c` 中 **`g_hardfault_dump`**；Watch 关注 **`pc`、`lr`、`cfsr`**。
- **栈空间**：启动文件中 **`Stack_Size`** 已加大；`main` 内大数组（如校验应答缓冲）注意栈使用。
- **清缓冲**：IAP 路径使用 **`IAP_RxBufferClear()`**，与 CAN 分片状态一致。
- **App 跳转**：确认 **`Flash_App_Addr`** 处向量表合法（栈顶在 RAM、入口 Thumb）。

---

## 三、构建说明

- **IDE**：Keil MDK，**`startup_stm32f40xx.s`**。
- 建议勿常规提交 **`Output/`**、**`Listing/`**；以 **`User/`**、**`Libraries/`** 与 **`README.md`** 为主。

---

*文档随工程演进更新。*
