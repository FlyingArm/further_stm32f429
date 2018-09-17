/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2015  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.30 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only  be used  in accordance  with  a license  and should  not be  re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : MainTask.c
Purpose     : Wheather forecast demo using multiple layers
Requirements: WindowManager - ( )
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )

              Can be used in a MS Windows environment only!
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>
#include "main_house.h"
#include "includes.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define XOFF_TEXT0 50
#define XOFF_TEXT1 55
#define XOFF_IMAGE -100//(-210u)

#define JOB_ANIM_IN            1
#define JOB_ANIM_FACE_CHANGE   2
#define JOB_ANIM_TEXT_IN       3
#define JOB_ANIM_IN_DROP       4
#define JOB_ANIM_OUT           5
#define JOB_ANIM_MODE_EXPERT   6	
#define JOB_ANIM_IDLE          7

#define YPOS_FORECAST       0
#define YSIZE_FORECAST      60
#define YSIZE_FORECAST_IMG  (320u)/*(479u)*/

#define YSIZE_FORECAST_FRAME ((YSIZE_FORECAST - YSIZE_FORECAST_IMG) / 2)
#define YPOS_FORECAST_IMG    YPOS_FORECAST

#define XPOS_TEXT  100
#define YPOS_TEXT   350
#define XSIZE_TEXT 280
#define YSIZE_TEXT  200

#define INDICATOR_R      5
#define INDICATOR_XDIST 20

#define TIME_IDLE 5000

//
// Recommended memory to run the sample with adequate performance
//
#define RECOMMENDED_MEMORY (512 * 1024) // Should be adapted, value is too less orignal is 24

#define GUI_Log printf

/*********************************************************************
*
*       Extern Fun
*
**********************************************************************
*/
extern int NormalWorkModeAnimation(void);

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
//
// Indices for Mode
//
typedef enum {
	  Mode_Compagnon_QAI = 0,
	  Sub_Mode_Compagnon_QAI,
	  Mode_Expert_QAI,
	  Sub_Mode_Expert_QAI,
	  Mode_Compagnon_QAE,
	  Sub_Mode_Compagnon_QAE,
	  Mode_Expert_QAE,
	  Sub_Mode_Expert_QAE,
	  Mode_Idle
} INDEX_WORK_MODE;
//
//
//
typedef struct _NORMAL_WORK_MODE
{
	int state;
	int QAI;
	int QAI_ALLOW;
	int QAI_EXT;
	int QAI_EXT_ALLOW;
	int QAE;
	int QAE_ALLOW;
	int QAE_EXT;
	int QAE_EXT_ALLOW;
}WORK_MODE;
//
//Indices for Mode QAI
//
typedef enum {
  QAI_Smile = 0,
  QAI_Light_Smile,
  QAI_Light_Pout,
  QAI_Pout,
} INDEX_QAI;
//
//Indices for Mode QAE
//
typedef enum {
  QAE_Smile = 0,
  QAE_Light_Smile,
  QAE_Light_Pout,
  QAE_Pout,
} INDEX_QAE;
//
// Indices for _apBmSym
//
typedef enum {
  INDEX_SYM_PARTLY_CLOUDY = 0,
  INDEX_SYM_CLOUD_LIGHTING,
  INDEX_SYM_RAIN,
  INDEX_SYM_CHANCE_OF_STORM,
  INDEX_SYM_SUN
} INDEX_SYM;

//
// Indices for images in _apBmBack
//
typedef enum {
  INDEX_BK_BERLIN = 0,
  INDEX_BK_MOSCOW,
  INDEX_BK_NY,
  INDEX_BK_TOKYO
} INDEX_BK;

//
// Positioning data for animated icons
//
typedef struct PARA PARA;
typedef struct WINDOW_DATA WINDOW_DATA;

struct PARA {
  int xPos;
  int xSizeScreen;
  int xSize;
  int OldPos;
  WINDOW_DATA * pData;
  int Index;
};

struct WINDOW_DATA {
  int xSize;
  int ySize;
  int xPos;
  int yOff;
  U8 Alpha;
  int xMag;
  int yMag;
  int xMag_pos;
  WM_HTIMER hTimer;
  WM_HWIN hWin;
  WM_HWIN hText;
  WM_HWIN hImageZoom;
  WM_HWIN hModeExpertQAI;
  GUI_ANIM_HANDLE hAnimMain;
  GUI_ANIM_HANDLE hAnimIdle;
  GUI_AUTODEV_INFO AutoDevInfo; // Information about what has to be displayed
  GUI_AUTODEV      AutoDev;
  PARA aPara[1];
  GUI_TIMER_TIME TimeLastTouch;
  int Job;
  int LastJob;
  int IndexCity;
  int IndexAnimIn;
  int IndexAnimOut;
  int HasStopped;
  int ShowText;
  int TextVisible;
  int Diff;
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
#if 0
GUI_CONST_STORAGE GUI_BITMAP * _apBmBack[] = {
  //&bmberlin_480x272,
  //&bmmoscow_480x272,
  //&bmnew_york_480x272,
  //&bmtokyo_480x272,
	NULL,
};

static GUI_CONST_STORAGE GUI_BITMAP * _apBmSym[] = {
  //&bmpartly_cloudy_day40x40,
  //&bmcloud_lighting40x40,
  //&bmrain40x40,
  //&bmchance_of_storm40x40,
  //&bmberlin_480x272
  //&bmsun40x40,
	NULL,
};

static const char * _pDays[] = { "MON", "TUE", "WED", "THU", "FRI" };

static const struct CITY {
  const char * pName;
  const char * pTime;
  int Temperature;
  int Weather;
  const unsigned aForeCast[5];
  const unsigned aTemp[5];
} _aCity[] = {
  {
    "BERLIN",
    "15:20",
    26,
    INDEX_BK_BERLIN,
    {
      INDEX_SYM_SUN ,
      INDEX_SYM_CLOUD_LIGHTING,
      INDEX_SYM_RAIN,
      INDEX_SYM_CHANCE_OF_STORM,
      INDEX_SYM_PARTLY_CLOUDY,
    },
    { 28, 24, 25, 29, 30 },
  },
  {
    "TOKYO",
    "23:20",
    22,
    INDEX_BK_TOKYO,
    {
      INDEX_SYM_PARTLY_CLOUDY,
      INDEX_SYM_CLOUD_LIGHTING,
      INDEX_SYM_RAIN,
      INDEX_SYM_CHANCE_OF_STORM,
      INDEX_SYM_SUN,
    },
    { 20, 18, 16, 19, 23 },
  },
  {
    "MOSCOW",
    "17:20",
    18,
    INDEX_BK_MOSCOW,
    {
      INDEX_SYM_SUN,
      INDEX_SYM_PARTLY_CLOUDY,
      INDEX_SYM_SUN,
      INDEX_SYM_CLOUD_LIGHTING,
      INDEX_SYM_RAIN,
    },
    { 21, 18, 16, 14, 14 },
  },
  {
    "NEW YORK",
    "10:20",
    25,
    INDEX_BK_NY,
    {
      INDEX_SYM_RAIN,
      INDEX_SYM_PARTLY_CLOUDY,
      INDEX_SYM_SUN,
      INDEX_SYM_SUN,
      INDEX_SYM_SUN,
    },
    { 28, 30, 32, 33, 30 },
  },
};
#endif
//
// Struct statement
//
GUI_ANIM_INFO ANIM_Para;
WINDOW_DATA ANIM_Data;
WORK_MODE mode;
int AnimFlyIn_xpos = 0;
static GUI_MEMDEV_Handle hMemBMP;
static GUI_MEMDEV_Handle hMemText;
GUI_ALPHA_STATE  AlphaState;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _SliceInfo
*/
static void _SliceInfo(int State, void * pVoid) {
  GUI_USE_PARA(pVoid);
  //
  // Avoid flickering of layer 0 during animation
  //
  switch (State) {
  case GUI_ANIM_START:
    GUI_MULTIBUF_Begin();
    break;
  case GUI_ANIM_END:
    GUI_MULTIBUF_End();
    break;
  }
}

/*********************************************************************
*
*       _MakeSymRect
*/
static void _MakeSymRect(GUI_RECT * pRect, int xPos, int xSize) {
  pRect->x0 = xPos + XOFF_IMAGE;
  pRect->x1 = pRect->x0 + xSize - 1;
  pRect->y0 = YPOS_FORECAST_IMG;
  pRect->y1 = pRect->y0 + YSIZE_FORECAST_IMG - 1;
}

static void _MakeSymTextRect(GUI_RECT * pRect, int xPos, int xSize) {
  pRect->x0 = xPos;
  pRect->x1 = pRect->x0 + xSize - 1;
  pRect->y0 = YPOS_TEXT;
  pRect->y1 = pRect->y0 + YSIZE_TEXT - 1;
}

/*********************************************************************
*
*       _AnimDrawSym
*/
static void _AnimDrawSym(PARA * pPara, int xPos, int IndexCity) {
  u32 t0, t1;
  WINDOW_DATA * pData;
	
  pData = &ANIM_Data;
  
  if((mode.state == Sub_Mode_Compagnon_QAI) && (pData->IndexAnimOut == 1))
  {
  	//GUI_Log1("\r\n_AnimDrawSym xPos  + pData->xMag_pos",xPos  + pData->xMag_pos);
  	GUI_DrawBitmapEx(&bmhouse_, xPos + pData->xMag_pos - 315 , 0, 
  					 /* pData->xMag_pos + XOFF_IMAGE + 20 */(xPos + pData->xMag_pos - 315) / 2, 0,pData->xMag, pData->yMag);
  }
  else if(mode.state == Sub_Mode_Compagnon_QAI)
  {
  	AnimFlyIn_xpos = xPos  + XOFF_IMAGE;
	//t0 = GUI_GetTime();
  	GUI_DrawBitmap(&bmhouse_320X200/*bmhouse_ */, xPos  + XOFF_IMAGE, YPOS_FORECAST_IMG);
	//t1 = GUI_GetTime() - t0;
	//printf("fps = %dms\r\n", t1);
  }
  else if(mode.state == Sub_Mode_Compagnon_QAE)
  {
  	AnimFlyIn_xpos = xPos  + XOFF_IMAGE;
  	GUI_DrawBitmap(&bmcloud_002, xPos  + XOFF_IMAGE, YPOS_FORECAST_IMG);
  }
  else if((mode.state == Sub_Mode_Compagnon_QAE) && (pData->IndexAnimOut == 1))
  {
	GUI_DrawBitmapEx(&bmcloud_002, xPos + pData->xMag_pos - 315 , 0, 
  					 xPos + pData->xMag_pos - 315, 0,pData->xMag, pData->yMag);
  }
}

static void _AnimDrawText(PARA * pPara, int xPos) {
  GUI_SetFont(&GUI_Font32B_ASCII);
  GUI_SetTextMode(GUI_TM_TRANS);
  //GUI_SetColor(0xB56D30);
  GUI_SetColor(GUI_WHITE);
  GUI_AA_PreserveTrans(1);
  GUI_DispStringAt("SATURDAY", xPos, YPOS_TEXT);
  GUI_GotoXY(xPos + XOFF_TEXT1, YPOS_TEXT + 20);
  GUI_DispDecMin(26);
  GUI_DispString("\xb0""C");
  GUI_AA_PreserveTrans(0);
}
/************************************************************************
* 
*/
#define GUI_DARKORANGE   0x000042CC
#define DISP_xPos 80
#define DISP_xPos_OFFSET 140

static void CO2_display_value(int xPos,int co2_sample_val)
{
	int lcd_x_size,lcd_y_size;
	
	lcd_x_size = LCD_GetXSize();
	lcd_y_size = LCD_GetYSize(); 

	/* 根据 CO2 指定显示颜色*/
	if(co2_sample_val <= 800u) GUI_SetColor(GUI_WHITE); 
	else if(800u < co2_sample_val && co2_sample_val <= 1000u) GUI_SetColor(GUI_YELLOW); 
	else if(1000u < co2_sample_val && co2_sample_val <= 1400u) { GUI_SetColor(GUI_ORANGE); }
	else if(co2_sample_val > 1400u) GUI_SetColor(GUI_DARKORANGE); 

	GUI_SetFont(&GUI_FontD32 ); 
	GUI_GotoXY(xPos + DISP_xPos_OFFSET * 2 , lcd_y_size / 2 + 50);
	GUI_DispDecMin(co2_sample_val);
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 2);
	GUI_DispNextLine();
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 2);
	GUI_DispNextLine();
	
	GUI_SetFont(&GUI_Font32B_ASCII ); 
	GUI_DispString("CO2");
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 2);
	GUI_DispNextLine();
	GUI_DispString("ppm");
}


static void VOC_display_value(int xPos,int voc_sample_val)
{
	float voc_ppb = 0;
	char voc_disp_str[5] = { 0 };
	int lcd_x_size, lcd_y_size;
	
	lcd_x_size = LCD_GetXSize();
	lcd_y_size = LCD_GetYSize(); 
	

	voc_ppb = voc_sample_val / 1000.0f;
	if(voc_ppb <= 0.3f) 
	{
		GUI_SetColor(GUI_WHITE); 
		strcpy(voc_disp_str, "L1");
	}
	else if(0.3 < voc_ppb && voc_ppb <= 1.0f) 
	{
		GUI_SetColor(GUI_YELLOW); 
		strcpy(voc_disp_str, "L2");
	}
	else if(1.0f < voc_ppb && voc_ppb <= 3.0f) 
	{ 
		GUI_SetColor(GUI_ORANGE); 
		strcpy(voc_disp_str, "L3");
	}
	else if(voc_ppb > 3.0f) 
	{
		GUI_SetColor(GUI_DARKORANGE); 
		strcpy(voc_disp_str, "L4");
	}
	
	GUI_SetFont(&GUI_Font32B_ASCII ); 
	GUI_GotoXY(xPos + DISP_xPos_OFFSET * 1, lcd_y_size / 2 + 50);
	GUI_DispString(voc_disp_str);
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 1);
	GUI_DispNextLine();
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 1);
	GUI_DispNextLine();
	
	GUI_SetFont(&GUI_Font32B_ASCII ); 
	GUI_DispString("VOC");
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 1);
	GUI_DispNextLine();
	GUI_DispString("ppb");
}


static void PM2005_display_value(int xPos,int pm2005_sample_val)
{
	int lcd_x_size, lcd_y_size;
	
	/* 获取 LCD x y 轴 */
	lcd_x_size = LCD_GetXSize();
	lcd_y_size = LCD_GetYSize(); 
	
	//GUI_SetTextMode(GUI_TM_TRANS);	// 设置为透明文本
	//GUI_SetBkColor(GUI_BLACK);
	/* 根据 PM2005  指定显示颜色*/
	if(pm2005_sample_val <= 25u) GUI_SetColor(GUI_WHITE); 
	else if(25u < pm2005_sample_val && pm2005_sample_val <= 50u) GUI_SetColor(GUI_YELLOW); 
	else if(50u < pm2005_sample_val && pm2005_sample_val <= 75u) { GUI_SetColor(GUI_ORANGE); }
	else if(pm2005_sample_val > 75u) GUI_SetColor(GUI_DARKORANGE); 
	
	GUI_SetFont(&GUI_FontD32 ); 
	GUI_GotoXY(xPos, lcd_y_size / 2 + 50);
	GUI_DispDecMin(pm2005_sample_val);
	
	GUI_SetLBorder(xPos);
	GUI_DispNextLine();
	
	GUI_SetLBorder(xPos);
	GUI_DispNextLine();
	
	GUI_SetFont(&GUI_Font32B_ASCII ); 
	GUI_DispString("PM2.5");
	
	GUI_SetLBorder(xPos);
	GUI_DispNextLine();
	GUI_DispString("ug/m3");
}

static void H2O_display_value(int xPos,int h2o_sample_val)
{
	int lcd_x_size,lcd_y_size;
	
	/* 获取 LCD x y 轴 */
	lcd_x_size = LCD_GetXSize();
	lcd_y_size = LCD_GetYSize(); 
	
	//GUI_SetTextMode(GUI_TM_TRANS);	// 设置为透明文本
	//GUI_SetBkColor(GUI_BLACK);
	/* 根据 H2O 指定显示颜色*/
	if(h2o_sample_val <= 40u) GUI_SetColor(GUI_WHITE); 
	else if(40u < h2o_sample_val && h2o_sample_val <= 60u) GUI_SetColor(GUI_YELLOW); 
	else if(60u < h2o_sample_val && h2o_sample_val <= 70u) { GUI_SetColor(GUI_ORANGE); }
	else if(h2o_sample_val > 70u) GUI_SetColor(GUI_DARKORANGE); 
	
	GUI_SetFont(&GUI_FontD32 ); 
	GUI_GotoXY(xPos + DISP_xPos_OFFSET * 3, lcd_y_size / 2 + 50);
	GUI_DispDecMin(h2o_sample_val);
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 3);
	GUI_DispNextLine();
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 3);
	GUI_DispNextLine();
	
	GUI_SetFont(&GUI_Font32B_ASCII ); 
	GUI_DispString("H2O");
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 3);
	GUI_DispNextLine();
	GUI_DispString("%");
}

static void TEMP_display_value(int xPos,int temp_sample_val)
{
	int lcd_x_size,lcd_y_size;
	
	/* 获取 LCD x y 轴 */
	lcd_x_size = LCD_GetXSize();
	lcd_y_size = LCD_GetYSize(); 
	
	//GUI_SetTextMode(GUI_TM_TRANS);	// 设置为透明文本
	//GUI_SetBkColor(GUI_BLACK);
    GUI_SetColor(GUI_WHITE); 

	#if 0
	/* 根据 Temp 指定显示颜色*/
	if(temp_sample_val <= 10u) GUI_SetColor(GUI_WHITE); 
	else if(10u < temp_sample_val && temp_sample_val <= 20u) GUI_SetColor(GUI_YELLOW); 
	else if(20u < temp_sample_val && temp_sample_val <= 35u) { GUI_SetColor(GUI_ORANGE); }
	else if(temp_sample_val > 35u) GUI_SetColor(GUI_DARKORANGE); 
	#endif
	
	GUI_SetFont(&GUI_FontD32 ); 
	GUI_GotoXY(xPos + DISP_xPos_OFFSET * 4, lcd_y_size / 2 + 50);
	GUI_DispDecMin(temp_sample_val);
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 4);
	GUI_DispNextLine();
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 4);
	GUI_DispNextLine();
	
	GUI_SetFont(&GUI_Font32B_ASCII ); 
	GUI_DispString("Temp");
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 4);
	GUI_DispNextLine();
	GUI_DispString("\xb0""C");
}

static void PM10_display_value(int xPos,int pm10_sample_val)
{
	int lcd_x_size, lcd_y_size;
	
	lcd_x_size = LCD_GetXSize();
	lcd_y_size = LCD_GetYSize(); 
	
	if(pm10_sample_val <= 25u) GUI_SetColor(GUI_WHITE); 
	else if(25u < pm10_sample_val && pm10_sample_val <= 50u) GUI_SetColor(GUI_YELLOW); 
	else if(50u < pm10_sample_val && pm10_sample_val <= 75u) { GUI_SetColor(GUI_ORANGE); }
	else if(pm10_sample_val > 75u) GUI_SetColor(GUI_DARKORANGE); 
	
	GUI_SetFont(&GUI_FontD32 ); 
	GUI_GotoXY(xPos + DISP_xPos_OFFSET * 1, lcd_y_size / 2 + 50);
	GUI_DispDecMin(pm10_sample_val);
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 1);
	GUI_DispNextLine();
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 1);
	GUI_DispNextLine();
	
	GUI_SetFont(&GUI_Font32B_ASCII ); 
	GUI_DispString("PM10");
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 1);
	GUI_DispNextLine();
	GUI_DispString("ug/m3");
}

static void NO2_display_value(int xPos,int no2_sample_val)
{
	int lcd_x_size,lcd_y_size;
	
	lcd_x_size = LCD_GetXSize();
	lcd_y_size = LCD_GetYSize(); 
	
	if(no2_sample_val <= 40u) GUI_SetColor(GUI_WHITE); 
	else if(40u < no2_sample_val && no2_sample_val <= 60u) GUI_SetColor(GUI_YELLOW); 
	else if(60u < no2_sample_val && no2_sample_val <= 70u) { GUI_SetColor(GUI_ORANGE); }
	else if(no2_sample_val > 70u) GUI_SetColor(GUI_DARKORANGE); 
	
	GUI_SetFont(&GUI_FontD32 ); 
	GUI_GotoXY(xPos + DISP_xPos_OFFSET * 3, lcd_y_size / 2 + 50);
	GUI_DispDecMin(no2_sample_val);
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 3);
	GUI_DispNextLine();
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 3);
	GUI_DispNextLine();
	
	GUI_SetFont(&GUI_Font32B_ASCII ); 
	GUI_DispString("NO2");
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 3);
	GUI_DispNextLine();
	GUI_DispString("ppb");
}

static void O3_display_value(int xPos,int o3_sample_val)
{
	int lcd_x_size,lcd_y_size;
	
	lcd_x_size = LCD_GetXSize();
	lcd_y_size = LCD_GetYSize(); 

	if(o3_sample_val <= 800u) GUI_SetColor(GUI_WHITE); 
	else if(800u < o3_sample_val && o3_sample_val <= 1000u) GUI_SetColor(GUI_YELLOW); 
	else if(1000u < o3_sample_val && o3_sample_val <= 1400u) { GUI_SetColor(GUI_ORANGE); }
	else if(o3_sample_val > 1400u) GUI_SetColor(GUI_DARKORANGE); 

	GUI_SetFont(&GUI_FontD32 ); 
	GUI_GotoXY(xPos + DISP_xPos_OFFSET * 2 , lcd_y_size / 2 + 50);
	GUI_DispDecMin(o3_sample_val);
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 2);
	GUI_DispNextLine();
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 2);
	GUI_DispNextLine();
	
	GUI_SetFont(&GUI_Font32B_ASCII ); 
	GUI_DispString("O3");
	
	GUI_SetLBorder(xPos + DISP_xPos_OFFSET * 2);
	GUI_DispNextLine();
	GUI_DispString("ppb");
}


static void _AnimDrawModeExpertQAI(PARA * pPara, int xPos) {
	CO2_display_value(xPos,900u);
	VOC_display_value(xPos,3000u);
	PM2005_display_value(xPos,76u);
	H2O_display_value(xPos,30u);
	TEMP_display_value(xPos,15u);
}

static void _AnimDrawModeExpertQAE(PARA * pPara, int xPos) {
	O3_display_value(xPos,900u);
	PM10_display_value(xPos,52u);
	PM2005_display_value(xPos,76u);
	NO2_display_value(xPos,30u);
	TEMP_display_value(xPos,15u);
}

/*********************************************************************
*
*       _MakeTextRect
*/
static void _MakeTextRect(GUI_RECT * pRect, int yOff) {
  pRect->x0 = XPOS_TEXT - 1;
  pRect->y0 = YPOS_TEXT - yOff - 1;
  pRect->x1 = pRect->x0 + XSIZE_TEXT - 1;
  pRect->y1 = pRect->y0 + YSIZE_TEXT - 1;
}

/*********************************************************************
*
*       _DrawBar
*/
static void _DrawBar(int x0, int y0, int x1, int y1, int SkipBorder) {
  if (SkipBorder) {
    //
    // Draw only inner part
    // 
	GUI_DrawGradientV(x0, y0, x1, y1, GUI_BLACK, GUI_BLACK);
  } 
}

/*********************************************************************
*
*       _DrawBarEx
*/
static void _DrawBarEx(GUI_RECT * pRect) {
  _DrawBar(pRect->x0, pRect->y0, pRect->x1, pRect->y1, 1);
}

/*********************************************************************
*
*       _DrawIndicators
*/
static void _DrawIndicators(WINDOW_DATA * pData) {

}

/*********************************************************************
*
*       _AnimFlyIn
*/
extern void LCD_Clear(uint16_t Color);

static void _AnimFlyIn(GUI_ANIM_INFO * pInfo, void * pVoid) {
  PARA * pPara;
  GUI_RECT Rect;
  int Dyn, Fix, xPos;

  pPara = (PARA *)pVoid;
  WM_SelectWindow(pPara->pData->hWin);
  //GUI_SelectLayer(1);
  //GUI_SetBkColor(0x55FFFFFF);
  Fix   = pPara->xSizeScreen + pPara->xPos - pPara->xSize / 2;
  if (pInfo->State != GUI_ANIM_START) {
    Dyn = (pPara->xSizeScreen * pPara->OldPos) / GUI_ANIM_RANGE;
    _MakeSymRect(&Rect, Fix - Dyn, pPara->xSize);
	//GUI_ClearRectEx(&Rect);
    //_DrawBarEx(&Rect);
    //GUI_Clear();
    LCD_Clear(0x00);
  }
  pPara->OldPos = (pInfo->State == GUI_ANIM_END) ? 0 : pInfo->Pos;
  Dyn = (pPara->xSizeScreen * pInfo->Pos) / ((GUI_ANIM_RANGE * 3)/2);
  xPos = Fix - Dyn;
  
  memcpy(&ANIM_Para,pInfo,sizeof(GUI_ANIM_INFO)); /* TODO:注意保护数据 */

  _AnimDrawSym(pPara, xPos, pPara->pData->IndexAnimIn);
}

static void _AnimTextFlyIn(GUI_ANIM_INFO * pInfo, void * pVoid) {
  PARA * pPara;
  GUI_RECT Rect;
  int Dyn, Fix, xPos;

  pPara = (PARA *)pVoid;
  WM_SelectWindow(pPara->pData->hWin);

  Fix   = pPara->xSizeScreen + pPara->xPos - pPara->xSize / 2;
  if (pInfo->State != GUI_ANIM_START) {
    Dyn = (pPara->xSizeScreen * pPara->OldPos) / GUI_ANIM_RANGE;
    _MakeSymTextRect(&Rect, Fix - Dyn, pPara->xSize);
	//GUI_ClearRectEx(&Rect);
    _DrawBarEx(&Rect);
  }
  pPara->OldPos = (pInfo->State == GUI_ANIM_END) ? 0 : pInfo->Pos;
  Dyn = (pPara->xSizeScreen * pInfo->Pos)/ ((GUI_ANIM_RANGE * 3)/2);
  xPos = Fix - Dyn;

  _AnimDrawText(pPara,xPos);
}

/*********************************************************************
*
*       _AnimFlyOut
*/
static void _AnimFlyOut(GUI_ANIM_INFO * pInfo, void * pVoid) {
  PARA * pPara;
  GUI_RECT Rect;
  int Dyn, Fix, xPos;

  pPara = (PARA *)pVoid;
  WM_SelectWindow(pPara->pData->hWin);

  Fix   = pPara->xPos - pPara->xSize  - 480;
  Dyn   = (pPara->xSizeScreen * (GUI_ANIM_RANGE - pPara->OldPos)) / GUI_ANIM_RANGE;
  _MakeSymRect(&Rect, Fix + Dyn - 30, pPara->xSize);
  //GUI_ClearRectEx(&Rect);
  _DrawBarEx(&Rect);
  pPara->OldPos = pInfo->Pos;
  Dyn = (pPara->xSizeScreen * (GUI_ANIM_RANGE - pInfo->Pos)) / GUI_ANIM_RANGE;
  xPos = Fix + Dyn;

  if(pInfo->State == GUI_ANIM_END) 
  {
  	GUI_Log("\r\nFlyOut_ANIM_END");
	/////////////////////for testing/////////////////////////////
	if(mode.state == Sub_Mode_Compagnon_QAI)
	{
		mode.QAI_ALLOW = 1; 
	}
	else if(mode.state == Sub_Mode_Compagnon_QAE)
	{
		mode.QAE_ALLOW = 1;
	}
	//////////////////////////////////////////////////
  }
  _AnimDrawText(pPara,xPos);
  _AnimDrawSym(pPara, xPos, pPara->pData->IndexAnimOut);
}

static void _AnimModeExpertOut(GUI_ANIM_INFO * pInfo, void * pVoid) {

  PARA * pPara;
  GUI_RECT Rect;
  int Dyn, Fix, xPos;

  pPara = (PARA *)pVoid;
  WM_SelectWindow(pPara->pData->hWin);

  Fix   = pPara->xPos - pPara->xSize  - 480;
  Dyn   = (pPara->xSizeScreen * (GUI_ANIM_RANGE - pPara->OldPos)) / GUI_ANIM_RANGE;
  _MakeSymRect(&Rect, Fix + Dyn - 30, pPara->xSize + 200);
  //GUI_ClearRectEx(&Rect);
  _DrawBarEx(&Rect);
  pPara->OldPos = pInfo->Pos;
  Dyn = (pPara->xSizeScreen * (GUI_ANIM_RANGE - pInfo->Pos)) / GUI_ANIM_RANGE;
  xPos = Fix + Dyn;

  if(pInfo->State == GUI_ANIM_END) 
  {
  	GUI_Log("\r\nMode_Expert_ANIM_END");
	
	/////////////////////for testing/////////////////////////////
	if(mode.state == Sub_Mode_Expert_QAI)
	{
		mode.QAI_EXT_ALLOW = 1; 
	}
	else if(mode.state == Sub_Mode_Expert_QAE)
	{
		mode.QAE_EXT_ALLOW = 1;
	}
	//////////////////////////////////////////////////
  }

  if(mode.state == Sub_Mode_Expert_QAI)
  {
	  _AnimDrawModeExpertQAI(pPara,xPos - 304);
	  GUI_DrawBitmapEx(&bmhouse_, xPos + 96, 0,
	                  (xPos + 96) / 2, 0, 400, 400);
  }
  else if(mode.state == Sub_Mode_Expert_QAE)
  {
  	_AnimDrawModeExpertQAE(pPara,xPos - 304);
	  GUI_DrawBitmapEx(&bmcloud_002, xPos + 96, 0,
	                  (xPos + 96) / 2, 0, 400, 400);
  }
}

/*********************************************************************
*
*       _AnimDropIn
*/
static void _AnimDropIn(GUI_ANIM_INFO * pInfo, void * pVoid) {
  GUI_RECT Rect;
  WINDOW_DATA * pData;
  int yOff;
  static int OldPos;

  #ifdef ANIM_DEBUG
  
  pData = (WINDOW_DATA *)pVoid;
  WM_SelectWindow(pData->hWin);
  GUI_SelectLayer(1);
  GUI_SetBkColor(GUI_TRANSPARENT);
  pData->TextVisible = 1;
  if (pInfo->State != GUI_ANIM_START) {
    yOff = 150 - (150 * OldPos) / GUI_ANIM_RANGE;
    _MakeTextRect(&Rect, yOff);
    GUI_ClearRectEx(&Rect);
  }
  yOff = 150 - (150 * pInfo->Pos) / GUI_ANIM_RANGE;
  if (pInfo->State == GUI_ANIM_END) {
    WM_InvalidateWindow(pData->hWin);
    pData->ShowText = 1;
  } else {
    GUI_AA_PreserveTrans(1);
    _AnimDrawText(pData, yOff);
    GUI_AA_PreserveTrans(0);
    OldPos = pInfo->Pos;
  }
  GUI_SelectLayer(0);

  #endif
}

/*********************************************************************
*
*       _AnimIdle
*/
static void _AnimIdle(GUI_ANIM_INFO * pInfo, void * pVoid) {
  WINDOW_DATA * pData;
  GUI_PID_STATE State;
  static int Sign;

  pData = (WINDOW_DATA *)pVoid;
  pData->TimeLastTouch = GUI_GetTime();
  if (pInfo->State == GUI_ANIM_START) {
    Sign = ((GUI_GetTime() & 1) * 2) - 1;
  }
  State.Layer = 0;
  State.Pressed = 1;
  switch (Sign) {
  case -1:
    //State.x = ((pData->xSize / 2 + 10) * pInfo->Pos) / GUI_ANIM_RANGE;
    break;
  case +1:
    State.x = ((pData->xSize / 2 + 10) * (GUI_ANIM_RANGE - pInfo->Pos)) / GUI_ANIM_RANGE;
    break;
  }
  State.y = 0;
  GUI_PID_StoreState(&State);
  if (pInfo->State == GUI_ANIM_END) {
    State.Pressed = 0;
    GUI_PID_StoreState(&State);
  }
}

static void _cbModeExpertQAI(WM_MESSAGE * pMsg){
  WINDOW_DATA * pData;

  WM_GetUserData(pMsg->hWin, &pData, sizeof(WINDOW_DATA *));

  switch (pMsg->MsgId) {
  case WM_PAINT:
  	if(mode.state == Sub_Mode_Expert_QAI)
  	{
	   	GUI_Clear();
		_AnimDrawModeExpertQAI(NULL,pData->xPos);
		GUI_DrawBitmapEx(&bmhouse_, 400, 0, 200, 0,400, 400);
  	}
	else if(mode.state == Sub_Mode_Expert_QAE)
	{
		GUI_Clear();
		_AnimDrawModeExpertQAE(NULL,pData->xPos);
		GUI_DrawBitmapEx(&bmcloud_002, 400, 0, 200, 0,400, 400);
	}
	break;
  }
}

/*********************************************************************
*
*       _cbText
*
* Purpose:
*   Callback function of text window responsible for drawing
*   city, time and temperature
*/
static void _cbText(WM_MESSAGE * pMsg) {

  WINDOW_DATA * pData;
  GUI_ALPHA_STATE  AlphaState;

  WM_GetUserData(pMsg->hWin, &pData, sizeof(WINDOW_DATA *));
 
  switch (pMsg->MsgId) {
  case WM_PAINT:
  	//GUI_Clear();
  	//GUI_DrawGradientV(0, 0, 200, 200, GUI_BLACK, GUI_BLACK);
  	//GUI_MEMDEV_Select(hMemBMP);
    //
    // Set alpha value (on demand) for fading effect
    //
	//GUI_EnableAlpha(1);
	//GUI_SetUserAlpha(&AlphaState, pData->Alpha);
    //GUI_SetBkColor(GUI_MAKE_ALPHA(0xB0, GUI_BLACK));
    GUI_SetColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetAlpha(80u);
	GUI_SetFont(&GUI_FontD80); 
	GUI_DispStringAt("12", 50, 20);
	//GUI_MEMDEV_CopyToLCD(hMemBMP);
	//GUI_RestoreUserAlpha(&AlphaState);
	//GUI_EnableAlpha(0);
    break;
  }
}

static void _cbImageZoom(WM_MESSAGE * pMsg) {
	WINDOW_DATA * pData;

	WM_GetUserData(pMsg->hWin, &pData, sizeof(WINDOW_DATA *));

	switch (pMsg->MsgId) {
	case WM_PAINT:
    GUI_Clear();
    //LCD_Clear(0x00);
	GUI_MEMDEV_Select(hMemBMP);
	GUI_DrawBitmapEx(&bmhouse_320X200/* bmhouse_ */, 80/* pData->xMag_pos */, 0, 40/* pData->xMag_pos + XOFF_IMAGE + 20 */, 0, pData->xMag, pData->yMag);
	GUI_MEMDEV_CopyToLCD(hMemBMP);
	  break;
	default:
    	WM_DefaultProc(pMsg);
	}
}


/*********************************************************************
*
*       
*/
#if 0
static void _DrawTextWithAlpha(WINDOW_DATA * pData) {
	int iAlphaVal = 0;
	U8 alpha_val = 0xff;
	
    //WM_Paint(pData->hText);
	//WM_BringToTop(pData->hText);
	//WM_InvalidateWindow(pData->hText);
	
	pData->Alpha = 0;

	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(&GUI_FontD80);
	
	for (iAlphaVal = 0; iAlphaVal < 52; iAlphaVal ++)
    {
		pData->Alpha += 0x04;
		if(pData->Alpha > 254) break;
		//alpha_val -= 0x05;
		//if(alpha_val <= 1) break;
		//LCD_Clear(0x00);
		//_DrawAlphaCtl(alpha_val);
		//WM_InvalidateWindow(pData->hText);
		
		LCD_SetTransparency(pData->Alpha);
		GUI_DispStringAt("12", 50, 20);
		GUI_Delay(200);
    }
	//LCD_Clear(0x00);
	for (iAlphaVal = 0; iAlphaVal < 52; iAlphaVal ++)
    {
		pData->Alpha -= 0x04;
		if(pData->Alpha < 1) break;
		//alpha_val += 0x05;
		//if(alpha_val >= 254) break;
		//LCD_Clear(0x00);
		//_DrawAlphaCtl(alpha_val);
		//WM_InvalidateWindow(pData->hText);
		LCD_SetTransparency(pData->Alpha);
		GUI_DispStringAt("12", 50, 20);
		GUI_Delay(100);
    }

	//WM_BringToBottom(pData->hText);
	//WM_InvalidateWindow(pData->hText);
}
#else
static int GUIDrawTextWithAlpha(const char *pStr, int xPos, int yPos) {
	int iAlphaVal = 0xff;

	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(&GUI_FontD80);
	
	while(1)
    {
		iAlphaVal += 0x02;
		if(iAlphaVal > 253) break;
		LCD_SetTransparency(iAlphaVal);
		GUI_DispStringAt(pStr, xPos, yPos);
		GUI_Delay(100);
    }
	
	GUI_Delay(1000);
	
	while(1)
    {
		iAlphaVal -= 0x04;
		if(iAlphaVal <= 1) return 1;
		LCD_SetTransparency(iAlphaVal);
		GUI_DispStringAt(pStr, xPos, yPos);
		GUI_Delay(100);
    }
}

#endif

static void _ImageShrink(WINDOW_DATA * pData,void * pVoid) {
	PARA * pPara;
	GUI_ANIM_INFO * pInfo;
  	int Dyn, Fix, xPos;
	GUI_RECT Rect;
	int iShrinkVal = 0;

	//WM_BringToTop(pData->hImageZoom);
  	//WM_InvalidateWindow(pData->hImageZoom);

	pData->xMag = 1000;
	pData->yMag = 1000;

	pPara = (PARA *)pVoid;
	pInfo = &ANIM_Para;
		
	Fix   = pPara->xSizeScreen + pPara->xPos - pPara->xSize / 2;
	Dyn = (pPara->xSizeScreen * pInfo->Pos)/ ((GUI_ANIM_RANGE * 3)/2);
	xPos = Fix - Dyn;
		
	pData->xMag_pos = xPos;

	for(iShrinkVal = 0;iShrinkVal < 30;iShrinkVal ++)
	{
		pData->xMag -= 0x14;
		pData->yMag -= 0x14;
		
		GUI_Clear();
		//LCD_Clear(0x00);
		GUI_MEMDEV_Select(hMemBMP);
		GUI_DrawBitmapEx(&bmhouse_320X200, 80, 0, 40, 0, pData->xMag, pData->yMag);
		GUI_MEMDEV_CopyToLCD(hMemBMP);
		GUI_X_Delay(15);
	}
	//WM_BringToBottom(pData->hImageZoom);
	//WM_InvalidateWindow(pData->hImageZoom);
}

static void _ModeExpert(WINDOW_DATA * pData) {

	WM_BringToTop(pData->hModeExpertQAI);
	WM_InvalidateWindow(pData->hModeExpertQAI);
}

static void _OnTimer(WINDOW_DATA * pData) {
  //
  // Execute idle animation
  //
  #if 0
  if (pData->hAnimIdle) {
    if (GUI_ANIM_Exec(pData->hAnimIdle)) {
      GUI_ANIM_Delete(pData->hAnimIdle);
      pData->hAnimIdle = 0;
    }
  }
  #endif
  
  //
  // Execute main animation
  //
  if (pData->hAnimMain) {
    if (GUI_ANIM_Exec(pData->hAnimMain)) {
	   while (GUI_ANIM_Exec(pData->hAnimMain) == 0) {
		GUI_Delay(5);
		GUI_Log("\r\nExecute main animation>>>");
	   }
      GUI_ANIM_Delete(pData->hAnimMain);
      pData->hAnimMain = 0;
    }
  } else {
		if((mode.state == Sub_Mode_Compagnon_QAI) && (mode.QAI == 1))
		{
			mode.QAI = 0;
			pData->Job = JOB_ANIM_IN;
			pData->IndexAnimOut = 0; /* image zooming control */
		  	memcpy(&ANIM_Data,pData,sizeof(WINDOW_DATA));  /* TODO:注意保护数据 */
		}
		else if((mode.state == Sub_Mode_Expert_QAI) && (mode.QAI_EXT == 1))
		{
			mode.QAI_EXT = 0;
			pData->Job = JOB_ANIM_MODE_EXPERT;

			pData->TimeLastTouch = GUI_GetTime(); 		// for testing
		}
		else if((mode.state == Sub_Mode_Compagnon_QAE) && (mode.QAE == 1))
		{
			mode.QAE = 0;
			pData->Job = JOB_ANIM_IN;

			pData->IndexAnimOut = 0; /* image zooming control */
		  	memcpy(&ANIM_Data,pData,sizeof(WINDOW_DATA));  /* TODO:注意保护数据 */
		}
		else if((mode.state == Sub_Mode_Expert_QAE) && (mode.QAE_EXT == 1))
		{
			mode.QAE_EXT = 0;
			pData->Job = JOB_ANIM_MODE_EXPERT;

			pData->TimeLastTouch = GUI_GetTime(); 		// for testing
		}
		
	    //
	    // Create animations on demand
	    //
	    switch (pData->Job) {
	    case JOB_ANIM_IDLE:
	      if (pData->hAnimIdle == 0) {
	        pData->hAnimIdle = GUI_ANIM_Create(1000, 100, NULL, NULL);
	        GUI_ANIM_AddItem(pData->hAnimIdle, 0, 1000, ANIM_LINEAR, pData, _AnimIdle);
	        GUI_ANIM_Start(pData->hAnimIdle);
			pData->LastJob = pData->Job;
	        pData->Job = 0;
	      }
	      break;
	    case JOB_ANIM_IN_DROP:
	    case JOB_ANIM_IN:
		  GUI_Log("\r\nJOB_ANIM_IN");
	      if (pData->hAnimMain == 0) {
	        pData->hAnimMain = GUI_ANIM_Create(1500, 130, NULL, _SliceInfo);
	        //
	        // Add 'Fly in of symbols' to animation
	        //
	        GUI_ANIM_AddItem(pData->hAnimMain, 0, 1000, ANIM_LINEAR , &pData->aPara[0], _AnimFlyIn);

	        GUI_ANIM_Start(pData->hAnimMain);
	        pData->LastJob = pData->Job;
			if(mode.state == Sub_Mode_Compagnon_QAI)
			{
				pData->Job = JOB_ANIM_FACE_CHANGE; 
			}
			else if(mode.state == Sub_Mode_Compagnon_QAE)
			{
				pData->Job =  JOB_ANIM_TEXT_IN ; 
			}
	      }
	      break;
		case JOB_ANIM_FACE_CHANGE:
			GUI_Log("\r\nJOB_ANIM_FACE_CHANGE");
			pData->hAnimMain = 1;

			#if 1
			NormalWorkModeAnimation();   /* startup picture */
			_DrawTextWithAlpha(pData); /* months fading */
			_ImageShrink(pData,&pData->aPara[0]); /* Animation zooming */
			#endif
			
			pData->LastJob = pData->Job;
			pData->Job = JOB_ANIM_TEXT_IN;
			WM_BringToTop(pData->hWin);
			WM_InvalidateWindow(pData->hWin);
			pData->hAnimMain = 0;
		  break;
		case JOB_ANIM_TEXT_IN:
			 GUI_Log("\r\nJOB_ANIM_TEXT_IN");
			 if (pData->hAnimMain == 0) {
			
	         pData->hAnimMain = GUI_ANIM_Create(2000, 150, NULL, _SliceInfo);
	    
		     GUI_ANIM_AddItem(pData->hAnimMain, 0, 1500, ANIM_LINEAR, &pData->aPara[0], _AnimTextFlyIn);
			 
			 GUI_ANIM_Start(pData->hAnimMain);
			 pData->LastJob = pData->Job;
			 pData->Job = JOB_ANIM_OUT;

			 WM_InvalidateWindow(pData->hWin);
			}
		  break;
	    case JOB_ANIM_OUT:
			GUI_Log("\r\nJOB_ANIM_OUT");
			
			pData->IndexAnimOut = 1; /* image zooming control */
			
			memcpy(&ANIM_Data,pData,sizeof(WINDOW_DATA));  /* TODO:注意保护数据 */
			
	      	if (pData->hAnimMain == 0) {

	        pData->hAnimMain = GUI_ANIM_Create(2000, 150, NULL, _SliceInfo);
	        //
	        // Add 'Fly out of symbols' to animation
	        //
	        GUI_ANIM_AddItem(pData->hAnimMain, 0, 1500, ANIM_LINEAR, &pData->aPara[0], _AnimFlyOut);
			
	        GUI_ANIM_Start(pData->hAnimMain);
	        pData->LastJob = pData->Job;
	        pData->Job = 0;
	      }
	      break;
		  case JOB_ANIM_MODE_EXPERT:
			GUI_Log("\r\nJOB_ANIM_MODE_EXPERT_QAI");

			_ModeExpert(pData);
			
			if (GUI_GetTime() >= (pData->TimeLastTouch + TIME_IDLE))
			{
				/* hModeExpertQAI Brings to Bottom*/
				WM_BringToBottom(pData->hModeExpertQAI);
				WM_InvalidateWindow(pData->hModeExpertQAI); 
	
				pData->TimeLastTouch = GUI_GetTime();
				
		      	if (pData->hAnimMain == 0) {

		        pData->hAnimMain = GUI_ANIM_Create(2000, 150, NULL, _SliceInfo);
		        //
		        // Add 'Fly out of symbols' to animation
		        //
		        GUI_ANIM_AddItem(pData->hAnimMain, 0, 1500, ANIM_LINEAR, &pData->aPara[0], _AnimModeExpertOut);
				
		        GUI_ANIM_Start(pData->hAnimMain);
		        pData->LastJob = pData->Job;
		        pData->Job = 0;
		        }

			}
			break;
	    }
  }
  //GUI_Log("\r\nWM_RestartTimer");
  WM_RestartTimer(pData->hTimer, 5/* 20 */ /* 25 */);
}

/*********************************************************************
*
*       _OnPaint
*/
static void _OnPaint(WINDOW_DATA * pData) {
  int i, x0, Index, NumItems/*, Diff*/;
  GUI_RECT Rect;
  U8 Alpha;
  
  x0 = pData->xPos;
}

/*********************************************************************
*
*       _cbWin
*/
static void _cbWin(WM_MESSAGE * pMsg) {  // 0opo(WM_MESSAGE * pMsg) {  ???
  WM_MOTION_INFO * pInfo;
  WM_HWIN hWin;
  WINDOW_DATA * pData;
  GUI_PID_STATE State;
  int NumItems;

  hWin = pMsg->hWin;
//  NumItems = GUI_COUNTOF(_apBmBack);
  WM_GetUserData(hWin, &pData, sizeof(WINDOW_DATA *));
  if (pData) {
    if (pData->hTimer == 0) {
      pData->hTimer = WM_CreateTimer(hWin, 0, 0, 0);
    }
  }
  
  switch (pMsg->MsgId) {
  //
  // Avoid flickering of layer 1 during repaint of WM
  //
  case WM_PRE_PAINT:
  	#if 0
    GUI_MULTIBUF_BeginEx(1);
	#endif
    break;
  case WM_POST_PAINT:
  	#if 0
    GUI_MULTIBUF_EndEx(1);
	#endif
    break;
  //
  // Drawing of background window
  //
  case WM_PAINT:
  	#if 0
  	if(((mode.state == Sub_Mode_Expert_QAI) && (mode.QAI_ext== 1)) ||\
	   ((mode.state == Sub_Mode_Expert_QAE) && (mode.QAE_ext== 1)))
  	{
		if(mode.QAI_ext)
		{	
			GUI_Log("\r\nWM_PAINT QAI_ext 2");
			mode.state = Mode_Compagnon_QAE;
		}
		else if(mode.QAE_ext)
		{
			GUI_Log("\r\nWM_PAINT QAE_ext 4");
			mode.state = Mode_Idle;
		}
  	}
	#endif
    break;
  //
  // Timer keeps the demo alive
  //
  case WM_TIMER:
  	//_OnTimer(pData);
  	#if 0
  	if(((mode.state == Sub_Mode_Compagnon_QAE) && (mode.QAE == 1)) ||\
	   ((mode.state == Sub_Mode_Compagnon_QAI) && (mode.QAI == 1)))
  	{
		if(mode.QAI)
		{
			GUI_Log("\r\nWM_TIMER QAI 1");
			_OnTimer(pData);
			//mode.QAI = 0;
		}
		else if(mode.QAE)
		{
			GUI_Log("\r\nWM_TIMER QAE 3");
			mode.QAE = 0;
		}
  	}
	#endif
    break;
  //
  // React on window motion messages
  //
  case WM_MOTION:
    //
    // Stop idle animation if it is currently running
    //
    GUI_PID_GetCurrentState(&State);
    if (State.Pressed) {
      if (State.y != 0) {
        if (pData->hAnimIdle) {
          GUI_ANIM_Delete(pData->hAnimIdle);
          pData->hAnimIdle = 0;
        }
      }
    }
    //
    // Remember moment of last touch event
    //
    GUI_Log("\r\nWM_MOTION");
    pData->TimeLastTouch = GUI_GetTime();
    //
    // Process motion commands
    //
    pInfo = (WM_MOTION_INFO *)pMsg->Data.p;
    switch (pInfo->Cmd) {
    case WM_MOTION_INIT:
      //
      // Tell motion support that window manages motion messages by itself
      //
      pInfo->Flags  = WM_MOTION_MANAGE_BY_WINDOW;
      pInfo->SnapX  = pData->xSize;
      pInfo->Period = 300;
      break;
    case WM_MOTION_MOVE:
      //
      // Manage motion message
      //
      pData->xPos += pInfo->dx;
      if (pData->xPos >= (int)(NumItems * pData->xSize)) {
        pData->xPos -= NumItems * pData->xSize;
      }
      if (pData->xPos < 0) {
        pData->xPos += NumItems * pData->xSize;
      }
      //
      // Calculate distance
      //
      pData->Diff = pData->xPos - pData->IndexCity * pData->xSize;
      if ((pData->IndexCity == 0) && (pData->Diff > pData->xSize)) {
        pData->Diff -= pData->xSize * NumItems;
      }
      pData->Diff = (pData->Diff > 0) ? pData->Diff : -pData->Diff;
      if (pInfo->FinalMove) {
        //
        // After last move timer method should show forecast
        //
        pData->HasStopped = 1;
		#if 0
        _DrawIndicators(pData);
		#endif
      } else {
        //
        // On first move hide forecast
        //
        pData->HasStopped = 0;
        if (pData->Diff > (pData->xSize / 3)) {
          if (pData->LastJob != JOB_ANIM_OUT) {
            pData->IndexAnimOut = pData->IndexCity;
            pData->Job = JOB_ANIM_OUT;
          }
        }
      }
      //
      // Make sure that WM redraws window
      //
      WM_Invalidate(hWin);
      break;
    case WM_MOTION_GETPOS:
      pInfo->xPos = pData->xPos;
	  
	  //GUI_Log1("\r\n_AnimFlyIn pInfo->xPos",pInfo->xPos);
	  
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

static void _AnimDemoTest(WINDOW_DATA * pData) {
  GUI_ANIM_HANDLE hAnim;

  //
  // Create animation object
  //
  //   Remark: The min time/frame here is 100 to be able to notice
  //           the colors. In a real application this value should
  //           be significantly smaller to ensure a smooth motion.
  //
  //   Slice callback routine --------------+
  //   Custom *void pointer ---------+      |
  //   Minimum time per frame --+    |      |
  //   Duration ----------+     |    |      |
  //                      |     |    |      |
  hAnim = GUI_ANIM_Create(2000, 130, NULL, _SliceInfo);
  //
  // Add animation items
  //
  //   Animation routine to be called ------------------------+
  //   Custom *void pointer ---------------------------+      |
  //   Method of position calculation +                |      |
  //   End on timeline ---------+     |                |      |
  //   Start on timeline -+     |     |                |      |
  //                      |     |     |                |      |
  GUI_ANIM_AddItem(hAnim, 0, 1100, ANIM_LINEAR,     &pData->aPara[0], _AnimFlyIn);
  //
  // Animation item with custom defined position calculation
  //
  //GUI_ANIM_AddItem(hAnim, 2000, 4000, _CalcPosition,   pData, _AnimDrawCross);
  //
  // Start animation
  //
  GUI_ANIM_Start(hAnim);
  //
  // Execute animation until end is reached
  //
  while (GUI_ANIM_Exec(hAnim) == 0) {
    //
    // Idle time for other tasks
    //
    //printf("\r\nExecute main animation>>>");
    //GUI_X_Delay(5);
    GUI_Exec();
    //GUI_Delay(5);
  }
  //
  // Delete animation object
  //
  GUI_ANIM_Delete(hAnim);
}

/*********************************************************************
*
*       _ClearScreen
*/
static void _ClearScreen(void) {

  GUI_SelectLayer(0);
  GUI_SetBkColor(GUI_BLACK);
  GUI_Clear();
  GUI_SelectLayer(1);
  GUI_SetBkColor(GUI_TRANSPARENT);
  GUI_Clear();
  GUI_SetColor(0x55FFFFFF);
  _DrawBar(0, YPOS_FORECAST, 479, YPOS_FORECAST + YSIZE_FORECAST - 1, 0);
  GUI_SelectLayer(0);
}
/*********************************************************************
*
*       _InitData
*/
static void _InitData(WINDOW_DATA * pData, WM_HWIN hWin) {
  int i, NumItems, xSizeScreen, xSize;

  NumItems = GUI_COUNTOF(pData->aPara);
  xSizeScreen = LCD_GetXSize();
  xSize = xSizeScreen / NumItems;
  for (i = 0; i < NumItems; i++) {
    pData->aPara[i].xSizeScreen = (xSizeScreen + 240);
    pData->aPara[i].xSize       = xSize;
    pData->aPara[i].xPos        = (xSize * i) + xSize / 2;
    pData->aPara[i].pData       = pData;
    pData->aPara[i].Index       = i;
    pData->hWin                 = hWin;
  }
}

static int GUI_Set_Bitmap_Scale(const GUI_BITMAP *pBM, int xPos, int yPos, int xMag, int yMag, int Step)
{
	int xScaleFactor = 1000;
	int yScaleFactor = 1000;
	
	while(1)
  	{
	   xScaleFactor -= Step;
	   yScaleFactor -= Step;

	   if((xScaleFactor <= xMag) || (xScaleFactor <= yMag)) return 1;
		
	   GUI_Clear();
	   GUI_MEMDEV_Select(hMemBMP);
	   GUI_DrawBitmapEx(pBM, xPos, yPos, xPos / 2, yPos, xScaleFactor, yScaleFactor);
	   GUI_MEMDEV_CopyToLCD(hMemBMP);
	   	
	   GUI_Delay(50);
	}
}

static int GUI_Set_Ring_Indicator(int SensorVal, int xPos, int yPos,int r1 ,int r2)
{
	int r = 0;
	float MeasuRang = 1000.0f;
	int ArcStart = 0;
	int ArcRefer = 0;

	if(SensorVal > (int)MeasuRang) return -1;
		
	r = (r1 + r2) / 2;
	
	ArcRefer = (int)((SensorVal / MeasuRang) * 360);
	
	GUI_SetPenSize(r2 - r1);
	GUI_MEMDEV_Select(hMemText);
	
	if(ArcRefer <= 270) 
	{
		ArcStart = 270 - ArcRefer;
	  	GUI_SetColor(GUI_BLACK);
		GUI_AA_DrawArc(xPos, yPos, r, r, 0, ArcStart);
		if((ArcStart >= 180) && (ArcStart <= 270)) 
		{
			GUI_SetColor(GUI_WHITE);
			GUI_AA_DrawArc(xPos, yPos, r, r, ArcStart, 270);
		}
		else if((ArcStart < 180) && (ArcStart >= 90)) 
		{
			GUI_SetColor(GUI_YELLOW);
		 	GUI_AA_DrawArc(xPos, yPos, r, r, ArcStart, 270);
		}
		else if((ArcStart < 90) && (ArcStart >= 0))
		{
			GUI_SetColor(GUI_ORANGE);
		 	GUI_AA_DrawArc(xPos, yPos, r, r, ArcStart, 270);
		}
		GUI_SetColor(GUI_BLACK);
	 	GUI_AA_DrawArc(xPos, yPos, r, r, 270, 360);
	}
	else if(ArcRefer > 270)
	{
		ArcStart = (630 - ArcRefer);
		GUI_SetColor(GUI_DARKORANGE);
		GUI_AA_DrawArc(xPos, yPos, r, r, 0, 270);
		GUI_SetColor(GUI_BLACK);
	 	GUI_AA_DrawArc(xPos, yPos, r, r, 270, ArcStart);
		GUI_SetColor(GUI_DARKORANGE);
	 	GUI_AA_DrawArc(xPos, yPos, r, r, ArcStart, 360);
	}
	
 	GUI_SetPenSize(1);
 	GUI_SetColor(GUI_DARKGRAY);
 	GUI_AA_DrawArc(xPos, yPos, r1, r1, 0, 360);
 	GUI_AA_DrawArc(xPos, yPos, r2, r2, 0, 360);
	GUI_MEMDEV_CopyToLCD(hMemText);

	return 0;
}

/************************************************************************
*
*   AnimStateManagemeng
*/
static int AnimStateManagement(WINDOW_DATA * pData)  
{  
    switch(mode.state)  
    {  
	    case Mode_Compagnon_QAI: 
			//GUI_Log("\r\nMode_Compagnon_QAI");
			mode.QAI = 1;
			mode.QAI_EXT = 0;
			mode.QAE = 0;
			mode.QAE_EXT = 0;
			mode.state = Sub_Mode_Compagnon_QAI;
			break;
		case Sub_Mode_Compagnon_QAI:
			//GUI_Log("\r\nSub_Mode_Compagnon_QAI");
	   		if(mode.QAI_ALLOW == 1)
	   		{
				mode.QAI_ALLOW = 0;
				mode.state = Mode_Expert_QAI;
	   		}
			//WM_RestartTimer(pData->hTimer, ANIM_RESTAR_TIMER);
			//WM_InvalidateWindow(pData->hWin);
	        break;  
	    case Mode_Expert_QAI:
			//GUI_Log("\r\nMode_Expert_QAI");
			mode.QAI = 0;
			mode.QAI_EXT = 1;
			mode.QAE = 0;
			mode.QAE_EXT = 0;
			mode.state = Sub_Mode_Expert_QAI;
			break;
		case Sub_Mode_Expert_QAI:	
			//GUI_Log("\r\nSub_Mode_Expert_QAI");
			if(mode.QAI_EXT_ALLOW == 1)
	   		{
				mode.QAI_EXT_ALLOW = 0;
				mode.state = Mode_Compagnon_QAE;
	   		}
			//WM_RestartTimer(pData->hTimer, ANIM_RESTAR_TIMER);
			//WM_InvalidateWindow(pData->hWin);
			break;
	    case Mode_Compagnon_QAE:
			//GUI_Log("\r\nMode_Compagnon_QAE");
			mode.QAI = 0;
			mode.QAI_EXT = 0;
			mode.QAE = 1;
			mode.QAE_EXT = 0;
			mode.state = Sub_Mode_Compagnon_QAE;
			break;
		case Sub_Mode_Compagnon_QAE:
			GUI_Log("\r\nSub_Mode_Compagnon_QAE");
			if(mode.QAE_ALLOW == 1)
	   		{
				mode.QAE_ALLOW = 0;
				mode.state = Mode_Expert_QAE;
	   		}
			//WM_RestartTimer(pData->hTimer, ANIM_RESTAR_TIMER);
			//WM_InvalidateWindow(pData->hWin);
	        break;  
	    case Mode_Expert_QAE:
			mode.QAI = 0;
			mode.QAI_EXT = 0;
			mode.QAE = 0;
			mode.QAE_EXT = 1;
			mode.state = Sub_Mode_Expert_QAE;
			break;
		case Sub_Mode_Expert_QAE:
			GUI_Log("\r\nSub_Mode_Expert_QAE");
			if(mode.QAE_EXT_ALLOW == 1)
	   		{
				mode.QAE_EXT_ALLOW = 0;
				mode.state = Mode_Idle;
	   		}
			//WM_RestartTimer(pData->hTimer, ANIM_RESTAR_TIMER);
			//WM_InvalidateWindow(pData->hWin);
	        break;
	    case Mode_Idle:
			GUI_Log("\r\nMode_Idle");
			mode.state = Mode_Compagnon_QAI;
	        break;   
	    default:  
	        break;
    }  
      
    return 0;
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
*/
void GUIMainTask(void) {

 int Xmag = 1000;
 int Ymag = 1000;
 GUI_ALPHA_STATE  AlphaState; // for testing
 int iAlpha = 0;
 int i = 0;

  WINDOW_DATA * pData;
  WM_HWIN hWin;
  static WINDOW_DATA Data;

  pData = &Data;
  memset(pData,0,sizeof(WINDOW_DATA *));
  memset(&mode,0,sizeof(WORK_MODE));
  mode.state = Mode_Compagnon_QAI;

  //WM_SetCreateFlags(WM_CF_MEMDEV);

  GUI_Init();
  //
  // Check if recommended memory for the sample is available
  //
  #if 0
  if (GUI_ALLOC_GetNumFreeBytes() < RECOMMENDED_MEMORY) {
    GUI_ErrorOut("Not enough memory available."); 
    return;
  }
  #endif
  //
  // Set up used tools
  //
  WM_MULTIBUF_Enable(1);
	
  #if 0
  _ClearScreen();
  WM_MOTION_Enable(1);
  #endif
  //
  // Shrink desktop window
  //
  Data.xSize = LCD_GetXSize();
  Data.ySize = LCD_GetYSize();
  Data.TimeLastTouch = GUI_GetTime();
  WM_SetSize(WM_HBKWIN, Data.xSize, Data.ySize);
  //
  // Create initial window
  //
  pData->hWin = WM_CreateWindowAsChild(0, 0, Data.xSize, Data.ySize, WM_HBKWIN, WM_CF_HIDE | WM_CF_MEMDEV | WM_CF_SHOW,/*_cbWin */NULL, sizeof(WINDOW_DATA *));
  //WM_HideWindow(pData->hWin);

  pData->hImageZoom = WM_CreateWindowAsChild(0, 0, 612, 474, WM_HBKWIN, WM_CF_HIDE | WM_CF_MEMDEV | WM_CF_SHOW, _cbImageZoom, sizeof(WINDOW_DATA *));
  WM_HideWindow(pData->hImageZoom);
  WM_BringToBottom(pData->hImageZoom);
  WM_InvalidateWindow(pData->hImageZoom);
  
  pData->hText = WM_CreateWindowAsChild(300, 100, 100, 100, WM_HBKWIN, WM_CF_HIDE |/* WM_CF_MEMDE */WM_CF_MEMDEV_ON_REDRAW | WM_CF_SHOW, _cbText, sizeof(WINDOW_DATA *));
  WM_HideWindow(pData->hText);
  WM_BringToBottom(pData->hText);
  WM_InvalidateWindow(pData->hText);

  pData->hModeExpertQAI =  WM_CreateWindowAsChild(0, 0, Data.xSize, Data.ySize, WM_HBKWIN, WM_CF_HIDE | WM_CF_MEMDEV | WM_CF_SHOW, _cbModeExpertQAI, sizeof(WINDOW_DATA *));
  WM_HideWindow(pData->hModeExpertQAI);
  WM_BringToBottom(pData->hModeExpertQAI);
  WM_InvalidateWindow(pData->hModeExpertQAI);

  //
  // Initialize animation data
  //
  _InitData(pData, pData->hWin);
  //
  // Make WINDOW_DATA available for window
  //
  WM_SetUserData(pData->hWin,  &pData, sizeof(WINDOW_DATA *));
  WM_SetUserData(pData->hImageZoom, &pData, sizeof(WINDOW_DATA *));
  WM_SetUserData(pData->hText, &pData, sizeof(WINDOW_DATA *));
  WM_SetUserData(pData->hModeExpertQAI, &pData, sizeof(WINDOW_DATA *));

  hMemBMP = GUI_MEMDEV_CreateFixed(0, 
								   0, 
								   LCD_GetXSize(), 
								   LCD_GetYSize(), 
								   GUI_MEMDEV_HASTRANS, 
								   GUI_MEMDEV_APILIST_16, 
								   GUICC_M565);

  hMemText = GUI_MEMDEV_CreateEx(0, 
								 0, 
								 LCD_GetXSize(), 
								 LCD_GetYSize(), 
								 GUI_MEMDEV_HASTRANS);

  #if 1
  while(1)
  {
  	#if 1
	Xmag -= 50;
	Ymag -= 50;
	if(Xmag <= 10)Xmag = 1000;
	if(Ymag <= 10)Ymag = 1000;
	GUI_Clear();
	//LCD_Clear(0x00);
	GUI_MEMDEV_Select(hMemBMP);
	GUI_DrawBitmapEx(&bmhouse_320X200, 80, 0, 40, 0, Xmag, Ymag);
	GUI_MEMDEV_CopyToLCD(hMemBMP);
	GUI_X_Delay(50);
	#endif
	
	#if 0
    //
    // Set alpha value (on demand) for fading effect
    //
    iAlpha = 0xFF;
	//LCD_Clear(0x00);
	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(&GUI_FontD80);
	//LCD_WindowModeDisable();
	for(i = 0; i < 51; i ++)
	{
		iAlpha -= 0x05;
		//GUI_DrawGradientV(0, 0, 200, 200, GUI_BLACK, GUI_BLACK);
		//LCD_Clear(0x00);
		//GUI_Clear();
		//GUI_EnableAlpha(1);
		//GUI_SetUserAlpha(&AlphaState, iAlpha);
		//GUI_MEMDEV_Select(hMemBMP);
		//GUI_SetAlpha(iAlpha);
		LCD_SetTransparency(iAlpha);
	    //GUI_SetBkColor(GUI_MAKE_ALPHA(0xB0, GUI_BLACK));
		GUI_DispStringAt("12", 50, 20);
		//GUI_MEMDEV_CopyToLCD(hMemBMP);
		//GUI_RestoreUserAlpha(&AlphaState);
		//GUI_EnableAlpha(0);
		GUI_Delay(100);
		//GUI_Exec();
	}
	#endif
  }
  #endif
  //
  // Super loop
  //
  while (1) {
	AnimStateManagement(pData);
	//_OnTimer(pData);
	_AnimDemoTest(pData);
	//_ImageShrink(pData,&pData->aPara[0]);
	//NormalWorkModeAnimation();
	//_DrawTextWithAlpha(pData);
	//GUI_Exec();
  }
}

/*************************** End of file ****************************/

