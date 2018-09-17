/*
*********************************************************************************************************
*
*	???? : CGK??????
*	???? : bsp_CGK_gpio.c
*	?    ? : V1.0
*	?    ? : ?gpio??CGK??, ???STM32F4??CPU?????????????,???CGK?????????
*
*	???? :
*		???  ??        ??     ??
*		V1.0    2013-02-01 armfly  ????
*
*	Copyright (C), 2013-2014, ????? www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp_i2c_cgk.h"
#include "includes.h"


/*
*********************************************************************************************************
*	? ? ?: CGK_Delay
*	????: CGK?????,??400KHz
*	?    ?:  ?
*	? ? ?: ?
*********************************************************************************************************
*/
static void CGK_Delay(void)
{
// uint8_t i;
// if you don't want to use systick timer , can use for 	
	//for (i = 0; i < 100; i++);
	bsp_DelayUS(5);
}


/**
  * @brief  CGK_Stop
  *        
  * @param  None
  * @retval None
  */
static void CGK_Stop(void)
{
	/* ?SCL????,SDA?????????CGK?????? */
	CGK_SDA_0();
	CGK_SCL_1();
	CGK_Delay();
	CGK_SDA_1();
	CGK_Delay();
}

/*
*********************************************************************************************************
*	? ? ?: bsp_InitCGK
*	????: ??CGK???GPIO,????IO?????
*	?    ?:  ?
*	? ? ?: ?
*********************************************************************************************************
*/
void bsp_InitI2CCgk(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_CGK_PORT, ENABLE);	/* ??GPIO?? */

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ????? */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		/* ?????? */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ???????? */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO????? */

	GPIO_InitStructure.GPIO_Pin = CGK_SCL_PIN | CGK_SDA_PIN;
	GPIO_Init(GPIO_PORT_CGK, &GPIO_InitStructure);

	/* ???????, ??CGK????????????? */
	CGK_Stop();
}


/**
  * @brief  CGK_Start
  *        
  * @param  None
  * @retval None
  */
static void CGK_Start(void)
{
	/* ?SCL????,SDA?????????CGK?????? */
	CGK_SDA_1();
	CGK_SCL_1();
	CGK_Delay();
	CGK_SDA_0();
	CGK_Delay();
	
	CGK_SCL_0();
	CGK_Delay();
}


/**
  * @brief  CGK_SendByte
  *        
  * @param  _ucByte is you want to send data
  * @retval None
  */
static void CGK_SendByte(uint8_t _ucByte)
{
	uint8_t i;

	/* ????????bit7 */
	for (i = 0; i < 8; i++)
	{
		if (_ucByte & 0x80)
		{
			CGK_SDA_1();
		}
		else
		{
			CGK_SDA_0();
		}
		CGK_Delay();
		CGK_SCL_1();
		CGK_Delay();
		CGK_SCL_0();
//		if (i == 7)
//		{
//			 CGK_SDA_1(); // ????
//		}
		_ucByte <<= 1;	/* ????bit */
		CGK_Delay();
	}
}

/**
  * @brief  CGK_ReadByte
  *        
  * @param  None 
  * @retval return you want to read data
  */
static uint8_t CGK_ReadByte(void)
{
	uint8_t i;
	uint8_t value;

	/* ???1?bit????bit7 */
	value = 0;
	for (i = 0; i < 8; i++)
	{
		value <<= 1;
		CGK_SCL_1();
		CGK_Delay();
		if (CGK_SDA_READ())
		{
			value++;
		}
		CGK_SCL_0();
		CGK_Delay();
	}
	return value;
}

/**
  * @brief  CGK_WaitAck , Waiting for reply Using the timeout mechanism
  *         
  * @param  None 
  * @retval None 
  */
static uint8_t CGK_WaitAck(void)
{
	uint8_t re;

	CGK_SDA_1();	/* CPU??SDA?? */
	CGK_Delay();
	CGK_SCL_1();	/* CPU??SCL = 1, ???????ACK?? */
	CGK_Delay();
	if (CGK_SDA_READ())	/* CPU??SDA???? */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	CGK_SCL_0();
	CGK_Delay();
	return re;
}

/**
  * @brief  CGK_Ack , send ACK
  *         
  * @param  None 
  * @retval None 
  */
static void CGK_Ack(void)
{
	CGK_SDA_0();	/* CPU??SDA = 0 */
	CGK_Delay();
	CGK_SCL_1();	/* CPU??1??? */
	CGK_Delay();
	CGK_SCL_0();
	CGK_Delay();
	CGK_SDA_1();	/* CPU??SDA?? */
}

/**
  * @brief  CGK_NAck , send NO ACK
  *         
  * @param  None 
  * @retval None 
  */
static void CGK_NAck(void)
{
	CGK_SDA_1();	/* CPU??SDA = 1 */
	CGK_Delay();
	CGK_SCL_1();	/* CPU??1??? */
	CGK_Delay();
	CGK_SCL_0();
	CGK_Delay();
}


/**
  * @brief  CGKSendByte_S , 
  *         
  * @param  SlaAdd is the device addr , SendDat is send cmd 
  * @retval None 
  */
uint8_t CGK_SendCmd(uint8_t _ucSlaAdd,uint8_t _ucSendDat)
{
	CGK_Start();
	CGK_SendByte(_ucSlaAdd);

	if( CGK_WaitAck() ) {
		return 2;
	}
//	if(CGK_WaitAck() == 0)
//	{
// 	   printf("CGKAck\n");
//	}
//	else
//		printf("CGKNoAck\n");

	CGK_SendByte(_ucSendDat);
	if( CGK_WaitAck() ) {
		return 2;
	}
//	if(CGK_WaitAck() == 0) {
//		 printf("CGKAck\n");
//	}
//	else{
//		printf("CGKNoAck\n");
//	}
		
	CGK_Stop();
	return 0;
}

/**
  * @brief  CGKReceiveByte_S 
  *         
  * @param  SlaAdd is the device addr , RcvDat is ready receive data , Rcv_Num is receive number of data 
  * @retval None 
  */
static uint8_t CGKReceiveByte_S(uint8_t _ucSlaAdd,uint8_t *_usRcvDat, uint8_t _ucRcvNum)
{
	uint8_t i;
	CGK_Start();
	CGK_SendByte(_ucSlaAdd);
	CGK_Delay();
	
  if( CGK_WaitAck() ) {
		return 2;
	}
//	if(CGK_WaitAck() == 0) {
//		printf("CGKAck\n");
//	} else{
//		printf("CGKNoAck\n");
//	}
//	
	//CGK_Delay();
	
	for (i = 0;i < _ucRcvNum; i++)
	{
		_usRcvDat[i] = CGK_ReadByte();	
    CGK_Ack();		
	}
	CGK_NAck();
	CGK_Stop();
	return 0;
}

uint8_t AutoEleCo2_Read(uint8_t _ucSlaAdd, uint32_t *_uiElecCo2Dat)
{
	uint8_t ucPmret = 0,iCount = 0, checkSum = 0;
	uint8_t tempDat[7] = {0};
	ucPmret = CGKReceiveByte_S(_ucSlaAdd,tempDat,7);
	if(ucPmret == 0x00) {
		// check the sum
		for(iCount =0 ; iCount < 7; iCount++ ){
			checkSum += tempDat[iCount];			
			//putchar(tempDat[iCount]);
		}		
		if(checkSum != 0x00){
			return 1;
		}
	}else if( ucPmret == 0x02 ) {
		return ucPmret;
	}

	// checksum is ok
	//*_uiPm03Dat = (uint32_t)( tempDat[0]*256*256*256 + tempDat[1]*256*256 + tempDat[2]*256 + tempDat[3] ) ;
	*_uiElecCo2Dat = (uint32_t)( (tempDat[0]<<8) | tempDat[1] ) ;
	return 0;
}


uint8_t AutoPM2005_Read(uint8_t _ucSlaAdd, uint32_t *_uiPm03Dat,uint32_t *_uiPm25Dat, uint32_t *_uiPm10Dat)
{
  uint8_t ucPmret = 0,iCount = 0, checkSum = 0;
	uint8_t tempDat[17] = {0};
	ucPmret = CGKReceiveByte_S(_ucSlaAdd,tempDat,17);
	if(ucPmret == 0x00) {
		// check the sum
		for(iCount =0 ; iCount < 17; iCount++ ){
			checkSum += tempDat[iCount];			
			//putchar(tempDat[iCount]);
		}		
		if(checkSum != 0x00){
			return 1;
		}
	}else if( ucPmret == 0x02 ) {
		return ucPmret;
	}

	// checksum is ok
	//*_uiPm03Dat = (uint32_t)( tempDat[0]*256*256*256 + tempDat[1]*256*256 + tempDat[2]*256 + tempDat[3] ) ;
	*_uiPm03Dat = (uint32_t)( (tempDat[0]<<24) | (tempDat[1]<<16) | (tempDat[2]<<8) | tempDat[3] ) ;
	*_uiPm25Dat = (uint32_t)( (tempDat[4]<<24) | (tempDat[5]<<16) | (tempDat[6]<<8) | tempDat[7] ) ;
	*_uiPm10Dat = (uint32_t)( (tempDat[8]<<24) | (tempDat[9]<<16) | (tempDat[10]<<8) | tempDat[11] ) ;
	
	return 0;
}


