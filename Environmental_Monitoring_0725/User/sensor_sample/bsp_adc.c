/**
  ******************************************************************************
  * @file    bsp_bsp_adc.c
  * @author  
  * @version V1.0
  * @date    
  * @brief   adc����
  ******************************************************************************
  */ 
#include "bsp_adc.h"

__IO uint16_t ADC_ConvertedValue;

static void Rheostat_ADC_GPIO_Config(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
	
	// ʹ�� GPIO ʱ��
	RCC_AHB1PeriphClockCmd(RHEOSTAT_ADC_GPIO_CLK, ENABLE);
		
	// ���� IO
	GPIO_InitStructure.GPIO_Pin = RHEOSTAT_ADC_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	    
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ; //������������
	GPIO_Init(RHEOSTAT_ADC_GPIO_PORT, &GPIO_InitStructure);		
}

static void Rheostat_ADC_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
  // ------------------DMA Init �ṹ����� ��ʼ��--------------------------
  // ADC1ʹ��DMA2��������0��ͨ��0��������ֲ�̶�����
  // ����DMAʱ��
  RCC_AHB1PeriphClockCmd(RHEOSTAT_ADC_DMA_CLK, ENABLE); 
	// �����ַΪ��ADC ���ݼĴ�����ַ
	DMA_InitStructure.DMA_PeripheralBaseAddr = RHEOSTAT_ADC_DR_ADDR;	
  // �洢����ַ��ʵ���Ͼ���һ���ڲ�SRAM�ı���	
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)&ADC_ConvertedValue;  
  // ���ݴ��䷽��Ϊ���赽�洢��	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;	
	// ��������СΪ��ָһ�δ����������
	DMA_InitStructure.DMA_BufferSize = 1;	
	// ����Ĵ���ֻ��һ������ַ���õ���
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  // �洢����ַ�̶�
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable; 
  // // �������ݴ�СΪ���֣��������ֽ� 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; 
  //	�洢�����ݴ�СҲΪ���֣����������ݴ�С��ͬ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	
	// ѭ������ģʽ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  // DMA ����ͨ�����ȼ�Ϊ�ߣ���ʹ��һ��DMAͨ��ʱ�����ȼ����ò�Ӱ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  // ��ֹDMA FIFO	��ʹ��ֱ��ģʽ
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;  
  // FIFO ��С��FIFOģʽ��ֹʱ�������������	
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
	// ѡ�� DMA ͨ����ͨ������������
  DMA_InitStructure.DMA_Channel = RHEOSTAT_ADC_DMA_CHANNEL; 
  //��ʼ��DMA�������൱��һ����Ĺܵ����ܵ������кܶ�ͨ��
	DMA_Init(RHEOSTAT_ADC_DMA_STREAM, &DMA_InitStructure);
	// ʹ��DMA��
  DMA_Cmd(RHEOSTAT_ADC_DMA_STREAM, ENABLE);
	
	// ����ADCʱ��
	RCC_APB2PeriphClockCmd(RHEOSTAT_ADC_CLK , ENABLE);
  // -------------------ADC Common �ṹ�� ���� ��ʼ��------------------------
	// ����ADCģʽ
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  // ʱ��Ϊfpclk x��Ƶ	
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  // ��ֹDMAֱ�ӷ���ģʽ	
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  // ����ʱ����	
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;  
  ADC_CommonInit(&ADC_CommonInitStructure);
	
  // -------------------ADC Init �ṹ�� ���� ��ʼ��--------------------------
	ADC_StructInit(&ADC_InitStructure);
  // ADC �ֱ���
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  // ��ֹɨ��ģʽ����ͨ���ɼ�����Ҫ	
  ADC_InitStructure.ADC_ScanConvMode = DISABLE; 
  // ����ת��	
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 
  //��ֹ�ⲿ���ش���
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  //�ⲿ����ͨ����������ʹ�������������ֵ��㸳ֵ����
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  //�����Ҷ���	
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  //ת��ͨ�� 1��
  ADC_InitStructure.ADC_NbrOfConversion = 1;                                    
  ADC_Init(RHEOSTAT_ADC, &ADC_InitStructure);
  //---------------------------------------------------------------------------
	
  // ���� ADC ͨ��ת��˳��Ϊ1����һ��ת��������ʱ��Ϊ3��ʱ������
  ADC_RegularChannelConfig(RHEOSTAT_ADC, RHEOSTAT_ADC_CHANNEL, 1, ADC_SampleTime_56Cycles);

  // ʹ��DMA���� after last transfer (Single-ADC mode)
  ADC_DMARequestAfterLastTransferCmd(RHEOSTAT_ADC, ENABLE);
  // ʹ��ADC DMA
  ADC_DMACmd(RHEOSTAT_ADC, ENABLE);
	
	// ʹ��ADC
  ADC_Cmd(RHEOSTAT_ADC, ENABLE);  
  //��ʼadcת�����������
  ADC_SoftwareStartConv(RHEOSTAT_ADC);
}

extern __IO uint16_t ADC_ConvertedValue;

// ADC ת������жϷ������
void ADC_IRQHandler(void)
{
	if(ADC_GetITStatus(RHEOSTAT_ADC,ADC_IT_EOC)==SET)
	{
  		// ��ȡADC��ת��ֵ
		ADC_ConvertedValue = ADC_GetConversionValue(RHEOSTAT_ADC);

	}
	ADC_ClearITPendingBit(RHEOSTAT_ADC,ADC_IT_EOC);
}	

void Rheostat_Init(void)
{
	Rheostat_ADC_GPIO_Config();
	Rheostat_ADC_Mode_Config();
}

int GetMedianNum(int * bArray, int iFilterLen)  
{  
    int i,j;// ѭ������  
    int bTemp;  
      
    // ��ð�ݷ��������������  
    for (j = 0; j < iFilterLen - 1; j ++)  
    {  
        for (i = 0; i < iFilterLen - j - 1; i ++)  
        {  
            if (bArray[i] > bArray[i + 1])  
            {  
                // ����  
                bTemp = bArray[i];  
                bArray[i] = bArray[i + 1];  
                bArray[i + 1] = bTemp;  
            }  
        }  
    }  
      
    // ������ֵ  
    if ((iFilterLen & 1) > 0)  
    {  
        // ������������Ԫ�أ������м�һ��Ԫ��  
        bTemp = bArray[(iFilterLen + 1) / 2];  
    }  
    else  
    {  
        // ������ż����Ԫ�أ������м�����Ԫ��ƽ��ֵ  
        bTemp = (bArray[iFilterLen / 2] + bArray[iFilterLen / 2 + 1]) / 2;  
    }  
  
    return bTemp;  
}  

void GetVocValue(int * pData,int FitterValue)
{
	//*pData = FitterValue[0]/3300.0 * 300;
	static unsigned int voc_ad_temp[16];                   // voc����ֵ ����
	static unsigned char voc_current_first = 8;            // ��һ�����鵱ǰλ��
	static unsigned int voc_first_temp;                    // ��һ������ ƽ��ֵ
	static unsigned char voc_current_second = 3;           // �ڶ������鵱ǰλ��
	static unsigned int voc_second_temp;                   // �ڶ������� ƽ��ֵ
	static unsigned char voc_save_count = 13;              // ��ǰ����λ��
	static unsigned int voc_ref = 0xffff;                  // �ο�
	static unsigned short int voc_init_count=0;            // ��ʼ��ʱ�����
	unsigned int voc_k =0;                                 // ���� voc ϵ��
	
	//  voc ��⴦��
	voc_init_count ++;
	if(voc_init_count > 14)
	{	
		voc_save_count ++;   
		if(voc_save_count > 15 )    //Save 15 times
			voc_save_count = 0;
		voc_ad_temp[voc_save_count] = FitterValue;   // �˿�AD����ֵ
		
		voc_current_first ++;
		voc_current_second ++;
		
		if(voc_current_first > 15)   voc_current_first = 0;
		if(voc_current_second > 15)  voc_current_second = 0;
		// voc_first_temp[9-13�β���] ,voc_second_temp[0-4�β���]
		voc_first_temp = ((4*voc_first_temp  + voc_ad_temp[voc_save_count])+2)/5;     
		voc_second_temp = ((4*voc_second_temp + voc_ad_temp[voc_current_second]) + 2)/5;
	
		if(voc_ref > voc_second_temp )
			voc_ref = voc_second_temp; //ȡСֵ
		
		if(voc_first_temp > voc_ref)
		{
			voc_k = (voc_first_temp - voc_ref)*10 / 70;            // ��ϵ���Ŵ� 100 ��
			if(voc_k < 50)                                         // ϵ������С�� 0.5
				voc_k = 35;
			if(voc_k > 200)                                        // ϵ�����ܴ���  2
				voc_k = 200;
			
			* pData = voc_k * (float)(voc_first_temp - voc_ref)/(float)1000.0 ;
		}
		else
			* pData = 0;
		
		if(voc_init_count < 300)
			* pData = 0;
		else
			voc_init_count = 610;
	}
	else
	{
		voc_ad_temp[voc_init_count-1] = FitterValue; //voc_ad_temp[0-13]
		if(voc_init_count == 14)
		{
			voc_second_temp = (voc_ad_temp[0] + voc_ad_temp[1] +\
												voc_ad_temp[2] +voc_ad_temp[3]+\
												voc_ad_temp[4] )/5;
			voc_first_temp = (voc_ad_temp[9] + voc_ad_temp[10] +\
												voc_ad_temp[11] +voc_ad_temp[12]+\
												voc_ad_temp[13] )/5;
			
			voc_ref = voc_second_temp;
		}
		* pData = 0;
	}
}


