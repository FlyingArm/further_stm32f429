#include <includes.h>
#include "bsp_adc.h"
#include <stdio.h>
#include <string.h>
#include "HT1080.h"
#include "bsp.h"
#include "bsp_uart_fifo.h"
#include "bsp_spi_flash.h"
#include "usart_sensor_cfg.h"	
#include "sensor_measure.h"

extern void  SENSORTaskPost ( void );
extern void  SENSORTaskPend ( void);

// ADC1ת���ĵ�ѹֵͨ��MDA��ʽ����SRAM
extern __IO uint16_t ADC_ConvertedValue;

#define FILTER_BUFF_LEN 5
#define	CO2_DEBUG		0
#define	PM2005_DEBUG	0
#define RS485_DEBUG   	0
#define SPI_FLASH_

static int adSampleValue[FILTER_BUFF_LEN];
static int h2o_adSampleValue[FILTER_BUFF_LEN];
static int pm2005_adSampleValue[FILTER_BUFF_LEN];
static int temp_adSampleValue[FILTER_BUFF_LEN];

SENSOR_DATA 		sensor_cali_data;
UNION_CALI_DATA 	union_cali_data;
USART_Buffer		gMyUsartBuffer;
MEASURE_DATE    	gMyData;

// �ֲ����������ڱ���ת�������ĵ�ѹֵ 	 
static float 	ADC_Vol; 
float 			pm2005_coef = 0.0f;
int 			iTemp_Diff = 0;


static void sensor_pwr_enable(void)
{
	PM2005_EN();
	CO2_EN();
	GPIO_SetBits(PORT_CO2_EN, PIN_CO2_EN);
	GPIO_SetBits(PORT_PM2005_EN, PIN_PM2005_EN);
}

static void sensor_en_config(void)
{

  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOs clock*/
  RCC_AHB1PeriphClockCmd(RCC_PM2005_EN | RCC_CO2_EN , ENABLE);

  GPIO_InitStructure.GPIO_Pin = PIN_CO2_EN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(PORT_CO2_EN, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = PIN_PM2005_EN;
  GPIO_Init(PORT_PM2005_EN, &GPIO_InitStructure);
}


static void CO2_sensor_data_rcv(void)
{
	static long sRxCnt4 = 0;
	uint8_t Receive_byte = 0;//,tempCnt;
	static char sRxCntFlag = 0;	

	//printf("\r\n **********  sensor testing usart2 *********** \r\n");
	
	while(comGetChar(COM5,&Receive_byte))
	{
		#if CO2_DEBUG
		printf("\r\n **********  sensor testing usart2 %02x *********** \r\n",Receive_byte);
		#endif
		
		if(Receive_byte == 0x16)
		{
         	sRxCntFlag = 1;
		}
		if(sRxCntFlag)
		{
			sRxCnt4++;
		}
		if(sRxCnt4 >= 4 && sRxCnt4 <= 5)
		{	
			gMyUsartBuffer.CO2[sRxCnt4 - 4] = Receive_byte;	
		}
		if(sRxCnt4 == 0x08)
		{
			sRxCntFlag = 0;
			sRxCnt4 = 0;
		}
	}
}
 
static unsigned char pm25_rx[8] = { 0 };
static unsigned char pm25_rx_len = 0;

static void PM2005_sensor_data_rcv(void)
{
	//static long sRxCnt4 = 0;
	unsigned char Receive_byte = 0;// tempCnt;
	static char sRxCntFlag = 0;	
	
	while(comGetChar(COM4,&Receive_byte))
    {
		#if PM2005_DEBUG
		printf("\r\n **********  sensor testing usart4 %02x *********** \r\n",Receive_byte);
		#endif

		#if 0
		if(Receive_byte == 0x16)
		{
         	sRxCntFlag = 1;
		}
		if(sRxCntFlag)
		{
			sRxCnt4++;
		}
		if(sRxCnt4 >= 6 && sRxCnt4 <= 7)
		{	
			gMyUsartBuffer.PM2005[sRxCnt4 - 6] = Receive_byte;	
		}
		if(sRxCnt4 == 0x08)
		{
			sRxCntFlag = 0;
			sRxCnt4 = 0;
		}

		#else

		if(Receive_byte == 0x16)
		{
         	sRxCntFlag = 1;
			pm25_rx_len = 0;
		}
		
		if(sRxCntFlag)
		{
			pm25_rx[pm25_rx_len++] = Receive_byte;
			
			while(pm25_rx_len < 8)
			{
				comGetChar(COM4,&Receive_byte);
				pm25_rx[pm25_rx_len++] = Receive_byte;
			}

			if(pm25_rx[2] == 0x0B)
			{
				//printf("\r\n%02x %02x %02x %02x %02x %02x %02x %02x\r\n",pm25_rx[0],pm25_rx[1],pm25_rx[2],pm25_rx[3],pm25_rx[4],pm25_rx[5],pm25_rx[6],pm25_rx[7]);

				gMyUsartBuffer.PM2005[0] = pm25_rx[5];	
				gMyUsartBuffer.PM2005[1] = pm25_rx[6];
			}

		}

		sRxCntFlag = 0;
		
		#endif
	}	

}

/*
*********************************************************************************************************
*	�� �� ��: 
*	����˵��: ��ѯ���
*	��    ��: ��
*	�� �� ֵ: ��
*		CMD:11 02 1F IP CS
*		ACK:16 0C 1F 02 FF FF FF FF FF FF FF FF FF FF C7

*********************************************************************************************************
*/
static void send_cmd_1FH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[32] = { 0 };
	
	cmd_buffer[num++] 	= 0x16;
	cmd_buffer[num++] 	= 0x0C;
	cmd_buffer[num++]	= 0x1F;
	cmd_buffer[num++]	= sensor_cali_data.ip;
	cmd_buffer[num++]	= sensor_cali_data.num;
	
	for(i = 0; i < 9;i++)	
	{
		 cmd_buffer[num++] = 0x00;
	}
	
	for(i = 0; i < 14; i++)
	{
		cmd_buffer[14] += cmd_buffer[i];
	}
	
	cmd_buffer[14]=(uint8_t)(-cmd_buffer[14]);
	
	comSendBuf(COM1,cmd_buffer,15);
}
/*
*********************************************************************************************************
*	�� �� ��: 
*	����˵��: ��ȡ��������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void send_cmd_0BH(void)
{

	uint16_t num = 0;
	uint16_t i = 0;
	unsigned char cmd_buffer[32] = { 0 };
	/*
		��������:
			11 ��ʼ��
			len ����
			0b ָ��
			id �ӻ���ַ
		 	CRC  =256-(HEAD+LEN+CMD+IP+DATA)
			����:11 02 0b IP CS
	    		  0  1  2  3   4   5   6   7   8   9   10  11  12  13   14   15   16   17   18  19  20
			�ӻ�:16 12 0B IP DF1 DF2 DF3 DF4 DF5 DF6 DF7 DF8 DF9 DF10 DF11 DF12 DF13 DF14 DF15 DF16 CS
		  	          		 PM2.5   �¶�    ʪ��     CO2     VOC      ��ȩ    WIFI ����    ����

	*/
	cmd_buffer[num++] = 0x16;
	cmd_buffer[num++] = 0x12;
	cmd_buffer[num++] = 0x0B;
	cmd_buffer[num++] = sensor_cali_data.ip;
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.pm2005_cali_data >> 8);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.pm2005_cali_data);
	cmd_buffer[num++] = (uint8_t)((sensor_cali_data.temp_cail_data + 500) >> 8);
	cmd_buffer[num++] = (uint8_t)((sensor_cali_data.temp_cail_data + 500));
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.h20_cali_data * 10 >> 8);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.h20_cali_data * 10);
	cmd_buffer[num++] = (uint8_t)(gMyData.iCO2 >> 8);
	cmd_buffer[num++] = (uint8_t)(gMyData.iCO2);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.voc_cali_data >> 8);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.voc_cali_data);
	cmd_buffer[num++] = 0;
	cmd_buffer[num++] = 0;
	cmd_buffer[num++] = 0;
	cmd_buffer[num++] = 1;
	cmd_buffer[num++] = 0;
	cmd_buffer[num++] = 0;
	
	for(i=0;i<20;i++)
	{
		cmd_buffer[20] += cmd_buffer[i];
	}
	
	cmd_buffer[20] = (uint8_t)(-cmd_buffer[20]);
	comSendBuf(COM1,cmd_buffer,21);
}

/*
*********************************************************************************************************
*	�� �� ��: 
*	����˵��: PM2.5�궨
*	��    ��: ��
*	�� �� ֵ: ��
����:11 05 0E IP 10 PM_25[0] PM_25[1] CS
Ӧ��:16 05 0E IP 10 PM_25[0] PM_25[1] CS
���ܣ����±궨

*********************************************************************************************************
*/
static void send_cmd_0EH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[32] = { 0 };
	
	cmd_buffer[num++]	= 0x16;
	cmd_buffer[num++]	= 0x05;
	cmd_buffer[num++]	= 0x0E;
	cmd_buffer[num++]	= sensor_cali_data.ip;
	cmd_buffer[num++]	= 0x10;
	cmd_buffer[num++]	= sensor_cali_data.pm2005_cali_data >> 8;
	cmd_buffer[num++]	= sensor_cali_data.pm2005_cali_data;
	
	for(i = 0; i < 7; i++)
	{
		cmd_buffer[7] += cmd_buffer[i];
	}
	
	cmd_buffer[7]=(uint8_t)(-cmd_buffer[7]);
	
	comSendBuf(COM1,cmd_buffer,8);
}

/*
*********************************************************************************************************
*	�� �� ��: 
*	����˵��: У׼�ӻ�CO2ֵ
*	��    ��: ��
*	�� �� ֵ: ��
����:11 05 0F IP 10 CM[0] CM_25[1] CS
Ӧ��:16 05 0F IP 10 CM[0] CM_25[1] CS
���ܣ�


*********************************************************************************************************
*/
static void send_cmd_0FH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[32] = { 0 };
	
	cmd_buffer[num++]	= 0x16;
	cmd_buffer[num++]	= 0x05;
	cmd_buffer[num++]	= 0x0F;
	cmd_buffer[num++]	= sensor_cali_data.ip;
	cmd_buffer[num++]	= 0x10;
	cmd_buffer[num++]	= gMyData.iCO2 >> 8;
	cmd_buffer[num++]	= gMyData.iCO2;
	
	for(i = 0; i < 7; i++)
	{
		cmd_buffer[7] += cmd_buffer[i];
	}
	
	cmd_buffer[7]=(uint8_t)(-cmd_buffer[7]);
	
	comSendBuf(COM1,cmd_buffer,8);
}

static void send_cmd_0DH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[32] = { 0 };
	
	cmd_buffer[num++]	= 0x16;
	cmd_buffer[num++]	= 0x04;
	cmd_buffer[num++]	= 0x0D;
	cmd_buffer[num++]	= sensor_cali_data.ip;
	cmd_buffer[num++]	= 0x10;
	cmd_buffer[num++]	= 01;
	
	for(i = 0; i < 6; i++)
	{
		cmd_buffer[6] += cmd_buffer[i];
	}
	
	cmd_buffer[6]=(uint8_t)(-cmd_buffer[6]);
	
	comSendBuf(COM1,cmd_buffer,7);
}



#if 0
static void calibration_command_rcv(void)
{
	DATA_CAL_TYPE command;
	static long sRxCnt4 = 0;
	unsigned char Receive_byte = 0;
	static char sRxCntFlag = 0;	
	unsigned int command_rx_len = 0;
	unsigned char command_rx[8];
	unsigned char sensor_data_for_cali[2];
	unsigned char sensor_data_for_cali_read[2];
	unsigned int sensor_data = 0;
	int sensor_temp_data = 0;

	while(comGetChar(COM1,&Receive_byte))
    {
			printf("\r\n **********  sensor testing usart1 %02x *********** \r\n",Receive_byte);

			if(Receive_byte == 0x11)
			{
	         	sRxCntFlag = 1;
			}
			if(sRxCntFlag)
			{
				sRxCnt4++;
			}
			if(sRxCnt4 > 2 && sRxCnt4 <= 6)
			{
				command_rx[command_rx_len++] = Receive_byte;
				if(command_rx[0] == 0x0F) command = CO2;
				else if(command_rx[0] == 0x0E) command = PM2005;
				else if(command_rx[0] == 0x0D) command = TEMP;
			}
			
			if(sRxCnt4 == 0x08)
			{
				sRxCntFlag = 0;
				sRxCnt4 = 0;
			}
	}
		
	switch(command)
	{
		case VOC:
			break;
		case CO2:
			sRxCntFlag = 0;
			sRxCnt4 = 0;
			printf("\r\n **********  sensor CO2 *********** \r\n");
			sensor_data_for_cali[0] = command_rx[2];
			sensor_data_for_cali[1] = command_rx[3];
			sensor_data = sensor_data_for_cali[0] << 8 | sensor_data_for_cali[1];
			printf("\r\nthe writing data:%02x%02x\r\n", sensor_data_for_cali[0],sensor_data_for_cali[1]);
			SendtoCO2(0x02,sensor_data);
			break;
		case H2O:
			break;	
		case PM2005:
			sRxCntFlag = 0;
			sRxCnt4 = 0;
			printf("\r\n **********  sensor PM2005 *********** \r\n");
			sensor_data_for_cali[0] = command_rx[2];
			sensor_data_for_cali[1] = command_rx[3];

			if(gMyData.iPM2005 != 0)
			{
				sensor_data = sensor_data_for_cali[0]<< 8 | sensor_data_for_cali[1];
				union_cali_data.cali_f = (float)sensor_data / gMyData.iPM2005;
				pm2005_coef = union_cali_data.cali_f;
				printf("\r\n pm2005_coef = %f \r\n", pm2005_coef);
			}
			else
			{
				printf("\r\n gMyData.iPM2005 is error(0)!!\r\n");
			}
			#ifdef SPI_FLASH_
			//printf("\r\nд�������Ϊ��%02x%02x\r\n", sensor_data_for_cali[0],sensor_data_for_cali[1]);
			SPI_FLASH_SectorErase(PM2005_CAL_SAVE_ADDR_0);/* ������Ҫд��� SPI FLASH ������FLASHд��ǰҪ�Ȳ��� */	 	
			SPI_FLASH_BufferWrite(union_cali_data.cali_ch, PM2005_CAL_SAVE_ADDR_0, 4);/* �����ͻ�����������д��flash�� */
			#endif
			break;
		case TEMP:
			sRxCntFlag = 0;
			sRxCnt4 = 0;
			printf("\r\n **********  sensor TEMP *********** \r\n");
			sensor_data_for_cali[0] = command_rx[2];
			sensor_data_for_cali[1] = command_rx[3];
			sensor_temp_data = sensor_data_for_cali[0] << 8 | sensor_data_for_cali[1];
			sensor_temp_data -= gMyData.iTemperature;
			iTemp_Diff = sensor_temp_data;
			printf("\r\n iTemp_Diff = %d \r\n", iTemp_Diff);
		  	#ifdef SPI_FLASH_
			//printf("\r\nд�������Ϊ��%02x%02x\r\n", sensor_data_for_cali[0],sensor_data_for_cali[1]);
			SPI_FLASH_SectorErase(TEMP_CAL_SAVE_ADDR_0);	 	
			SPI_FLASH_BufferWrite((u8 *)&iTemp_Diff, TEMP_CAL_SAVE_ADDR_0, 4);
		  	#endif
			break;	
		default:
			break;
	}
}
#else
static unsigned char command_rx_len = 0;
static unsigned char command_rx[8] = { 0 };

static void calibration_command_rcv(void)
{
	char command = -1;
	static unsigned char sRxCnt4 = 5;
	unsigned char Receive_byte = 0;
	static char sRxCntFlag = 0;	
	
	unsigned char command_rx[8];
	unsigned char sensor_data_for_cali[2];
	//unsigned char sensor_data_for_cali_read[2];
	unsigned int sensor_data = 0;
	int sensor_temp_data = 0;

	while(comGetChar(COM1,&Receive_byte))
    {
		//printf("\r\n ********** COM1 Receive %02x *********** \r\n",Receive_byte);

		if(Receive_byte == 0x11)
		{
	        sRxCntFlag = 1;
			command_rx_len = 0;
		}
			
		if(sRxCntFlag)
		{
			command_rx[command_rx_len++] = Receive_byte;
			
			while(command_rx_len < 3)
			{
				comGetChar(COM1,&Receive_byte);
				command_rx[command_rx_len++] = Receive_byte;
			}

			if(command_rx[2] == 0x0F)  
			{
				while(command_rx_len < 8)
				{
					comGetChar(COM1,&Receive_byte);
					command_rx[command_rx_len++] = Receive_byte;
				}

				if(sensor_cali_data.ip == command_rx[3])
				{
					command = CO2;
					break;
				}
			}
			else if(command_rx[2] == 0x0E) 
			{
				while(command_rx_len < 8)
				{
					comGetChar(COM1,&Receive_byte);
					command_rx[command_rx_len++] = Receive_byte;
				}

				if(sensor_cali_data.ip == command_rx[3])
				{
					command = PM2005;
					break;
				}
			}
			else if(command_rx[2] == 0x0D) 
			{
				while(command_rx_len < 8)
				{
					comGetChar(COM1,&Receive_byte);
					command_rx[command_rx_len++] = Receive_byte;
				}

				if(sensor_cali_data.ip == command_rx[3])
				{
					command = TEMP;
					break;
				}
			}
			else if(command_rx[2] == 0xAB) 
			{
				while(command_rx_len < 8)
				{
					comGetChar(COM1,&Receive_byte);
					command_rx[command_rx_len++] = Receive_byte;
				}

				if(command_rx[3] == 0xCD)
				{
					command = IP_NUM_SET;
					break;
				}
			}
			else if(command_rx[2] == 0x0B) 
			{
				while(command_rx_len < 5)
				{
					comGetChar(COM1,&Receive_byte);
					command_rx[command_rx_len++] = Receive_byte;
				}

				if(sensor_cali_data.ip == command_rx[3])
				{
					command = SEN_VALUE;
					break;
				}
			}
			else if(command_rx[2] == 0x1F) 
			{

				while(command_rx_len < 5)
				{
					comGetChar(COM1,&Receive_byte);
					command_rx[command_rx_len++] = Receive_byte;
				}

				if(sensor_cali_data.ip == command_rx[3])
				{
					command = NUM_ACK;
					break;
				}
			}
		}

		sRxCntFlag = 0;
	}
		
	switch(command)
	{
		case IP_NUM_SET:

			sensor_cali_data.ip  = command_rx[4];
			sensor_cali_data.num = command_rx[5];
			
			sf_WriteBuffer((u8 *)&sensor_cali_data.ip , IP_SET_ADDR, 1);
			sf_WriteBuffer((u8 *)&sensor_cali_data.num,NUM_SET_ADDR, 1);
			
			printf("Device IP:%02x Device NUM:%02x\r\n", sensor_cali_data.ip,sensor_cali_data.num);
			
			break;
	
		case SEN_VALUE:

			if(sensor_cali_data.ip == command_rx[3])
				send_cmd_0BH();
			
			break;
		
		case NUM_ACK:
			
			if(sensor_cali_data.ip == command_rx[3])
				send_cmd_1FH();
			
			break;	
			
		case VOC:
			break;
			
		case CO2:
			if(sensor_cali_data.ip == command_rx[3])
			{
				sensor_data_for_cali[0] = command_rx[5];
				sensor_data_for_cali[1] = command_rx[6];
				sensor_data = sensor_data_for_cali[0] << 8 | sensor_data_for_cali[1];
				SendtoCO2(0x02,sensor_data);
				
				//printf("CO2 calibration data:%02x%02x\r\n",sensor_data_for_cali[0],sensor_data_for_cali[1]);

				send_cmd_0FH();
			}
			
			break;
			
		case H2O:
			break;
			
		case PM2005:
			if(sensor_cali_data.ip == command_rx[3])
			{
				sensor_data_for_cali[0] = command_rx[5];
				sensor_data_for_cali[1] = command_rx[6];

				if(gMyData.iPM2005 != 0)
				{
					sensor_data = sensor_data_for_cali[0]<< 8 | sensor_data_for_cali[1];

					//printf("PM2005 calibration data:%02x%02x\r\n",sensor_data_for_cali[0],sensor_data_for_cali[1]);
					
					union_cali_data.cali_f = (float)sensor_data / gMyData.iPM2005;
					pm2005_coef = union_cali_data.cali_f;
					sf_WriteBuffer(union_cali_data.cali_ch,PM25_CAL_ADDR,4);

					//printf("\r\n pm2005_coef = %f \r\n", pm2005_coef);
				}
				else
				{
					printf("\r\n gMyData.iPM2005 is error!\r\n");
				}

				send_cmd_0EH();
			}
		
			break;
			
		case TEMP:
			if(sensor_cali_data.ip == command_rx[3])
			{
				sensor_data_for_cali[0] = command_rx[5];
				sensor_data_for_cali[1] = command_rx[6];
				sensor_temp_data = sensor_data_for_cali[0] << 8 | sensor_data_for_cali[1];

				//printf("TEMP calibration data:%02x%02x\r\n",sensor_data_for_cali[0],sensor_data_for_cali[1]);
				
				sensor_temp_data -= gMyData.iTemperature;
				iTemp_Diff = sensor_temp_data;
				
				//printf("\r\n iTemp_Diff = %d \r\n", iTemp_Diff);
				
				sf_WriteBuffer((u8 *)&iTemp_Diff,TEMP_CAL_ADDR,4);

				send_cmd_0DH();
			}

			break;
			
		default:
			break;
	}
}

#endif


/**
  * @author ��Add by Tang Yousheng at 2016.11.20 
  * @brief  ��CO2��������
							���ͣ�11 01 01 ED	
							Ӧ��16 05 01 DF1- DF4 [CS]
							���ܣ���ȡCO2�����������λ��ppm��
							˵����CO2����ֵ = DF1*256 + DF2
							��ȡģ������汾�� 
							���ͣ�11 01 1E D0					
							Ӧ��16 0C 1E DF1-DF11 CS  
							˵����
							1��DF1-DF11 ��ʾ��ϸ�汾�ŵ�ASCII�롣
							���磺��ģ��汾��ΪCM V0.0.16ʱ��Ӧ���������£�
							Ӧ��16 0C 1E 43 4D 20 56 30 2E 30 2E 31 36 00 97   
							ASCII���ӦΪCM V0.0.16
  * @param  ���ڴ������н����յ�����Ϣ��
							�͵�Uart3RxProcess( unsigned long)��ȡ��Ϣ
  * @note   ��Called by Main function a  by PutChar3()
  * @retval ��No retval
  */
static void SendtoCO2(unsigned char CMD,uint16_t co2data)
{
	uint8_t coreg_value[2];
	int8_t crc;
	if(CMD==0x01)
	{
		comSendChar(COM5,0x11);
		comSendChar(COM5,0x01);
		comSendChar(COM5,0x01);
		comSendChar(COM5,0xed);
	}	
	if(CMD==0x02)
	{
		coreg_value[0] = co2data >> 8;
	  	coreg_value[1] = co2data;
	  	crc = -(0x11+0x03+0x03+coreg_value[0]+coreg_value[1]);

		comSendChar(COM5,0x11);
		comSendChar(COM5,0x03);
		comSendChar(COM5,0x03);
		comSendChar(COM5,coreg_value[0]);
		comSendChar(COM5,coreg_value[1]);
		comSendChar(COM5,crc);
	}
}
/**
  * @author ��Add by Tang Yousheng at 2016.11.20 
  * @brief  ��PB14  PM2005_RXD					
							PB15  PM2005_TXD
  * @param  �������CMD��	����/�رշ۳�����						   	0x0C   
													��ȡ�۳��������								0x0B
													�������ȡ�۳�����ʱ��					0x0D
													�������ȡ��ʱ��������ģʽ			0x05
													�������ȡ��̬����ģʽ					0x06
													�������ȡ�۳�У׼ϵ��					0x07
													�رշ۳�����						       	0x1C  
									 ע�⣺ �رշ۳���������رռ���ܣ�
								         	������ΪVivao��Ŀ������������汾��ͨ�ã�
									        ��Ϊ����ܵ�����������
												//E:\tang\Aldes\Zhang\������ͨѶЭ��
    ��ѯʵʱŨ�ȣ�11 01 FD F1
  * @note   ��ָ���Ӧ����PB14���ŵ��ⲿ�½����ж��¼����
  * @retval ��No retval
  */
static void SendtoPM2005(unsigned char CMD)
{
	if(CMD==0x0B)
	{
		comSendChar(COM4,0x11);
		comSendChar(COM4,0x02);
		comSendChar(COM4,0x0B);
		comSendChar(COM4,0x01);
		comSendChar(COM4,0xE1);
	}
	else if(CMD==0x0C) 
	{
		comSendChar(COM4,0x11);
		comSendChar(COM4,0x03);
		comSendChar(COM4,0x0C);
		comSendChar(COM4,0x02);
		comSendChar(COM4,0x1E);	
		comSendChar(COM4,0xC0);	
	}
	else if(CMD==0x1C) //Vivao PM2005 ���й���
	{
		comSendChar(COM4,0x11);
		comSendChar(COM4,0x03);
		comSendChar(COM4,0x0C);
		comSendChar(COM4,0x01);
		comSendChar(COM4,0x1E);	
		comSendChar(COM4,0xC1);	
	}
	else if(CMD==0x0D)
	{
		comSendChar(COM4,0x00);
		comSendChar(COM4,0x11);
		comSendChar(COM4,0x03);
		comSendChar(COM4,0x0d);
		comSendChar(COM4,0xff);
		comSendChar(COM4,0xff);	
		comSendChar(COM4,0xe1);	
	}
}

u8 RS485_RX_BUF[8]; 	//���ջ���,���64���ֽ�.
u8 RS485_RX_CNT=0;		//���յ������ݳ��� 
char CO2CntFlag = 0;

static void delay_ms(unsigned int n_ms)
{
	OS_ERR      os_err;
	OSTimeDlyHMSM(0, 0, 0, n_ms,OS_OPT_TIME_DLY,&os_err);	
}

static void MeasureData(void)
{
	long temp,hum;
	short cosvalue = 0;
	
	
  	gMyData.iCO2 = gMyUsartBuffer.CO2[0]*256 + gMyUsartBuffer.CO2[1];
	gMyData.iPM2005 = gMyUsartBuffer.PM2005[0]*256 + gMyUsartBuffer.PM2005[1];
	
	HDC_TrigMeasure();
	
	delay_ms(200);
	
	HDC_ReadT_RH(&temp,&hum);
	
	gMyData.iTemperature = temp;
	gMyData.iHumidity = hum;

	#if 0
	if(CO2CntFlag)
	{
		cosvalue = RS485_RX_BUF[3]*256+RS485_RX_BUF[4];
		SendtoCO2(0x02,cosvalue);
		CO2CntFlag = 0;
	}
	#endif
}

static void calibration_sample_data(DATA_CAL_TYPE sensor_type,int sample_val)
{ 
	static int adSampleIndex = 0;
	static int h2oSampleIndex = 0;
	static int pm2005SampleIndex = 0;
	static int tempSampleIndex = 0;

	switch(sensor_type)
	{
		case VOC:
			//adSampleValue[adSampleIndex++] = sample_val;
			//if(adSampleIndex >= 10)
			{
				//int ad_filter_value = 0;
				
				//adSampleIndex = 0;
				//ad_filter_value = GetMedianNum(adSampleValue,FILTER_BUFF_LEN);
				
				GetVocValue(&sensor_cali_data.voc_cali_data,sample_val);
				#if RS485_DEBUG
				printf("\r\n sensor_cali_data.voc_cali_data = %d\r\n",sensor_cali_data.voc_cali_data);
				#endif
			}
			break;
		case H2O:
			h2o_adSampleValue[h2oSampleIndex++] = sample_val;
			if(h2oSampleIndex >= 5)
			{
				int ad_filter_value = 0;
				
				h2oSampleIndex = 0;
				ad_filter_value = GetMedianNum(h2o_adSampleValue,FILTER_BUFF_LEN);
				sensor_cali_data.h20_cali_data = (int)(ad_filter_value / 100.0f);
				#if RS485_DEBUG
				printf("\r\n sensor_cali_data.h20_cali_data = %d\r\n",sensor_cali_data.h20_cali_data);
				#endif
			}
			break;
		case PM2005:
			//pm2005_adSampleValue[pm2005SampleIndex++] = sample_val;
			
			//if(pm2005SampleIndex >= 10)
			//{
			//	int ad_filter_value = 0;
				
			//	pm2005SampleIndex = 0;
			//	ad_filter_value = GetMedianNum(pm2005_adSampleValue,FILTER_BUFF_LEN);
				sensor_cali_data.pm2005_cali_data = (int)(pm2005_coef * sample_val/* ad_filter_value */);
				#if RS485_DEBUG
				printf("\r\n sensor_cali_data.pm2005_cali_data = %d\r\n",sensor_cali_data.pm2005_cali_data);
				#endif
			//}
			break;
		case TEMP:
			temp_adSampleValue[tempSampleIndex++] = sample_val;
			if(tempSampleIndex >= 5)
			{
				int ad_filter_value = 0;
				
				tempSampleIndex = 0;
				ad_filter_value = GetMedianNum(temp_adSampleValue,FILTER_BUFF_LEN);
				sensor_cali_data.temp_cail_data = ad_filter_value + iTemp_Diff;
				#if RS485_DEBUG
				printf("\r\n sensor_cali_data.temp_cail_data = %d\r\n",sensor_cali_data.temp_cail_data);
				#endif
			}
			break;	
		default:
			break;
	}
					
}

/* ��x��0֮��ľ���ֵС��0.00001������1e-5��ʱ ��Ϊx����y  */   
static int compare_float_num(float x, float y)
{  
	static const double delta = 1e-5;
  
    if((x - y) > delta) 
		return 1;  
    else if((x - y) < -delta) 
		return -1;  
    else 
		return 0;  
} 


void sensor_measure(void)
{ 
	OS_ERR      os_err;
		
	sensor_en_config();
	sensor_pwr_enable();
	HDC_Init(); 		 // HDC1080   
	Rheostat_Init();	 // VOC
	
	sf_ReadBuffer((u8 *)union_cali_data.cali_ch, PM25_CAL_ADDR, 4);
	pm2005_coef = union_cali_data.cali_f;

	if(compare_float_num(0.0f,pm2005_coef) > 0 || compare_float_num(pm2005_coef,10.0f) > 0) 
		pm2005_coef = 1.0f;

	printf("\r\nPM 2.5��%f\r\n", pm2005_coef);

	sf_ReadBuffer((u8 *)&iTemp_Diff, TEMP_CAL_ADDR, 4);

	printf("Temperature��%02x\r\n", iTemp_Diff);

	sf_ReadBuffer((u8 *)&sensor_cali_data.num, NUM_SET_ADDR, 1);
	sf_ReadBuffer((u8 *)&sensor_cali_data.ip , IP_SET_ADDR , 1);
 
	printf("IP ��%d NUM : %d\r\n", sensor_cali_data.ip,sensor_cali_data.num);

	while(1)
	{
		/* У׼������ܴ��� */
		calibration_command_rcv();

		SENSORTaskPend();
		
		SendtoPM2005(0x0B);
		OSTimeDlyHMSM(0, 0, 0, 3,OS_OPT_TIME_DLY,&os_err);
		PM2005_sensor_data_rcv();
		
    	SendtoCO2(0x01,0);
    	OSTimeDlyHMSM(0, 0, 0, 20,OS_OPT_TIME_DLY,&os_err);
		CO2_sensor_data_rcv();

		MeasureData();

		#if RS485_DEBUG
		printf("\r\n%d,%d,%d,%d\r\n",gMyData.iCO2,gMyData.iPM2005,gMyData.iTemperature,gMyData.iHumidity);
		#endif

		ADC_Vol =(float) ADC_ConvertedValue / 4096 * (float)3.3; 	// ��ȡת����ADֵ
		
		//printf("\r\n The current AD value = %f V \r\n",ADC_Vol); 

		calibration_sample_data(VOC,ADC_ConvertedValue); 			// У׼ VOC
		calibration_sample_data(H2O,gMyData.iHumidity);
		calibration_sample_data(PM2005,gMyData.iPM2005);
		calibration_sample_data(TEMP,gMyData.iTemperature);
		
		SENSORTaskPost();
		
		//printf("\r\n sensor_cali_data.temp_cail_data = %d\r\n",sensor_cali_data.temp_cail_data); 

		OSTimeDlyHMSM(0, 0, 0, 100,OS_OPT_TIME_DLY,&os_err);   		// ����Ƶ��100ms
	}
}


