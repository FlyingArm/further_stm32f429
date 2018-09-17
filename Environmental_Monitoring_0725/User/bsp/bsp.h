/*
*********************************************************************************************************
*
*
*********************************************************************************************************
*/

#ifndef _BSP_H_
#define _BSP_H

#define STM32_V5    /* V5��V6������һ���ģ���������ʹ�ܺ궨��STM32_V5���� */
//#define STM32_X3

/* ����Ƿ����˿������ͺ� */
#if !defined (STM32_V5) && !defined (STM32_X3)
	#error "Please define the board model : STM32_X3 or STM32_V5"
#endif

/* ���� BSP �汾�� */
#define __STM32F1_BSP_VERSION		"1.1"

/* CPU����ʱִ�еĺ��� */
//#define CPU_IDLE()		bsp_Idle()

/* ʹ����Դ�ļ���ʹ��uCOS-III�ĺ���, �����Դ�ļ���Ҫ��ָBSP�����ļ� */
#define  uCOS_EN      1

#if uCOS_EN == 1    
	#include "os.h"   

	#define  ENABLE_INT()      CPU_CRITICAL_EXIT()     /* ʹ��ȫ���ж� */
	#define  DISABLE_INT()     CPU_CRITICAL_ENTER()    /* ��ֹȫ���ж� */
#else
	/* ����ȫ���жϵĺ� */
	#define ENABLE_INT()	__set_PRIMASK(0)	/* ʹ��ȫ���ж� */
	#define DISABLE_INT()	__set_PRIMASK(1)	/* ��ֹȫ���ж� */
#endif

/* ���������ڵ��Խ׶��Ŵ� */
#define BSP_Printf		printf
//#define BSP_Printf(...)

#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef TRUE
	#define TRUE  1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

/*
	EXTI9_5_IRQHandler ���жϷ�������ɢ�ڼ��������� bsp�ļ��С�
	��Ҫ���ϵ� stm32f4xx_it.c �С�

	���������б�ʾEXTI9_5_IRQHandler��ں������зŵ� stm32f4xx_it.c��
*/
#define EXTI9_5_ISR_MOVE_OUT

#define DEBUG_GPS_TO_COM1	/* ��ӡGPS���ݵ�����1 */

/* ͨ��ȡ��ע�ͻ������ע�͵ķ�ʽ�����Ƿ�����ײ�����ģ�� */
//#include "bsp_ext_io.h"
#include "bsp_uart_fifo.h"
//#include "bsp_led.h"
//#include "bsp_timer.h"
//#include "bsp_key.h"
//#include "bsp_dwt.h"
//#include "bsp_msg.h"
#include "bsp_rtc.h"
//#include "bsp_tim_pwm.h"
//#include "bsp_cpu_flash.h"
//#include "bsp_sdio_sd.h"
#include "bsp_i2c_gpio.h"
//#include "bsp_eeprom_24xx.h"
//#include "bsp_si4730.h"
//#include "bsp_hmc5883l.h"
//#include "bsp_mpu6050.h"
//#include "bsp_bh1750.h"
//#include "bsp_bmp085.h"
//#include "bsp_wm8978.h"
//#include "bsp_gt811.h"

#include "bsp_fmc_sdram.h"
//#include "bsp_nand_flash.h"

//#include "bsp_tft_429.h"
//#include "bsp_tft_lcd.h"

#include "bsp_touch.h"
//#include "TOUCH_STMPE811.h"
#include "bsp_ts_ft5x06.h"
//#include "bsp_ts_ft5x06_v2.h"
#include "bsp_ts_gt911.h"

//#include "bsp_camera.h"

//#include "bsp_ad7606.h"

//#include "bsp_gps.h"
//#include "bsp_oled.h"
////#include "bsp_mg323.h"
//#include "bsp_sim800.h"

#include "bsp_spi_bus.h"
#include "bsp_spi_flash.h"
//#include "bsp_tm7705.h"
//#include "bsp_ads1256.h"
//#include "bsp_vs1053b.h"

//#include "bsp_ds18b20.h"
//#include "bsp_dac8501.h"
//#include "bsp_dht11.h"

//#include "bsp_ir_decode.h"
//#include "bsp_ps2.h"

//#include "bsp_modbus.h"
//#include "bsp_rs485_led.h"
//#include "bsp_user_lib.h"

//#include "bsp_dac8501.h"
//#include "bsp_dac8562.h"
//#include "bsp_beep.h"

//#include "bsp_adc_dso.h"
//#include "bsp_dac_wave.h"

//#include "bsp_esp8266.h"
//#include "bsp_ad9833.h"

#include "tm_stm32f4_rtc.h"

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 


#define GPIOA_ODR_Addr    (GPIOA_BASE+20) //0x40020014
#define GPIOB_ODR_Addr    (GPIOB_BASE+20) //0x40020414 
#define GPIOC_ODR_Addr    (GPIOC_BASE+20) //0x40020814 
#define GPIOD_ODR_Addr    (GPIOD_BASE+20) //0x40020C14 
#define GPIOE_ODR_Addr    (GPIOE_BASE+20) //0x40021014 
#define GPIOF_ODR_Addr    (GPIOF_BASE+20) //0x40021414    
#define GPIOG_ODR_Addr    (GPIOG_BASE+20) //0x40021814   
#define GPIOH_ODR_Addr    (GPIOH_BASE+20) //0x40021C14    
#define GPIOI_ODR_Addr    (GPIOI_BASE+20) //0x40022014     

#define GPIOA_IDR_Addr    (GPIOA_BASE+16) //0x40020010 
#define GPIOB_IDR_Addr    (GPIOB_BASE+16) //0x40020410 
#define GPIOC_IDR_Addr    (GPIOC_BASE+16) //0x40020810 
#define GPIOD_IDR_Addr    (GPIOD_BASE+16) //0x40020C10 
#define GPIOE_IDR_Addr    (GPIOE_BASE+16) //0x40021010 
#define GPIOF_IDR_Addr    (GPIOF_BASE+16) //0x40021410 
#define GPIOG_IDR_Addr    (GPIOG_BASE+16) //0x40021810 
#define GPIOH_IDR_Addr    (GPIOH_BASE+16) //0x40021C10 
#define GPIOI_IDR_Addr    (GPIOI_BASE+16) //0x40022010 
 

#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)   
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n) 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  

#define PHout(n)   BIT_ADDR(GPIOH_ODR_Addr,n)  
#define PHin(n)    BIT_ADDR(GPIOH_IDR_Addr,n)  

#define PIout(n)   BIT_ADDR(GPIOI_ODR_Addr,n)  
#define PIin(n)    BIT_ADDR(GPIOI_IDR_Addr,n)  


/* �ṩ������C�ļ����õĺ��� */
void bsp_DelayMS(uint32_t _ulDelayTime);
void bsp_DelayUS(uint32_t _ulDelayTime);
void bsp_Init(void);
void bsp_Idle(void);
void BSP_Tick_Init (void);

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
