/*
*********************************************************************************************************
*
*	???? : GT911??????????
*	???? : bsp_ts_GT911.h
*	?    ? : ???
*	?    ? : V1.0
*
*********************************************************************************************************
*/

#include "stm32f4xx.h"

#ifndef _BSP_TS_GT911_H
#define _BSP_TS_GT911_H

#define LCD_XSIZE       480
#define LCD_YSIZE       854

/* I2C??,??ID */
#define GT911_I2C_ADDR       0xBA
 
#define GT911_TOUCH_POINTS  1

#define GT911_READ_DATANUM  7
typedef struct
{
	uint32_t ChipID;
	uint8_t  Enable;
	uint8_t  TimerCount;
	
	uint8_t  Count;			/* ????? */
	
	uint16_t X[GT911_TOUCH_POINTS];
	uint16_t Y[GT911_TOUCH_POINTS];	
	uint8_t  id[GT911_TOUCH_POINTS];
	uint8_t  Event[GT911_TOUCH_POINTS];
}GT911_T;


#define GT_INTMODE_REG  0x804D

#define GT911_READ_XY_REG          0x814E	/* ????? */ 
#define GT911_CLEARBUF_REG         0x814E /* ??????? */ 
#define GT911_CONFIG_REG	         0x8047	/* ??????? */ 
//#define GT911_CONFIG_REG	         0x8050	/* ??????? */ 
#define GT911_COMMAND_REG          0x8040 /* ???? */ 
#define GT911_PRODUCT_ID_REG       0x8140 /*productid*/ 
#define GT911_VENDOR_ID_REG        0x814A /* ???????? */ 
#define GT911_CONFIG_VERSION_REG   0x8047 /* ??????? */ 
#define GT911_CONFIG_CHECKSUM_REG  0x80FF /* ??????? */ 
#define GT911_FIRMWARE_VERSION_REG 0x8144 /* ????? */ 

void     GT911_InitHard(void);
uint32_t GT911_ReadID(void);
uint8_t  GT911_PenInt(void);
uint16_t GT911_ReadVersion(void);
void     GT911_Timer1ms(void);
void     GT911_OnePiontScan(void);

extern   GT911_T g_tGT911;

#endif

/***************************** ????? www.armfly.com (END OF FILE) *********************************/
