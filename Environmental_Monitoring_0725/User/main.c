/*
*********************************************************************************************************
*
中断优先级说明：
*               NVIC优先级分组采用4，仅抢占优先级，范围0-15。
*               LTDC中断，       	抢占优先级 		0  
*				DMA2D中断,		 	抢占优先级 		0
*				USART6中断，     	抢占优先级 		1
*				USART7中断，     	抢占优先级 		1
*               DMA2_Stream6中断，	抢占优先级 		1
*               DMA2_Stream1中断，	抢占优先级 		1
*				DMA1_Stream1中断，	抢占优先级 		1
*				DMA1_Stream3中断，	抢占优先级 		1
*				USART1中断，     	抢占优先级 		2
*				USART2中断，     	抢占优先级 		2
*				USART3中断，     	抢占优先级 		2
*				USART4中断，     	抢占优先级 		2
*				USART5中断，     	抢占优先级 		2
*				EXTI1中断，			抢占优先级 		3
*               
*****************************************************************************************               
*
*/


#include "includes.h"
/*
*********************************************************************************************************
*                                       静态全局变量
*********************************************************************************************************
*/                                                          
static  OS_TCB   AppTaskStartTCB;
static  CPU_STK  AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB   AppTaskMsgProTCB;
static  CPU_STK  AppTaskMsgProStk[APP_CFG_TASK_MsgPro_STK_SIZE];

static  OS_TCB   AppTaskCOMTCB;
static  CPU_STK  AppTaskCOMStk[APP_CFG_TASK_COM_STK_SIZE];

static  OS_TCB   AppTaskUserIFTCB;
static  CPU_STK  AppTaskUserIFStk[APP_CFG_TASK_USER_IF_STK_SIZE];

static  OS_TCB   AppTaskGUITCB;
static  CPU_STK  AppTaskGUIStk[APP_CFG_TASK_GUI_STK_SIZE];

static  OS_TCB   AppTaskWIFITCB;
static  CPU_STK  AppTaskWIFIStk[APP_CFG_TASK_USER_WIFI_STK_SIZE];

static  OS_TCB   AppTaskBLETCB;
static  CPU_STK  AppTaskBLEStk[APP_CFG_TASK_USER_BLE_STK_SIZE];

/*
*********************************************************************************************************
*                                      函数声明
*********************************************************************************************************
*/
static  void  AppTaskStart       	(void     *p_arg);
static  void  AppTaskMsgPro    (void     *p_arg);
static  void  AppTaskUserIF   	(void     *p_arg);
static  void  AppTaskCOM		(void 	  *p_arg);
static  void  AppTaskWIFI		(void 	  *p_arg);
static void   AppTaskBLE			(void 	  *p_arg);
static  void  AppTaskCreate      (void);
static  void  DispTaskInfo          (void);
static  void  AppObjCreate          (void);
static  void  App_Printf (CPU_CHAR *format, ...);

/*
*******************************************************************************************************
*                               变量
*******************************************************************************************************
*/
static  OS_SEM       AppPrintfSemp;	/* 用于printf互斥 */
static  OS_SEM       SEM_SYNCH;	    /* 用于同步 */
static	OS_SEM		 WIFI_SEM; 		/* 互斥，保护密钥*/
static	OS_SEM		 SENSOR_SEM; 	/* 互斥，保护传感器数据*/

OS_TMR 			 	 tmr1;			/* timer1 */
OS_TMR			 	 tmr2;			/* timer2 Response超时 */
OS_TMR			 	 tmr3;			/* timer3 POST数据超时 */
OS_TMR			 	 tmr4;			/* timer4 GET para数据超时 */
OS_TMR			 	 tmr5;			/* timer5 WIFI模块控制超时，硬件重启 */
OS_TMR			 	 tmr6;			/* USART3 接收超时 */
OS_TMR			     tmr7;			/* touch key 检测 */
OS_TMR			     tmr8;			/* touch 检测 */


////////////////////////////////////////////////

void tmr_callback(void *p_tmr, void *p_arg); 	// timer回调函数
void tmr2_callback(void *p_tmr,void *p_arg);
void tmr3_callback(void *p_tmr,void *p_arg);
void tmr4_callback(void *p_tmr,void *p_arg);
void tmr5_callback(void *p_tmr,void *p_arg);
void tmr7_callback(void *p_tmr,void *p_arg);
void tmr8_callback(void *p_tmr,void *p_arg);


#define nReset_Low  GPIO_ResetBits(GPIOC, GPIO_Pin_1)
#define nReset_High GPIO_SetBits(GPIOC, GPIO_Pin_1)

#define nReload_Low  GPIO_ResetBits(GPIOF, GPIO_Pin_9)
#define nReload_High GPIO_SetBits(GPIOF, GPIO_Pin_9)

extern WIFI_CTR WiFi_State;
extern Bluetooth_Login Bluetooth_State;

typedef enum
{ TIMER_SWITCH_ON,TIMER_SWITCH_OFF}Timer_State;

static char sw_version[16] = "V0.0.9"; 

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
    OS_ERR  err;  
	
	/* 初始化uC/OS-III 内核 */
    OSInit(&err); 

	/* 创建一个启动任务（也就是主任务）。启动任务会创建所有的应用程序任务 */
	OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,  /* 任务控制块地址 */           
                 (CPU_CHAR     *)"App Task Start",  /* 任务名 */
                 (OS_TASK_PTR   )AppTaskStart,      /* 启动任务函数地址 */
                 (void         *)0,                 /* 传递给任务的参数 */
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO, /* 任务优先级 */
                 (CPU_STK      *)&AppTaskStartStk[0],     /* 堆栈基地址 */
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE / 10, /* 堆栈监测区，这里表示后10%作为监测区 */
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,      /* 堆栈空间大小 */
                 (OS_MSG_QTY    )0,  /* 本任务支持接受的最大消息数 */
                 (OS_TICK       )0,  /* 设置时间片 */
                 (void         *)0,  /* 堆栈空间大小 */  
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		
				 /*  定义如下：
					OS_OPT_TASK_STK_CHK      使能检测任务栈，统计任务栈已用的和未用的
					OS_OPT_TASK_STK_CLR      在创建任务时，清零任务栈
					OS_OPT_TASK_SAVE_FP      如果CPU有浮点寄存器，则在任务切换时保存浮点寄存器的内容
				 */  
                 (OS_ERR       *)&err);

	/* 启动多任务系统，控制权交给uC/OS-III */
    OSStart(&err);                                               
    
    (void)&err;
    
    return (0);
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskStart
*	功能说明: 这是一个启动任务，在多任务系统启动后，必须初始化滴答计数器。本任务主要实现按键和触摸检测。
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级: 2
*********************************************************************************************************
*/

void FT5X06_OnePiontScan(void);


static  void  AppTaskStart (void *p_arg)
{
	OS_ERR      err;
	uint8_t  ucCount = 0;
	uint8_t  ucCount1 = 0;
	
    /* 仅用于避免编译器告警，编译器不会产生任何目标代码 */	
    (void)p_arg;  
	
	/* BSP 初始化。 BSP = Board Support Package 板级支持包，可以理解为底层驱动。*/
	CPU_Init();  /* 此函数要优先调用，因为外设驱动中使用的us和ms延迟是基于此函数的 */
	bsp_Init();       
	BSP_Tick_Init();  
	
#if OS_CFG_STAT_TASK_EN > 0u
     OSStatTaskCPUUsageInit(&err);   
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
		
	/* 创建应用程序的任务 */
	AppTaskCreate();
	
	/* 创建任务通信 */ 
	AppObjCreate();
	
  while(1)
	{
		//APP_TRACE_DBG(("cpu usage:%.2f%%\r\n",((float)OSStatTaskCPUUsage/100)));

		//FT5X06_OnePiontScan();
		
		OSTimeDly(2000, OS_OPT_TIME_DLY, &err);
	}    
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskUserIF
*	功能说明: 按键消息处理
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级: 4
*********************************************************************************************************
*/
extern void HardKeyProc(void);

static void AppTaskUserIF(void *p_arg)
{
	OS_ERR      err;	
	(void)p_arg;	              /* 避免编译器报警 */

	App_Printf("\r\n...key...\r\n");
	
	while (1) 
	{ 
		HardKeyProc();

		OSTimeDly(100, OS_OPT_TIME_DLY, &err);
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCom
*	功能说明: 暂未使用
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级: 5
*********************************************************************************************************
*/
//extern void sensor_measure(void);

 extern DEV_STATE_CTR_T dev_state;

 static void AppTaskCOM(void *p_arg)
 {
	OS_ERR  err;

	static unsigned char cgk_CO2_cout = 0;
	static unsigned char cgk_PM_cout = 0;
	 
	(void)p_arg;
	
	bsp_InitI2CCgk();
	 
	App_Printf("\r\n...sensor...\r\n");
	
	while(1)
	{	
		if(dev_state.IsPM25Pressed)
			cgk_PM_cout++;
		
		if(dev_state.IsCO2Pressed)
		{
			cgk_CO2_cout++;

			if(g_CgkSensorData.uiNowCmdStatus == CO2MEASURE && (cgk_CO2_cout % 8 == 0) ) {
				cgk_sendcmd(ELECMEASURE);
			}
			else if(g_CgkSensorData.uiNowCmdStatus == ELECMEASURE && (cgk_CO2_cout % 8 == 0) )  {
				cgk_sendcmd(CO2MEASURE);
			}
		}

		if(dev_state.IsPM25Pressed && cgk_PM_cout > 40u)
		{
			dev_state.IsPM25Pressed = 0;
			cgk_PM_cout = 0;
			cgk_sendcmd(PMCLOSE);
		}
		
		if(dev_state.IsCO2Pressed && cgk_CO2_cout > 30u)
		{
			dev_state.IsCO2Pressed = 0;
			cgk_CO2_cout = 0;
			cgk_sendcmd(CO2CLOSE);
		}
		
	  if(dev_state.IsCO2Pressed && dev_state.CO2PressedCtr == 0)
		{
			cgk_sendcmd(CO2OPEN);
			OSTimeDly(100, OS_OPT_TIME_DLY, &err);
			cgk_sendcmd(CO2MEASURE);
			OSTimeDly(100, OS_OPT_TIME_DLY, &err);

			dev_state.CO2PressedCtr = 1;
		}
		
		if(dev_state.IsPM25Pressed && dev_state.PM25PressedCtr == 0)
		{
			cgk_sendcmd(PMMEASURE);
			OSTimeDly(100, OS_OPT_TIME_DLY, &err);

			dev_state.PM25PressedCtr = 1;
		}
		
		switch(g_CgkSensorData.uiNowCmdStatus) {
			
			case 0x01: 
				
				CgkElecDataCollection(); 
				
				break;
			case 0x05: 

				CgkPMDataCollection(); 
				
				break;
			case 0x06: 

				CgkCO2DataCollection(); 
				
				break;
			default:  				

				break;
		}
		
	  OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
	} 						  	 	       											   
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskGUI
*	功能说明: GUI任务，最低优先级		  			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级：OS_CFG_PRIO_MAX - 4u
*********************************************************************************************************
*/
extern void SmartPhomeMenuMainTask(void);

static void AppTaskGUI(void *p_arg)
{
	OS_ERR  err; 
    (void)p_arg;		/* 避免编译器告警 */

	App_Printf("\r\n...emWin...\r\n");

	OSTmrCreate((OS_TMR		*)&tmr8,		
                (CPU_CHAR	*)"touch", 		
                (OS_TICK	 )10,	
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr8_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);
	OSTmrStart(&tmr8,&err); 				 // start tmr8

	OSTmrCreate((OS_TMR		*)&tmr7,		
                (CPU_CHAR	*)"key",		
                (OS_TICK	 )10,	
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr7_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);
	OSTmrStart(&tmr7,&err); 				 // start tmr7
	
	while (1) 
	{
	  SmartPhomeMenuMainTask();
	  OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskWIFI
*	功能说明: WIFI	  			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级：OS_CFG_PRIO_MAX - 4u
*********************************************************************************************************
*/

static void AppTaskWIFI(void *p_arg)
{

}

/*
*********************************************************************************************************
*	函 数 名: BLE任务
*	功能说明: 创建应用任务
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/

static void AppTaskBLE(void *p_arg)
{
	
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  AppTaskCreate (void)
{
	OS_ERR      err;

	
	/**************创建USER IF任务*********************/
	#if 0
	OSTaskCreate((OS_TCB       *)&AppTaskUserIFTCB,             
                 (CPU_CHAR     *)"App Task UserIF",
                 (OS_TASK_PTR   )AppTaskUserIF, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_USER_IF_PRIO,
                 (CPU_STK      *)&AppTaskUserIFStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_USER_IF_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_USER_IF_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_SAVE_FP | OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	#endif
	#if 1
	/**************创建COM任务*********************/
	OSTaskCreate((OS_TCB       *)&AppTaskCOMTCB,            
                 (CPU_CHAR     *)"App Task COM",
                 (OS_TASK_PTR   )AppTaskCOM,    
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_COM_PRIO,
                 (CPU_STK      *)&AppTaskCOMStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_COM_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_COM_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);	
	
	#endif
	#if 1
	/**************创建GUI任务*********************/			 
	OSTaskCreate((OS_TCB       *)&AppTaskGUITCB,              
                 (CPU_CHAR     *)"App Task GUI",
                 (OS_TASK_PTR   )AppTaskGUI, 
                  (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_GUI_PRIO,
                 (CPU_STK      *)&AppTaskGUIStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_GUI_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_GUI_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 (OS_ERR       *)&err);	

	#endif
	#if 0
	/**************创建WIFI任务*********************/			 
	OSTaskCreate((OS_TCB	   *)&AppTaskWIFITCB,			  
				 (CPU_CHAR	   *)"App Task WIFI",
				 (OS_TASK_PTR	)AppTaskWIFI, 
				 (void		   *)0,
				 (OS_PRIO		)APP_CFG_TASK_WIFI_PRIO,
				 (CPU_STK	   *)&AppTaskWIFIStk[0],
				 (CPU_STK_SIZE	)APP_CFG_TASK_USER_WIFI_STK_SIZE / 10,
				 (CPU_STK_SIZE	)APP_CFG_TASK_USER_WIFI_STK_SIZE,
				 (OS_MSG_QTY	)0,
				 (OS_TICK		)0,
				 (void		   *)0,
				 (OS_OPT		)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR	   *)&err); 
	
	/**************创建BLE任务*********************/	
	OSTaskCreate((OS_TCB	   *)&AppTaskBLETCB,			  
				 (CPU_CHAR	   *)"App Task BLE",
				 (OS_TASK_PTR	)AppTaskBLE, 
				 (void		   *)0,
				 (OS_PRIO		)APP_CFG_TASK_BLE_PRIO,
				 (CPU_STK	   *)&AppTaskBLEStk[0],
				 (CPU_STK_SIZE	)APP_CFG_TASK_USER_BLE_STK_SIZE / 10,
				 (CPU_STK_SIZE	)APP_CFG_TASK_USER_BLE_STK_SIZE,
				 (OS_MSG_QTY	)0,
				 (OS_TICK		)0,
				 (void		   *)0,
				 (OS_OPT		)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR	   *)&err); 
	#endif
}

/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通讯
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  AppObjCreate (void)
{
	OS_ERR      err;

	/* 
	   创建信号量数值为1的时候可以实现互斥功能，也就是只有一个资源可以使用 
	   本例程是将串口1的打印函数作为保护的资源。防止串口打印的时候被其它任务抢占
	   造成串口打印错乱。
	*/
	OSSemCreate((OS_SEM    *)&AppPrintfSemp,
				(CPU_CHAR  *)"AppPrintfSemp",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	/* 保护密钥 */
	OSSemCreate((OS_SEM    *)&WIFI_SEM,
				(CPU_CHAR  *)"WIFI_SEM",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	
	/* 保护传感器数据 */
	OSSemCreate((OS_SEM    *)&SENSOR_SEM,
				(CPU_CHAR  *)"SENSOR_SEM",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	
	/* 创建计数值为0，用于实现任务同步功能 */
	OSSemCreate((OS_SEM    *)&SEM_SYNCH,
				(CPU_CHAR  *)"SEM_SYNCH",
				(OS_SEM_CTR )0,
				(OS_ERR    *)&err);

	#if 0
	OSTmrCreate((OS_TMR		*)&tmr1,		
                (CPU_CHAR	*)"tmr1",		
                (OS_TICK	 )3000,		// delay 3s AT指令响应超时时间
                (OS_TICK	 )0,          
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 
                (OS_TMR_CALLBACK_PTR)tmr_callback,
                (void	    *)0,			
                (OS_ERR	    *)&err);						

	OSTmrCreate((OS_TMR		*)&tmr2,		
                (CPU_CHAR	*)"tmr2",		
                (OS_TICK	 )30*1000,	// delay 30s	http响应超时时间		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr2_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);	

	OSTmrCreate((OS_TMR		*)&tmr3,		
                (CPU_CHAR	*)"tmr3",		
                (OS_TICK	 )40*1000,	// delay 40s	post时间间隔		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr3_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);	

	OSTmrCreate((OS_TMR		*)&tmr4,		
                (CPU_CHAR	*)"tmr4",		
                (OS_TICK	 )10*1000,	// delay 10s	get para时间间隔		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr4_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);	
	OSTmrCreate((OS_TMR		*)&tmr5,		
                (CPU_CHAR	*)"tmr5",		
                (OS_TICK	 )50*1000,	// delay 50s	重启时间间隔		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr5_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);
	
	#endif
}

/*
*********************************************************************************************************
*	函 数 名: App_Printf
*	功能说明: 线程安全的printf方式		  			  
*	形    参: 同printf的参数。
*             在C中，当无法列出传递函数的所有实参的类型和数目时,可以用省略号指定参数表
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  App_Printf(CPU_CHAR *format, ...)
{
    CPU_CHAR  buf_str[80 + 1];
    va_list   v_args;
    OS_ERR    os_err;


    va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

	/* 互斥操作 */
    OSSemPend((OS_SEM  *)&AppPrintfSemp,
              (OS_TICK  )0u,
              (OS_OPT   )OS_OPT_PEND_BLOCKING,
              (CPU_TS  *)0,
              (OS_ERR  *)&os_err);

    printf("%s", buf_str);

   (void)OSSemPost((OS_SEM  *)&AppPrintfSemp,
                   (OS_OPT   )OS_OPT_POST_1,
                   (OS_ERR  *)&os_err);

}

/*
*********************************************************************************************************
*	函 数 名: DispTaskInfo
*	功能说明: 将uCOS-III任务信息通过串口打印出来
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispTaskInfo(void)
{
	OS_TCB      *p_tcb;	        /* 定义一个任务控制块指针, TCB = TASK CONTROL BLOCK */
	float CPU = 0.0f;
	CPU_SR_ALLOC();

	CPU_CRITICAL_ENTER();
    p_tcb = OSTaskDbgListPtr;
    CPU_CRITICAL_EXIT();
	
	/* 打印标题 */
	App_Printf("===============================================================\r\n");
	App_Printf(" 优先级 使用栈 剩余栈 百分比 利用率   任务名\r\n");
	App_Printf("  Prio   Used  Free   Per    CPU     Taskname\r\n");

	/* 遍历任务控制块列表(TCB list)，打印所有的任务的优先级和名称 */
	while (p_tcb != (OS_TCB *)0) 
	{
		CPU = (float)p_tcb->CPUUsage / 100;
		App_Printf("   %2d  %5d  %5d   %02d%%   %5.2f%%   %s\r\n", 
		p_tcb->Prio, 
		p_tcb->StkUsed, 
		p_tcb->StkFree, 
		(p_tcb->StkUsed * 100) / (p_tcb->StkUsed + p_tcb->StkFree),
		CPU,
		p_tcb->NamePtr);		
	 	
		CPU_CRITICAL_ENTER();
        p_tcb = p_tcb->DbgNextPtr;
        CPU_CRITICAL_EXIT();
	}
}

void tmr_callback(void *p_tmr,void *p_arg)
{

}

void tmr2_callback(void *p_tmr,void *p_arg)
{

}

void tmr3_callback(void *p_tmr,void *p_arg)
{

}

void tmr4_callback(void *p_tmr,void *p_arg)
{

}

extern AzureState_MODE  AzureState;

void tmr5_callback(void *p_tmr,void *p_arg)
{

}

void tmr7_callback(void *p_tmr,void *p_arg)
{
	OS_ERR    err;

	HardKeyProc();

	OSTmrStart(&tmr7,&err);
}

void tmr8_callback(void *p_tmr,void *p_arg)
{
	OS_ERR    err;

	GT911_OnePiontScan();

	OSTmrStart(&tmr8,&err);
}


/*
*********************************************************************************************************
*                                          POST TASK
*********************************************************************************************************
*/
void  WIFITaskPend ( void)
{
	OS_ERR      err;

	OSSemPend((OS_SEM  *)&WIFI_SEM,
              (OS_TICK  )1000u,
              (OS_OPT   )OS_OPT_PEND_BLOCKING,
              (CPU_TS  *)0,
              (OS_ERR  *)&err);            
}

void  SENSORTaskPend ( void)
{
	OS_ERR      err;

	OSSemPend((OS_SEM  *)&SENSOR_SEM,
              (OS_TICK  )1000u,
              (OS_OPT   )OS_OPT_PEND_BLOCKING,
              (CPU_TS  *)0,
              (OS_ERR  *)&err);            
}

/*
*********************************************************************************************************
*                                          PEND TASK
*********************************************************************************************************
*/
void  WIFITaskPost (void)
{
	OS_ERR         err;

	(void)OSSemPost((OS_SEM  *)&WIFI_SEM,
                  	 (OS_OPT   )OS_OPT_POST_1,
                  	 (OS_ERR  *)&err); 
	
}

void  SENSORTaskPost (void)
{
	OS_ERR         err;

	(void)OSSemPost((OS_SEM  *)&SENSOR_SEM,
                  	 (OS_OPT   )OS_OPT_POST_1,
                  	 (OS_ERR  *)&err); 
	
}

#if 0
void  Uart3RecTimerStart (void)
{
	OS_ERR         err;

	WiFi_State.UartRecTimeout = 0;

	OSTmrStart(&tmr6,&err); 	// 启动定时器6	
}

void  Uart3RecTimerStop (void)
{
	OS_ERR         err;

	OSTmrStop(&tmr6,OS_OPT_TMR_NONE,0,&err); 	// 停止定时器6	
}
#endif


/***************************** Cubic (END OF FILE) *********************************/
