#include "HT1080.h"
#include <includes.h>

#include "string.h"
/**
  * @brief   ：This Function is Reset HDC1080 by software
  * @param   ：No parameters
  * @retval  ：No retva
  * @note    ：Called by HDC_Init or in Main function
  *@author   ：Add byTang Yousheng at 2016.11.20 
  */
static unsigned char HDC_Soft_Reset(void)
{
    uint8_t temp[4];
	unsigned char error=0;           //error variable 

    temp[0] = HDC1080_Read_Config;  //0x02 配置寄存器
    temp[1] = 0x80; 							  //配置温湿度测量精度为14位，配置温湿度同时测量
    temp[2] = 0x00;
	 
	I2CSendByte_S(HDC1080_Device_Adderss, &temp[0], 3);     // Device_Address On the Bus = 0x40
	memset(temp,0,4);
	I2CReceiveByte_S(HDC1080_Device_Adderss ,&temp[0], 3);     // Device_Address On the Bus = 0x40
	return error;
}
/**
  * @brief   ：This Function is Reset HDC1080 by software
  * @param   ：No parameters
  * @retval  ：No retva
  * @note    ：setting temperature and humidity presice 12bit。
							 when called ，you should firstly send the device address with the R/W_ bit
  *@author   ：Add byTang Yousheng at 2016.11.20 
  */
static void HDC_Setting(void)
{
    uint16_t tempcom = 0;
    uint8_t temp[2];
    tempcom = HDC1080_CONFIG ;//连续读取数据使能
    // 温度与温度都为14bit
    temp[0] = (uint8_t)(tempcom >> 8); 
    temp[1] = (uint8_t)tempcom;
    I2CSendByte_S(0x02, temp, 2);
}
/**
  * @brief   ：This Function is Initialize HDC1080 ,setting temperature and humidity presice 14bit。
  * @param   ：No parameters
  * @retval  ：No retva
  * @note    ：called by Main function
  * @author  ：Add byTang Yousheng at 2016.11.20 
  */

void HDC_Init(void)
{
	//此处初始化
		GPIO_InitTypeDef   GPIO_InitStructure;
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC , ENABLE);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //上拉GPIO_PuPd_UP
		GPIO_Init(GPIOA,&GPIO_InitStructure); 

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //上拉GPIO_PuPd_UP
		GPIO_Init(GPIOC,&GPIO_InitStructure); 
		
  		HDC_Soft_Reset();
  		HDC_Setting();
}

/**
  * @brief   ：开启转换
  * @param   ：No parameters
  * @retval  ：No retva
  * @note    ：called by Main function
  * @author  ：Add byTang Yousheng at 2016.11.20 
  */
void HDC_TrigMeasure(void)
{
		unsigned char SetT_SendByte[1] = {0x00};
		I2CSendByte_S(0x40, SetT_SendByte,1);//开启转换	
}
/**
  * @brief   ：获取测量数据
  * @param   ：pTBuffer ：pointer to temperature Data
							 pTHBuffer：pointer to Humidity    Data
  * @retval  ：No retva
  * @note    ：called by Main function
  * @author  ：Add byTang Yousheng at 2016.11.20 
  */
void HDC_ReadT_RH(long *pTBuffer,long *pRHBuffer)
{
	unsigned char RHT_ReadByte[4]={0};
	
	I2CReceiveByte_S(0x40, RHT_ReadByte,4);//读取温湿度测量结果	<<16 
	*pTBuffer  = ((((RHT_ReadByte[0]<<8) + RHT_ReadByte[1])*165 * 100) >>16) - 4000;               //  乘以 100
	*pRHBuffer = 	(((RHT_ReadByte[2]<<8) + RHT_ReadByte[3])*100 * 100) >>16;    									//  乘以 100
}

int Humidity_Calib(unsigned int input)
{
	unsigned long output;
		int CorrctT,CorrctRH;
	int  Delta_T,Delta_RH,Delta_RH_Charg;
	int  CorrctT_Charg,CorrctRH_Charg;
	float Tempfloat;
	int CorrctT_Time,CorrctT_Time_Charg;
		unsigned char Tstatus1[2];
	#define  DELTA_TEMP_CHAG		-12
	#define  DELTA_TIME_CHAG		(60*25)
//	if(input >= 9999)
//		output =  99;
//	else 
//		output =  input/100;
int ResultTemp_PH,ResultRH_PH,ResultRH_JZ;
	ResultTemp_PH = gMyData.iTemperature/10;
	ResultRH_PH = input/10;
	if(ResultTemp_PH>=400)  	
	//	Delta_T = 0.125*ResultTemp_PH - 97.5;  		//  = 0.125x - 97.5
		Delta_T = 0.12*ResultTemp_PH - 92;//0.12x - 92
	else 				        Delta_T = -44;
	if(Delta_T<-60)		Delta_T=-60;
	if(Delta_T>-30)		Delta_T=-30;
	
	Tempfloat = (float)ResultTemp_PH/10;
	Tempfloat = -0.00000005223 *Tempfloat*Tempfloat*Tempfloat  + 0.0000080135 *Tempfloat*Tempfloat - 0.0008659255 *Tempfloat + 0.08385;// 计算不同温度水汽含量差值。
	Delta_RH = Delta_T*Tempfloat*-ResultRH_PH/10;
//	Delta_RH_Charg = DELTA_TEMP_CHAG*Tempfloat*-ResultRH_PH/10;

	if(Delta_T>=0)    
	{
		Delta_T  = 0;
		Delta_RH = 0;
	}
//	CorrctT_Time++;
//	if(CorrctT_Time<900)							
//	{
//		CorrctT = (int)(Delta_T/2);
//		CorrctRH = (int)(Delta_RH/2);
//	}
//	else if(CorrctT_Time<1800)							
//	{
//		CorrctT = (int)(CorrctT_Time*Delta_T/1800);
//		CorrctRH = (int)(CorrctT_Time*Delta_RH/1800);
//	}
//	else
//	{
	//	CorrctT_Time = 1800;
	//	CorrctT = Delta_T;
		CorrctRH = Delta_RH;
	//}
	

	ResultRH_JZ  = ResultRH_PH   + CorrctRH ;		//温升处理 
	ResultRH_JZ = ResultRH_JZ*0.92; 

	if(ResultRH_JZ<0)		ResultRH_JZ=0;
	if(ResultRH_JZ>990)		ResultRH_JZ=990;	
	//output =  input;
	
	gMyData.iHumidity = ResultRH_JZ /10;
//	Tstatus1[0]= input/100;
//	Tstatus1[1]= gMyData.iHumidity;	
	//RS485_SendBuf(Tstatus1,2);
	gMyData.bChanged |= Flag_Humidity;
	return output;
}



