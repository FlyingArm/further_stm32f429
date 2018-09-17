/*
*********************************************************************************************************
*
�ж����ȼ�˵����
*               NVIC���ȼ��������4������ռ���ȼ�����Χ0-15��
*               LTDC�жϣ�       	��ռ���ȼ� 		0  
*				DMA2D�ж�,		 	��ռ���ȼ� 		0
*				USART6�жϣ�     	��ռ���ȼ� 		1
*				USART7�жϣ�     	��ռ���ȼ� 		1
*               DMA2_Stream6�жϣ�	��ռ���ȼ� 		1
*               DMA2_Stream1�жϣ�	��ռ���ȼ� 		1
*				DMA1_Stream1�жϣ�	��ռ���ȼ� 		1
*				DMA1_Stream3�жϣ�	��ռ���ȼ� 		1
*				USART1�жϣ�     	��ռ���ȼ� 		2
*				USART2�жϣ�     	��ռ���ȼ� 		2
*				USART3�жϣ�     	��ռ���ȼ� 		2
*				USART4�жϣ�     	��ռ���ȼ� 		2
*				USART5�жϣ�     	��ռ���ȼ� 		2
*				EXTI1�жϣ�			��ռ���ȼ� 		3
*               
*****************************************************************************************               
*
*/


#include "includes.h"
/*
*********************************************************************************************************
*                                       ��̬ȫ�ֱ���
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
*                                      ��������
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
*                               ����
*******************************************************************************************************
*/
static  OS_SEM       AppPrintfSemp;	/* ����printf���� */
static  OS_SEM       SEM_SYNCH;	    /* ����ͬ�� */
static	OS_SEM		 WIFI_SEM; 		/* ���⣬������Կ*/
static	OS_SEM		 SENSOR_SEM; 	/* ���⣬��������������*/

OS_TMR 			 	 tmr1;			/* timer1 */
OS_TMR			 	 tmr2;			/* timer2 Response��ʱ */
OS_TMR			 	 tmr3;			/* timer3 POST���ݳ�ʱ */
OS_TMR			 	 tmr4;			/* timer4 GET para���ݳ�ʱ */
OS_TMR			 	 tmr5;			/* timer5 WIFIģ����Ƴ�ʱ��Ӳ������ */
OS_TMR			 	 tmr6;			/* USART3 ���ճ�ʱ */
OS_TMR			     tmr7;			/* touch key ��� */
OS_TMR			     tmr8;			/* touch ��� */


////////////////////////////////////////////////

void tmr_callback(void *p_tmr, void *p_arg); 	// timer�ص�����
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
*	�� �� ��: main
*	����˵��: ��׼c������ڡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int main(void)
{
    OS_ERR  err;  
	
	/* ��ʼ��uC/OS-III �ں� */
    OSInit(&err); 

	/* ����һ����������Ҳ���������񣩡���������ᴴ�����е�Ӧ�ó������� */
	OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,  /* ������ƿ��ַ */           
                 (CPU_CHAR     *)"App Task Start",  /* ������ */
                 (OS_TASK_PTR   )AppTaskStart,      /* ������������ַ */
                 (void         *)0,                 /* ���ݸ�����Ĳ��� */
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO, /* �������ȼ� */
                 (CPU_STK      *)&AppTaskStartStk[0],     /* ��ջ����ַ */
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE / 10, /* ��ջ������������ʾ��10%��Ϊ����� */
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,      /* ��ջ�ռ��С */
                 (OS_MSG_QTY    )0,  /* ������֧�ֽ��ܵ������Ϣ�� */
                 (OS_TICK       )0,  /* ����ʱ��Ƭ */
                 (void         *)0,  /* ��ջ�ռ��С */  
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
		
				 /*  �������£�
					OS_OPT_TASK_STK_CHK      ʹ�ܼ������ջ��ͳ������ջ���õĺ�δ�õ�
					OS_OPT_TASK_STK_CLR      �ڴ�������ʱ����������ջ
					OS_OPT_TASK_SAVE_FP      ���CPU�и���Ĵ��������������л�ʱ���渡��Ĵ���������
				 */  
                 (OS_ERR       *)&err);

	/* ����������ϵͳ������Ȩ����uC/OS-III */
    OSStart(&err);                                               
    
    (void)&err;
    
    return (0);
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskStart
*	����˵��: ����һ�����������ڶ�����ϵͳ�����󣬱����ʼ���δ����������������Ҫʵ�ְ����ʹ�����⡣
*	��    ��: p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ��: 2
*********************************************************************************************************
*/

void FT5X06_OnePiontScan(void);


static  void  AppTaskStart (void *p_arg)
{
	OS_ERR      err;
	uint8_t  ucCount = 0;
	uint8_t  ucCount1 = 0;
	
    /* �����ڱ���������澯����������������κ�Ŀ����� */	
    (void)p_arg;  
	
	/* BSP ��ʼ���� BSP = Board Support Package �弶֧�ְ����������Ϊ�ײ�������*/
	CPU_Init();  /* �˺���Ҫ���ȵ��ã���Ϊ����������ʹ�õ�us��ms�ӳ��ǻ��ڴ˺����� */
	bsp_Init();       
	BSP_Tick_Init();  
	
#if OS_CFG_STAT_TASK_EN > 0u
     OSStatTaskCPUUsageInit(&err);   
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
		
	/* ����Ӧ�ó�������� */
	AppTaskCreate();
	
	/* ��������ͨ�� */ 
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
*	�� �� ��: AppTaskUserIF
*	����˵��: ������Ϣ����
*	��    ��: p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ��: 4
*********************************************************************************************************
*/
extern void HardKeyProc(void);

static void AppTaskUserIF(void *p_arg)
{
	OS_ERR      err;	
	(void)p_arg;	              /* ������������� */

	App_Printf("\r\n...key...\r\n");
	
	while (1) 
	{ 
		HardKeyProc();

		OSTimeDly(100, OS_OPT_TIME_DLY, &err);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCom
*	����˵��: ��δʹ��
*	��    ��: p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ��: 5
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
*	�� �� ��: AppTaskGUI
*	����˵��: GUI����������ȼ�		  			  
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ����OS_CFG_PRIO_MAX - 4u
*********************************************************************************************************
*/
extern void SmartPhomeMenuMainTask(void);

static void AppTaskGUI(void *p_arg)
{
	OS_ERR  err; 
    (void)p_arg;		/* ����������澯 */

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
*	�� �� ��: AppTaskWIFI
*	����˵��: WIFI	  			  
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ����OS_CFG_PRIO_MAX - 4u
*********************************************************************************************************
*/

static void AppTaskWIFI(void *p_arg)
{

}

/*
*********************************************************************************************************
*	�� �� ��: BLE����
*	����˵��: ����Ӧ������
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*********************************************************************************************************
*/

static void AppTaskBLE(void *p_arg)
{
	
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static  void  AppTaskCreate (void)
{
	OS_ERR      err;

	
	/**************����USER IF����*********************/
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
	/**************����COM����*********************/
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
	/**************����GUI����*********************/			 
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
	/**************����WIFI����*********************/			 
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
	
	/**************����BLE����*********************/	
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
*	�� �� ��: AppObjCreate
*	����˵��: ��������ͨѶ
*	��    ��: p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static  void  AppObjCreate (void)
{
	OS_ERR      err;

	/* 
	   �����ź�����ֵΪ1��ʱ�����ʵ�ֻ��⹦�ܣ�Ҳ����ֻ��һ����Դ����ʹ�� 
	   �������ǽ�����1�Ĵ�ӡ������Ϊ��������Դ����ֹ���ڴ�ӡ��ʱ������������ռ
	   ��ɴ��ڴ�ӡ���ҡ�
	*/
	OSSemCreate((OS_SEM    *)&AppPrintfSemp,
				(CPU_CHAR  *)"AppPrintfSemp",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	/* ������Կ */
	OSSemCreate((OS_SEM    *)&WIFI_SEM,
				(CPU_CHAR  *)"WIFI_SEM",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	
	/* �������������� */
	OSSemCreate((OS_SEM    *)&SENSOR_SEM,
				(CPU_CHAR  *)"SENSOR_SEM",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	
	/* ��������ֵΪ0������ʵ������ͬ������ */
	OSSemCreate((OS_SEM    *)&SEM_SYNCH,
				(CPU_CHAR  *)"SEM_SYNCH",
				(OS_SEM_CTR )0,
				(OS_ERR    *)&err);

	#if 0
	OSTmrCreate((OS_TMR		*)&tmr1,		
                (CPU_CHAR	*)"tmr1",		
                (OS_TICK	 )3000,		// delay 3s ATָ����Ӧ��ʱʱ��
                (OS_TICK	 )0,          
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 
                (OS_TMR_CALLBACK_PTR)tmr_callback,
                (void	    *)0,			
                (OS_ERR	    *)&err);						

	OSTmrCreate((OS_TMR		*)&tmr2,		
                (CPU_CHAR	*)"tmr2",		
                (OS_TICK	 )30*1000,	// delay 30s	http��Ӧ��ʱʱ��		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr2_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);	

	OSTmrCreate((OS_TMR		*)&tmr3,		
                (CPU_CHAR	*)"tmr3",		
                (OS_TICK	 )40*1000,	// delay 40s	postʱ����		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr3_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);	

	OSTmrCreate((OS_TMR		*)&tmr4,		
                (CPU_CHAR	*)"tmr4",		
                (OS_TICK	 )10*1000,	// delay 10s	get paraʱ����		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr4_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);	
	OSTmrCreate((OS_TMR		*)&tmr5,		
                (CPU_CHAR	*)"tmr5",		
                (OS_TICK	 )50*1000,	// delay 50s	����ʱ����		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr5_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);
	
	#endif
}

/*
*********************************************************************************************************
*	�� �� ��: App_Printf
*	����˵��: �̰߳�ȫ��printf��ʽ		  			  
*	��    ��: ͬprintf�Ĳ�����
*             ��C�У����޷��г����ݺ���������ʵ�ε����ͺ���Ŀʱ,������ʡ�Ժ�ָ��������
*	�� �� ֵ: ��
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

	/* ������� */
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
*	�� �� ��: DispTaskInfo
*	����˵��: ��uCOS-III������Ϣͨ�����ڴ�ӡ����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispTaskInfo(void)
{
	OS_TCB      *p_tcb;	        /* ����һ��������ƿ�ָ��, TCB = TASK CONTROL BLOCK */
	float CPU = 0.0f;
	CPU_SR_ALLOC();

	CPU_CRITICAL_ENTER();
    p_tcb = OSTaskDbgListPtr;
    CPU_CRITICAL_EXIT();
	
	/* ��ӡ���� */
	App_Printf("===============================================================\r\n");
	App_Printf(" ���ȼ� ʹ��ջ ʣ��ջ �ٷֱ� ������   ������\r\n");
	App_Printf("  Prio   Used  Free   Per    CPU     Taskname\r\n");

	/* ����������ƿ��б�(TCB list)����ӡ���е���������ȼ������� */
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

	OSTmrStart(&tmr6,&err); 	// ������ʱ��6	
}

void  Uart3RecTimerStop (void)
{
	OS_ERR         err;

	OSTmrStop(&tmr6,OS_OPT_TMR_NONE,0,&err); 	// ֹͣ��ʱ��6	
}
#endif


/***************************** Cubic (END OF FILE) *********************************/
