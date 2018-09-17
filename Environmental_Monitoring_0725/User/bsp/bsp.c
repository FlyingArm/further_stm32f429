/*
*********************************************************************************************************

*
*********************************************************************************************************
*/
#include "includes.h"
#include "tty.h"

/*
*********************************************************************************************************
*	函 数 名: bsp_Init
*	功能说明: 初始化所有的硬件设备。该函数配置CPU寄存器和外设的寄存器并初始化一些全局变量。只需要调用一次
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/

void bsp_Init(void)
{
	//unsigned int DeviceID = 0;
	//unsigned int FlashID = 0;
	/*
		由于ST固件库的启动文件已经执行了CPU系统时钟的初始化，所以不必再次重复配置系统时钟。
		启动文件配置了CPU主时钟频率、内部Flash访问速度和可选的外部SRAM FSMC初始化。

		系统时钟缺省配置为168MHz，如果需要更改，可以修改 system_stm32f4xx.c 文件(已调整为180MHz)
	*/
	/* 使能CRC 因为使用STemWin前必须要使能 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
	
	/* 优先级分组设置为4，可配置0-15级抢占式优先级，0级子优先级，即不存在子优先级。*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	SystemCoreClockUpdate();	/* 根据PLL配置更新系统时钟频率变量 SystemCoreClock */

	bsp_InitUart(); 	/* 初始化串口 DMA */
	tty.init();
	
	#if 0
	SPI_FLASH_Init(); /* 初始化SPI_FLASH */
	DeviceID = SPI_FLASH_ReadDeviceID();	/* 获取 Flash Device ID */
	FlashID = SPI_FLASH_ReadID();	/* 获取 SPI Flash ID */
	printf("\r\nFlashID is 0x%X, Manufacturer Device ID is 0x%X\r\n", FlashID, DeviceID);
	#else
	bsp_InitSPIBus();
	bsp_InitSFlash(); // 未加Flash芯片，暂不启用
	printf("\r\nFlashID is 0x%X, Manufacturer Device ID is %s\r\n", g_tSF.ChipID, g_tSF.ChipName);
	#endif

	/* Initialize the SDRAM */
	bsp_InitExtSDRAM();
	/* 配置I2C总线 */
	bsp_InitI2C();			
	
	GT911_InitHard();
	
	//bsp_DetectLcdType();    /* 检测触摸板和LCD面板型号, 结果存在全局变量 g_TouchType, g_LcdType */
	//TOUCH_InitHard();	    /* 初始化配置触摸芯片 */

	InitTouchKey();
	/* 初始化RTC */
	TM_RTC_Init(TM_RTC_ClockSource_External); 

	//TM_USB_VCP_Init();
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DelayMS
*	功能说明: 为了让底层驱动在带RTOS和裸机情况下有更好的兼容性
*             专门制作一个阻塞式的延迟函数，在底层驱动中ms毫秒延迟主要用于初始化，并不会影响实时性。
*	形    参: n 延迟长度，单位1 ms
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_DelayMS(uint32_t _ulDelayTime)
{
	bsp_DelayUS(1000*_ulDelayTime);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DelayUS
*	功能说明: 这里的延时采用CPU的内部计数实现，32位计数器
*             	OSSchedLock(&err);
*				bsp_DelayUS(5);
*				OSSchedUnlock(&err); 根据实际情况看看是否需要加调度锁或选择关中断
*	形    参: _ulDelayTime  延迟长度，单位1 us
*	返 回 值: 无
*   说    明: 1. 主频168MHz的情况下，32位计数器计满是2^32/168000000 = 25.565秒
*                建议使用本函数做延迟的话，延迟在1秒以下。  
*             2. 实际通过示波器测试，微妙延迟函数比实际设置实际多运行0.25us左右的时间。
*             下面数据测试条件：
*             （1）. MDK5.15，优化等级0, 不同的MDK优化等级对其没有影响。
*             （2）. STM32F407IGT6
*             （3）. 测试方法：
*				 GPIOI->BSRRL = GPIO_Pin_8;
*				 bsp_DelayUS(10);
*				 GPIOI->BSRRH = GPIO_Pin_8;
*             -------------------------------------------
*                测试                 实际执行
*             bsp_DelayUS(1)          1.2360us
*             bsp_DelayUS(2)          2.256us
*             bsp_DelayUS(3)          3.256us
*             bsp_DelayUS(4)          4.256us
*             bsp_DelayUS(5)          5.276us
*             bsp_DelayUS(6)          6.276us
*             bsp_DelayUS(7)          7.276us
*             bsp_DelayUS(8)          8.276us
*             bsp_DelayUS(9)          9.276us
*             bsp_DelayUS(10)         10.28us
*            3. 两个32位无符号数相减，获取的结果再赋值给32位无符号数依然可以正确的获取差值。
*              假如A,B,C都是32位无符号数。
*              如果A > B  那么A - B = C，这个很好理解，完全没有问题
*              如果A < B  那么A - B = C， C的数值就是0xFFFFFFFF - B + A + 1。这一点要特别注意，正好用于本函数。
*********************************************************************************************************
*/
void bsp_DelayUS(uint32_t _ulDelayTime)
{
    uint32_t tCnt, tDelayCnt;
	uint32_t tStart;
		
	tStart = (uint32_t)CPU_TS_TmrRd();                       /* 刚进入时的计数器值 */
	tCnt = 0;
	tDelayCnt = _ulDelayTime * (SystemCoreClock / 1000000);	 /* 需要的节拍数 */ 		      

	while(tCnt < tDelayCnt)
	{
		tCnt = (uint32_t)CPU_TS_TmrRd() - tStart; /* 求减过程中，如果发生第一次32位计数器重新计数，依然可以正确计算 */	
	}
}

/*
*********************************************************************************************************
*	函 数 名: BSP_CPU_ClkFreq
*	功能说明: 获取系统时钟，uCOS-II需要使用
*	形    参: 无
*	返 回 值: 系统时钟
*********************************************************************************************************
*/
CPU_INT32U  BSP_CPU_ClkFreq (void)
{
    RCC_ClocksTypeDef  rcc_clocks;

    RCC_GetClocksFreq(&rcc_clocks);
    return ((CPU_INT32U)rcc_clocks.HCLK_Frequency);
}

/*
*********************************************************************************************************
*	函 数 名: BSP_Tick_Init
*	功能说明: 初始化系统滴答时钟做为uCOS-II的系统时钟节拍，1ms一次
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void BSP_Tick_Init (void)
{
    CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    
    cpu_clk_freq = BSP_CPU_ClkFreq();                           /* 获取系统时钟  */
    
#if (OS_VERSION >= 30000u)
    cnts  = cpu_clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;     
#else
    cnts  = cpu_clk_freq / (CPU_INT32U)OS_TICKS_PER_SEC;        /* 获得滴答定时器的参数  */
#endif
    
	OS_CPU_SysTickInit(cnts);                                   /* 这里默认的是最低优先级            */
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
