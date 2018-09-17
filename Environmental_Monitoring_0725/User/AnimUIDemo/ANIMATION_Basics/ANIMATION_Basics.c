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

** emWin V5.32 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only  be used  in accordance  with  a license  and should  not be  re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : ANIMATION_Basics.c
Purpose     : Sample showing the basics of using the animation object.
Requirements: WindowManager - ( )
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
----------------------------------------------------------------------
*/

//#include <stddef.h>
#include "GUI.h"
#include "WM.h"
#include "stm32f429i_discovery_lcd.h"
#include "main_house.h"
#include "sensor_measure.h"
#include "usart_sensor_cfg.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  int xSize, ySize;
  int xPart, yPart;
  int xPos, yPos;
  int Dir;
  int Size;
  int ObjectSize;
} ANIM_DATA;

uint32_t key_mode = 0;
U8 bl_ctl = 0;
U8 animation_ctl = 0;


/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  const GUI_BITMAP GUI_UNI_PTR * pBitmap;
  GUI_MEMDEV_Handle              hMem;
  int                            xSize;
  int                            ySize;
} IMAGE;

static GUI_MEMDEV_Handle hMemBMP_;
static GUI_MEMDEV_Handle hMemText;
static WM_HWIN hText;

typedef enum {
	  Mode_Compagnon_AQI_House = 0,
	  Mode_Compagnon_AQI_FACE,
	  Mode_Compagnon_AQI_NUM,
	  Mode_Compagnon_AQI_SCALE,
	  Mode_Expert_AQI,
	  Mode_Black_Screen,
	  Mode_Compagnon_AQE_Cloud,
	  Mode_Compagnon_AQE_FACE,
	  Mode_Compagnon_AQE_NUM,
	  Mode_Compagnon_AQE_SCALE,
	  Mode_Expert_AQE,
	  Mode_AQE_Black_Screen,
	  Mode_Idle
} INDEX_WORK_MODE;

typedef struct _NORMAL_WORK_MODE
{
	U8 state;
	U8 startup_state;
	U8 gui_run_state;
	
}WORK_MODE;

WORK_MODE run_mode;
//int anim_xpos = 0;
//int anim_scale_xpos = 0;

#define GUI_DARKORANGE   0x000042CC
#define MIN_TIME_PER_FRAME_ENLARGE 30

typedef enum {
	  STARTUP_LOGO = 0,
	  STARTUP_APPDOWNLOAD,
	  STARTUP_Bluetooth,
	  STARTUP_Pairing,
	  STARTUP_Black_Screen,
	  STARTUP_Pairing_Sta,
	  STARTUP_NO_WIFI,
	  STARTUP_Idle
} INDEX_STARTUP;

WORK_MODE startup_mode ;

typedef enum {
	  GUI_Startup = 0,
	  GUI_GO,
	  GUI_Idle,
} GUI_MODE;

WORK_MODE gui_run;

//////////////////////////////////////////////////////////////////////////////////
/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
#if 0

static IMAGE _aImage[] = {
  { &bmmouth },
 // { &_bmJTrace_300x164     },
 // { &_bmJLink_300x132      },
 // { &_bmPlatine_250x200    }
};

#endif

typedef struct {
  int XPos_Poly;
  int YPos_Poly;
  int XPos_Text;
  int YPos_Text;
  GUI_POINT aPointsDest[8];
} tDrawItContext;

tDrawItContext DrawItContext;
static int _step;

GUI_RECT Rect = {350, 120, 450,195};
GUI_RECT Rect_pBm = {100, -100, 700, 260};
GUI_RECT Rect_BLEpBm = {380, 0, 540, 480};

extern void  SENSORTaskPost ( void );
extern void  SENSORTaskPend ( void);

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief	Background callback
  * @param	pMsg: pointer to a data structure of type WM_MESSAGE
  * @retval None
  */
static void _cbWiFiBk(WM_MESSAGE * pMsg) {

#if 0

  int	xPos, Step = 32;
  static unsigned char WiFiSignalCount = 0;

  switch (pMsg->MsgId) 
  {
	  case WM_PAINT:
			
		GUI_AA_FillCircle(LCD_GetXSize() / 2, LCD_GetYSize() / 2 + 80,30);
		
		if(WiFiSignalCount == 0)
		{
			GUI_DrawBitmap(&bmwifi0, (LCD_GetXSize() - bmwifi0.XSize) / 2, (LCD_GetYSize() - bmwifi0.YSize) / 2 - Step * WiFiSignalCount);
			WiFiSignalCount++;
		}
		else if(WiFiSignalCount == 1)
		{
			GUI_DrawBitmap(&bmwifi1, (LCD_GetXSize() - bmwifi1.XSize) / 2, (LCD_GetYSize() - bmwifi1.YSize) / 2 - Step * WiFiSignalCount);
			WiFiSignalCount++;
		}
		else if(WiFiSignalCount == 2)
		{
			GUI_DrawBitmap(&bmwifi2, (LCD_GetXSize() - bmwifi2.XSize) / 2, (LCD_GetYSize() - bmwifi2.YSize) / 2 - Step * WiFiSignalCount);
			WiFiSignalCount++;
		}
		else if(WiFiSignalCount == 3)
		{
			GUI_DrawBitmap(&bmwifi3, (LCD_GetXSize() - bmwifi3.XSize) / 2, (LCD_GetYSize() - bmwifi3.YSize) / 2 - Step * WiFiSignalCount);
			WiFiSignalCount++;
		}
		else if(WiFiSignalCount >= 4) 
		{
			WiFiSignalCount = 0;
			GUI_ClearRectEx(&Rect_pBm);
		}
  }

  #endif
}

/**
  * @brief	DEMO_Starup
  * @param	None
  * @retval None
  */
void DEMO_WiFiSigStarup(unsigned char loop)
{
  //unsigned char loop = 32 ;
  
  //GUI_RECT Rect = {200, 0, 600, 240};
  
  WM_SetCallback(WM_GetDesktopWindowEx(0), _cbWiFiBk);
  
  while (loop--)
  {
	//idx = (16- loop) % 5;

	WM_InvalidateArea(&Rect_pBm);
	
	GUI_Delay(300);
  }
}


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Background callback
  * @param  pMsg: pointer to a data structure of type WM_MESSAGE
  * @retval None
  */
static void _cbBLEBk(WM_MESSAGE * pMsg) {

#if 0
  int   Step = 25;
  static unsigned char BLESignalCount = 0;

  switch (pMsg->MsgId) 
  {
	  case WM_PAINT:
	    GUI_DrawBitmap(&bmbluetooth, (LCD_GetXSize() - bmbluetooth.XSize)/2 , (LCD_GetYSize() - bmbluetooth.YSize)/2);
		
		if(BLESignalCount == 0)
		{
			GUI_DrawBitmap(&bmbluetooth0, (LCD_GetXSize() - bmbluetooth0.XSize) / 2 + Step * BLESignalCount, (LCD_GetYSize() - bmbluetooth0.YSize) / 2 );
			BLESignalCount++;
		}
		else if(BLESignalCount == 1)
		{
			GUI_DrawBitmap(&bmbluetooth1, (LCD_GetXSize() - bmbluetooth1.XSize) / 2 + Step * BLESignalCount, (LCD_GetYSize() - bmbluetooth1.YSize) / 2);
			BLESignalCount++;
		}
		else if(BLESignalCount == 2)
		{
			GUI_DrawBitmap(&bmbluetooth2, (LCD_GetXSize() - bmbluetooth2.XSize) / 2 + Step * BLESignalCount, (LCD_GetYSize() - bmbluetooth2.YSize) / 2);
			BLESignalCount++;
		}
		else if(BLESignalCount == 3)
		{
			GUI_DrawBitmap(&bmbluetooth3, (LCD_GetXSize() - bmbluetooth3.XSize) / 2 + Step * BLESignalCount , (LCD_GetYSize() - bmbluetooth3.YSize) / 2);
			BLESignalCount++;
		}
		else if(BLESignalCount >= 4) 
		{
			BLESignalCount = 0;
			GUI_ClearRectEx(&Rect_BLEpBm);
		}

  }

#endif
}

/**
  * @brief  DEMO_Starup
  * @param  None
  * @retval None
  */
void DEMO_Ble_Starup(unsigned char loop)
{
  //unsigned char loop = 32 ;
  
  //GUI_RECT Rect = {200, 0, 600, 240};
  
  WM_SetCallback(WM_GetDesktopWindowEx(0), _cbBLEBk);
  
  while (loop--)
  {
    //idx = (16- loop) % 5;

    WM_InvalidateArea(&Rect_BLEpBm);
    
    GUI_Delay(300);
  }
}

/*******************************************************************
*
*       _DrawIt
*/
static void _DrawIt(void * pData) {
  tDrawItContext * pDrawItContext = (tDrawItContext *)pData;
  GUI_Clear();

  GUI_SetColor(GUI_WHITE);
  GUI_AA_FillCircle(pDrawItContext->XPos_Text + 35,170,13);
  GUI_AA_FillCircle(pDrawItContext->XPos_Text - 35,170,13);
  //
  // draw background
  //
  GUI_SetColor(GUI_BLACK);
  GUI_FillRect(pDrawItContext->XPos_Text, 
               115 + pDrawItContext->YPos_Text,
               100 + pDrawItContext->XPos_Text,
               130 + pDrawItContext->YPos_Text);

  //
  // draw foreground
  //
  GUI_SetColor(GUI_BLACK);
  GUI_FillRect(pDrawItContext->XPos_Text, 
               210 - pDrawItContext->YPos_Text,
               100 + pDrawItContext->XPos_Text,
               225 - pDrawItContext->YPos_Text);
}

/*******************************************************************
*
*       _DemoBandingMemdev
*/

static void _Demo_Squint(void) {

	int swap = 1;
	
	DrawItContext.XPos_Text = 400;
	
	_step += 2;
	
  	DrawItContext.YPos_Text = (swap) ? _step : 40 - _step;
  	/* Use banding memory device for drawing */
  	GUI_MEMDEV_Draw(&Rect, &_DrawIt, &DrawItContext, 0, 0);
  	GUI_Delay(70);
}

static void _Demo_Open_Eye(void) {

	int swap = 0;
	
	 DrawItContext.XPos_Text = 400;
	 
  	 _step += 2;
	 
     DrawItContext.YPos_Text = (swap) ? _step : 40 - _step;
     /* Use banding memory device for drawing */
     GUI_MEMDEV_Draw(&Rect, &_DrawIt, &DrawItContext, 0, 0);
	 GUI_Delay(70);
}


static void GUI_Pairing_Successful(void) {
#if 0
	int ieyeCoun = 0;
	
	GUI_DrawBitmap(&bmmaihouse, (LCD_GetXSize() - bmmaihouse.XSize) / 2, (LCD_GetYSize() - bmmaihouse.YSize) / 2 - 60);
	GUI_DrawBitmap(&bmmouth, (LCD_GetXSize() - bmmouth.XSize) / 2, (LCD_GetYSize() - bmmouth.YSize) / 2 - 10);

    for(ieyeCoun = 0; ieyeCoun < 10;ieyeCoun++)
    {
    	_Demo_Squint();
    }

		_step = 0;
	
		for(ieyeCoun = 0; ieyeCoun < 10;ieyeCoun++)
    {
    	_Demo_Open_Eye();
    }
		
		_step = 0;
#endif
}


static int GUI_Display_Text(const char *text)
{
	//extern GUI_CONST_STORAGE GUI_FONT GUI_FontVerdana78;

	GUI_SetFont(&GUI_Font32B_ASCII);
	GUI_MEMDEV_Select(hMemBMP_);
	GUI_DispStringHCenterAt(text, LCD_GetXSize() / 2, LCD_GetYSize() / 2);
	GUI_MEMDEV_CopyToLCD(hMemBMP_);

	return NULL;
}

//
//
//
static int GUI_Set_Bitmap_Scale(const GUI_BITMAP *pBM, int xMag, int yMag, int Step)
{
	int xScaleFactor = 1000;
	int yScaleFactor = 1000;


	while(1)
  	{
	   xScaleFactor -= Step;
	   yScaleFactor -= Step;

	   if((xScaleFactor <= xMag) || (xScaleFactor <= yMag)) return 1;
		
	   GUI_Clear();
	   GUI_MEMDEV_Select(hMemBMP_);
	   GUI_DrawBitmapEx(pBM, LCD_GetXSize() / 2, (LCD_GetYSize() - pBM->YSize) / 2, pBM->XSize / 2, (LCD_GetYSize() - pBM->YSize) / 4, xScaleFactor, yScaleFactor);
	   GUI_MEMDEV_CopyToLCD(hMemBMP_);
	   	
	   GUI_Delay(25);
	}
}

#define Set_Bitmap_xMag 650
#define Set_Bitmap_yMag 650

static int GUI_Set_Scale_Pos(const GUI_BITMAP *pBM, const GUI_BITMAP *pBMTemp,int xMag, int yMag, int Step)
{
	int xScaleFactor = Set_Bitmap_xMag;
	int yScaleFactor = Set_Bitmap_yMag;
	int y_move_pos = 0;

	while(1)
  	{
	   xScaleFactor -= Step;
	   yScaleFactor -= Step;
	   y_move_pos += 4;

	   if((xScaleFactor <= xMag) || (xScaleFactor <= yMag)) 
	   {
	   		GUI_Clear();
			GUI_MEMDEV_Select(hMemBMP_);
			GUI_DrawBitmap(pBMTemp, (LCD_GetXSize() - pBMTemp->XSize) / 2, (LCD_GetYSize() - pBMTemp->YSize) / 2 - 105) ;
			GUI_MEMDEV_CopyToLCD(hMemBMP_);
		   	return 1;
	   }
		
	   GUI_Clear();
	   GUI_MEMDEV_Select(hMemBMP_);
	   GUI_DrawBitmapEx(pBM, LCD_GetXSize() / 2, (LCD_GetYSize() - pBM->YSize) / 2 - y_move_pos, pBM->XSize / 2, (LCD_GetYSize() - pBM->YSize) / 4, xScaleFactor, yScaleFactor);
	   GUI_MEMDEV_CopyToLCD(hMemBMP_);
	   	
	   GUI_Delay(20);
	}
}

//
//
//
static int GUI_Set_Text_Position(char *pText, int xStart, int xEnd, int dx, int yPos)
{
	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(GUI_FONT_8X16X3X3);

	GUI_MEMDEV_Clear(hMemText);
	
	while(1)
  	{
		xStart -= dx;
		
	   if(xStart <= xEnd) 
	   {
	   		return xStart;
	   }

	   GUI_ClearRect(0, 260, 800,480);
	   GUI_MEMDEV_Select(hMemText);
	   GUI_DispStringAt(pText, xStart, yPos);
	   GUI_MEMDEV_CopyToLCD(hMemText);
	   	
	   GUI_Delay(50);
	}

}

//
//
//
static int GUI_Set_Bitmap_Scale_Pos(const GUI_BITMAP *pBM, int xStart, int xEnd, int dx, int yPos)
{
	//int text_xpos = 0;
		
	GUI_Set_Text_Position("HELLO WORLD!", 800, 400 - 120, 15,360);
	//printf("text xpos = %d\r\n",text_xpos);
	//printf("Scale xStart = %d\r\n",xStart);

	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(GUI_FONT_8X16X3X3);
	
	while(1)
  	{
		xStart -= dx;
		
	   if(xStart <= (xEnd - (pBM->XSize / 2))) 
	   {
	   		return 1;
	   }

	   GUI_MEMDEV_Select(hMemBMP_);
	   GUI_Clear();
	   GUI_DispStringAt("HELLO WORLD!", xStart - 213, 360);
	   GUI_DrawBitmapEx(pBM, (xStart - (pBM->XSize / 4)), yPos, (xStart - (pBM->XSize / 4)) / 2, yPos, 450, 450);
	   GUI_MEMDEV_CopyToLCD(hMemBMP_);
	   
	   GUI_Exec();
	}

//	return 1;
}
//
//
//
static int GUI_Set_Bitmap_Position(const GUI_BITMAP *pBM, int xStart, int xEnd, int dx, int yPos)
{
	while(1)
  	{
		xStart -= dx;
		
	   if(xStart <= (xEnd - (pBM->XSize / 2))) 
	   {
	   		/*anim_xpos = xStart + pBM->XSize*/;
	   		return 1;
	   }

	   GUI_Clear();
	   GUI_MEMDEV_Select(hMemBMP_);
	   GUI_DrawBitmap(pBM, xStart, yPos);
	   GUI_MEMDEV_CopyToLCD(hMemBMP_);
	   	
	   GUI_Exec();
	}
}
//
//
//
static int GUI_Draw_Bitmap_Centre(const GUI_BITMAP *pBM)
{
	   GUI_Clear();
	   GUI_MEMDEV_Select(hMemBMP_);
	   GUI_DrawBitmap(pBM, (LCD_GetXSize() - pBM->XSize) / 2, (LCD_GetYSize() - pBM->YSize) / 2 - 60);
	   GUI_MEMDEV_CopyToLCD(hMemBMP_);

	   return NULL;
}

//
//
//
typedef enum {
  CO2_TY = 0,
  PM2005_TY,
  VOC_TY,
  H20_TY,
  PM25_Cloud,
  PM10_Cloud,
  NO2_Cloud,
  O3_Cloud,
} SENSOR_TY;

int XPOS_OFFSET = 18;
int XPOS_OFFSET_2 = 36;
int XPOS_OFFSET_3 = 60;
int XPOS_OFFSET_4 = 40;
int YPOS_OFFSET = 40;
int YPOS_TY_OFF = 35;
int XPOS_OFFSET_TY = 0;
int YPOS_OFFSET_TY = 70;

extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro65Md41;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro45Lt41;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro65Md75;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro65Md38;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro65Md113;


static int GUI_Display_IAQ_Value(const char *iaq_value,int x_offset,int y_offset)
{
	GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md113);
	GUI_DispStringHCenterAt(iaq_value, LCD_GetXSize() / 2 + x_offset, LCD_GetYSize() / 2 - 25 + y_offset);

	return NULL;
}
//
//
//
static int GUI_Temp_Disp(int TempVal, int x_offset,int y_offset)
{
	#if 0
	
	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md75);
	
	GUI_MEMDEV_Select(hMemBMP_);
	GUI_DrawBitmap(&bmdegree,(LCD_GetXSize() - bmdegree.XSize) / 2 - 3 + x_offset, (LCD_GetYSize() - bmdegree.YSize) / 2 - 90);
	if(0<= TempVal && TempVal < 10)
	{
		GUI_GotoXY(LCD_GetXSize() / 2 - 80 + x_offset, (LCD_GetYSize() - bmdegree.YSize) / 2 - 110);
	}
	else if(TempVal < 0 || TempVal >= 10)
	{
		GUI_GotoXY(LCD_GetXSize() / 2 - 110 + x_offset, (LCD_GetYSize() - bmdegree.YSize) / 2 - 110);
	}
	GUI_DispDecMin(TempVal);
	GUI_MEMDEV_CopyToLCD(hMemBMP_);

	return NULL;

	#endif
}

//
//
//
static int GUI_Set_Ring_Font_Pos(int sample_val,int xPos, int yPos)
{
	if(sample_val < 10)
	{
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md75);
 		GUI_GotoXY(xPos - XPOS_OFFSET, yPos - YPOS_OFFSET);
	}
	else if((sample_val >= 10) && (sample_val < 100))
	{
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md75);
		GUI_GotoXY(xPos - XPOS_OFFSET_2, yPos - YPOS_OFFSET);
	}
	else if((sample_val >= 100) && (sample_val < 1000))
	{
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md75);
		GUI_GotoXY(xPos - XPOS_OFFSET_3, yPos - YPOS_OFFSET);
	}
	else if((sample_val >= 1000) && (sample_val < 10000))
	{
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md38);
		GUI_GotoXY(xPos - XPOS_OFFSET_4, yPos - YPOS_OFFSET / 2);
	}
}
//
//
//
static int GUI_Set_Ring_Sensor_Pos(int SENSOR,int xPos, int yPos)
{
	if(SENSOR == CO2_TY)
	{
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("CO2", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt("ppm", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);
	}
	else if(SENSOR == PM2005_TY)
	{
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("PM 2.5", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt("ug/m3", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);

	}
	else if(SENSOR == VOC_TY)
	{
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("COV", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt("ppb", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);

	}
	else if(SENSOR == H20_TY)
	{
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("H2O", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt(" % ", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);
	}
	else if(SENSOR == PM25_Cloud)
	{
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("PM 2.5", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt("ug/m3", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);
	}
	else if(SENSOR == PM10_Cloud)
	{
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("PM 10", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt("ppb", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);

	}
	else if(SENSOR == NO2_Cloud)
	{
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("NO2", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt("ppm", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);

	}
	else if(SENSOR == O3_Cloud)
	{
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("O3", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt(" % ", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);
	}
}

//
//
//
static int GUI_Set_Ring_Indicator(int SensorVal, int sample_val,int xPos, int yPos, int SENSOR)
{
	int r = 0;
	int ArcStart = 0;
	int ArcRefer = 0;
	float MeasuRang = 0.0f;

	if(SENSOR == CO2_TY) 
	{
		MeasuRang = 2000.0f;
	}
	else if(SENSOR == PM2005_TY)
	{
		MeasuRang = 1000.0f;
	}
	else if(SENSOR == VOC_TY)
	{
		MeasuRang = 2000.0f;
	}
	else if(SENSOR == H20_TY)
	{
		MeasuRang = 100.0f;
	}
	else if(SENSOR == PM25_Cloud)
	{
		MeasuRang = 500.0f;
	}
	else if(SENSOR == PM10_Cloud)
	{
		MeasuRang = 500.0f;
	}
	else if(SENSOR == NO2_Cloud)
	{
		MeasuRang = 1000.0f;
	}
	else if(SENSOR == O3_Cloud)
	{
		MeasuRang = 1000.0f;
	}
	
	//if(SensorVal > (int)MeasuRang) return -1;
		
	r = 90;
	
	ArcRefer = (int)((SensorVal / MeasuRang) * 270);
	GUI_SetPenSize(10);
	GUI_MEMDEV_Select(hMemBMP_);

	if(ArcRefer <= 225) 
	{
		ArcStart = 225 - ArcRefer;

		GUI_SetColor(GUI_DARKGRAY);
		GUI_AA_DrawArc(xPos, yPos, r, r, 0, 225);
	  	
		if((ArcStart >= 180) && (ArcStart <= 270)) 
		{
			GUI_SetColor(GUI_DARKGRAY);
			GUI_AA_DrawArc(xPos, yPos, r, r, 0, ArcStart);
			GUI_SetColor(GUI_DARKGRAY);
			GUI_AA_DrawArc(xPos, yPos, r, r, 315, 360);
			GUI_SetColor(GUI_WHITE);
			GUI_AA_DrawArc(xPos, yPos, r, r, ArcStart, 225);

			GUI_Set_Ring_Font_Pos(sample_val,xPos,yPos);
		 	GUI_DispDecMin(sample_val);
			
			GUI_Set_Ring_Sensor_Pos(SENSOR,xPos,yPos);
		}
		else if((ArcStart < 180) && (ArcStart >= 90)) 
		{
			GUI_SetColor(GUI_DARKGRAY);
			GUI_AA_DrawArc(xPos, yPos, r, r, 0, ArcStart);
			GUI_SetColor(GUI_DARKGRAY);
			GUI_AA_DrawArc(xPos, yPos, r, r, 315, 360);
			GUI_SetColor(GUI_YELLOW);
		 	GUI_AA_DrawArc(xPos, yPos, r, r, ArcStart, 225);

			GUI_Set_Ring_Font_Pos(sample_val,xPos,yPos);
		 	GUI_DispDecMin(sample_val);
			GUI_Set_Ring_Sensor_Pos(SENSOR,xPos,yPos);
		}
		else if((ArcStart < 90) && (ArcStart >= 0))
		{
			GUI_SetColor(GUI_DARKGRAY);
			GUI_AA_DrawArc(xPos, yPos, r, r, 0, ArcStart);
			GUI_SetColor(GUI_DARKGRAY);
			GUI_AA_DrawArc(xPos, yPos, r, r, 315, 360);
			GUI_SetColor(GUI_ORANGE);
		 	GUI_AA_DrawArc(xPos, yPos, r, r, ArcStart, 225);

			GUI_Set_Ring_Font_Pos(sample_val,xPos,yPos);
		 	GUI_DispDecMin(sample_val);
			GUI_Set_Ring_Sensor_Pos(SENSOR,xPos,yPos);
		}

		GUI_SetColor(GUI_DARKGRAY);
		GUI_AA_DrawArc(xPos, yPos, r, r, 315, 360);	
	}
	else if(ArcRefer > 225)
	{
		ArcStart = (585 - ArcRefer);

		GUI_SetColor(GUI_DARKGRAY);
		GUI_AA_DrawArc(xPos, yPos, r, r, 315, ArcStart);
		GUI_SetColor(GUI_DARKORANGE);
		GUI_AA_DrawArc(xPos, yPos, r, r, 0, 225);
		GUI_SetColor(GUI_DARKORANGE);
	 	GUI_AA_DrawArc(xPos, yPos, r, r,  ArcStart, 360);

		GUI_Set_Ring_Font_Pos(sample_val,xPos,yPos);
		GUI_DispDecMin(sample_val);

		GUI_Set_Ring_Sensor_Pos(SENSOR,xPos,yPos);
	}


	GUI_SetColor(GUI_BLACK);
	GUI_AA_DrawArc(xPos, yPos, r, r, 0, 5);
	GUI_AA_DrawArc(xPos, yPos, r, r, 88, 93);
	GUI_AA_DrawArc(xPos, yPos, r, r, 175, 180);
	//GUI_AA_DrawArc(xPos, yPos, r, r, 268, 273);

	GUI_MEMDEV_CopyToLCD(hMemBMP_);

	return 1;
}

static int GUI_Draw_Arc_Standard(int ym)
{
	int step = 4;
	const int time_delay = 10;
	
	int hr_line = 322;
	int rc_lr_line = 270;
	int rc_line = ym + 255;
	//int rd_line = ym + 20;
	int rd_line_dx = 0;
	int rd_line_dy = 0;
	int rc_lc_line = 0;
	int rc_uc_line = 40;
	//int lb_line = ym + 20;
	int lb_line_dx = 0;
	int lb_line_dy = 0;
	
	int rdd_line = ym + 138;
	int rc_lcc_line = 130;
	int rc_rdd_line = 180;
	int dy = 0;
	
	GUI_SetPenSize(8);
	GUI_SetColor(GUI_WHITE);
	
	while(hr_line <= 478)
	{
		hr_line += step;
		GUI_AA_DrawLine(322, ym + 285, hr_line/*478*/, ym + 285);  // 下横线
		GUI_Delay(time_delay);
	}

	while(rc_lr_line <= 360)
	{
		rc_lr_line += step;
		GUI_AA_DrawArc (478, ym + 245, 40, 40, 270, rc_lr_line); // 右下弧线，弧长90
		GUI_Delay(time_delay);
	}

	while(rc_line >= ym + 138)
	{
		dy += step;
		rc_line = ym + 255 - dy;
		GUI_AA_DrawLine(518, rc_line/*ym + 138*/, 518, ym + 255); // 右垂线
		GUI_Delay(time_delay);
	}

	while(rc_lc_line <= 50)
	{
		rc_lc_line += step;
		GUI_AA_DrawArc (468, ym + 140, 50, 50, 355, 360); // 右中弧线，弧长50
		GUI_AA_DrawArc (468, ym + 140, 50, 50, 0, rc_lc_line/*50*/);
		GUI_Delay(time_delay);
	}

	while(((500 - rd_line_dx)>420)&&((ym + 100 - rd_line_dy)> (ym + 20)))
	{
		rd_line_dx += step;
		rd_line_dy += step;
		
		GUI_AA_DrawLine(500 - rd_line_dx, ym + 100 - rd_line_dy, 500, ym + 100); //  右斜线
		GUI_Delay(time_delay + 10);
	}

	while(rc_uc_line <= 135)
	{
		rc_uc_line += step;
		GUI_AA_DrawArc (400, ym + 50,  35, 35, 40, rc_uc_line); // 上弧线，弧长95
		GUI_Delay(time_delay);
	}

	while(((380 - lb_line_dx)>300)&&((ym + 20 + lb_line_dy)<(ym + 100)))
	{
		lb_line_dx += step;
		lb_line_dy += step;
		
		GUI_AA_DrawLine(380 - lb_line_dx,ym + 20 + lb_line_dy, 380, ym + 20);   //  左斜线
		GUI_Delay(time_delay + 10);
	}

	while(rc_lcc_line <= 180)
	{
		rc_lcc_line += step;
		GUI_AA_DrawArc (332, ym + 140, 50, 50, 130, rc_lcc_line/*180*/); // 左中弧线，弧长50
		GUI_Delay(time_delay);
	}

	while(rdd_line <= (ym + 255))
	{
		rdd_line += step;
		GUI_AA_DrawLine(282, ym + 138, 282, rdd_line/*ym + 255*/); // 左垂线
		GUI_Delay(time_delay);
	}
	
	while(rc_rdd_line <= 270)
	{
		rc_rdd_line += step;
		GUI_AA_DrawArc (322, ym + 245, 40, 40, 180, rc_rdd_line/*270*/); // 左下弧线，弧长95
		GUI_Delay(time_delay);
	}
	
	return NULL;
}


#define LCD_BKL_On   (GPIO_SetBits(LED_BKL_GPIO_PORT, LED_BKL_PIN))
#define LCD_BKL_Off  (GPIO_ResetBits(LED_BKL_GPIO_PORT, LED_BKL_PIN))
//
//
//
void Touchkey_Process(void)
{
	U32 t0, t1;
	OS_ERR  err;

	if(GetTouchKeyState() == Bit_RESET)
	{
		t0 = GUI_GetTime();
		while(GetTouchKeyState() == Bit_RESET)
		{
			t1 = GUI_GetTime() - t0;
			if(key_mode == 0)
			{
				if(t1 > 6000)
				{
					key_mode = 1;
					t0 = GUI_GetTime();
					LCD_BKL_On;

					animation_ctl = 1;
					//printf("back light on %dms\r\n", t1);
				}
			}
			else if(key_mode == 1)
			{
				if(t1 > 6000)
				{
					t0 = GUI_GetTime();
					bl_ctl = 1;
					LCD_BKL_Off;

					animation_ctl = 0;
					//printf("back light off %dms\r\n", t1);
				}
				else if((t1 > 3000) && (bl_ctl == 1))
				{
					bl_ctl = 0;
					LCD_BKL_On;

					animation_ctl = 1;
					
					//printf("back light on %dms\r\n", t1);
				}
			}

			OSTimeDly(50, OS_OPT_TIME_DLY, &err);
		}
	}
}

/************************************************************************
*
*Extern
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmhouse_smile;
extern int NormalWorkModeAnimation(void);
extern int Face_Appear_Fading_Smile(const GUI_BITMAP *pBM, int x_offset, int y_offset);


/************************************************************************
*
*   AnimStateManagemeng
*/
static void AnimRunStateManagement(void)  
{  
	#if 0
	
    switch(run_mode.state)  
    {  
	    case Mode_Compagnon_AQI_House: 
			//if(GUI_Set_Bitmap_Position(&bmhouse, 800, 400, 15, 20) == 1)
			if(GUI_Draw_Bitmap_Centre(&bmmaihouse) == NULL)
			{
				//printf("I am GUI_Set_Bitmap_Position!\r\n");
				run_mode.state = Mode_Compagnon_AQI_FACE;
				GUI_MEMDEV_Select(0);
			}
			break; 
	    case Mode_Compagnon_AQI_FACE:
			//if(NormalWorkModeAnimation() == 1)
			if(Face_Appear_Fading_Smile(&bmhouse_smile,NULL,-45) == NULL)
			{
				//printf("I am NormalWorkModeAnimation!\r\n");
				run_mode.state = Mode_Compagnon_AQI_NUM;
			}
			break;
	    case Mode_Compagnon_AQI_NUM:
			//if(NumAqiAnimatiom() == 1)
			if(GUI_Display_IAQ_Value("99",NULL,-80) == NULL)
			{
				//printf("I am GUIDrawTextWithAlpha!\r\n");
				GUI_Delay(1500);
				run_mode.state = Mode_Compagnon_AQI_SCALE;
			}
			break;
	    case Mode_Compagnon_AQI_SCALE:
			//GUI_MEMDEV_Select(0);
			//GUI_MEMDEV_Clear(hMemBMP_);
			//GUI_Clear();
			if ((GUI_Set_Bitmap_Scale(&bmmaihouse, 650, 650, 15) == 1))
			{	    	
			    //GUI_Display_Text("HELLO WORLD");
				//printf("I am GUI_Set_Bitmap_Scale!\r\n");
				GUI_Delay(1500);
				run_mode.state = Mode_Expert_AQI;
			}
			break;
			case Mode_Expert_AQI:
			if ((GUI_Set_Scale_Pos(&bmmaihouse, &bmtemperature,520, 520, 15) == 1))
			{
				//GUI_MEMDEV_Select(0);
				//GUI_MEMDEV_Clear(hMemBMP_);
				//GUI_Clear();
				SENSORTaskPend();
				GUI_Set_Ring_Indicator(sensor_cali_data.pm2005_cali_data, sensor_cali_data.pm2005_cali_data,100, 320,PM2005_TY);
				GUI_Set_Ring_Indicator(sensor_cali_data.voc_cali_data, sensor_cali_data.voc_cali_data,300, 320,VOC_TY);
				GUI_Set_Ring_Indicator(gMyData.iCO2, gMyData.iCO2,500, 320,CO2_TY);
				GUI_Set_Ring_Indicator(sensor_cali_data.h20_cali_data, sensor_cali_data.h20_cali_data,700, 320,H20_TY);
				GUI_Temp_Disp(sensor_cali_data.temp_cail_data / 1000,-12,NULL);
				SENSORTaskPost();	
				GUI_Delay(12*1000);
			
				//printf("I am GUI_Set_Bitmap_Scale_Pos!\r\n");
				run_mode.state = Mode_Black_Screen;
			}
			break;
			case Mode_Black_Screen:
			//if (GUI_Set_Bitmap_Scale_Pos(&bmmaihouse,anim_xpos - 120, 0, 15, 20) == 1)
			GUI_MEMDEV_Select(0);
		    GUI_MEMDEV_Clear(hMemBMP_);
			GUI_Clear();
			
			//printf("I am Mode_Compagnon_Black_Screen!\r\n");
			GUI_Delay(1500);
			run_mode.state = Mode_Compagnon_AQE_Cloud;
			break;
		case Mode_Compagnon_AQE_Cloud:
			if(GUI_Draw_Bitmap_Centre(&bmcloud) == NULL)
			{
				//printf("I am Mode_Compagnon_AQE_Cloud!\r\n");
				run_mode.state = Mode_Compagnon_AQE_FACE;
				GUI_MEMDEV_Select(0);
			}
	        break; 
		case Mode_Compagnon_AQE_FACE:
			if(Face_Appear_Fading_Smile(&bmhouse_smile,40,-45) == NULL)
			{
				run_mode.state = Mode_Compagnon_AQE_NUM;
			}
        	break; 
		case Mode_Compagnon_AQE_NUM:
			if(GUI_Display_IAQ_Value("99",45,-75) == NULL)
			{
				GUI_Delay(1500);
				run_mode.state = Mode_Compagnon_AQE_SCALE;
			}
        	break; 
		case Mode_Compagnon_AQE_SCALE:
			if ((GUI_Set_Bitmap_Scale(&bmcloud, 650, 650, 15) == 1))
			{	    	
			    //GUI_Display_Text("HELLO ALDES");
				//printf("I am Mode_Compagnon_AQE_SCALE!\r\n");
				GUI_Delay(1500);
				run_mode.state = Mode_Expert_AQE;
			}
        	break; 
	    case Mode_Expert_AQE:
			if ((GUI_Set_Scale_Pos(&bmcloud,&bmcloud_temp, 520, 520, 15) == 1))
			{				
				GUI_Set_Ring_Indicator(NULL, NULL,100, 320,PM25_Cloud);
				GUI_Set_Ring_Indicator(NULL, NULL,300, 320,PM10_Cloud);
				GUI_Set_Ring_Indicator(NULL, NULL,500, 320,NO2_Cloud);
				GUI_Set_Ring_Indicator(NULL, NULL,700, 320,O3_Cloud);
				GUI_Temp_Disp(sensor_cali_data.temp_cail_data / 100,80,NULL);
					
				GUI_Delay(12*1000);
				run_mode.state = Mode_AQE_Black_Screen;
			}
	    	break;
		case Mode_AQE_Black_Screen:
			GUI_MEMDEV_Select(0);
		    GUI_MEMDEV_Clear(hMemBMP_);
			GUI_Clear();
			
			//printf("I am Mode_AQE_Black_Screen!\r\n");
			GUI_Delay(1500);
			run_mode.state = Mode_Idle;
	    	break;	
		case Mode_Idle:
			run_mode.state = Mode_Compagnon_AQI_House;
			gui_run.gui_run_state = GUI_Idle;
        	break; 
	    default:  
	        break;
    }  

	#endif
} 

/************************************************************************
*
*   AnimStartStateManagement
*/
static void AnimStartStateManagement(void)  
{
	#if 0
	
	unsigned char pairing_loop = 3;
	unsigned char bluetooth_loop = 32;
	unsigned char wifi_loop = 32;
	
	switch(startup_mode.startup_state)  
    {  
	    case STARTUP_LOGO:
			GUI_DrawBitmap(&bmlogo, (LCD_GetXSize() - bmlogo.XSize) / 2, (LCD_GetYSize() - bmlogo.YSize) / 2);
			GUI_Delay(5000);
			GUI_Clear();
			startup_mode.startup_state = STARTUP_APPDOWNLOAD;
			break; 
		case STARTUP_APPDOWNLOAD: 
			GUI_DrawBitmap(&bmpp_download, (LCD_GetXSize() - bmpp_download.XSize) / 2, (LCD_GetYSize() - bmpp_download.YSize) / 2);
			GUI_Delay(5000);
			GUI_Clear();
			startup_mode.startup_state = STARTUP_Bluetooth;
			break; 
		case STARTUP_Bluetooth: 
			WM_MULTIBUF_Enable(0);
			DEMO_Ble_Starup(bluetooth_loop);
			WM_MULTIBUF_Enable(1);
			GUI_Clear();
			startup_mode.startup_state = STARTUP_Pairing;
			break; 
		case STARTUP_Pairing: 
			while(pairing_loop--)
			{
				GUI_Draw_Arc_Standard(60);
				GUI_Clear();
			}
			startup_mode.startup_state = STARTUP_Black_Screen;
			break; 
		case STARTUP_Black_Screen:
			GUI_Clear();
			GUI_Delay(1000);
			startup_mode.startup_state = STARTUP_Pairing_Sta;
			break;
		case STARTUP_Pairing_Sta: 
			GUI_Pairing_Successful();
			GUI_Delay(1000);
			GUI_Clear();
			startup_mode.startup_state = STARTUP_NO_WIFI;
			break;
		case STARTUP_NO_WIFI:
			WM_MULTIBUF_Enable(0);
			DEMO_WiFiSigStarup(wifi_loop);
			WM_MULTIBUF_Enable(1);
			GUI_Clear();
			startup_mode.startup_state = STARTUP_Idle;
			break;
		case STARTUP_Idle:
			GUI_Clear();
			GUI_Delay(1000);
			//startup_mode.startup_state = STARTUP_LOGO;
			gui_run.gui_run_state = GUI_GO;
			break;
	    default:  
	        break;
	}

	#endif
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
extern void LCD_Clear(uint16_t Color);

void MainTask(void) {

  OS_ERR  err;
  //WM_SetCreateFlags(WM_CF_MEMDEV);  
  GUI_Init();
  //WM_MULTIBUF_Enable(1);



  hMemBMP_ = GUI_MEMDEV_CreateFixed(0, 
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

  //hText = WM_CreateWindowAsChild(300, 240, 200, 200, WM_HBKWIN, WM_CF_HASTRANS | WM_CF_MEMDEV | WM_CF_SHOW, _cbText, 0);

  run_mode.state = Mode_Compagnon_AQI_House;
  startup_mode.startup_state = STARTUP_LOGO;
  gui_run.gui_run_state = GUI_Startup;
  
#if 0
  while(1)
  {
  	//if(animation_ctl)
  	//{
	  	switch(gui_run.gui_run_state)  
	    {  
		    case GUI_Startup:
				AnimStartStateManagement(); 	// startup management
				break;
			case GUI_GO:
				AnimRunStateManagement();
				break;
			case GUI_Idle:
				gui_run.gui_run_state = GUI_GO;
				break;
	  	}
  	//}
			
	  GUI_Delay(1000);
  }
  
#endif
  
#if 0
  while(1)
  {
  	AnimRunStateManagement(); // GUI management
	GUI_Delay(500);
  }
#endif

#if 1
  while(1)
  { 
	  //GUI_SetColor(GUI_RED);
	  //GUI_Display_Text("HELLO WORLD");
	  GUI_SetBkColor(GUI_GREEN);
	  //GUI_SetBkColor(GUI_RED);
	  //GUI_SetBkColor(GUI_BLUE);
	  //GUI_SetBkColor(GUI_BLACK);
	  GUI_Clear();

	  GUI_SetFont(&GUI_Font32B_ASCII);
	  GUI_SetColor(GUI_WHITE);
	  //GUI_DispStringHCenterAt("m", 20, 30);
	  GUI_DispStringHCenterAt("%", 470, 0);
	  GUI_DispStringHCenterAt("%", 470, 820);

		
	  //GUI_SetFont(&GUI_FontD32);
	  //GUI_SetColor(GUI_WHITE);
	  //GUI_DispDecAt(0,450,820,1);
	  //GUI_DispDecAt(1,1,0,1);
	  //GUI_DispDecAt(1,2,0,1);
	  
	  GUI_Delay(1000);

	  //GUI_SetPenSize(8);
	  //GUI_SetColor(GUI_YELLOW);
	  //GUI_AA_DrawArc(LCD_GetXSize() / 2, LCD_GetYSize() / 3,50,50,180,360);
	  //GUI_Delay(100);
  }
#endif

#if 0
  //NormalWorkModeAnimation();
  while(1)
  {
	  GUI_Draw_Arc_Standard(60);
	  GUI_Clear();
	  GUI_Delay(1000);
  }
#endif

#if 0
while(1){
	
  GUI_Pairing_Successful();
}
#endif

  #if 0
    while(1)
    {
		val += 5;
		if(val > 1000) val = 0;
		GUI_Set_Ring_Indicator(val, 150, 160, 50, 80);
    	GUI_Delay(100);
    }
 #endif

 	#if 0

  //	_GetImage(&_aImage[0]);
	//_RotateMouthAndEye();
	WM_MULTIBUF_Enable(0);
	while(1)
	{
		//DEMO_WiFiSigStarup();
		DEMO_Ble_Starup();
	}
	#endif
}

/*************************** End of file ****************************/
