/*
*********************************************************************************************************
*	                                  
*	模块名称 : GUI头文件
*	文件名称 : MainTask.c
*	版    本 : V1.0
*	说    明 : 所有GUI应用公用的头文件
*
*********************************************************************************************************
*/
#ifndef __MainTask_H
#define __MainTask_H

#include "GUI.h"
#include "DIALOG.h"
#include "WM.h"
#include "BUTTON.h"
#include "CHECKBOX.h"
#include "DROPDOWN.h"
#include "EDIT.h"
#include "FRAMEWIN.h"
#include "LISTBOX.h"
#include "MULTIEDIT.h"
#include "RADIO.h"
#include "SLIDER.h"
#include "TEXT.h"
#include "PROGBAR.h"
#include "SCROLLBAR.h"
#include "LISTVIEW.h"
#include "GRAPH.h"
#include "MENU.h"
#include "MULTIPAGE.h"
#include "ICONVIEW.h"
#include "TREEVIEW.h"
#include "IMAGE.h"

/*
*********************************************************************************************************
*                                       函数和变量
*********************************************************************************************************
*/
/* 主界面上的系统信息窗口和状态栏 */
//extern WM_HWIN hWinStatus;

/* 我的电脑APP */
//extern void App_Computer(WM_HWIN hWin);

/* 收音机APP */
//extern void App_Radio(WM_HWIN hWin);


/* 所有未制作APP 都使用这个界面 */
//extern void App_Reserved(WM_HWIN hWin);


/*
*********************************************************************************************************
*                                       emWin字体
*********************************************************************************************************
*/
//extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ_SimSun_16;
///extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ16;

/* 默认情况下，系统字体采用GUI_FontHZ16 */
//#define  GUI_SysFontHZ16  GUI_FontHZ16

/*
*********************************************************************************************************
*                                       图片和图标
*********************************************************************************************************
*/
//extern const unsigned char _acbkpic[13632UL + 1];

extern GUI_CONST_STORAGE GUI_BITMAP _bmWhiteCircle_10x10;
extern GUI_CONST_STORAGE GUI_BITMAP _bmWhiteCircle_6x6;

/* 第1个ICONVIEW界面 */
extern GUI_CONST_STORAGE GUI_BITMAP bmComputer;


/* 第2个ICONVIEW界面 */
extern GUI_CONST_STORAGE GUI_BITMAP bmRadio;


/* 第3个ICONVIEW界面 */
extern GUI_CONST_STORAGE GUI_BITMAP bmGPS;

/*
*********************************************************************************************************
*                                       emWin自定义消息宏定义
*********************************************************************************************************
*/
#define MSG_TextStart     (GUI_ID_USER + 0x01)
#define MSG_TextCancel    (GUI_ID_USER + 0x02)

#define MSG_SDPlugIn      (GUI_ID_USER + 0x10)
#define MSG_SDPlugOut     (GUI_ID_USER + 0x11)
#define MSG_NETPlugIn     (GUI_ID_USER + 0x12)
#define MSG_NETPlugOut    (GUI_ID_USER + 0x13)

#define MSG_DispSpec      (GUI_ID_USER + 0x30)
#define MSG_MusicStart    (GUI_ID_USER + 0x31)
#define MSG_NextMusic     (GUI_ID_USER + 0x32)
#define MSG_MusicCancel   (GUI_ID_USER + 0x33)

#define MSG_CANReceive  (GUI_ID_USER + 0x40)

#define MSG_ModbusErr     (GUI_ID_USER + 0x50)
#define MSG_ModbusSuccess (GUI_ID_USER + 0x51)

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
