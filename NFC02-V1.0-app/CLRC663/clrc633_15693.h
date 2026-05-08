Load Protocol (ISO15693)

SR  0F 98 //Configure T0
          //设置定时器0的控制寄存器
          //定时器在传输结束时自动开始 
          //计数器值达到零时，定时器用T0ReloadValue的预设值重启计时 
          //定时器0输入时钟设置为13.56 MHz

SR  14 92 //Configure T1 and cascade it with T0
          //设置定时器1的控制寄存器
          //定时器在传输结束时自动开始 
          //定时器1输入时钟为定时器0的下溢

SR  19 20 //Configure T2 for LFO AutoTrimm 
          //设置定时器2的控制寄存器
          //定时器用于无下溢时的LFO微调（PosEdge时启动/停止）
          //定时器递减到0且停止。当定时器下溢时， Timer2Irq位被设置为1。 
          //定时器2输入时钟在13.56 MHz  

SR  1A 03 //T2 reload value for LFO AutoTrimm
          //设置定时器2预设值的高字节
          //定时器以启动事件加载寄存器T2ReloadValHi和寄存器T2ReloadValLo值。
          //改变此寄存器仅会在下次启动事件影响定时器。
            
SR  1B FF //设置定时器2预设值的低字节

SR  1E 00 //Configure T3 (for LPCD/ AutoTrimm) 
          //定时器3的控制寄存器
          //定时器3不会自动开始
          //定时器3输入时钟在13.56 MHz
  
SR  02 90 //Set FiFo-Size and Waterlevel
          //设置FIFO控制寄存器
          //FIFO大小为255字节
          //设置为空清空FIFO缓冲器
          
SR  03 FE 

SR  0C 80 // Init. RxBitCtrl register
          //设置接收器控制寄存器
          //如果清除，在接收发生碰撞位置后的每个位都被0取代
          //此功能用于ISO/IEC 14443防碰撞 
          //定义用于接收到的第一个有效位的位置。
          //后续接收到的位则根据第一位的位置递增
          //0x80设置接收位的LSB存储在第0位，第二个接收的位存储在第1位位置 
          //注意：如果RxAlign(bit6-bit4)=0，数据接收以字节为基本单位，否则以位为基本单位。
          //定义在以位为基本单位通信中接收到的最后数据字节的有效位数。如果为零，整个字节都为有效。

SR  28 80 //设置接收器控制寄存器
          //在TX2引脚反转发射机2的信号
          //设置TX1 和 TX2 高阻抗模式
          
SR  29 00 // Init. TxAmp register 
          //设置TxAmp寄存器。TX1的电压将被上拉到可能的最大化
          //允许发射机输出振幅减少固定值。
          //设置4个从TVDD扣除掉的预设值：TVDD-100 mV    
          //设定剩余载波百分比
          //设置剩余载波和调制指数
          //剩余载波:99%，调制指数：0.5% 

SR  2A 01 // Init. DrvCon register
          //设置TxEnvelope为调制源
          
SR  2B 05 // Init. TxI register
          //设置工厂微调值，设置预期Tx 的负载电流
          //根据这个预期负载电流，可以用较优化的方式控制调制指数。
          //此时设置为0x05
              
SR  34 00 // Init RxSOFD register 
          //主要是设置检测到SOF，会发出RxSOFIrq等等，相关。
          //0x00就是不设置这些 

SR  38 12 // Init. RCV register
          //设置为全差分RXP输入引脚模式
          //ADC的正常接收模式
          //设置为具有至少1/4的信号强度，即被认为碰撞

SR  00 00 // Cancel any command execution.Flush Fifo, Clear IRQ1, IRQ2
          //停止任何命令， 
					
SR  02 B0 //FIFO大小为255字节
          //并设置为可清空FIFO缓冲器
           
SR  06 7F //在位6到位0写入1时清除一些中断请求
          //包括HiAlertIrq、LoQlertIrq、IdleIrq、TxIrq、RxIrq、ErrIrq、RxSOFIrq

SR  07 7F //在位5到位0写入1时清除该位的中断请求
          //bit6   GlobalIrq   当有效的Irq 发生时设置
          //bit5   LPCD_Irq    在低功耗卡检测序列（LPCD）中检测到卡时设置  
          //bit4   Timer4Irq   设置为逻辑1，当定时器4产生下溢。
          //bit3   Timer3Irq   设置为逻辑1，当定时器3产生下溢。
          //bit2   Timer2Irq   设置为逻辑1，当定时器2产生下溢。
          //bit1   Timer1Irq   设置为逻辑1，当定时器1产生下溢。
          //bit0   Timer0Irq   设置为逻辑1，当定时器0产生下溢。
// Write in FIFO "Load protocol" params(TxProtocol=Iso15693(0a), RxProtocol=Iso15693(0a),
SR  05 0A 
SR  05 0A 
SR  08 10 // Enable IRQ0, IRQ1 interrupt sources
          //使能IRQ0，IRQ1中断源
          //设置为允许空闲中断请求（IdleIrqEn=1）
 
SR  09 40 //设置为允许全局中断请求
          
SR  00 0D // Execute Rc663 command: "Load protocol"
          //执行Rc663命令：“加载协议”

SLP 100

SR  08 00 //IRQ0的中断请求使能寄存器，所属中断全部不使能
SR  09 00 //IRQ1的中断请求使能寄存器，所属中断全部不使能

SR  02 B0 // Flush FIFO
          //设置为可清空FIFO缓冲器

//> Apply RegisterSet
SR  2C 7B //设置TxCrcPreset寄存器（发送CRC预置）
          //设置为CRC16
          //CRC的传输预设值为crc16->FFFFH
 
SR  2D 7B //设置RxCrcPreset寄存器（接收CRC预置）
          //设置为CRC16
          //CRC的传输预设值为crc16->FFFFH

/////////////以下为发送器配置寄存器/////////////////           
SR  2E 08 //TxDataNum寄存器  
          //设置bit3=1，数据将会被发送
        
SR  2F 00 //发射机数据调制宽带寄存器。
          //定义在脉冲调制时发送数据的脉冲长度。长度由载波时钟数+1来给定。
             
SR  30 00 //TxSym10BurstLen寄存器
          //如果协议需要一个突发信号（未调制的副载波）
          //其长度可由TxSymBurstLen来定义，值的高低可由TxSym10BurstCtrl定义。
          //此处0x00设置突发符号1数据位数为8位

SR  31 00 //TxWaitCtrl寄存器
          //设置停止位和发送的EGT（即停止位+额外保护时间EGT）
          //0x00是设置为无停止位，无EGT
          //注意：仅使用于ISO/IEC 14443 Type B

SR  33 0F //设置Symbol3作为停止符号发送
          //设置Symbol3作为开始符号发送  

SR  35 02 //设置为会收到在RxEOFSymbolReg中定义的结束符号
          //设置信号的传输速度为 26kbd

SR  37 4E //设置接收的MinLevel，（注意：MinLevel应高于系统噪声电平）
          //设置相移检测器单元的MinLevel

SR  39 04 //这个寄存器设置增益（rcv_gain）和高通转角频率（rcv_hpcf）
          //设置结果见图1

SR  36 8C // Set the RxWait register 
          //设置RxWait的时间为 RxWait x（0.5/DBFreq）
          //此时RxWait=0x0C

SR  31 C0 //设置TxWait的时间在数据接收结束时开始
          //设置TxWait的时间为TxWait x 16/13.56 MHz

SR  32 00 //设置接收和发送之间或2个发送数据流之间的最短时间

// Write Timer-0, Timer-1 reload values(high,low)
//写定时器0，定时器1重装值
SR  10 18 //设置定时器0预设值的高字节为0x18
SR  11 86 //设置定时器0预设值的低字节为0x86
SR  15 00 //设置定时器1预设值的高字节为0x00
SR  16 00 //设置定时器1预设值的低字节为0x00
SR  29 0A //TxAmp寄存器（发射放大）
          //设置TxAmp寄存器。TX1的电压将被上拉到可能的最大化
          //允许发射机输出振幅减少固定值。
          //设置4个从TVDD扣除掉的预设值：TVDD-100 mV    
          //设定剩余载波百分比
          //设置剩余载波和调制指数
          //剩余载波:83%，调制指数：9.3% 

SR  28 81 //设置TX2引脚翻转发射机2的信号
          //任何情况下，输出被拉至0。即拉低模式 

SR  0B 00 // Disable MIFARE Crypto1
          //关闭MIFARE加密 

//> =============================================
//> FieldOn
//> =============================================
SR  28 89 //设置TX2引脚翻转发射机2的信号
          //两个TX1和TX2引脚都使能
          //任何情况下，输出被拉至0。即拉低模式 
//> =============================================
//> ActivateCard  激活卡
//> =============================================

// Set short timeout. Timer-0,Timer-1 reload values(hi,lo) 
//写定时器0，定时器1重装值
SR  10 24 //设置定时器0预设值的高字节为0x24
SR  11 EB //设置定时器0预设值的低字节为0xEB
SR  15 00 //设置定时器1预设值的高字节为0x00
SR  16 00 //设置定时器1预设值的低字节为0x00

// Cancel any command execution.Flush FIFO. Clear IRQ0,IRQ1
SR  00 00 //停止任何命令

SR  02 B0 //FIFO大小为255字节
          //并设置为可清空FIFO缓冲器
SR  06 7F //在位6到位0写入1时清除一些中断请求
          //包括HiAlertIrq、LoQlertIrq、IdleIrq、TxIrq、RxIrq、ErrIrq、RxSOFIrq

SR  07 7F //在位5到位0写入1时清除该位的中断请求

// Write: "Flags" and "Inventory" cmd in FIFO 
//用于内部FIFO缓冲的数据输入与输出
//看663数据手册7.5章节-Buffer。
SR  05 36 
SR  05 01 
SR  05 00 
SR  05 00 

SR  00 07 // Execute RC663 "Transceive" command.
          //执行RC663“收发”命令。

// Wait until the command is finished
//等待命令完成
// Enable IRQ0,IRQ1 interrupt sources
// 使能IRQ0，IRQ1中断源
GR  08    
SR  08 18 //IRQ0的中断请求使能寄存器，允许空闲中断请求、允许发射器中断请求
GR  09  
SR  09 42 //IRQ1的中断请求使能寄存器，允许全局中断请求、允许定时器1中断请求

   ::: L_WaitTransceiveFinished
   GR  07    // Response:  60
   JNM IOR 40 40 L_WaitTransceiveFinished

// Disable IRQ0,IRQ1 interrupt sources 
// 使能IRQ0，IRQ1中断源
GR  08  
SR  08 00 //IRQ0的中断请求使能寄存器，所属中断请求全部不允许
GR  09  
SR  09 00 //IRQ1的中断请求使能寄存器，所属中断请求全部不允许

// Get IRQ0 status
//获取IRQ0 状态
GR  06   
GR  07   

//> Read FIFO, UID
//读取 FIFO, UID
GR  04   
GR  05   
GR  05  
GR  05   
GR  05   
GR  05   
GR  05   
GR  05   
GR  05  
GR  05    
GR  05    
GR  06    
GR  07    
GR  04    

// Read Error status register
//读取错误状态寄存器
GR  0A    // Response:  00

GR  2E    // Response:  08
SR  2E 08 
GR  0C    // Response:  80
GR  2E    // Response:  08
SR  2E 08 


//> =============================================
//> Apply Waiting time 
//> =============================================

SR  10 20 //设置定时器0预设值的高字节为0x20
SR  11 FF //设置定时器0预设值的低字节为0xFF
SR  15 00 //设置定时器1预设值的高字节为0x00
SR  16 00 //设置定时器1预设值的低字节为0x00

GR  0E    // Response:  00

// Clear all IRQ1 flags
//清除所有中断标志
SR  07 7F 



//> End of the script
//> .............................................

          
