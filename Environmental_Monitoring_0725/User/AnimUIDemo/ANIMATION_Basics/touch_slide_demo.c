/*
*********************************************************************************************************
*	                                  
*	ģ������ : GUI��������
*	�ļ����� : MainTask.c
*	��    �� : V1.0
*	˵    �� : ���洰�����潨��JPG����ͼƬ������һ��֧����������С��motion���ڣ�����ʵ��������������л���
*              ÿ������С��λ�ô���һ��ICONVIEW�ؼ���������������
*              1. ��������21��Ӧ��ͼ�꣬ʵ������13��������8����ʱδ���������������8��ͼ�����һ����ǰ
*                 8��ͼ����һ���ģ������ڲ���Ŀ�ġ�
*              2. ������ʱ�������ͼ����������򻬶�����Ȼ���Ե��ͼ���������л������������Ļ������á�
*              3. ״̬�����ļ�App_StatusDlg.c��ͷ��˵����ϵͳ��Ϣ�����ļ�App_SysInfoDlg.c��ͷ��˵����
*
*
*********************************************************************************************************
*/
#include "includes.h"
#include "touch_slide_demo.h"
#include "main_house.h"

/*
*********************************************************************************************************
*	                                  ���ڱ��ļ��ĵ���
*********************************************************************************************************
*/
#if 1
	#define printf_main printf
#else
	#define printf_main(...)
#endif


/*
*********************************************************************************************************
*                                         �궨�� 
*  ICONVIEW�����沼�� 800*480��ÿ��ͼ��̫С��72*72
*  1. ͼ��Ŀ��λ100���ء�
*  2. ICONVIEW�ؼ������������߽�ľ��붼��38�����ء�
*  3. ICONVIEW�ؼ��ඥ�߽���10�����ء�
*  4. ͼ����ͼ��֮���X������4�����ء�
*  5. ͼ����ͼ��֮���Y������5�����ء�
* 
*  |-----------------------------------800------------------------------|---
*  |    | 100|   | 100|   | 100|   | 100|   | 100|   | 100|   | 100|    | 10                                                          |
*  |-38-|-ͼ-|-4-|-ͼ-|-4-|-ͼ-|-4-|-ͼ-|-4-|-ͼ-|-4-|-ͼ-|-4-|-ͼ-|-38-|-----��95 ͼ��߶�
*  |                                                                    | 5 
*  |-38-|-ͼ-|-4-|-ͼ-|-4-|-ͼ-|-4-|-ͼ-|-4-|-ͼ-|-4-|-ͼ-|-4-|-ͼ-|-38-|-----��95 ͼ��߶�
*  |                                                                    |
* 480                                                                   |
*  |                                                                    |
*  |                                                                    |
*  |                                                                    |
*  |                                                                    |
*  |                                                                    |
*  |--------------------------------------------------------------------|
*
*********************************************************************************************************
*/ 
#define ICONVIEW_TBorder    10   /* �ؼ�ICONVIEW���ϱ߾� */
#define ICONVIEW_LBorder    38   /* �ؼ�ICONVIEW����߾� */  
#define ICONVIEW_Width      854  /* �ؼ�ICONVIEW�Ŀ� */  
#define ICONVIEW_Height     480  /* �ؼ�ICONVIEW�ĸ� */  
#define ICONVIEW_YSpace     5    /* �ؼ�ICONVIEW��Y������ */
#define ICONVIEW_XSpace     4    /* �ؼ�ICONVIEW��Y������ */

#define ICON_Width     		100      /* �ؼ�ICONVIEW��ͼ��Ŀ�� */  
#define ICON_Height    		95       /* �ؼ�ICONVIEW��ͼ��ĸ߶�, ��ͼ��������ı���һ��ĸ߶� */ 

#define SCREEN_Height   	800     /* ��ʾ���߶� */  
#define SCREEN_Width   		480     /* ��ʾ���Ŀ��  */ 
#define LineCap         	16      /* �ַ�����ĸ���16 */


/*
*********************************************************************************************************
*                                         ����
*********************************************************************************************************
*/ 
static WM_HWIN hIcon1, hIcon2, hIcon3;
//static GUI_MEMDEV_Handle   hMempic;
static uint8_t s_ucIconSwitch = 0;
WM_HWIN hMotion;


/*
*********************************************************************************************************
*                               ����ICONVIEW�ؼ��������
*********************************************************************************************************
*/ 
typedef struct 
{
	const GUI_BITMAP * pBitmap;
	const char       * pTextEn;
	const char       * pTextCn;
} BITMAP_ITEM;

typedef struct 
{
  int xPos;
  int yPos;
  int Index;
} ITEM_INFO;

typedef struct 
{
  int          Pos;
  int          FinalMove;
  ITEM_INFO    * pItemInfo;
} PARA;

/* ���ڵ�һ��������ICONVIEWͼ��Ĵ��� */
static const BITMAP_ITEM _aBitmapItem1[] = 
{
	{&bmComputer, "Computer",  "Computer"},
	{&bmRadio,    "FM/AM",     "Radio"},

};

/* ���ڵڶ���������ICONVIEWͼ��Ĵ��� */
static const BITMAP_ITEM _aBitmapItem2[] = 
{
	{&bmGPS,      "GPS",      "GPS"},	
};

/* ���ڵ�����������ICONVIEWͼ��Ĵ��� */
static const BITMAP_ITEM _aBitmapItem3[] = 
{
	{&bmComputer, "Computer",  "Computer"},
};


/*
*********************************************************************************************************
*                                  Ӧ�ó�����ں���
*********************************************************************************************************
*/ 
static void (* _apModules0[])( WM_HWIN hWin) = 
{
	//App_Computer,
	NULL,
};

static void (* _apModules1[])( WM_HWIN hWin) = 
{
	//App_Reserved,
	NULL,
};

static void (* _apModules2[])( WM_HWIN hWin) = 
{
	//App_Computer,
	NULL,
};


/*
*********************************************************************************************************
*	�� �� ��: _cbBkWindow
*	����˵��: ���洰�ڵĻص�����,������Ҫ�ǻ��Ʊ������ںͽ����л�ʱ���л���־�Ļ��� 
*	��    ��: pMsg  WM_MESSAGE����ָ�����   
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _cbBkWindow(WM_MESSAGE * pMsg) 
{
	const GUI_BITMAP  *pBm;
	int x, y;
	
	switch (pMsg->MsgId) 
	{
		/* �ػ���Ϣ*/
		case WM_PAINT:
			
			//GUI_DrawGradientV(0, 0, LCD_GetXSize(),LCD_GetYSize(), GUI_LIGHTCYAN,GUI_DARKRED); 	/* for testing */
		
			/* �Ƚ�3��6x6ԲȦ��ʾ���� */
			pBm = &_bmWhiteCircle_6x6;
			x = LCD_GetXSize()/2 - pBm->XSize / 2 - 20;
			y = LCD_GetYSize() - pBm->YSize / 2 - 60;
		
			GUI_DrawBitmap(pBm, x,      y);
			GUI_DrawBitmap(pBm, x + 20, y);
			GUI_DrawBitmap(pBm, x + 40, y);
		
		   /* ��ʾ10*10ԲȦ����ʾ�Ѿ��л�����Ӧ�Ľ����� */
			pBm = &_bmWhiteCircle_10x10;
		    GUI_DrawBitmap(pBm, x + s_ucIconSwitch*20, y - 2); 
			break;
	
		default:
			WM_DefaultProc(pMsg);
			break;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _cbMotion
*	����˵��: Motion���ڵĻص���������Ҫ������ͼ��Ļ�������
*	��    ��: pMsg  WM_MESSAGE����ָ�����   
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _cbMotion(WM_MESSAGE * pMsg) 
{
	WM_MOTION_INFO * pInfo;
	WM_HWIN          hWin = pMsg->hWin;
	PARA           * pPara;
	static uint32_t  tStart, tEnd;
	int NCode, Id;
	
	switch (pMsg->MsgId) 
	{
		case WM_PRE_PAINT:
			GUI_MULTIBUF_Begin();
			break;
		
		case WM_POST_PAINT:
			GUI_MULTIBUF_End();
			break;
		
		case WM_NOTIFY_PARENT:
			Id    = WM_GetId(pMsg->hWinSrc);     
			NCode = pMsg->Data.v;                 
			switch (Id) 
			{
				/* ��һ�������ϵ�ͼ�� */
				case GUI_ID_ICONVIEW0:
					switch (NCode) 
					{
						/* ICON�ؼ������Ϣ */
						case WM_NOTIFICATION_CLICKED:
							tStart = GUI_GetTime();
							break;

						/* ICON�ؼ��ͷ���Ϣ */
						case WM_NOTIFICATION_RELEASED: 
							tEnd = GUI_GetTime() - tStart;
							if(tEnd > 800)
							{
								WM_SetFocus(WM_HBKWIN);
								break;							
							}
							//_apModules0[ICONVIEW_GetSel(pMsg->hWinSrc)](WM_HBKWIN);
							break;
					}
					break;
					
				/* �ڶ��������ϵ�ͼ�� */
				case GUI_ID_ICONVIEW1:
					switch (NCode) 
					{
						/* ICON�ؼ������Ϣ */
						case WM_NOTIFICATION_CLICKED:
							tStart = GUI_GetTime();
							break;

						/* ICON�ؼ��ͷ���Ϣ */
						case WM_NOTIFICATION_RELEASED: 
							tEnd = GUI_GetTime() - tStart;
							if(tEnd > 800)
							{
								WM_SetFocus(WM_HBKWIN);
								break;							
							}
							//_apModules1[ICONVIEW_GetSel(pMsg->hWinSrc)](WM_HBKWIN);
							break;
					}
					break;
					
					
				/* �����������ϵ�ͼ�꣬��������ʾĿ�� */
				case GUI_ID_ICONVIEW2:
					switch (NCode) 
					{
						/* ICON�ؼ������Ϣ */
						case WM_NOTIFICATION_CLICKED:
							tStart = GUI_GetTime();
							break;

						/* ICON�ؼ��ͷ���Ϣ */
						case WM_NOTIFICATION_RELEASED: 
							tEnd = GUI_GetTime() - tStart;
							if(tEnd > 800)
							{
								WM_SetFocus(WM_HBKWIN);
								break;							
							}
							//_apModules2[ICONVIEW_GetSel(pMsg->hWinSrc)](WM_HBKWIN);
							break;
					}
					break;
			}
			break;
				
	case WM_MOTION:
		WM_GetUserData(hWin, &pPara, sizeof(pPara));
		pInfo = (WM_MOTION_INFO *)pMsg->Data.p;
		switch (pInfo->Cmd) 
		{
			#if 1   /* F429���Ӳ��õ�ʵ�ַ�ʽ ********************************
			           WM_MOTION_SetDefaultPeriod(20);��������С�㣬���ٷ���
			        */
				case WM_MOTION_INIT:
					pInfo->Flags =  WM_CF_MOTION_X | WM_MOTION_MANAGE_BY_WINDOW;
					pInfo->SnapX = 1;
					break;
		
				case WM_MOTION_MOVE:
					pPara->FinalMove = pInfo->FinalMove;
					pPara->Pos += pInfo->dx;
					printf_main("pData->xPos = %d %d\r\n", pPara->Pos, pInfo->dx);
				
					/* �����м��ٶȣ�ֹͣ�󣬼��ٶ�*/
					if(pPara->FinalMove)
					{
						printf_main("pData->FinalMove = %d\r\n", pPara->Pos);
						/* ����ͼ���ƶ����˷�Χ�ھͽ���̶��ڵ�3��ͼ����ʾ */
						if(pPara->Pos <= -(LCD_GetXSize() + LCD_GetXSize()/2)) 
						{
							s_ucIconSwitch = 2;
							pPara->Pos = -(2 * LCD_GetXSize());
						}
						/* ����ͼ���ƶ����˷�Χ�ھͽ���̶��ڵ�2��ͼ����ʾ */
						else if((pPara->Pos > -(LCD_GetXSize() + LCD_GetXSize()/2))&&(pPara->Pos <= -LCD_GetXSize()/2)) 
						{
							s_ucIconSwitch = 1;
							pPara->Pos = -(LCD_GetXSize());
						}
						/* ����ͼ���ƶ����˷�Χ�ھͽ���̶��ڵ�1��ͼ����ʾ */
						else if(pPara->Pos > -LCD_GetXSize()/2) 
						{
							s_ucIconSwitch = 0;
							pPara->Pos = 0;
						}
					}
					
					/* WM_MOTION_SetDefaultPeriod���õ�����30ms��������ʱdx�����ϴ�
					   ����ȡֵ35-50���ɡ�
					*/
					if((pInfo->dx < -4) || (pInfo->dx > 4) || (pPara->FinalMove))
					/* �ƶ�����ͼ�� */
					WM_MoveTo(hWin, pPara->Pos, 0);
					
			#else  /* emWinĬ�ϵ�Motionʹ�÷��� ********************************
					 WM_MOTION_SetDefaultPeriod(100);���õ�ʱ�䳤����Կ���Ч��
					*/
				case WM_MOTION_INIT:
					pInfo->Flags =  WM_CF_MOTION_X | WM_MOTION_MANAGE_BY_WINDOW;
					pInfo->SnapX = 800; /* ����դ���С */
					break;
		
				case WM_MOTION_MOVE:
					pPara->FinalMove = pInfo->FinalMove;
					pPara->Pos += pInfo->dx;
					printf_main("pData->xPos = %d %d\r\n", pPara->Pos, pInfo->dx);
				
					
					/* ���û����������ʱ�Ĵ���취������һ��Ͳ������ٻ����ˣ�����Ử��������һ��դ��*/
					if(pPara->Pos >= LCD_GetXSize()/2)  
					   pPara->Pos = LCD_GetXSize()/2 - 1;
				
					/* ���û��������Ҳ�ʱ�Ĵ���취������һ��Ͳ������ٻ����ˣ�����Ử��������һ��դ��*/
					if(pPara->Pos <= -(LCD_GetXSize()*2 + LCD_GetXSize()/2))  
						pPara->Pos = -(LCD_GetXSize()*2 + LCD_GetXSize()/2 - 1);
				
					/* �������ֺ󣬻ᰴ��WM_MOTION_SetDefaultPeriod���õ�ʱ����ٲ�ֹͣ�������դ�� */
					if(pPara->FinalMove)
					{
						printf_main("pData->FinalMove = %d\r\n", pPara->Pos);
						/* ����ͼ���ƶ����˷�Χ�ھͽ���̶��ڵ�3��ͼ����ʾ */
						if(pPara->Pos <= -(LCD_GetXSize() + LCD_GetXSize()/2)) 
						{
							s_ucIconSwitch = 2;
						}
						/* ����ͼ���ƶ����˷�Χ�ھͽ���̶��ڵ�2��ͼ����ʾ */
						else if((pPara->Pos > -(LCD_GetXSize() + LCD_GetXSize()/2))&&(pPara->Pos <= -LCD_GetXSize()/2)) 
						{
							s_ucIconSwitch = 1;
						}
						/* ����ͼ���ƶ����˷�Χ�ھͽ���̶��ڵ�1��ͼ����ʾ */
						else if(pPara->Pos > -LCD_GetXSize()/2) 
						{
							s_ucIconSwitch = 0;
						}
					}
					
					/* �ƶ�����ͼ�� */
					WM_MoveTo(hWin, pPara->Pos, 0);
					
					/* �����������δʹ�ã������û������ڵĴ�С��800*480ʱ�����Խ��϶�ICONVIEW�ؼ����� 
					    ��ǰ�ǲ���800*3��ȵĽ����С�����϶���ICONVIEW�ؼ��ĸ����ڡ�
					*/
					//WM_MoveTo(hIcon1, pPara->Pos + ICONVIEW_LBorder , ICONVIEW_TBorder);
					//WM_MoveTo(hIcon2, pPara->Pos + 800 + ICONVIEW_LBorder, ICONVIEW_TBorder);
					//WM_MoveTo(hIcon3, pPara->Pos + 1600 + ICONVIEW_LBorder, ICONVIEW_TBorder);
				#endif
				break;
		
			case WM_MOTION_GETPOS:
				pInfo->xPos = pPara->Pos;
				break;
    }
    break;
  }
}

/*
*********************************************************************************************************
*	�� �� ��: _CreateICONVIEW
*	����˵��: ����ICONVIEW
*	��    �Σ�hParent   ������
*             pBm       ICONVIEW�ϵ�λͼ
*             BitmapNum ICONVIEW��ͼ�����       
*             x         x������
*             y         y������
*             w         ICONVIEW��
*             h         ICONVIEW��   
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static WM_HWIN _CreateICONVIEW(WM_HWIN hParent, const BITMAP_ITEM *pBm, int BitmapNum, int Id, int x, int y, int w, int h) 
{
	WM_HWIN hIcon;
	int i;
	
	/*��ָ��λ�ô���ָ���ߴ��ICONVIEW С����*/
	hIcon = ICONVIEW_CreateEx(x, 					/* С���ߵ��������أ��ڸ������У�*/
						     y, 					/* С���ߵ��������أ��ڸ������У�*/
							 w,                     /* С���ߵ�ˮƽ�ߴ磨��λ�����أ�*/
							 h, 	                /* С���ߵĴ�ֱ�ߴ磨��λ�����أ�*/
	                         hParent, 				            /* �����ڵľ�������Ϊ0 ������С���߽���Ϊ���棨�������ڣ����Ӵ��� */
							 WM_CF_SHOW | WM_CF_HASTRANS,       /* ���ڴ�����ǡ�ΪʹС���������ɼ���ͨ��ʹ�� WM_CF_SHOW */ 
	                         0,//ICONVIEW_CF_AUTOSCROLLBAR_V, 	/* Ĭ����0�����������ʵ������������ֱ������ */
							 Id, 			                    /* С���ߵĴ���ID */
							 ICON_Width, 				        /* ͼ���ˮƽ�ߴ� */
							 ICON_Height);						/* ͼ��Ĵ�ֱ�ߴ�,ͼ����ļ������������棬��Ҫ����ICONVIEW�ĸ߶ȣ�����Text��ʾ������*/
	
	
	/* ��ICONVIEW С���������ͼ�� */
	for (i = 0; i < BitmapNum; i++) 
	{
		ICONVIEW_AddBitmapItem(hIcon, pBm[i].pBitmap, pBm[i].pTextCn);
	}
	
	ICONVIEW_SetFont(hIcon, &GUI_Font8x16x1x2);
	
	/* ����С���ߵı���ɫ 32 λ��ɫֵ��ǰ8 λ������alpha��ϴ���Ч��*/
	ICONVIEW_SetBkColor(hIcon, ICONVIEW_CI_SEL, GUI_WHITE | 0x80000000);
	
	/* ����X����ı߽�ֵΪ0��Ĭ�ϲ���0, Y����Ĭ����0����������Ҳ����һ�����ã������Ժ��޸� */
	ICONVIEW_SetFrame(hIcon, GUI_COORD_X, 0);
	ICONVIEW_SetFrame(hIcon, GUI_COORD_Y, 0);
	
	/* ����ͼ����x ��y �����ϵļ�ࡣ*/
	ICONVIEW_SetSpace(hIcon, GUI_COORD_X, ICONVIEW_XSpace);
	ICONVIEW_SetSpace(hIcon, GUI_COORD_Y, ICONVIEW_YSpace);
	
	/* ���ö��뷽ʽ ��5.22�汾�����¼���� */
	ICONVIEW_SetIconAlign(hIcon, ICONVIEW_IA_HCENTER|ICONVIEW_IA_TOP);
	
	//ICONVIEW_SetTextColor(hIcon, ICONVIEW_CI_UNSEL, 0xF020A0);
	return hIcon;
}

/*
*********************************************************************************************************
*	�� �� ��: MainTask
*	����˵��: GUI������
*	��    ��: ��   	
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Touch_Demo_Task(void) 
{ 
	uint16_t usY = 0;
	PARA Para;
	PARA *pPara;
	
	/* ���ݳ�ʼ�� */
	pPara = &Para;
    pPara->Pos   = 0;
	
    GUI_Init();  /* ��ʼ�� */
	
	/*
	 ���ڶ໺��ʹ����ڴ��豸������˵��
	   1. ʹ�ܶ໺���ǵ��õ����º������û�Ҫ��LCDConf_Lin_Template.c�ļ��������˶໺�壬���ô˺�������Ч��
		  WM_MULTIBUF_Enable(1);
	   2. ����ʹ��ʹ���ڴ��豸�ǵ��ú�����WM_SetCreateFlags(WM_CF_MEMDEV);
	   3. ���emWin�����ö໺��ʹ����ڴ��豸��֧�֣���ѡһ���ɣ����������ѡ��ʹ�ö໺�壬ʵ��ʹ��
		  STM32F429BIT6 + 32λSDRAM + RGB565/RGB888ƽ̨���ԣ��໺�������Ч�Ľ��ʹ����ƶ����߻���ʱ��˺��
		  �У�����Ч����������ԣ�ͨ��ʹ�ܴ���ʹ���ڴ��豸���������ġ�
	   4. ����emWin����Ĭ���ǿ��������塣
	*/
	WM_MULTIBUF_Enable(1);
	
	/*
       ����У׼����Ĭ����ע�͵��ģ���������ҪУ׼������������У׼������û���ҪУ׼�������Ļ���ִ��
	   �˺������ɣ��Ὣ����У׼�������浽EEPROM���棬�Ժ�ϵͳ�ϵ���Զ���EEPROM������ء�
	*/
    //TOUCH_Calibration();
	
	WM_MOTION_Enable(1);    /* ʹ�ܻ��� */
	
	/* Ĭ����500ms�����ｫ���޸�Ϊ50ms���������һ���̶��Ͼ����������ȣ��ܾ��������ȣ���Ҫ����
	   ��ΪF429���������ޡ� 
	*/
	//WM_MOTION_SetDefaultPeriod(50);
	WM_MOTION_SetDefaultPeriod(20);
	
	/* ��0����ϵͳ���ؽ��� ------------------------------------------*/	
	
	
	/* ��1�����������洰�ڵı���ͼƬ ------------------------------------------*/
	
	/* ��2����ʹ�����洰�ڵ� ------------------------------------------*/
	WM_SetCallback(WM_HBKWIN, _cbBkWindow);

	/* ��3�����������洰�ڵı���ͼƬ ------------------------------------------*/
	hMotion = WM_CreateWindowAsChild(0, 
	                                 0, 
									 SCREEN_Width * 2, 
									 SCREEN_Height, 
					                 WM_HBKWIN,  
									 WM_CF_MOTION_X | WM_CF_SHOW | WM_CF_HASTRANS, 
									 _cbMotion, 
									 sizeof(pPara));
									 
    WM_SetUserData(hMotion, &pPara, sizeof(pPara));
	

	/* ��4�����������洰�ڵı���ͼƬ ------------------------------------------*/
	/* ��1������ͼ�� */
	hIcon1 = _CreateICONVIEW(hMotion, 
	                _aBitmapItem1, 
					GUI_COUNTOF(_aBitmapItem1), 
					GUI_ID_ICONVIEW0, 
					ICONVIEW_LBorder, 
					ICONVIEW_TBorder, 
					ICONVIEW_Width, 
					ICONVIEW_Height*2);
	
	/* ��2������ͼ�� */
	hIcon2 =_CreateICONVIEW(hMotion, 
	                _aBitmapItem2, 
					GUI_COUNTOF(_aBitmapItem2), 
					GUI_ID_ICONVIEW1, 
					SCREEN_Width + ICONVIEW_LBorder, 
					ICONVIEW_TBorder, 
					ICONVIEW_Width, 
					ICONVIEW_Height);
	
	/* ��3������ͼ�� */
	hIcon3 =_CreateICONVIEW(hMotion, 
	                _aBitmapItem3, 
					GUI_COUNTOF(_aBitmapItem3), 
					GUI_ID_ICONVIEW2, 
					SCREEN_Width*2 + ICONVIEW_LBorder, 
					ICONVIEW_TBorder, 
					ICONVIEW_Width, 
					ICONVIEW_Height);
	
	/* ��ֹ���棬�����Ժ��� */
	(void)hIcon1;
	(void)hIcon2;
	(void)hIcon3;	

	/* ��5�����������洰�ڵı���ͼƬ ------------------------------------------*/
	
	/* ��6�����������洰�ڵı���ͼƬ ------------------------------------------*/

	while(1) 
	{
		GUI_Delay(10);
	}
}

/***************************** cubic (END OF FILE) *********************************/
