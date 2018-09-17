/**
  ******************************************************************************
  * @file    bsp_bsp_adc.c
  * @author  
  * @version V1.0
  * @date    
  * @brief   adc驱动
  ******************************************************************************
  */ 
#include "bsp_adc.h"

__IO uint16_t ADC_ConvertedValue;

static void Rheostat_ADC_GPIO_Config(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
	
	// 使能 GPIO 时钟
	RCC_AHB1PeriphClockCmd(RHEOSTAT_ADC_GPIO_CLK, ENABLE);
		
	// 配置 IO
	GPIO_InitStructure.GPIO_Pin = RHEOSTAT_ADC_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	    
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ; //不上拉不下拉
	GPIO_Init(RHEOSTAT_ADC_GPIO_PORT, &GPIO_InitStructure);		
}

static void Rheostat_ADC_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
  // ------------------DMA Init 结构体参数 初始化--------------------------
  // ADC1使用DMA2，数据流0，通道0，这个是手册固定死的
  // 开启DMA时钟
  RCC_AHB1PeriphClockCmd(RHEOSTAT_ADC_DMA_CLK, ENABLE); 
	// 外设基址为：ADC 数据寄存器地址
	DMA_InitStructure.DMA_PeripheralBaseAddr = RHEOSTAT_ADC_DR_ADDR;	
  // 存储器地址，实际上就是一个内部SRAM的变量	
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)&ADC_ConvertedValue;  
  // 数据传输方向为外设到存储器	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;	
	// 缓冲区大小为，指一次传输的数据量
	DMA_InitStructure.DMA_BufferSize = 1;	
	// 外设寄存器只有一个，地址不用递增
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  // 存储器地址固定
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable; 
  // // 外设数据大小为半字，即两个字节 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; 
  //	存储器数据大小也为半字，跟外设数据大小相同
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	
	// 循环传输模式
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  // DMA 传输通道优先级为高，当使用一个DMA通道时，优先级设置不影响
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  // 禁止DMA FIFO	，使用直连模式
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;  
  // FIFO 大小，FIFO模式禁止时，这个不用配置	
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
	// 选择 DMA 通道，通道存在于流中
  DMA_InitStructure.DMA_Channel = RHEOSTAT_ADC_DMA_CHANNEL; 
  //初始化DMA流，流相当于一个大的管道，管道里面有很多通道
	DMA_Init(RHEOSTAT_ADC_DMA_STREAM, &DMA_InitStructure);
	// 使能DMA流
  DMA_Cmd(RHEOSTAT_ADC_DMA_STREAM, ENABLE);
	
	// 开启ADC时钟
	RCC_APB2PeriphClockCmd(RHEOSTAT_ADC_CLK , ENABLE);
  // -------------------ADC Common 结构体 参数 初始化------------------------
	// 独立ADC模式
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  // 时钟为fpclk x分频	
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  // 禁止DMA直接访问模式	
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  // 采样时间间隔	
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;  
  ADC_CommonInit(&ADC_CommonInitStructure);
	
  // -------------------ADC Init 结构体 参数 初始化--------------------------
	ADC_StructInit(&ADC_InitStructure);
  // ADC 分辨率
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  // 禁止扫描模式，多通道采集才需要	
  ADC_InitStructure.ADC_ScanConvMode = DISABLE; 
  // 连续转换	
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 
  //禁止外部边沿触发
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  //外部触发通道，本例子使用软件触发，此值随便赋值即可
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  //数据右对齐	
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  //转换通道 1个
  ADC_InitStructure.ADC_NbrOfConversion = 1;                                    
  ADC_Init(RHEOSTAT_ADC, &ADC_InitStructure);
  //---------------------------------------------------------------------------
	
  // 配置 ADC 通道转换顺序为1，第一个转换，采样时间为3个时钟周期
  ADC_RegularChannelConfig(RHEOSTAT_ADC, RHEOSTAT_ADC_CHANNEL, 1, ADC_SampleTime_56Cycles);

  // 使能DMA请求 after last transfer (Single-ADC mode)
  ADC_DMARequestAfterLastTransferCmd(RHEOSTAT_ADC, ENABLE);
  // 使能ADC DMA
  ADC_DMACmd(RHEOSTAT_ADC, ENABLE);
	
	// 使能ADC
  ADC_Cmd(RHEOSTAT_ADC, ENABLE);  
  //开始adc转换，软件触发
  ADC_SoftwareStartConv(RHEOSTAT_ADC);
}

extern __IO uint16_t ADC_ConvertedValue;

// ADC 转换完成中断服务程序
void ADC_IRQHandler(void)
{
	if(ADC_GetITStatus(RHEOSTAT_ADC,ADC_IT_EOC)==SET)
	{
  		// 读取ADC的转换值
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
    int i,j;// 循环变量  
    int bTemp;  
      
    // 用冒泡法对数组进行排序  
    for (j = 0; j < iFilterLen - 1; j ++)  
    {  
        for (i = 0; i < iFilterLen - j - 1; i ++)  
        {  
            if (bArray[i] > bArray[i + 1])  
            {  
                // 互换  
                bTemp = bArray[i];  
                bArray[i] = bArray[i + 1];  
                bArray[i + 1] = bTemp;  
            }  
        }  
    }  
      
    // 计算中值  
    if ((iFilterLen & 1) > 0)  
    {  
        // 数组有奇数个元素，返回中间一个元素  
        bTemp = bArray[(iFilterLen + 1) / 2];  
    }  
    else  
    {  
        // 数组有偶数个元素，返回中间两个元素平均值  
        bTemp = (bArray[iFilterLen / 2] + bArray[iFilterLen / 2 + 1]) / 2;  
    }  
  
    return bTemp;  
}  

void GetVocValue(int * pData,int FitterValue)
{
	//*pData = FitterValue[0]/3300.0 * 300;
	static unsigned int voc_ad_temp[16];                   // voc采样值 数组
	static unsigned char voc_current_first = 8;            // 第一个数组当前位置
	static unsigned int voc_first_temp;                    // 第一个数组 平均值
	static unsigned char voc_current_second = 3;           // 第二个数组当前位置
	static unsigned int voc_second_temp;                   // 第二个数组 平均值
	static unsigned char voc_save_count = 13;              // 当前数组位置
	static unsigned int voc_ref = 0xffff;                  // 参考
	static unsigned short int voc_init_count=0;            // 初始化时间计数
	unsigned int voc_k =0;                                 // 计算 voc 系数
	
	//  voc 检测处理
	voc_init_count ++;
	if(voc_init_count > 14)
	{	
		voc_save_count ++;   
		if(voc_save_count > 15 )    //Save 15 times
			voc_save_count = 0;
		voc_ad_temp[voc_save_count] = FitterValue;   // 此刻AD采样值
		
		voc_current_first ++;
		voc_current_second ++;
		
		if(voc_current_first > 15)   voc_current_first = 0;
		if(voc_current_second > 15)  voc_current_second = 0;
		// voc_first_temp[9-13次采样] ,voc_second_temp[0-4次采样]
		voc_first_temp = ((4*voc_first_temp  + voc_ad_temp[voc_save_count])+2)/5;     
		voc_second_temp = ((4*voc_second_temp + voc_ad_temp[voc_current_second]) + 2)/5;
	
		if(voc_ref > voc_second_temp )
			voc_ref = voc_second_temp; //取小值
		
		if(voc_first_temp > voc_ref)
		{
			voc_k = (voc_first_temp - voc_ref)*10 / 70;            // 将系数放大 100 倍
			if(voc_k < 50)                                         // 系数不能小于 0.5
				voc_k = 35;
			if(voc_k > 200)                                        // 系数不能大于  2
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


