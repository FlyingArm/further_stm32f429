#ifndef __BSP_IIC_CGK_H
#define __BSP_IIC_CGK_H

#include "stdint.h" 
   	   		   
/*
 		PB6 --> I2C1_SCL
 		PB7 --> I2C1_SDA
*/
// IIC GPIO defines 
#define GPIO_PORT_CGK	GPIOB			/* GPIO?? */
#define RCC_CGK_PORT 	RCC_AHB1Periph_GPIOB		/* GPIO???? */
#define CGK_SCL_PIN		GPIO_Pin_6			/* ???SCL????GPIO */
#define CGK_SDA_PIN		GPIO_Pin_7			/* ???SDA????GPIO */

/* ????SCL?SDA?? */
#define CGK_SCL_1()  GPIO_PORT_CGK->BSRRL = CGK_SCL_PIN				/* SCL = 1 */
#define CGK_SCL_0()  GPIO_PORT_CGK->BSRRH = CGK_SCL_PIN				/* SCL = 0 */

#define CGK_SDA_1()  GPIO_PORT_CGK->BSRRL = CGK_SDA_PIN				/* SDA = 1 */
#define CGK_SDA_0()  GPIO_PORT_CGK->BSRRH = CGK_SDA_PIN				/* SDA = 0 */

#define CGK_SDA_READ()  ((GPIO_PORT_CGK->IDR & CGK_SDA_PIN) != 0)	/* ?SDA???? */
#define CGK_SCL_READ()  ((GPIO_PORT_CGK->IDR & CGK_SCL_PIN) != 0)	/* ?SCL???? */



// CGK CMD defgines 
#define CGKWRITE      0x22
#define CGKREAD       0x23

#define ELWAME  			0x01
#define UVUVIN  			0x02
#define HCHOME  			0x03
#define PMMEAS        0x05 
#define CO2MEA   			0x06
#define CO2CAL 				0x07
#define CO2PON 			  0x10
#define VOCCAL 			  0x11
#define CO2POF 				0x12
#define PMPOWF        0x15


//void i2c_Start(void);
//void i2c_Stop(void);
//void i2c_SendByte(uint8_t _ucByte);
//void i2c_Ack(void);
//void i2c_NAck(void);
//uint8_t i2c_ReadByte(void);
//uint8_t i2c_WaitAck(void);
//uint8_t i2c_CheckDevice(uint8_t _Address);

//uint8_t CGK_SendCmd(uint8_t _ucSlaAdd,uint8_t _ucSendDat);
//uint8_t I2CReceiveByte_S(uint8_t _ucSlaAdd,uint8_t *_usRcvDat, uint8_t _ucRcvNum);


// init the IIC gpio PB6 SCL PB9 SDA
void bsp_InitI2CCgk(void);

uint8_t CGK_SendCmd(uint8_t _ucSlaAdd,uint8_t _ucSendDat);
/**
  * @brief  AutoPM2005_Read , Measuring dust command function , read tht pm0.3 pm 2.5 pm10
  *         
  * @param  _ucSlaAdd is the device addr 
	          _uiPm03Dat is pm0.3 dat
						_uiPm25Dat is pm2.5 dat
						_uiPm10Dat is pm10  dat
  * @retval if return 0 is ok , no error the data is correct
					  if return 2 is error the device is no ack , Data is not available
						if return 1 the data checksum is wrong , Data is not available
  */
uint8_t AutoPM2005_Read(uint8_t _ucSlaAdd, uint32_t *_uiPm03Dat,uint32_t *_uiPm25Dat, uint32_t *_uiPm10Dat);
uint8_t AutoEleCo2_Read(uint8_t _ucSlaAdd, uint32_t *_uiElecCo2Dat);
#endif






