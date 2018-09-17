/*
*********************************************************************************************************
*
*	???? : ft5x0x??????????
*	???? : bsp_ts_GT911.c
*	?    ? : V1.0
*	?    ? : FocalTech ft5x0x?????????4.3??480*272.
*
*********************************************************************************************************
*/
#include "GUI.h"
#include "bsp.h"

#if 1
	#define printf_dbg printf
#else
	#define printf_dbg(...)
#endif

GT911_T g_tGT911;

/* ??GPIO?? */
#define RCC_TP_INT			RCC_AHB1Periph_GPIOH
#define PORT_TP_INT			GPIOH
#define PIN_TP_INT			GPIO_Pin_7

#define RCC_TP_RST          RCC_AHB1Periph_GPIOH
#define PORT_TP_RST         GPIOH
#define PIN_TP_RST          GPIO_Pin_6


/* ???RST?INT?? */ 
#define GT911_RST_1() GPIO_SetBits(PORT_TP_RST,PIN_TP_RST);	// PORT_TP_RST->BSRRL = PIN_TP_RST	/* RST = 1 */ 
#define GT911_RST_0() GPIO_ResetBits(PORT_TP_RST,PIN_TP_RST);  // PORT_TP_RST->BSRRH = PIN_TP_RST	/* RST = 0 */ 

#define GT911_INT_1() GPIO_SetBits(PORT_TP_INT,PIN_TP_INT);   //PORT_TP_INT->BSRRL = PIN_TP_INT	/* INT = 1 */ 
#define GT911_INT_0() GPIO_ResetBits(PORT_TP_INT,PIN_TP_INT); //PORT_TP_INT->BSRRH = PIN_TP_INT	/* INT = 0 */ 


static void    GT911_ReadReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen);
static uint8_t GT911_WriteReg(uint8_t addr,uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen) ;

/*
*********************************************************************************************************
*	? ? ?: GT911_InitHard
*	????: ??????
*	?    ?: ?
*	? ? ?: ?
*********************************************************************************************************
*/

static void GT911_Delay(void)
{
	uint16_t i;

	/* 
		CPU??168MHz?,???Flash??, MDK?????????????????
		?????5?,SCL?? = 1.78MHz (???: 92ms, ????,??????????????????????)
		?????10?,SCL?? = 1.1MHz (???: 138ms, ???: 118724B/s)
		?????30?,SCL?? = 440KHz, SCL?????1.0us,SCL?????1.2us

		??????2.2K??,SCL??????0.5us,???4.7K?,?????1us

		??????400KHz???????
	*/
	for (i = 0; i < 360; i++);
}


void GT911_InitHard(void)
{	
	uint8_t buf[4]; 
	buf[0] = 0x01; 
	uint8_t i = 0;
	uint32_t value = 0; 
	GPIO_InitTypeDef GPIO_InitStructure; 

	RCC_AHB1PeriphClockCmd(RCC_TP_INT | RCC_TP_RST, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	/* ????? */ 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	/* ?????? */ 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ???????? */ 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO????? */ 
	GPIO_InitStructure.GPIO_Pin = PIN_TP_RST; 
	GPIO_Init(PORT_TP_RST, &GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	/* ?????? */ 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ???????? */ 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO????? */ 
	GPIO_InitStructure.GPIO_Pin = PIN_TP_INT; 
	GPIO_Init(PORT_TP_INT, &GPIO_InitStructure); 
	

	GT911_RST_0(); //RST????? 
	GT911_INT_0(); //INT????? 
	bsp_DelayMS(1); //??30ms,??1 
	GT911_RST_1(); //RST????? 
	GT911_INT_0(); //INT????? 
	bsp_DelayMS(10); //??30ms,??20 
	GT911_INT_0(); 
	bsp_DelayMS(10); //??30ms,??20 
	GT911_INT_1(); 
	
	RCC_AHB1PeriphClockCmd(RCC_TP_INT, ENABLE);	/* ??GPIO?? */ 

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	/* ????? */ 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	/* ?????? */ 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ???????? */ 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO????? */ 

	GPIO_InitStructure.GPIO_Pin = PIN_TP_INT; 
	GPIO_Init(PORT_TP_INT, &GPIO_InitStructure); 

// soft reset
	buf[0] = 0x03; 
	GT911_WriteReg(GT911_I2C_ADDR,GT_INTMODE_REG, (uint8_t *)buf, 1);
	buf[0] = 0;
	GT911_ReadReg(GT_INTMODE_REG, buf, 1); 
	
	bsp_DelayMS(20); 


#if 0	/* ?????????? for debug */
	{
		uint8_t i;
		uint8_t reg_value;
		
		printf_dbg("[FTS] Touch Chip\r\n");
		
		bsp_DelayMS(10);
		
		for (i = 0; i < 255; i++)
		{
			GT911_ReadReg(i, &reg_value, 1);
			printf_dbg(" 0x%02X = 0x%02X\r\n", i, reg_value);
		}
	}
#endif	
	
#if 0
	{
		uint8_t reg_addr;
		uint8_t reg_value;
		
		//get some register information
		reg_addr = GT911_REG_FW_VER;
		GT911_ReadReg(reg_addr, &reg_value, 1);
		printf_dbg("[FTS] Firmware version = 0x%x\r\n", reg_value);
		
		reg_addr = GT911_REG_POINT_RATE;
		GT911_ReadReg(reg_addr, &reg_value, 1);
		printf_dbg("[FTS] report rate is %dHz.\r\n", reg_value * 10);
		
		reg_addr = GT911_REG_THGROUP;
		GT911_ReadReg(reg_addr, &reg_value, 1);
		printf_dbg("[FTS] touch threshold is %d.\r\n", reg_value * 4);
	}
#endif	
	
	
	//value = GT911_ReadID();
	g_tGT911.TimerCount = 0;
	g_tGT911.Enable = 1;
}

/*
*********************************************************************************************************
*	? ? ?: GT911_PenInt
*	????: ??????
*	?    ?: ?
*	? ? ?: ?
*********************************************************************************************************
*/
uint8_t GT911_PenInt(void)
{
	if ((PORT_TP_INT->IDR & PIN_TP_INT) == 0)
	{
		return 1;
	}
	return 0;
}

/*
*********************************************************************************************************
*	? ? ?: GT911_ReadReg
*	????: ?1??????????
*	?    ?: _usRegAddr : ?????
*			  _pRegBuf : ????????
*			 _ucLen : ????
*	? ? ?: ?
*********************************************************************************************************
*/
static void GT911_ReadReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	uint8_t i;

	i2c_Start();							/* ?????? */

	i2c_SendByte(GT911_I2C_ADDR);			/* ??????+??? */
	i2c_WaitAck();

	i2c_SendByte(_usRegAddr >> 8);		/* ???8? */
	i2c_WaitAck();

	i2c_SendByte(_usRegAddr);				/* ???8? */
	i2c_WaitAck();

	i2c_Start();
	i2c_SendByte(GT911_I2C_ADDR + 0x01);	/* ??????+??? */
	i2c_WaitAck();

	for (i = 0; i < _ucLen - 1; i++)
	{
		_pRegBuf[i] = i2c_ReadByte();		/* ?????? */
		i2c_Ack();
	}

	/* ?????? */
	_pRegBuf[i] = i2c_ReadByte();			/* ?????? */
	i2c_NAck();

	i2c_Stop();								/* ?????? */
}

static uint8_t GT911_WriteReg(uint8_t addr,uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen) 
{ 
	uint8_t i; 

	i2c_Start();	/* ?????? */ 

	i2c_SendByte(addr);	/* ??????+??? */ 
	if( 0 !=i2c_WaitAck() ) {
		return 1;
	}

	i2c_SendByte(_usRegAddr >> 8);	/* ???8? */ 
	if( 0 !=i2c_WaitAck() ) {
		return 1;
	}

	i2c_SendByte(_usRegAddr);	/* ???8? */ 
	if( 0 !=i2c_WaitAck() ) {
		return 1;
	}

	for (i = 0; i < _ucLen; i++) 
	{ 
		i2c_SendByte(_pRegBuf[i]);	/* ????? */ 
		if( 0 !=i2c_WaitAck() ) {
			return 1;
		}
	} 

	i2c_Stop(); /* ?????? */ 
	return 0;
} 

/*
*********************************************************************************************************
*	? ? ?: GT911_Timer1ms
*	????: ??1ms??1?
*	?    ?: ?
*	? ? ?: ?
*********************************************************************************************************
*/
void GT911_Timer1ms(void)
{
	g_tGT911.TimerCount++;
}

/*
*********************************************************************************************************
*	? ? ?: GT911_OnePiontScan
*	????: GT911_OnePiontScan????????????????,?bsp_ts_GT911.h????????
*             #define GT911_TOUCH_POINTS   1
*             ????emWin?
*	?    ?: ?
*	? ? ?: ?
*********************************************************************************************************
*/
GUI_PID_STATE State;

void GT911_OnePiontScan(void)
{
  uint8_t buf[GT911_READ_DATANUM]; 
	uint8_t Clearbuf = 0; 
	uint8_t i; 
	uint16_t x , y ;
	static uint8_t s_tp_down = 0; 
	
	if (g_tGT911.Enable == 0)
	{
		return;
	}
		
	if (GT911_PenInt() == 0)
	{
		return;
	}
	
	GT911_ReadReg(GT911_READ_XY_REG, buf, 1);	
	
    /* ????????? */	
	if ((buf[0] & 0x0F) == 0)
	{
		if (s_tp_down == 1)
		{
			/* State.x?State.y???????,State?????,???????????? */
			s_tp_down = 0;
			State.Pressed = 0;
			GUI_PID_StoreState(&State);	/* ?? */
		}
		GT911_WriteReg(GT911_I2C_ADDR,GT911_READ_XY_REG, (uint8_t *)&Clearbuf, 1); 
		return;
	}
	GT911_ReadReg(GT911_READ_XY_REG+1, &buf[1], 6); 
	GT911_WriteReg(GT911_I2C_ADDR,GT911_READ_XY_REG, (uint8_t *)&Clearbuf, 1); 
	
	g_tGT911.X[0] = ((uint16_t)buf[3] << 8) + buf[2]; 
	g_tGT911.Y[0] = ((uint16_t)buf[5] << 8) + buf[4]; 
	

	{
		x = g_tGT911.X[0];
		y = g_tGT911.Y[0];	
		
		/* ???? */
		if (x > LCD_XSIZE)
		{
			x = LCD_XSIZE;
		}			
		if (y > LCD_YSIZE)
		{
			y = LCD_YSIZE;
		}			
	}

	if (s_tp_down == 0)
	{
		s_tp_down = 1;
		State.x = x;
		State.y = y;
		State.Pressed = 1;
		GUI_PID_StoreState(&State);	
	}
	else
	{
		State.x = x;
		State.y = y;
		State.Pressed = 1;
		GUI_PID_StoreState(&State);	
	}

#if 0
	for (i = 0; i < CFG_POINT_READ_BUF; i++)
	{
		printf("%02X ", buf[i]);
	}
	printf("\r\n");

#endif

#if 0  /* ??5?????? */	
	printf("(%5d,%5d,%3d,%3d) ",  g_tGT911.X[0], g_tGT911.Y[0], g_tGT911.Event[0],  g_tGT911.id[0]);
	printf("(%5d,%5d,%3d,%3d) ",  g_tGT911.X[1], g_tGT911.Y[1], g_tGT911.Event[1],  g_tGT911.id[1]);
	printf("(%5d,%5d,%3d,%3d) ",  g_tGT911.X[2], g_tGT911.Y[2], g_tGT911.Event[2],  g_tGT911.id[2]);
	printf("(%5d,%5d,%3d,%3d) ",  g_tGT911.X[3], g_tGT911.Y[3], g_tGT911.Event[3],  g_tGT911.id[3]);
	printf("(%5d,%5d,%3d,%3d) ",  g_tGT911.X[4], g_tGT911.Y[4], g_tGT911.Event[4],  g_tGT911.id[4]);
	printf("\r\n");
#endif	

}


/*
*********************************************************************************************************
*	? ? ?: GT911_ReadID
*	????: ???ID, ???4.3?5.0?7.0????
*			????:  4.3?id = 0x55    5.0?id = 0x0A  7.0?id = 0x06
*	?    ?: ?
*	? ? ?: 1????ID
*********************************************************************************************************
*/
uint32_t GT911_ReadID(void)
{
	uint8_t idbuf[4];
	uint32_t id;
	
		/* Product_ID*/ 
	GT911_ReadReg(GT911_PRODUCT_ID_REG, idbuf, 4); 
	id = ((uint32_t)idbuf[3]<<24)+((uint32_t)idbuf[2]<<16)+((uint32_t)idbuf[1]<<8)+idbuf[0]; 
	
	g_tGT911.ChipID = id;		/* ??id */
	return id;
}

/***************************** ????? www.armfly.com (END OF FILE) *********************************/
