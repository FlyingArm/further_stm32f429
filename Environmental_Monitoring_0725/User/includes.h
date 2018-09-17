/*
*********************************************************************************************************
*
*********************************************************************************************************
*/

#ifndef  __INCLUDES_H__
#define  __INCLUDES_H__

/*
*********************************************************************************************************
*                                         标准库
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>


/*
*********************************************************************************************************
*                                         其它库
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>
#include  <app_cfg.h>


/*
*********************************************************************************************************
*                                           OS
*********************************************************************************************************
*/

#include  <os.h>


/*
*********************************************************************************************************
*                                           宏定义
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               emWin
*********************************************************************************************************
*/
#include "GUI.h"
#include "WM.h"

/*
*********************************************************************************************************
*                                        APP / BSP
*********************************************************************************************************
*/

#include  <bsp.h>
#include "stm32f429i_discovery.h"
//#include "stm32f429i_discovery_sdram.h"
#include "stm32f429i_discovery_lcd.h"
#include "System.h"
#include "bsp_uart_fifo.h"
#include "HFLPT230.h"
#include "malloc.h"
#include "bsp_spi_flash.h"
#include "tty.h"
#include "sha.h"
#include "stm32f4xx_rtc.h"

#include "Cgk_module.h"
#include "bsp_i2c_cgk.h"
#include "touch_key.h"

#include "tm_stm32f4_usb_vcp.h"
#include "usbd_usr.h"

#define SPI_FLASH_WIFI

typedef enum
{
	ble_wifi_connect_request 	= 1,
	ble_wifi_connect_success 	= 2,
	ble_wifi_connect_error_401 	= 3,
	ble_wifi_connect_error_500 	= 4,
	ble_wifi_connect_error_598 	= 5,
	ble_wifi_connect_error_599 	= 6,
	ble_idle = 0,
}ble_request_t;

typedef struct _DEV_STATE_CTR
{
	U8 ButtonCtr;
	U8 SensorCtr;
	U8 IsPM25Pressed;
	U8 PM25PressedCtr;
	U8 IsCO2Pressed;
	U8 CO2PressedCtr;

}DEV_STATE_CTR_T;


#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
