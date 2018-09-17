/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2016  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.38 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only  be used  in accordance  with  a license  and should  not be  re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : MOTION_SmartPhoneMenu.c
Purpose     : Shows how to create a radial menu with motion support
Requirements: WindowManager - (x)
              MemoryDevices - (x)
              AntiAliasing  - (x)
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
----------------------------------------------------------------------
*/

/*********************************************************************
*
*       Includes
*
**********************************************************************
*/
#include "DIALOG.h"
#include "includes.h"

/*********************************************************************
*
*       Static (const) data
*
**********************************************************************
*/

/*********************************************************************
*
*       _aMenu
*/

typedef enum {
	
	BUTTON_PAGE_ONE     = 0,
	BUTTON_PAGE_TWO_L,
	BUTTON_PAGE_TWO_R,
	BUTTON_PAGE_THREE_L,
	BUTTON_PAGE_THREE_R,
	BUTTON_PAGE_FOUR_L,
	BUTTON_PAGE_FOUR_R,
	BUTTON_PAGE_FIVE,
	BUTTON_PAGE_IDLE,

} BUTTON_STATE_T;

BUTTON_STATE_T BUTTON_STATE = BUTTON_PAGE_ONE;

static void _cbMenu(WM_MESSAGE * pMsg);
static void _CreateSelected(int Index, WM_HWIN hWin) ;

DEV_STATE_CTR_T dev_state; 

GUI_RECT Rect_0 = {20,   90, 480, 130};
GUI_RECT Rect_1 = {150, 160, 300, 300};
GUI_RECT Rect_2 = {150, 380, 300, 500};
GUI_RECT Rect_3 = {20,  560, 400, 753};
GUI_RECT Rect_4 = {380, 400, 479, 450};

#define GUI_ID_USER_BUTTON_0 	(GUI_ID_USER + 0x01)
#define GUI_ID_USER_BUTTON_1 	(GUI_ID_USER + 0x0A)
#define GUI_ID_USER_BUTTON_2 	(GUI_ID_USER + 0x0C)
#define GUI_ID_USER_CLOSE 	   	(GUI_ID_BUTTON0 + 1000)
#define GUI_ID_USER_OK 		   	(GUI_ID_BUTTON0 + 1001)
#define GUI_ID_USER_CANCEL     	(GUI_ID_BUTTON0 + 1002)
#define GUI_ID_USER_PM 		   	(GUI_ID_BUTTON0 + 1003)
#define GUI_ID_USER_SYS 	   	(GUI_ID_BUTTON0 + 1004)
#define GUI_ID_USER_NET        	(GUI_ID_BUTTON0 + 1005)
#define GUI_ID_USER_BRL 	   	(GUI_ID_BUTTON0 + 1006)
#define GUI_ID_USER_SET        	(GUI_ID_BUTTON0 + 1007)
#define GUI_ID_USER_DET_M      	(GUI_ID_BUTTON0 + 1008)
#define GUI_ID_USER_DET_1      	(GUI_ID_BUTTON0 + 1009)
#define GUI_ID_USER_DET_2      	(GUI_ID_BUTTON0 + 1010)
#define GUI_ID_USER_DET_4      	(GUI_ID_BUTTON0 + 1011)

extern GUI_CONST_STORAGE GUI_FONT GUI_FontHoloLensMDL2Assets27;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontPingFang_Regular33;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontPingFang_Regular38;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontPingFang_Regular41;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontDingbats2114;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontPingFang_Regular45;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontPingFang_Regular67;

extern GUI_CONST_STORAGE GUI_BITMAP bmuv;
extern GUI_CONST_STORAGE GUI_BITMAP bmco2;
extern GUI_CONST_STORAGE GUI_BITMAP bmknow;
extern GUI_CONST_STORAGE GUI_BITMAP bmlook;

/*
*********************************************************************************************************
*				                         对话框初始化选项
*********************************************************************************************************
*/

static const GUI_POINT _aPointArrow[] = {
  {  0,   0 },
  { 29,  29 },
  { 29,  16 },
  { 68,  16 },
  { 68, -16 },
  { 29, -16 },
  { 29, -29 },
};

typedef struct 
{
	const GUI_BITMAP * pBitmap;
	const char       * pTextEn;
	const char       * pTextCn;
} BITMAP_ITEM;


static const BITMAP_ITEM _aBitmapItem[] = {
  {&bmuv, 		"" , 	""},
  {&bmco2,   	"" , 	""},
  {&bmknow,   	"" , 	""},
  {&bmlook,   	"" , 	""},
};

#define ICONVIEW_TBorder   30   		/* 控件ICONVIEW的上边距 */
#define ICONVIEW_LBorder   30   		/* 控件ICONVIEW的左边距 */  
#define ICONVIEW_Width     130  		/* 控件ICONVIEW的宽 */  
#define ICONVIEW_Height    130  		/* 控件ICONVIEW的高 */  
#define ICONVIEW_YSpace    30   		/* 控件ICONVIEW的Y方向间距 */
#define ICONVIEW_XSpace    30    		/* 控件ICONVIEW的X方向间距 */
#define ICON_Width     	   130      	/* 控件ICONVIEW的图标的宽度 */  
#define ICON_Height    	   130       	/* 控件ICONVIEW的图标的高度, 含图标下面的文本，一起的高度 */ 

static WM_HWIN hWinStatus;
static WM_HWIN hIcon;
static WM_HWIN hButton_Cancel 	= NULL;
static WM_HWIN hButton_OK		= NULL;
static WM_HWIN hButton_Close	= NULL;
static WM_HWIN hButton_PMSet 	= NULL;
static WM_HWIN hButton_SysSet 	= NULL;
static WM_HWIN hButton_NetSet 	= NULL;
static WM_HWIN hButton_BrlSet 	= NULL;
static WM_HWIN hButton_EnSet	= NULL;


/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _ButtonSkin
*
* Purpose:
*   Skinning function of a single button. Responsible for custom drawing only.
*/
static int _ButtonSkin(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  int Index, xSize, ySize, IsPressed;
  WM_HWIN hWin;
  const GUI_BITMAP * pBm;
  GUI_COLOR Color;

  hWin = pDrawItemInfo->hWin;
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_BACKGROUND:
    //
    // Get state of button
    //
    IsPressed = BUTTON_IsPressed(pDrawItemInfo->hWin);
    //
    // Get window dimension
    //
    xSize = WM_GetWindowSizeX(hWin);
    ySize = WM_GetWindowSizeY(hWin);
    //
    // Get button index
    //
    BUTTON_GetUserData(hWin, &Index, sizeof(Index));
    //
    // Draw background color depending on state
    //
    if (IsPressed) {
      Color = GUI_DARKGRAY;
    } else {
      Color = GUI_BLACK;
    }
    GUI_SetBkColor(Color);
    GUI_Clear();
    //
    // Draw antialiased rounded rectangle as background for image
    //
	GUI_SetColor(GUI_MAGENTA);
    GUI_SetFont(&GUI_FontDingbats2114);
    GUI_SetTextAlign(GUI_TA_LEFT | GUI_TA_VCENTER);
    GUI_DispCharAt(0x007c, 400, 0);
    break;
  }
  return 0;
}

static int _ButtonSkinPageTwoRight(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  int Index, xSize, ySize, IsPressed;
  WM_HWIN hWin;
  const GUI_BITMAP * pBm;
  GUI_COLOR Color;

  hWin = pDrawItemInfo->hWin;
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_BACKGROUND:
    //
    // Get state of button
    //
    IsPressed = BUTTON_IsPressed(pDrawItemInfo->hWin);
    //
    // Get window dimension
    //
    xSize = WM_GetWindowSizeX(hWin);
    ySize = WM_GetWindowSizeY(hWin);
    //
    // Get button index
    //
    BUTTON_GetUserData(hWin, &Index, sizeof(Index));
    //
    // Draw background color depending on state
    //
    if (IsPressed) {
      Color = GUI_DARKGRAY;
    } else {
      Color = GUI_BLACK;
    }
    GUI_SetBkColor(Color);
    GUI_Clear();
    //
    // Draw arrow at the right
    //
    GUI_SetColor(GUI_MAGENTA);
    GUI_SetFont(&GUI_FontDingbats2114);
    GUI_SetTextAlign(GUI_TA_LEFT | GUI_TA_VCENTER);
    GUI_DispCharAt(0x007c, 160, 0);
    break;
  }
  return 0;
}

static int _ButtonSkinPageTwoLeft(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  int Index, xSize, ySize, IsPressed;
  WM_HWIN hWin;
  const GUI_BITMAP * pBm;
  GUI_COLOR Color;

  hWin = pDrawItemInfo->hWin;
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_BACKGROUND:
    //
    // Get state of button
    //
    IsPressed = BUTTON_IsPressed(pDrawItemInfo->hWin);
    //
    // Get window dimension
    //
    xSize = WM_GetWindowSizeX(hWin);
    ySize = WM_GetWindowSizeY(hWin);
    //
    // Get button index
    //
    BUTTON_GetUserData(hWin, &Index, sizeof(Index));
    //
    // Draw background color depending on state
    //
    if (IsPressed) {
      Color = GUI_DARKGRAY;
    } else {
      Color = GUI_BLACK;
    }
    GUI_SetBkColor(Color);
    GUI_Clear();
    //
    // Draw arrow at the right
    //
    GUI_SetColor(GUI_MAGENTA);
    GUI_SetTextAlign(GUI_TA_LEFT | GUI_TA_VCENTER);
	GUI_FillPolygon (&_aPointArrow[0], 7, 10, 50);
    break;
  }
  return 0;
}

static int _ButtonSkinPageThreeLeft(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  int Index, xSize, ySize, IsPressed;
  WM_HWIN hWin;
  const GUI_BITMAP * pBm;
  GUI_COLOR Color;

  hWin = pDrawItemInfo->hWin;
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_BACKGROUND:
    //
    // Get state of button
    //
    IsPressed = BUTTON_IsPressed(pDrawItemInfo->hWin);
    //
    // Get window dimension
    //
    xSize = WM_GetWindowSizeX(hWin);
    ySize = WM_GetWindowSizeY(hWin);
    //
    // Get button index
    //
    BUTTON_GetUserData(hWin, &Index, sizeof(Index));
    //
    // Draw background color depending on state
    //
    if (IsPressed) {
      Color = GUI_DARKGRAY;
    } else {
      Color = GUI_BLACK;
    }
    GUI_SetBkColor(Color);
    GUI_Clear();

    //
    // Draw arrow at the right
    //
	GUI_SetColor(GUI_MAGENTA);
    GUI_SetTextAlign(GUI_TA_LEFT | GUI_TA_VCENTER);
	GUI_FillPolygon (&_aPointArrow[0], 7, 10, 50);
    break;
  }
  return 0;
}


static int _Button_PM25_Skin(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  int Index, xSize, ySize, IsPressed;
  WM_HWIN hWin;
  const GUI_BITMAP * pBm;
  GUI_COLOR Color;

  hWin = pDrawItemInfo->hWin;
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_BACKGROUND:
    //
    // Get state of button
    //
    IsPressed = BUTTON_IsPressed(pDrawItemInfo->hWin);
    //
    // Get window dimension
    //
    xSize = WM_GetWindowSizeX(hWin);
    ySize = WM_GetWindowSizeY(hWin);

    //
    // Draw background color depending on state
    //

    if (dev_state.IsPM25Pressed) {
      Color = GUI_DARKGRAY;
    } else {
      Color = GUI_MAGENTA;
    }
    GUI_Clear();
    //
    // Draw antialiased rounded rectangle as background for image
    //
    GUI_SetColor(Color);
	GUI_FillCircle(65,60,50);
	//
    // Draw button text
    //
    GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(&GUI_FontPingFang_Regular41);
	GUI_UC_SetEncodeUTF8();
	GUI_DispStringAt("\xe9\xa9\xac\xe4\xb8\x8a", 35, 18); // 马上
	GUI_DispStringAt("\xe6\xa3\x80\xe6\xb5\x8b", 35, 52); // 检测
    break;
  }
  return 0;
}

static int _Button_CO2_Skin(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  int Index, xSize, ySize, IsPressed;
  WM_HWIN hWin;
  const GUI_BITMAP * pBm;
  GUI_COLOR Color;

  hWin = pDrawItemInfo->hWin;
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_BACKGROUND:
    //
    // Get state of button
    //
    IsPressed = BUTTON_IsPressed(pDrawItemInfo->hWin);
    //
    // Get window dimension
    //
    xSize = WM_GetWindowSizeX(hWin);
    ySize = WM_GetWindowSizeY(hWin);

    //
    // Draw background color depending on state
    //

    if (dev_state.IsCO2Pressed) {
      Color = GUI_DARKGRAY;
    } else {
      Color = GUI_MAGENTA;
    }
	
    //GUI_SetBkColor(Color);
    GUI_Clear();
    //
    // Draw antialiased rounded rectangle as background for image
    //
    GUI_SetColor(Color);
	GUI_FillCircle(65,60,50);
	//
    // Draw button text
    //
    GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(&GUI_FontPingFang_Regular41);
	GUI_UC_SetEncodeUTF8();
	GUI_DispStringAt("\xe9\xa9\xac\xe4\xb8\x8a", 35, 18); // 马上
	GUI_DispStringAt("\xe6\xa3\x80\xe6\xb5\x8b", 35, 52); // 检测

    break;
  }
  return 0;
}

static int _ButtonSkinAddr(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  int Index, xSize, ySize, IsPressed;
  WM_HWIN hWin;
  const GUI_BITMAP * pBm;
  GUI_COLOR Color;

  hWin = pDrawItemInfo->hWin;
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_BACKGROUND:
    //
    // Get state of button
    //
    IsPressed = BUTTON_IsPressed(pDrawItemInfo->hWin);
    //
    // Get window dimension
    //
    xSize = WM_GetWindowSizeX(hWin);
    ySize = WM_GetWindowSizeY(hWin);

    //
    // Draw background color depending on state
    //

    if (IsPressed) {
      Color = GUI_DARKGRAY;
    } else {
      Color = GUI_MAGENTA;
    }
	
    GUI_Clear();
    //
    // Draw antialiased rounded rectangle as background for image
    //
    GUI_SetColor(Color);
    GUI_FillRoundedRect(0, 0, xSize, ySize, 10); 
	//
    // Draw button text
    //
    GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(&GUI_FontPingFang_Regular41);
	GUI_UC_SetEncodeUTF8();
	GUI_DispStringAt("\xe8\xbe\x93\xe5\x85\xa5\xe5\x9c\xb0\xe5\x9d\x80", 6, 5); // 输入地址
    break;
  }
  return 0;
}

/*********************************************************************
*
*       _cbHeading
*
* Purpose:
*   Callback function of heading window containing logo and battery symbol.
*/

unsigned short BatteryIndex[] = {0xEBA0,0xEBA1,0xEBA2,0xEBA3,0xEBA4,0xEBA5,0xEBA6,0xEBA7,
								 0xEBA8,0xEBA9,0xEBAA,0xEBAB,0xEBAC,0xEBAD,0xEBAE,0xEBAF,
								 0xEBB0,0xEBB1,0xEBB2,0xEBB3,0xEBB4,0xEBB5};

static void _cbHeading(WM_MESSAGE * pMsg) {
  int xSize, xPos;
  const GUI_BITMAP * pBm;
  WM_HWIN hWin;
  static int Index;
  TM_RTC_t DateTime;
  static char time_buf[30];
  static char date_buf[30];
  static char buf[30];
  
  hWin = pMsg->hWin;
  switch (pMsg->MsgId) {
  case WM_CREATE:
    //
    // Create timer to be used to modify the battery symbol
    //
    WM_CreateTimer(hWin, 0, 1000, 0);
    break;
  case WM_TIMER:
	  TM_RTC_GetDateTime(&DateTime,RTC_Format_BIN);

	  sprintf(time_buf, 
	          "%0.2d:%0.2d:%0.2d", 
			  DateTime.hours, 
			  DateTime.minutes,
			  DateTime.seconds);
 	  
	  sprintf(date_buf, 
	          "20%0.2d.%0.2d.%0.2d", 
			  DateTime.year, 
			  DateTime.month, 
			  DateTime.date);
    //
    // Modify battery symbol on timer message
    //
    Index++;
    if (Index == GUI_COUNTOF(BatteryIndex)) {
      Index = 0;
    }
	
    WM_InvalidateWindow(hWin);
    WM_RestartTimer(pMsg->Data.v, 0);
    break;
  case WM_PAINT:
    //
    // Get window dimension
    //
    xSize = WM_GetWindowSizeX(hWin);
    //
    // Draw logo, battery and clock
    //
    xPos = xSize;
	GUI_SetBkColor(GUI_BLACK);
	GUI_Clear();
	GUI_SetColor(GUI_GRAY);
	GUI_DrawHLine(38, 0, xSize);
	GUI_DrawHLine(39, 0, xSize);
	GUI_DrawHLine(40, 0, xSize);
	GUI_SetColor(GUI_WHITE);
    GUI_SetFont(&GUI_FontPingFang_Regular33);
    GUI_SetTextMode(GUI_TM_TRANS);
	GUI_DispStringAt(date_buf, 5, 0);
	GUI_DispStringAt(time_buf, 140, 0);
	
	sprintf(buf, "%0.2d", 80);
	GUI_DispStringAt(buf,382,0);
	GUI_DispStringAt("\x25",410,0); // "%"

	GUI_SetFont(&GUI_FontHoloLensMDL2Assets27);
	GUI_SetColor(GUI_WHITE);
	GUI_DispCharAt(BatteryIndex[7], 435, 5);
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

/*
*********************************************************************************************************
*	函 数 名: _CreateICONVIEW
*	功能说明: 创建ICONVIEW
*	形    参：hParent   父窗口
*             pBm       ICONVIEW上的位图
*             BitmapNum ICONVIEW上图标个数       
*             x         x轴坐标
*             y         y轴坐标
*             w         ICONVIEW宽
*             h         ICONVIEW高   
*	返 回 值: 无
*********************************************************************************************************
*/
static WM_HWIN _CreateICONVIEW(WM_HWIN hParent, const BITMAP_ITEM *pBm, int BitmapNum, int Id, int x, int y, int w, int h) 
{
	WM_HWIN hIcon;
	int i;
	
	/*在指定位置创建指定尺寸的ICONVIEW 小工具*/
	hIcon = ICONVIEW_CreateEx(x, 					/* 小工具的最左像素（在父坐标中）*/
						     y, 					/* 小工具的最上像素（在父坐标中）*/
							 w,                     /* 小工具的水平尺寸（单位：像素）*/
							 h, 	                /* 小工具的垂直尺寸（单位：像素）*/
	                         hParent, 				            /* 父窗口的句柄。如果为0 ，则新小工具将成为桌面（顶级窗口）的子窗口 */
							 WM_CF_SHOW | WM_CF_HASTRANS,       /* 窗口创建标记。为使小工具立即可见，通常使用 WM_CF_SHOW */ 
	                         0,//ICONVIEW_CF_AUTOSCROLLBAR_V, 	/* 默认是0，如果不够现实可设置增减垂直滚动条 */
							 Id, 			                    /* 小工具的窗口ID */
							 ICON_Width, 				        /* 图标的水平尺寸 */
							 ICON_Height);						/* 图标的垂直尺寸,图标和文件都包含在里面，不要大于ICONVIEW的高度，导致Text显示不完整*/
	

	/* 向ICONVIEW 小工具添加新图标 */
	
	for (i = 0; i < BitmapNum; i++) 
	{
		ICONVIEW_AddBitmapItem(hIcon, pBm[i].pBitmap, pBm[i].pTextCn);
	}
	
	//ICONVIEW_SetFont(hIcon, &GUI_SysFontHZ16);
	
	/* 设置小工具的背景色 32 位颜色值的前8 位可用于alpha混合处理效果*/
	ICONVIEW_SetBkColor(hIcon, ICONVIEW_CI_SEL, GUI_BLACK | 0x80000000);
	
	/* 设置X方向的边界值为0，默认不是0, Y方向默认是0，这里我们也进行一下设置，方便以后修改 */
	ICONVIEW_SetFrame(hIcon, GUI_COORD_X, 0);
	ICONVIEW_SetFrame(hIcon, GUI_COORD_Y, 0);
	
	/* 设置图标在x 或y 方向上的间距。*/
	ICONVIEW_SetSpace(hIcon, GUI_COORD_X, ICONVIEW_XSpace);
	ICONVIEW_SetSpace(hIcon, GUI_COORD_Y, ICONVIEW_YSpace);
	
	/* 设置对齐方式 在5.22版本中最新加入的 */
	ICONVIEW_SetIconAlign(hIcon, ICONVIEW_IA_HCENTER|ICONVIEW_IA_TOP);
	
	//ICONVIEW_SetTextColor(hIcon, ICONVIEW_CI_UNSEL, 0xF020A0);
	return hIcon;
}

static void _cbBkPMSet(WM_MESSAGE * pMsg) {

  int xSize, i, NCode;
  int Id = 0;

  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
    NCode = pMsg->Data.v;                 // Notification code

	switch (NCode) {
	case WM_NOTIFICATION_CLICKED:
        break;
    case WM_NOTIFICATION_RELEASED:
        break;
	}
  case WM_PAINT:
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();
	
	GUI_SetColor(GUI_WHITE);
	GUI_SetPenSize(4);
	GUI_FillCircle(22,70,6);
	GUI_DrawArc(22,70 ,13,13,0,360);
	GUI_DrawArc(22,115,13,13,0,360);
	GUI_DrawArc(22,160,13,13,0,360);
	GUI_DrawArc(22,205,13,13,0,360);
	GUI_SetColor(GUI_BLACK);
	GUI_FillCircle(22,115,6);
	GUI_FillCircle(22,160,6);
	GUI_FillCircle(22,205,6);
	
	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(&GUI_FontPingFang_Regular41);
	GUI_DispStringAt("\xe7\xb2\x89\xe5\xb0\x98\xe8\x87\xaa\xe5\x8a\xa8\xe6\xa3\x80\xe6\xb5\x8b\xe9\x97\xb4\xe9\x9a\x94",5,5);
	GUI_DispStringAt("\xe6\x89\x8b\xe5\x8a\xa8\xe6\xa3\x80\xe6\xb5\x8b",40,50); 	// 手动检测
	GUI_DispStringAt("\xe6\xaf\x8f\xe5\xa4\xa9\xe6\xa3\x80\xe6\xb5\x8b\x31\xe6\xac\xa1",40,95); 	// 每天检测1次
	GUI_DispStringAt("\xe6\xaf\x8f\xe5\xa4\xa9\xe6\xa3\x80\xe6\xb5\x8b\x32\xe6\xac\xa1",40,140); 	// 每天检测2次
	GUI_DispStringAt("\xe6\xaf\x8f\xe5\xa4\xa9\xe6\xa3\x80\xe6\xb5\x8b\x34\xe6\xac\xa1",40,185); 	// 每天检测4次
	break;
  }
}

static void _cbBkSysSet(WM_MESSAGE * pMsg) {

  int xSize, i, NCode;
  int Id = 0;

  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
    NCode = pMsg->Data.v;                 // Notification code

	switch (NCode) {
	case WM_NOTIFICATION_CLICKED:
        break;
    case WM_NOTIFICATION_RELEASED:
        break;
	}
  case WM_PAINT:
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();

	GUI_SetColor(GUI_WHITE);
    GUI_SetFont(&GUI_Font32B_1);
	GUI_DispStringAt("null", 5, 10);
	break;
  }
}

static void _cbBkNetSet(WM_MESSAGE * pMsg) {

  int xSize, i, NCode;
  int Id = 0;

  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
    NCode = pMsg->Data.v;                 // Notification code

	switch (NCode) {
	case WM_NOTIFICATION_CLICKED:
        break;
    case WM_NOTIFICATION_RELEASED:
        break;
	}
  case WM_PAINT:
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();
	
	GUI_SetColor(GUI_WHITE);
    GUI_SetFont(&GUI_Font32B_1);
	GUI_DispStringAt("null", 5, 10);
	break;
  }
}


static void _cbBkBrlSet(WM_MESSAGE * pMsg) {

  int xSize, i, NCode;
  int Id = 0;

  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
    NCode = pMsg->Data.v;                 // Notification code

	switch (NCode) {
	case WM_NOTIFICATION_CLICKED:
        break;
    case WM_NOTIFICATION_RELEASED:
        break;
	}
  case WM_PAINT:
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();
	
	GUI_SetColor(GUI_WHITE);
    GUI_SetFont(&GUI_Font32B_1);
	GUI_DispStringAt("null", 5, 10);
	break;
  }
}


static void _cbBkEnSet(WM_MESSAGE * pMsg) {

  int xSize, i, NCode;
  int Id = 0;

  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
    NCode = pMsg->Data.v;                 // Notification code

	switch (NCode) {
	case WM_NOTIFICATION_CLICKED:
        break;
    case WM_NOTIFICATION_RELEASED:
        break;
	}
  case WM_PAINT:
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();
	
	GUI_SetColor(GUI_WHITE);
    GUI_SetFont(&GUI_Font32B_1);
	GUI_DispStringAt("null", 5, 10);
	break;
  }
}

static void _cbBkCancel(WM_MESSAGE * pMsg) {

  int xSize, i, NCode;
  int Id = 0;

  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
    NCode = pMsg->Data.v;                 // Notification code

	switch (NCode) {
	case WM_NOTIFICATION_CLICKED:
		if(Id == GUI_ID_USER_CLOSE)
			_CreateSelected(Id - GUI_ID_BUTTON0, pMsg->hWin);
      break;
    case WM_NOTIFICATION_RELEASED:
        //
        // React only if window is not moving
        //
        break;
	}
  case WM_PAINT:
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();
	GUI_SetColor(GUI_WHITE);
    GUI_SetFont(&GUI_Font32B_1);
	GUI_DispStringAt("null", 5, 10);
    break;
  }
}

static void App_UVContrl(WM_HWIN hWin) 
{
	WM_HWIN hButton;

	//if(hButton_PMSet == NULL)
	{
		hButton_Cancel = WINDOW_CreateUser(0, 40, 480, 854, 0, WM_CF_SHOW, 0, 0, _cbBkCancel, 0);
		hButton = BUTTON_CreateUser(0, 730, 480, 124, hButton_Cancel, WM_CF_SHOW, 0, GUI_ID_USER_CLOSE, 0);
	  	BUTTON_SetSkin(hButton, _ButtonSkinPageTwoLeft);
	}
}

static void App_CO2Contrl(WM_HWIN hWin)
{
	WM_HWIN hButton;
	
	hButton_Cancel = WINDOW_CreateUser(0, 40, 480, 854, 0, WM_CF_SHOW, 0, 0, _cbBkCancel, 0);
	hButton = BUTTON_CreateUser(0, 730, 480, 124, hButton_Cancel, WM_CF_SHOW, 0, GUI_ID_USER_CLOSE, 0);
  	BUTTON_SetSkin(hButton, _ButtonSkinPageTwoLeft);

}

static void App_KnowContrl(WM_HWIN hWin)
{
	WM_HWIN hButton;
	
	hButton_Cancel = WINDOW_CreateUser(0, 40, 480, 854, 0, WM_CF_SHOW, 0, 0, _cbBkCancel, 0);
	hButton = BUTTON_CreateUser(0, 730, 480, 124, hButton_Cancel, WM_CF_SHOW, 0, GUI_ID_USER_CLOSE, 0);
  	BUTTON_SetSkin(hButton, _ButtonSkinPageTwoLeft);
}

static void App_LookContrl(WM_HWIN hWin)
{
	WM_HWIN hButton;
	
	hButton_Cancel = WINDOW_CreateUser(0, 40, 480, 854, 0, WM_CF_SHOW, 0, 0, _cbBkCancel, 0);
	hButton = BUTTON_CreateUser(0, 730, 480, 124, hButton_Cancel, WM_CF_SHOW, 0, GUI_ID_USER_CLOSE, 0);
  	BUTTON_SetSkin(hButton, _ButtonSkinPageTwoLeft);
}

static void (* _IconModules[])( WM_HWIN hWin) = 
{
	App_UVContrl,
	App_CO2Contrl,
	App_KnowContrl,
	App_LookContrl,
};

static void App_PMSetCtr(WM_HWIN hWin)
{
	if(hButton_PMSet == NULL)
		hButton_PMSet = WINDOW_CreateUser(0, 370, 480, 410, 0, WM_CF_SHOW, 0, 0, _cbBkPMSet, 0);
}

static void App_SysSetCtr(WM_HWIN hWin)
{
	hButton_SysSet = WINDOW_CreateUser(0, 370, 480, 410, 0, WM_CF_SHOW, 0, 0, _cbBkSysSet, 0);
}

static void App_NetSetCtr(WM_HWIN hWin)
{
	hButton_NetSet = WINDOW_CreateUser(0, 370, 480, 410, 0, WM_CF_SHOW, 0, 0, _cbBkNetSet, 0);
}

static void App_BrlSetCtr(WM_HWIN hWin)
{
	hButton_BrlSet = WINDOW_CreateUser(0, 370, 480, 410, 0, WM_CF_SHOW, 0, 0, _cbBkBrlSet, 0);
}

static void App_EnSetCtr(WM_HWIN hWin)
{
	hButton_EnSet = WINDOW_CreateUser(0, 370, 480, 410, 0, WM_CF_SHOW, 0, 0, _cbBkEnSet, 0);
}

static void WM_Custom_DeleteButtonWindow(void)
{
	WM_DeleteWindow(hButton_PMSet);
	hButton_PMSet = NULL;
	
	WM_DeleteWindow(hButton_SysSet);
	hButton_SysSet = NULL;
	
	WM_DeleteWindow(hButton_NetSet);
	hButton_NetSet = NULL;
	
	WM_DeleteWindow(hButton_BrlSet);
	hButton_BrlSet = NULL;
	
	WM_DeleteWindow(hButton_EnSet);
	hButton_EnSet = NULL;

	WM_DeleteWindow(hButton_Cancel);
	hButton_Cancel = NULL;
}

/*********************************************************************
*
*       _cbSelected
*
* Purpose:
*   Callback function of window created in reaction of pressing a button.
*   Responsible for drawing only.
*/

static int XPOS_OFFSET 			= 50;
static int XPOS_OFFSET_VAR 		= 250;
static int YPOS_OFFSET 			= 180;
static int YPOS_OFFSET_GRAD 	= 80;
static int YPOS_OFFSET_PROG 	= 100;

#define CO2_TY   	1
#define PM25_TY  	2
#define EMR_TY   	3

static void  _CreateProgressBar(int xPos0, int yPos0, int xPos1, int yPos1,int r ,int x_offset) {

  GUI_SetColor(0x62bb72);  // green
  GUI_FillRoundedRect(xPos0, yPos0, xPos0 + (xPos1 - xPos0) / 3 + 12, yPos1, r);
  GUI_SetColor(0x2745e6);  // red
  GUI_FillRoundedRect(xPos0 +  ((xPos1 - xPos0) * 2)  / 3 - 12, yPos0, xPos1, yPos1, r);
  GUI_SetColor(0x34b1f4);  // orenge
  GUI_FillRect(xPos0 + (xPos1 - xPos0) / 3, yPos0,xPos0 +  ((xPos1 - xPos0) * 2)  / 3, yPos1);

  if(x_offset >= yPos1 - yPos0)
  	 x_offset -= 25;
  
  GUI_SetColor(GUI_WHITE);	
  GUI_AA_FillRoundedRect(xPos0 + x_offset, yPos0 - (50 - (yPos1 - yPos0)) / 2, xPos0 + 20 + x_offset, yPos1 + (50 - (yPos1 - yPos0)) / 2, 10);
}

static void  _CreateProgressBarligttle(int xPos0, int yPos0, int xPos1, int yPos1, int r ,int x_offset) {

  GUI_SetColor(0x62bb72);
  GUI_FillRoundedRect(xPos0,yPos0,xPos0 + (xPos1 - xPos0) / 3 + 10, yPos1, r);
  GUI_SetColor(0x2745e6);
  GUI_FillRoundedRect(xPos0 +  ((xPos1 - xPos0) * 2)  / 3 - 10, yPos0, xPos1, yPos1, r);
  GUI_SetColor(0x34b1f4);
  GUI_FillRect(xPos0 + (xPos1 - xPos0) / 3,yPos0,xPos0 +  ((xPos1 - xPos0) * 2)  / 3, yPos1);

  if(x_offset >= yPos1 - yPos0)
  	 x_offset -= 14;
  	
  GUI_SetColor(GUI_WHITE);
  GUI_AA_FillRoundedRect(xPos0 + x_offset, yPos0 - (33 - (yPos1 - yPos0)) / 2, xPos0 + 14 + x_offset, yPos1 + (33 - (yPos1 - yPos0)) / 2, 7);
}


static int GUI_ProgbarCustom_PM(int SampleVal,int Sensor)
{
	int x_offset = 0;
	
	if(Sensor == PM25_TY)
		x_offset = SampleVal * 3;
	else
		 return 0;

	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_FontPingFang_Regular41);
	GUI_UC_SetEncodeUTF8();

	GUI_DispStringAt("PM2.5\xe7\xb2\x89\xe5\xb0\x98",5,14); // PM2.5粉尘
	GUI_DispStringAt("\xe5\xbe\xae\xe5\x85\x8b/\xe7\xab\x8b\xe6\x96\xb9\xe7\xb1\xb3 \x28        \x29",5,115); // 微克/立方米 (ug / m3)
	GUI_UC_SetEncodeNone();
	GUI_DispStringAt("\xB5g/",185,115); // ug/
	GUI_DispCharAt(0x33a5,230,115); 	// "m3

	GUI_UC_SetEncodeUTF8();
	if(SampleVal < 30) 	
		GUI_DispStringAt("\xe6\x97\xa0\xe6\xb1\xa1\xe6\x9f\x93",385,60); // 无污染
	else if(SampleVal <= 60 && SampleVal >= 30) 
		GUI_DispStringAt("\xe4\xb8\xad\xe6\xb1\xa1\xe6\x9f\x93",385,60); // 中污染
	else if(SampleVal > 60) 
		GUI_DispStringAt("\xe9\x87\x8d\xe6\xb1\xa1\xe6\x9f\x93",385,60); // 重污染

	GUI_SetFont(&GUI_FontPingFang_Regular33);
	GUI_UC_SetEncodeUTF8();
	// 可吸入颗粒物
	GUI_DispStringAt("\xe5\x8f\xaf\xe5\x90\xb8\xe5\x85\xa5\xe9\xa2\x97\xe7\xb2\x92\xe7\x89\xa9",160,20);
	
	_CreateProgressBar(5,70,365,95,10,x_offset);
}

static int GUI_ProgbarCustom_CO2(int SampleVal,int Sensor)
{
	int x_offset = 0;
	char buf[32];
	
	if(Sensor == PM25_TY)
		x_offset = SampleVal;
	else if(Sensor == CO2_TY)
		x_offset = (200 * SampleVal) / 5000;
	else 
		return 0;

	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_FontPingFang_Regular41);
	GUI_UC_SetEncodeUTF8();
	GUI_DispStringAt("\xe4\xba\x8c\xe6\xb0\xa7\xe5\x8c\x96\xe7\xa2\xb3 CO",5,496); // 二氧化碳
	GUI_UC_SetEncodeNone();
	GUI_DispCharAt(0x2082,172,494); // SUBSCRIPT 2

	GUI_SetFont(&GUI_FontPingFang_Regular38);
	GUI_UC_SetEncodeUTF8();
	if(SampleVal < 1200) 	
		GUI_DispStringAt("\xe4\xb8\x8d\xe7\xbc\xba\xe6\xb0\xa7",220,536); // 不缺氧
	else if(SampleVal >= 1200 && SampleVal <= 2400) 
		GUI_DispStringAt("\xe5\xbe\xae\xe7\xbc\xba\xe6\xb0\xa7",220,536); // 微缺氧
	else if(SampleVal > 2400 && SampleVal <= 4000) 
		GUI_DispStringAt("\xe7\xbc\xba\xe6\xb0\xa7",220,536); // 缺氧
	else if(SampleVal > 4000) 
		GUI_DispStringAt("\xe9\x87\x8d\xe7\xbc\xba\xe6\xb0\xa7",220,536); // 重缺氧

	sprintf(buf, "%d ppm", SampleVal);
	GUI_DispStringAt(buf,220,496); // ppm

	_CreateProgressBarligttle(5,550,205,565,5,x_offset);
}

static int GUI_ProgbarCustom_EMR(int SampleVal,int Sensor)
{
	int x_offset = 0;
	char buf[32];
	
	if(Sensor == PM25_TY)
		x_offset = SampleVal / 2;
	else if(Sensor == CO2_TY)
		x_offset = SampleVal / 5;
	else if(Sensor == EMR_TY)
		x_offset = (200 * SampleVal) / 500;
	else 
		return 0;

	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_FontPingFang_Regular41);
	GUI_UC_SetEncodeUTF8();
	GUI_DispStringAt("\xe7\x94\xb5\xe7\xa3\x81\xe8\xbe\x90\xe5\xb0\x84",5,645); // 电磁辐射

	GUI_SetFont(&GUI_FontPingFang_Regular38);
	if(SampleVal < 50) 	
		GUI_DispStringAt("\xe6\x97\xa0\xe6\xb1\xa1\xe6\x9f\x93",220,689); // 无污染
	else if(SampleVal >= 50 && SampleVal <= 120) 
		GUI_DispStringAt("\xe4\xb8\xad\xe6\xb1\xa1\xe6\x9f\x93",220,689); // 中污染
	else if(SampleVal > 120) 
		GUI_DispStringAt("\xe9\x87\x8d\xe6\xb1\xa1\xe6\x9f\x93",220,689); // 重污染

	sprintf(buf, "%d    W", SampleVal);
	GUI_DispStringAt(buf,220,648);
	GUI_UC_SetEncodeNone();
	GUI_DispCharAt(0xb5,255,645);

	GUI_SetFont(&GUI_FontPingFang_Regular33);
	GUI_UC_SetEncodeUTF8();
	GUI_DispStringAt("\xe5\xae\x9e\xe6\x97\xb6",400,699); // 实时

	_CreateProgressBarligttle(5,700,205,715,5,x_offset);
}


static void GUI_SampleValue_Disp(int PM25_Value,int PM10_Value,int CO2_Value,
											  int PM03_Count,int PM25_Count,int PM10_Count,
											  int ELEC_Value)
{
	GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();
	GUI_SetColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_TRANS);

	GUI_SetFont(&GUI_FontPingFang_Regular33);
	GUI_UC_SetEncodeUTF8();
	
	GUI_DispStringAt("\xe6\xbf\x80\xe5\x85\x89\xe4\xb8\xaa\xe6\x95\xb0 \xe4\xb8\xaa/\xe5\x8d\x87 \x28L\x29",30,320);// 激光个数
	GUI_DispStringAt("PM0.3\x7e 2.5",30,360);
	GUI_DispStringAt("PM2.5\x7e 10",30,395);
	GUI_DispStringAt("PM10\xe4\xbb\xa5\xe4\xb8\x8a",30,430);

	GUI_SetFont(&GUI_FontPingFang_Regular38);
	GUI_GotoXY(175, 360); 
	GUI_DispDecMin(PM03_Count); // pm0.3 pm counting
	GUI_GotoXY(175, 395);
	GUI_DispDecMin(PM25_Count); // pm2.5 pm counting
	GUI_GotoXY(175, 430);
	GUI_DispDecMin(PM10_Count); // pm10 pm counting

	GUI_SetFont(&GUI_FontPingFang_Regular45);
    GUI_DispStringAt("PM2.5",5,180);
	GUI_DispStringAt("PM10" ,5,245);

	GUI_SetFont(&GUI_FontPingFang_Regular67);
	GUI_GotoXY(120, 170); 
	GUI_DispDecMin(PM25_Value); // pm2.5
	GUI_GotoXY(120, 235); 
	GUI_DispDecMin(PM10_Value); // pm10
	
	GUI_ProgbarCustom_PM(PM25_Value,PM25_TY);
	GUI_ProgbarCustom_CO2(CO2_Value,CO2_TY);
	GUI_ProgbarCustom_EMR(ELEC_Value,EMR_TY);

}

/*********************************************************************
*
*       _CreateSelected
*
* Purpose:
*   This function is called when a button is pressed. It creates a new
*   window and shows a small screen animation.
*/
static void _CreateSelected(int Index, WM_HWIN hWin) {
  WM_HWIN hWinBase;     // Window to be shifted out
  WM_HWIN hWinSelected; // New window to become visible behind hWinBase
  
  int xSize, ySize;

  hWinBase = WM_GetParent(WM_GetParent(hWin));
  xSize = WM_GetXSize(hWinBase);
  ySize = WM_GetYSize(hWinBase);

  if(Index == 0)
	BUTTON_STATE = BUTTON_PAGE_TWO_L;	// go to page 2 (right arrow)
  else if(Index == 1)
	BUTTON_STATE = BUTTON_PAGE_THREE_L; // go to page 3 (right arrow)
  else if(Index == 2)
	BUTTON_STATE = BUTTON_PAGE_ONE; 	// back to page 1 (left arrow)
  else if(Index == 3)
	BUTTON_STATE = BUTTON_PAGE_FOUR_L;	// go to page 4 (right arrow)
  else if(Index == 4)
	BUTTON_STATE = BUTTON_PAGE_TWO_L;	// back to page 2 left arrow)
  else if(Index == 1000)
	BUTTON_STATE = BUTTON_PAGE_TWO_L;	// windows back to page 2
}

/*********************************************************************
*
*       _cbMenu
*
* Purpose:
*   Callback function of menu window.
*/
static void _cbMenuPageOne(WM_MESSAGE * pMsg) {
  int xSize, i, NCode;
  int Id = 0;
  
  WM_MOTION_INFO * pInfo;
  WM_HWIN hWin, hButton;
  static int IsPressed;

  hWin = pMsg->hWin;
  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
    NCode = pMsg->Data.v;                 // Notification code

	switch (NCode) {
    case WM_NOTIFICATION_CLICKED:
       	IsPressed = 1;
		if(Id == GUI_ID_BUTTON9){
			dev_state.IsPM25Pressed = 1;
			dev_state.PM25PressedCtr = 0;
			
		}
		if(Id == GUI_ID_USER_BUTTON_0){
		}
		if(Id == GUI_ID_USER_BUTTON_1){
			dev_state.IsCO2Pressed = 1;
			dev_state.CO2PressedCtr = 0;
		}
		if(Id == GUI_ID_USER_BUTTON_2){
		}
		else{
			_CreateSelected(Id - GUI_ID_BUTTON0, pMsg->hWin);
		}

		WM_InvalidateWindow(hWin);
      break;
    case WM_NOTIFICATION_RELEASED:
      if (IsPressed) {
        //
        // React only if window is not moving
        //
        	IsPressed = 0;
      }
      break;
    }
    break;
  case WM_CREATE:
	    //
	    // Create all buttons
	    //
	    xSize = WM_GetWindowSizeX(hWin);

		WM_CreateTimer(hWin, 0, 2000, 0);

		switch ((int)BUTTON_STATE) {
			case BUTTON_PAGE_ONE:
				i = 0;
	    		hButton = BUTTON_CreateUser(0, 730, xSize, 124, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
	    		BUTTON_SetSkin(hButton, _ButtonSkin);
	    		BUTTON_SetUserData(hButton, &i, sizeof(i));
				break;
			case BUTTON_PAGE_TWO_L:
				break;
			case BUTTON_PAGE_THREE_L:
				break;
			case BUTTON_PAGE_FOUR_L:
				break;
			case BUTTON_PAGE_FIVE:
				break;
		}
	    break;
  case WM_TIMER:
	    WM_InvalidateWindow(hWin);
	    WM_RestartTimer(pMsg->Data.v, 0);
    break;
	case WM_PAINT:
		GUI_SampleValue_Disp((int)(g_CgkSensorData.uiPm25Ug * 1.5f),(int)(g_CgkSensorData.uiPm10Ug * 1.5f),g_CgkSensorData.uiCO2Dat,
							 (int)(g_CgkSensorData.uiPm03Dat * 1.5f),(int)(g_CgkSensorData.uiPm25Dat * 1.5f),(int)(g_CgkSensorData.uiPm10Dat * 1.5f),
							 g_CgkSensorData.uiElecDat); // 1.5f for testing
		break;
  default:
    WM_DefaultProc(pMsg);
  }
}

/*********************************************************************
*
*       _cbMenu
*
* Purpose:
*   Callback function of menu window.
*/
static void _cbMenuPageTwo(WM_MESSAGE * pMsg) {
  int xSize, i, NCode;
  int Id = 0;
  
  WM_MOTION_INFO * pInfo;
  WM_HWIN hWin, hButton;
  static int IsPressed;

  hWin = pMsg->hWin;
  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
    NCode = pMsg->Data.v;                 // Notification code

	switch (Id) 
	{
		case GUI_ID_ICONVIEW0:
			switch (NCode) 
			{
				case WM_NOTIFICATION_SEL_CHANGED:
			        break;
				case WM_NOTIFICATION_CLICKED:
					_IconModules[ICONVIEW_GetSel(hIcon)](hWin);
					break;
				case WM_NOTIFICATION_RELEASED:
					
					break;
			}
			break;
		default:
			switch (NCode)
			{
				case WM_NOTIFICATION_CLICKED:
		  			IsPressed = 1;
		  			_CreateSelected(Id - GUI_ID_BUTTON0, pMsg->hWin);
		  			break;
				case WM_NOTIFICATION_RELEASED:
		  			if (IsPressed) {
			        //
			        // React only if window is not moving
			        //
			        IsPressed = 0;
					break;
		  			}
	    	}
	}
    break;
  case WM_CREATE:
	    //
	    // Create all buttons
	    //
	    xSize = WM_GetWindowSizeX(hWin);

		switch ((int)BUTTON_STATE) {
			
			case BUTTON_PAGE_ONE:
				break;
			case BUTTON_PAGE_TWO_L:
				i = 1;
	    		hButton = BUTTON_CreateUser(240, 730, xSize / 2, 124, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
	    		BUTTON_SetSkin(hButton, _ButtonSkinPageTwoRight);
	    		BUTTON_SetUserData(hButton, &i, sizeof(i));

				i = 2;
    			hButton = BUTTON_CreateUser(0, 730, xSize / 2, 124, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
    			BUTTON_SetSkin(hButton, _ButtonSkinPageTwoLeft);
    			BUTTON_SetUserData(hButton, &i, sizeof(i));
				break;
			case BUTTON_PAGE_THREE_L:
				break;
			case BUTTON_PAGE_FOUR_L:
				break;
			case BUTTON_PAGE_FIVE:
				break;
		}
	    break;
	case WM_PAINT:
		GUI_SetBkColor(GUI_BLACK);
	    GUI_Clear();
		GUI_SetColor(GUI_WHITE);
	    GUI_SetTextMode(GUI_TM_TRANS);
		GUI_SetFont(&GUI_FontPingFang_Regular45);
		GUI_UC_SetEncodeUTF8();
		GUI_DispStringAt("\xe7\xb4\xab\xe5\xa4\x96\xe7\xba\xbf",180,44); 				// 紫外线
		GUI_DispStringAt("\xe6\xa3\x80\xe6\xb5\x8b",180,90); 							// 检测
		GUI_DispStringAt("\xe4\xba\x8c\xe6\xb0\xa7\xe5\x8c\x96\xe7\xa2\xb3",180,200); 	// 二氧化碳
		GUI_DispStringAt("\xe6\x88\xb7\xe5\xa4\x96\xe6\xa0\xa1\xe5\x87\x86",180,246); 	// 户外校准
		break;
  default:
    WM_DefaultProc(pMsg);
  }
}

static int _ButtonSkinPageThreeSet(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  int Index,IsPressed;
  WM_HWIN hWin;
  GUI_COLOR Color;
  
  hWin = pDrawItemInfo->hWin;
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_BACKGROUND:
    //
    // Get state of button
    //
    IsPressed = BUTTON_IsPressed(pDrawItemInfo->hWin);
    //
    // Get button index
    //
    BUTTON_GetUserData(hWin, &Index, sizeof(Index));
    //
    // Draw background color depending on state
    //
    if (IsPressed) {
      Color = GUI_MAGENTA;
    } else {
      Color = GUI_BLACK;
    }
    GUI_SetBkColor(Color);
    GUI_Clear();
    //
    // Draw arrow at the right
    //
    GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetFont(&GUI_FontPingFang_Regular41);
	GUI_UC_SetEncodeUTF8();
	if(Index == GUI_ID_USER_PM)
		GUI_DispStringAt("\xe7\xb2\x89\xe5\xb0\x98\xe8\x87\xaa\xe5\x8a\xa8\xe6\xa3\x80\xe6\xb5\x8b\xe9\x97\xb4\xe9\x9a\x94",50,10); // 粉尘自动检测间隔
	else if(Index == GUI_ID_USER_SYS)
		GUI_DispStringAt("\xe7\xb3\xbb\xe7\xbb\x9f\xe8\xae\xbe\xe7\xbd\xae",50,10); 				// 系统设置
	else if(Index == GUI_ID_USER_NET)
		GUI_DispStringAt("\xe7\xbd\x91\xe7\xbb\x9c\xe8\xae\xbe\xe7\xbd\xae",50,10); 				// 网络设置
	else if(Index == GUI_ID_USER_BRL)
		GUI_DispStringAt("\xe5\xb1\x8f\xe5\xb9\x95\xe4\xba\xae\xe5\xba\xa6",50,10); 				// 屏幕亮度
	else if(Index == GUI_ID_USER_SET)
		GUI_DispStringAt("\xe5\xb7\xa5\xe7\xa8\x8b\xe5\xb8\x88\xe8\xae\xbe\xe7\xbd\xae",50,10); 	// 工程师设置
	else if(Index == GUI_ID_USER_DET_M)
		GUI_DispStringAt("\xe6\x89\x8b\xe5\x8a\xa8\xe6\xa3\x80\xe6\xb5\x8b",0,0); 	// 手动检测
	else if(Index == GUI_ID_USER_DET_1)
		GUI_DispStringAt("\xe6\xaf\x8f\xe5\xa4\xa9\xe6\xa3\x80\xe6\xb5\x8b\x31\xe6\xac\xa1",0,0); 	// 每天检测1次
	else if(Index == GUI_ID_USER_DET_2)
		GUI_DispStringAt("\xe6\xaf\x8f\xe5\xa4\xa9\xe6\xa3\x80\xe6\xb5\x8b\x32\xe6\xac\xa1",0,0); 	// 每天检测2次
	else if(Index == GUI_ID_USER_DET_4)
		GUI_DispStringAt("\xe6\xaf\x8f\xe5\xa4\xa9\xe6\xa3\x80\xe6\xb5\x8b\x34\xe6\xac\xa1",0,0); 	// 每天检测4次
    break;
  }
  return 0;
}

/*********************************************************************
*
*       _cbMenu
*
* Purpose:
*   Callback function of menu window.
*/
static void _cbMenuPageThree(WM_MESSAGE * pMsg) {
  int xSize, i, NCode;
  int Id = 0;
  
  WM_MOTION_INFO * pInfo;
  WM_HWIN hWin, hButton;
  static int IsPressed;

  hWin = pMsg->hWin;
  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
    NCode = pMsg->Data.v;                 // Notification code

	switch (NCode) {
    case WM_NOTIFICATION_CLICKED:
      	IsPressed = 1;
		
		if(Id == GUI_ID_USER_PM){
			App_PMSetCtr(hWin);
		}
		else if(Id == GUI_ID_USER_SYS){
			//App_SysSetCtr(hWin);
		}
		else if(Id == GUI_ID_USER_NET){
			//App_NetSetCtr(hWin);
		}
		else if(Id == GUI_ID_USER_SET){
			//App_EnSetCtr(hWin);
		}
		else if(Id == GUI_ID_USER_BRL){
			//App_BrlSetCtr(hWin);
		}
		else{
	  		_CreateSelected(Id - GUI_ID_BUTTON0, pMsg->hWin);
		}
      break;
    case WM_NOTIFICATION_RELEASED:
      if (IsPressed) {
        //
        // React only if window is not moving
        //
		
        IsPressed = 0;
      }
      break;
    }
    break;
  case WM_CREATE:
	    //
	    // Create all buttons
	    //
	    xSize = WM_GetWindowSizeX(hWin);

		switch ((int)BUTTON_STATE) {
			
			case BUTTON_PAGE_ONE:
				break;
			case BUTTON_PAGE_TWO_L:
				break;
			case BUTTON_PAGE_THREE_L:

				i = 4;
	    		hButton = BUTTON_CreateUser(0, 730, xSize, 124, hWin, WM_CF_SHOW, 0, GUI_ID_BUTTON0 + i, sizeof(i));
	    		BUTTON_SetSkin(hButton, _ButtonSkinPageThreeLeft);
	    		BUTTON_SetUserData(hButton, &i, sizeof(i));

				i = GUI_ID_USER_PM;
				hButton = BUTTON_CreateUser(0, 60, xSize, 50, hWin, WM_CF_SHOW, 0, GUI_ID_USER_PM, sizeof(i));
	    		BUTTON_SetSkin(hButton, _ButtonSkinPageThreeSet);
	    		BUTTON_SetUserData(hButton, &i, sizeof(i));

				i = GUI_ID_USER_SYS;
				hButton = BUTTON_CreateUser(0, 110, xSize, 50, hWin, WM_CF_SHOW, 0, GUI_ID_USER_SYS, sizeof(i));
	    		BUTTON_SetSkin(hButton, _ButtonSkinPageThreeSet);
	    		BUTTON_SetUserData(hButton, &i, sizeof(i));

				i = GUI_ID_USER_NET;
				hButton = BUTTON_CreateUser(0, 160, xSize, 50, hWin, WM_CF_SHOW, 0, GUI_ID_USER_NET, sizeof(i));
	    		BUTTON_SetSkin(hButton, _ButtonSkinPageThreeSet);
	    		BUTTON_SetUserData(hButton, &i, sizeof(i));

				i = GUI_ID_USER_BRL;
				hButton = BUTTON_CreateUser(0, 210, xSize, 50, hWin, WM_CF_SHOW, 0, GUI_ID_USER_BRL, sizeof(i));
	    		BUTTON_SetSkin(hButton, _ButtonSkinPageThreeSet);
	    		BUTTON_SetUserData(hButton, &i, sizeof(i));

				i = GUI_ID_USER_SET;
				hButton = BUTTON_CreateUser(0, 260, xSize, 50, hWin, WM_CF_SHOW, 0, GUI_ID_USER_SET, sizeof(i));
	    		BUTTON_SetSkin(hButton, _ButtonSkinPageThreeSet);
	    		BUTTON_SetUserData(hButton, &i, sizeof(i));
				break;
			case BUTTON_PAGE_FOUR_L:
				break;
			case BUTTON_PAGE_FIVE:
				break;
		}
	    break;
	case WM_PAINT:
		GUI_SetBkColor(GUI_BLACK);
	    GUI_Clear();
		GUI_SetColor(GUI_DARKGRAY);
    	GUI_DrawHLine(316, 0, 480);
		GUI_DrawHLine(317, 0, 480);
		GUI_DrawHLine(318, 0, 480);
		GUI_SetColor(GUI_WHITE);
	    GUI_SetTextMode(GUI_TM_TRANS);
		GUI_UC_SetEncodeUTF8();
		GUI_SetFont(&GUI_FontPingFang_Regular45);
		GUI_DispStringAt("\xe5\x8f\x82\xe6\x95\xb0\xe8\xae\xbe\xe7\xbd\xae",5,15); 				// 参数设置
		break;
  default:
    WM_DefaultProc(pMsg);
  }
}

/*********************************************************************
*
*       _cbDummy
*
* Purpose:
*   Required for base window and viewport window to make sure WM_DefaultProc() is called.
*/
static void _cbDummy(WM_MESSAGE * pMsg) {
  switch (pMsg->MsgId) {
  default:
    WM_DefaultProc(pMsg);
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MainTask
*
* Purpose:
*   
*/

void SmartPhomeMenuMainTask(void) {
  int xSize, ySize;
  static WM_HWIN hWinBase;     // Parent window for heading and viewport
  static WM_HWIN hWinHeading;  // Heading window
  static WM_HWIN hWinViewport; // Viewport window
  static WM_HWIN hWinMenu;     // Menu window moveable within viewport window
  static WM_HWIN hButton_0;
  static WM_HWIN hButton_1;
  static WM_HWIN hButton_2;
  static WM_HWIN hButton_3;

  WM_SetCreateFlags(WM_CF_MEMDEV);
  
  GUI_Init();
  
  //WM_MULTIBUF_Enable(1);

  BUTTON_SetReactOnLevel();
  //
  // Get display dimension
  // 
  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  //
  // Limit desktop window to display size
  //
  WM_SetSize(WM_HBKWIN, xSize, ySize);
  //
  //
  //
  
  BUTTON_STATE = BUTTON_PAGE_ONE;

  //
  // Create windows
  //
  hWinBase     = WM_CreateWindow       (0,    0, xSize, ySize,                             WM_CF_SHOW, _cbDummy,   0);
  hWinHeading  = WM_CreateWindowAsChild(0,    0, xSize, 40,                    hWinBase,   WM_CF_SHOW, _cbHeading, 0);

  memset(&dev_state, NULL, sizeof(DEV_STATE_CTR_T));
  //
  // Keep demo alive
  //
  while (1) {
	switch ((int)BUTTON_STATE) {
		case BUTTON_PAGE_ONE:
			WM_DeleteWindow(hWinMenu);
			WM_Custom_DeleteButtonWindow();
			
			hWinMenu = WM_CreateWindowAsChild(0,  40, xSize, ySize - 40, hWinBase, WM_CF_SHOW, _cbMenuPageOne, 0);

			hButton_0 = BUTTON_CreateUser(360, 160, 130, 120, hWinMenu, WM_CF_SHOW, 0, GUI_ID_BUTTON9, 0);
  			BUTTON_SetSkin(hButton_0, _Button_PM25_Skin);
			hButton_1 = BUTTON_CreateUser(350, 280, 130, 55, hWinMenu, WM_CF_SHOW, 0, GUI_ID_USER_BUTTON_0, 0);
  			BUTTON_SetSkin(hButton_1, _ButtonSkinAddr);
			
			hButton_2 = BUTTON_CreateUser(360, 450, 130, 120, hWinMenu, WM_CF_SHOW, 0, GUI_ID_USER_BUTTON_1, 0);
  			BUTTON_SetSkin(hButton_2, _Button_CO2_Skin);
			hButton_3 = BUTTON_CreateUser(350, 570, 130, 55, hWinMenu, WM_CF_SHOW, 0, GUI_ID_USER_BUTTON_2, 0);
  			BUTTON_SetSkin(hButton_3, _ButtonSkinAddr);

			BUTTON_STATE = BUTTON_PAGE_IDLE;
			
			break;
		case BUTTON_PAGE_TWO_L:
			WM_DeleteWindow(hWinMenu);
			WM_Custom_DeleteButtonWindow();
			
			hWinMenu = WM_CreateWindowAsChild(0,  40, xSize, ySize - 40, hWinBase, WM_CF_SHOW, _cbMenuPageTwo, 0);

			hIcon =  _CreateICONVIEW(hWinMenu, 
	                				_aBitmapItem,
									GUI_COUNTOF(_aBitmapItem), 
									GUI_ID_ICONVIEW0, 
									ICONVIEW_LBorder, 
									ICONVIEW_TBorder, 
									ICONVIEW_Width, 
									ICONVIEW_Height * 5);

			BUTTON_STATE = BUTTON_PAGE_IDLE;
			break;
		case BUTTON_PAGE_THREE_L:
			WM_DeleteWindow(hWinMenu);
			WM_Custom_DeleteButtonWindow();
			
			hWinMenu = WM_CreateWindowAsChild(0,  40, xSize, ySize - 40, hWinBase, WM_CF_SHOW, _cbMenuPageThree, 0);
			BUTTON_STATE = BUTTON_PAGE_IDLE;
			break;
		case BUTTON_PAGE_FOUR_L:
			break;
		case BUTTON_PAGE_FIVE:
			break;
		case BUTTON_PAGE_IDLE:
			break;
	}
	GUI_Delay(70);
  }
}

/*************************** End of file ****************************/

