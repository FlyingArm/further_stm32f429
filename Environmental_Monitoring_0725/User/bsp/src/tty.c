/******************************************************************************
 * Copyright (C) 2016, roger
 * All rights reserved.
 *
 * 文件名称: tty.c
 * 摘    要：打印串口驱动

 *             
 * 当前版本: 3.0
 * 作    者: roger
 * 完成日期: 2016-09-24
 *             
 * 取代版本: 2.0
 * 原作者  : roger
 * 完成日期: 2015-07-08
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "tty.h"
#include "ringbuffer.h"
#include "stm32f4xx.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static unsigned char rxbuf[TTY_TXBUF_SIZE];         /*接收缓冲区 ------------*/
static unsigned char txbuf[TTY_RXBUF_SIZE];         /*发送缓冲区 ------------*/
static unsigned char rxbuf_u6[TTY_TXBUF_SIZE_U6];         	/*接收缓冲区 ------------*/
static unsigned char txbuf_u6[TTY_RXBUF_SIZE_U6];         	/*发送缓冲区 ------------*/

static ring_buf_t ringbuf_send;       							/*收发缓冲区管理 ---------*/
static ring_buf_t ringbuf_recv;
static ring_buf_t ringbuf_send_u6;
static ring_buf_t ringbuf_recv_u6;

#if TTY_USE_DMA == 1
    static unsigned char dma_tx_buf[TTY_DMA_TX_LEN];/*DMA发送缓冲区 ---------*/
    static unsigned char dma_rx_buf[TTY_DMA_RX_LEN];/*DMA接收缓冲区 ---------*/
	static unsigned char dma_tx_buf_u6[TTY_DMA_TX_LEN_U6];		/*DMA发送缓冲区 ---------*/
    static unsigned char dma_rx_buf_u6[TTY_DMA_RX_LEN_U6];		/*DMA接收缓冲区 ---------*/ 
#endif

/*******************************************************************************
 * 函数名称：port_conf
 * 功能描述：打印串口配置(PD8->USART3_TX, PD9->USART3_RX)
 * 输入参数：none
 * 返 回 值：none
 * 作    者：roger.luo
 ******************************************************************************/
	static void port_conf(void)
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		/*console串口引脚配置 ----------------------------------------------------*/
		RCC_AHB1PeriphClockCmd(USARTx_TX_GPIO_CLK | USART6x_TX_GPIO_CLK, ENABLE);
	
		GPIO_PinAFConfig(USARTx_TX_GPIO_PORT, USARTx_TX_GPIO_PinSource, USARTx_TX_AF);
		GPIO_PinAFConfig(USARTx_RX_GPIO_PORT, USARTx_RX_GPIO_PinSource, USARTx_RX_AF);
		
		GPIO_PinAFConfig(USART6x_TX_GPIO_PORT, USART6x_TX_GPIO_PinSource, USART6x_TX_AF);
		GPIO_PinAFConfig(USART6x_RX_GPIO_PORT, USART6x_RX_GPIO_PinSource, USART6x_RX_AF);
		
		GPIO_InitStructure.GPIO_Pin = USARTx_TX_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	  
		GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStructure);  
		GPIO_InitStructure.GPIO_Pin = USARTx_RX_PIN;
		GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = USART6x_TX_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	  
		GPIO_Init(USART6x_TX_GPIO_PORT, &GPIO_InitStructure);  
		GPIO_InitStructure.GPIO_Pin = USART6x_RX_PIN;
		GPIO_Init(USART6x_RX_GPIO_PORT, &GPIO_InitStructure);
	}

/*******************************************************************************
 * 函数名称：DMA_Conf
 * 功能描述: 串口DMA配置(DMA1_Channel4_Stream1->USART3_RX,
 *                       DMA1_Channel4_Stream3->USART3_TX)
 * 输入参数：none
 * 返 回 值：none
 * 作    者：roger.luo
 ******************************************************************************/
#if TTY_USE_DMA == 1

#if 0
static void DMA_Conf(void)
{
    DMA_InitTypeDef DMA_Structure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable DMA clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);    
    DMA_DeInit(DMA1_Stream1);
    DMA_DeInit(DMA1_Stream3);
    while (DMA_GetCmdStatus(DMA1_Stream1) != DISABLE){}
    while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE){}
    /*配置串口3接收流 */
    DMA_Structure.DMA_Channel = DMA_Channel_4;                    /*DMA1通道4*/
    DMA_Structure.DMA_PeripheralBaseAddr = (uint32_t)(&USART3->DR);
    DMA_Structure.DMA_Memory0BaseAddr = (uint32_t)dma_rx_buf;
    DMA_Structure.DMA_DIR = DMA_DIR_PeripheralToMemory;           /*外设到内存*/
    DMA_Structure.DMA_BufferSize = sizeof(dma_rx_buf);
    DMA_Structure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_Structure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_Structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_Structure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_Structure.DMA_Mode = DMA_Mode_Circular;                   /*循环模式*/
    DMA_Structure.DMA_Priority = DMA_Priority_Low;
    DMA_Structure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
    DMA_Structure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_Structure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_Structure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA1_Stream1, &DMA_Structure); 

    /*配置串口3发送流 */
    DMA_Structure.DMA_PeripheralBaseAddr = (uint32_t)(&USART3->DR);
    DMA_Structure.DMA_Memory0BaseAddr = (uint32_t)dma_tx_buf;
    DMA_Structure.DMA_DIR = DMA_DIR_MemoryToPeripheral;            /*内存到外设*/
    DMA_Structure.DMA_BufferSize = sizeof(dma_tx_buf);
    DMA_Structure.DMA_Mode = DMA_Mode_Normal;                      /*正常模式 -*/
    DMA_Init(DMA1_Stream3, &DMA_Structure); 

    /* Enable DMA Stream Transfer Complete interrupt */
    DMA_ITConfig(DMA1_Stream1, DMA_IT_TC, ENABLE);    
    //DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
    /* DMA Stream enable */
    DMA_Cmd(DMA1_Stream1, ENABLE);                                 /*使能接收流*/

     /* Enable the DMA Stream IRQ Channel */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
    NVIC_Init(&NVIC_InitStructure); 
}

#else

static void DMA_Conf(void)
{
    DMA_InitTypeDef DMA_Structure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable DMA clock */
    RCC_AHB1PeriphClockCmd(USARTx_RCC_DMA, ENABLE);    
    DMA_DeInit(USARTx_TX_DMA_STREAM);
    DMA_DeInit(USARTx_RX_DMA_STREAM);
    while (DMA_GetCmdStatus(USARTx_TX_DMA_STREAM) != DISABLE){}
    while (DMA_GetCmdStatus(USARTx_RX_DMA_STREAM) != DISABLE){}
    /*配置串口7接收流 */
    DMA_Structure.DMA_Channel = USARTx_TX_DMA_CHANNEL;                    /*DMA1通道5*/
    DMA_Structure.DMA_PeripheralBaseAddr = (uint32_t)(&USARTx->DR);
    DMA_Structure.DMA_Memory0BaseAddr = (uint32_t)dma_rx_buf;
    DMA_Structure.DMA_DIR = DMA_DIR_PeripheralToMemory;           /*外设到内存*/
    DMA_Structure.DMA_BufferSize = sizeof(dma_rx_buf);
    DMA_Structure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_Structure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_Structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_Structure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_Structure.DMA_Mode = DMA_Mode_Circular;                   /*循环模式*/
    DMA_Structure.DMA_Priority = DMA_Priority_Low;
    DMA_Structure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
    DMA_Structure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_Structure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_Structure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(USARTx_RX_DMA_STREAM, &DMA_Structure); 

    /*配置串口7发送流 */
    DMA_Structure.DMA_PeripheralBaseAddr = (uint32_t)(&USARTx->DR);
    DMA_Structure.DMA_Memory0BaseAddr = (uint32_t)dma_tx_buf;
    DMA_Structure.DMA_DIR = DMA_DIR_MemoryToPeripheral;            /*内存到外设*/
    DMA_Structure.DMA_BufferSize = sizeof(dma_tx_buf);
    DMA_Structure.DMA_Mode = DMA_Mode_Normal;                      /*正常模式 -*/
    DMA_Init(USARTx_TX_DMA_STREAM, &DMA_Structure); 

    /* Enable DMA Stream Transfer Complete interrupt */
    DMA_ITConfig(USARTx_RX_DMA_STREAM, DMA_IT_TC, ENABLE);    
    //DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
    /* DMA Stream enable */
    DMA_Cmd(USARTx_RX_DMA_STREAM, ENABLE);                                 /*使能接收流*/

     /* Enable the DMA Stream IRQ Channel */
    NVIC_InitStructure.NVIC_IRQChannel = USARTx_DMA_RX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannel = USARTx_DMA_TX_IRQn;
    NVIC_Init(&NVIC_InitStructure); 
}

#if 1
static void DMA_Conf_U6(void)
{
    DMA_InitTypeDef DMA_Structure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable DMA clock */
    RCC_AHB1PeriphClockCmd(USART6x_RCC_DMA, ENABLE);    
    DMA_DeInit(USART6x_TX_DMA_STREAM);
    DMA_DeInit(USART6x_RX_DMA_STREAM);
    while (DMA_GetCmdStatus(USART6x_TX_DMA_STREAM) != DISABLE){}
    while (DMA_GetCmdStatus(USART6x_RX_DMA_STREAM) != DISABLE){}
	
    /*配置串口7接收流 */
    DMA_Structure.DMA_Channel = USART6x_TX_DMA_CHANNEL;                    /*DMA2通道5*/
    DMA_Structure.DMA_PeripheralBaseAddr = (uint32_t)(&USART6x->DR);
    DMA_Structure.DMA_Memory0BaseAddr = (uint32_t)dma_rx_buf_u6;
    DMA_Structure.DMA_DIR = DMA_DIR_PeripheralToMemory;           /*外设到内存*/
    DMA_Structure.DMA_BufferSize = sizeof(dma_rx_buf_u6);
    DMA_Structure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_Structure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_Structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_Structure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_Structure.DMA_Mode = DMA_Mode_Circular;                   /*循环模式*/
    DMA_Structure.DMA_Priority = DMA_Priority_Low;
    DMA_Structure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
    DMA_Structure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_Structure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_Structure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(USART6x_RX_DMA_STREAM, &DMA_Structure); 

    /*配置串口7发送流 */
    DMA_Structure.DMA_PeripheralBaseAddr = (uint32_t)(&USART6x->DR);
    DMA_Structure.DMA_Memory0BaseAddr = (uint32_t)dma_tx_buf_u6;
    DMA_Structure.DMA_DIR = DMA_DIR_MemoryToPeripheral;            /*内存到外设*/
    DMA_Structure.DMA_BufferSize = sizeof(dma_tx_buf_u6);
    DMA_Structure.DMA_Mode = DMA_Mode_Normal;                      /*正常模式 -*/
    DMA_Init(USART6x_TX_DMA_STREAM, &DMA_Structure); 

    /* Enable DMA Stream Transfer Complete interrupt */
    DMA_ITConfig(USART6x_RX_DMA_STREAM, DMA_IT_TC, ENABLE);    
    //DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
    /* DMA Stream enable */
    DMA_Cmd(USART6x_RX_DMA_STREAM, ENABLE);                                 /*使能接收流*/

     /* Enable the DMA Stream IRQ Channel */
    NVIC_InitStructure.NVIC_IRQChannel = USART6x_DMA_RX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannel = USART6x_DMA_TX_IRQn;
    NVIC_Init(&NVIC_InitStructure); 
}

#endif
#endif

#endif
/*******************************************************************************
 * 函数名称：uart_conf
 * 功能描述：TTY 串口配置
 * 输入参数：none
 * 返 回 值：none
 * 作    者：roger.luo
 ******************************************************************************/

#if 0

static void uart_conf(void)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	
    USART_DeInit(USART3);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    USART_InitStructure.USART_BaudRate = TTY_BAUDRATE;  
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 

    ring_buf_create(&ringbuf_send, txbuf, sizeof(txbuf));/*初始化环形缓冲区 --*/
    ring_buf_create(&ringbuf_recv, rxbuf, sizeof(rxbuf));     

#if TTY_USE_DMA == 1     
    USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);         /*开启DMA请求 --------*/
    USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE);   
    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);       /*打开空闲中断处理DMA接收 -------*/     
#else
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
#endif    
    USART_ITConfig(USART3, USART_IT_ERR, ENABLE);    
    USART_Cmd(USART3, ENABLE);    
}

#else

static void uart_conf(void)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	
    USART_DeInit(USARTx);
    RCC_APB1PeriphClockCmd(USARTx_RCC_APB1_AF, ENABLE);
    USART_InitStructure.USART_BaudRate = TTY_BAUDRATE;  
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USARTx, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USARTx_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 

    ring_buf_create(&ringbuf_send, txbuf, sizeof(txbuf));/*初始化环形缓冲区 --------*/
    ring_buf_create(&ringbuf_recv, rxbuf, sizeof(rxbuf));     

#if TTY_USE_DMA == 1     
    USART_DMACmd(USARTx,USART_DMAReq_Rx,ENABLE);         /*开启DMA请求 --------*/
    USART_DMACmd(USARTx,USART_DMAReq_Tx,ENABLE);   
    USART_ITConfig(USARTx, USART_IT_IDLE, ENABLE);       /*打开空闲中断处理DMA接收 -------*/     
#else
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
#endif    
    USART_ITConfig(USARTx, USART_IT_ERR, ENABLE);    
    USART_Cmd(USARTx, ENABLE);    
}

#if 1
static void uart_conf_u6(void)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	
    USART_DeInit(USART6x);

    RCC_APB2PeriphClockCmd(USART6x_RCC_APB2_AF, ENABLE);
    USART_InitStructure.USART_BaudRate = TTY_BAUDRATE;  
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART6x, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART6x_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 

    ring_buf_create(&ringbuf_send_u6, txbuf_u6, sizeof(txbuf_u6));/*初始化环形缓冲区 --------*/
    ring_buf_create(&ringbuf_recv_u6, rxbuf_u6, sizeof(rxbuf_u6));     

#if TTY_USE_DMA == 1     
    USART_DMACmd(USART6x,USART_DMAReq_Rx,ENABLE);         /*开启DMA请求 --------*/
    USART_DMACmd(USART6x,USART_DMAReq_Tx,ENABLE);   
    USART_ITConfig(USART6x, USART_IT_IDLE, ENABLE);       /*打开空闲中断处理DMA接收 -------*/     
#else
    USART_ITConfig(USART6x, USART_IT_RXNE, ENABLE);
#endif    
    USART_ITConfig(USART6x, USART_IT_ERR, ENABLE);    
    USART_Cmd(USART6x, ENABLE);    
}

#endif
#endif


/*******************************************************************************
 * 函数名称：init
 * 功能描述：打印驱动初始化
 * 输入参数：none
 * 返 回 值：none
 * 作    者：roger.luo
 ******************************************************************************/
static void init(void)
{
    port_conf();
    uart_conf();
	uart_conf_u6();
#if TTY_USE_DMA == 1    
    DMA_Conf(); 
	DMA_Conf_U6();
#endif    
}

/*******************************************************************************
 * 函数名称：send
 * 功能描述：向串口发送缓冲区内写入数据
 * 输入参数：buf       -  缓冲区
 *           len       -  缓冲区长度
 * 返 回 值：实际写入长度(如果此时缓冲区满,则返回len)
 * 作    者：roger.luo
 ******************************************************************************/
 
#if 0

static unsigned int send(void *buf, unsigned int len)
{

#if TTY_USE_DMA == 1    
    unsigned int ret;
    ret = ring_buf_put(&ringbuf_send, buf, len);  
    USART_ITConfig(USART3, USART_IT_TC, ENABLE);  
    return ret;
#else
    unsigned int ret;
    ret = ring_buf_put(&ringbuf_send, (unsigned char *)buf, len);       
    USART_ITConfig(USART3, USART_IT_TXE, ENABLE);    
    return ret;        
#endif    
}

#else

static unsigned int send(void *buf, unsigned int len)
{

#if TTY_USE_DMA == 1    
    unsigned int ret;
    ret = ring_buf_put(&ringbuf_send, buf, len);  
    USART_ITConfig(USARTx, USART_IT_TC, ENABLE);  
    return ret;
#else
    unsigned int ret;
    ret = ring_buf_put(&ringbuf_send, (unsigned char *)buf, len);       
    USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);    
    return ret;        
#endif    
}

static unsigned int send_u6(void *buf, unsigned int len)
{

#if TTY_USE_DMA == 1    
    unsigned int ret;
    ret = ring_buf_put(&ringbuf_send_u6, buf, len);  
    USART_ITConfig(USART6x, USART_IT_TC, ENABLE);  
    return ret;
#else
    unsigned int ret;
    ret = ring_buf_put(&ringbuf_send_u6, (unsigned char *)buf, len);       
    USART_ITConfig(USART6x, USART_IT_TXE, ENABLE);    
    return ret;        
#endif    
}

#endif
/*******************************************************************************
 * 函数名称：recv
 * 功能描述：读取tty接收缓冲区的数据
 * 输入参数：buf       -  缓冲区
 *           len       -  缓冲区长度
 * 返 回 值：(实际读取长度)如果接收缓冲区的有效数据大于len则返回len否则返回缓冲
 *            区有效数据的长度
 * 作    者：roger.luo
 ******************************************************************************/
unsigned int recv(void *buf, unsigned int len)
{
    return ring_buf_get(&ringbuf_recv, (unsigned char *)buf, len);
}

unsigned int recv_u6(void *buf, unsigned int len)
{
    return ring_buf_get(&ringbuf_recv_u6, (unsigned char *)buf, len);
}

#if TTY_USE_DMA == 1
/*******************************************************************************
 * 函数名称：DMA1_Stream1_IRQHandler
 * 功能描述：TTY串口DMA接收完成中断
 * 输入参数：none
 * 返 回 值：none
 ******************************************************************************/
#if 0
void DMA1_Stream1_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_Stream1, DMA_IT_TCIF1) != RESET)  
    { 
        ring_buf_put(&ringbuf_recv, dma_rx_buf, sizeof(dma_rx_buf));
        DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_TCIF1);
    }    
}

#else

void USARTx_DMA_RX_IRQHandler(void)
{
    if (DMA_GetITStatus(USARTx_RX_DMA_STREAM, USARTx_RX_DMA_FALG) != RESET)  
    { 
        ring_buf_put(&ringbuf_recv, dma_rx_buf, sizeof(dma_rx_buf));
        DMA_ClearITPendingBit(USARTx_RX_DMA_STREAM, USARTx_RX_DMA_FALG);
    }    
}

void USART6x_DMA_RX_IRQHandler(void)
{
    if (DMA_GetITStatus(USART6x_RX_DMA_STREAM, USART6x_RX_DMA_FALG) != RESET)  
    { 
        ring_buf_put(&ringbuf_recv_u6, dma_rx_buf_u6, sizeof(dma_rx_buf_u6));
        DMA_ClearITPendingBit(USART6x_RX_DMA_STREAM, USART6x_RX_DMA_FALG);
    }    
}

#endif
/*******************************************************************************
 * 函数名称：DMA1_Stream3_IRQHandler
 * 功能描述：TTY串口DMA发送完成中断
 * 输入参数：none
 * 返 回 值：none
 ******************************************************************************/
/*void DMA1_Stream3_IRQHandler(void)
{
    unsigned int len;
    if (DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);  
        if ((len = ring_buf_get(&ringbuf_send, dma_tx_buf, sizeof(dma_tx_buf))))
        {                
            DMA_SetCurrDataCounter(DMA1_Stream3, len);        
            DMA_Cmd(DMA1_Stream3, ENABLE);
            USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE);
        }
        else sending = 0;
    }   

}*/
#endif

/*******************************************************************************
 * 函数名称：USART3_IRQHandler
 * 功能描述：串口1收发中断
 * 输入参数：none
 * 返 回 值：none
 ******************************************************************************/

//extern unsigned int uart7_rec_timeout;

#if 0
void USART3_IRQHandler(void)
{    
	#if TTY_USE_DMA == 1    
    uint16_t len, retry = 0;
	//////////////////////////////////////////////////////////////////////
	//uart3_rec_timeout = GUI_GetTime();
	//////////////////////////////////////////////////////////////////////
    if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET || 
        USART_GetITStatus(USART3, USART_IT_FE) != RESET) 
    {
        /*获取DMA缓冲区内的有效数据长度 --------------------------------------*/
        len = sizeof(dma_rx_buf) - DMA_GetCurrDataCounter(DMA1_Stream1);
        DMA_Cmd(DMA1_Stream1, DISABLE);  
        ring_buf_put(&ringbuf_recv,dma_rx_buf, len);    /*将数据放入接收缓冲区*/   
        while (DMA_GetCmdStatus(DMA1_Stream1) != DISABLE && retry++ < 100){}  
        /*复位DMA当前计数器值 ------------------------------------------------*/
        DMA_SetCurrDataCounter(DMA1_Stream1, sizeof(dma_rx_buf));
        DMA_Cmd(DMA1_Stream1, ENABLE);         
        DMA_ClearFlag(DMA1_Stream1, DMA_FLAG_TCIF1);    /*清除传输完成标志,否则会进入传输完成中断*/
        len = USART3->DR;                               /*清除中断标志 -------*/
    }
    if (USART_GetITStatus(USART3, USART_IT_TC) != RESET)
    {  
        if ((len = ring_buf_get(&ringbuf_send, dma_tx_buf, sizeof(dma_tx_buf))))
        {                
            DMA_Cmd(DMA1_Stream3, DISABLE);
            DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);
            DMA_SetCurrDataCounter(DMA1_Stream3, len);        
            DMA_Cmd(DMA1_Stream3, ENABLE);
        }
        else 
        {
            USART_ITConfig(USART3, USART_IT_TC, DISABLE);       
        }
    }
	#else
    unsigned char data;
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) 
    {
        data = USART_ReceiveData(USART3);
        ring_buf_put(&ringbuf_recv,&data, 1);           /*将数据放入接收缓冲区*/  
    }
    if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET) 
    {
        if (ring_buf_get(&ringbuf_send, &data, 1))      /*从缓冲区中取出数据---*/
        {
            USART_SendData(USART3, data);            
        }
        else
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);    
    }
    if (USART_GetITStatus(USART3, USART_IT_FE) != RESET)
    {
        data = USART_ReceiveData(USART3);

    }    
	#endif
}

#else

void USARTx_IRQHandler(void)
{    
	#if TTY_USE_DMA == 1    
    uint16_t len, retry = 0;

    if (USART_GetITStatus(USARTx, USART_IT_IDLE) != RESET || 
        USART_GetITStatus(USARTx, USART_IT_FE) != RESET) 
    {
        len = sizeof(dma_rx_buf) - DMA_GetCurrDataCounter(USARTx_RX_DMA_STREAM);/*获取DMA缓冲区内的有效数据长度 --------------------------------------*/
        DMA_Cmd(USARTx_RX_DMA_STREAM, DISABLE);  
        ring_buf_put(&ringbuf_recv,dma_rx_buf, len);    /*将数据放入接收缓冲区*/   
        while (DMA_GetCmdStatus(USARTx_RX_DMA_STREAM) != DISABLE && retry++ < 100){}  
        DMA_SetCurrDataCounter(USARTx_RX_DMA_STREAM, sizeof(dma_rx_buf)); /*复位DMA当前计数器值 ------------------------------------------------*/
        DMA_Cmd(USARTx_RX_DMA_STREAM, ENABLE);         
        DMA_ClearFlag(USARTx_RX_DMA_STREAM, USARTx_RX_DMA_FALG);    /*清除传输完成标志,否则会进入传输完成中断*/
        len = USARTx->DR;                               /*清除中断标志 -------*/
    }
    if (USART_GetITStatus(USARTx, USART_IT_TC) != RESET)
    {  
        if ((len = ring_buf_get(&ringbuf_send, dma_tx_buf, sizeof(dma_tx_buf))))
        {                
            DMA_Cmd(USARTx_TX_DMA_STREAM, DISABLE);
            DMA_ClearFlag(USARTx_TX_DMA_STREAM, USARTx_TX_DMA_FALG);
            DMA_SetCurrDataCounter(USARTx_TX_DMA_STREAM, len);        
            DMA_Cmd(USARTx_TX_DMA_STREAM, ENABLE);
        }
        else 
        {
            USART_ITConfig(USARTx, USART_IT_TC, DISABLE);       
        }
    }
	#else
    unsigned char data;
    if (USART_GetITStatus(USARTx, USART_IT_RXNE) != RESET) 
    {
        data = USART_ReceiveData(USARTx);
        ring_buf_put(&ringbuf_recv,&data, 1);           /*将数据放入接收缓冲区*/  
    }
    if (USART_GetITStatus(USARTx, USART_IT_TXE) != RESET) 
    {
        if (ring_buf_get(&ringbuf_send, &data, 1))      /*从缓冲区中取出数据---*/
        {
            USART_SendData(USARTx, data);            
        }
        else
            USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);    
    }
    if (USART_GetITStatus(USARTx, USART_IT_FE) != RESET)
    {
        data = USART_ReceiveData(USARTx);

    }    
	#endif
}

void USART6x_IRQHandler(void)
{    
	#if TTY_USE_DMA == 1    
    uint16_t len, retry = 0;

    if (USART_GetITStatus(USART6x, USART_IT_IDLE) != RESET || 
        USART_GetITStatus(USART6x, USART_IT_FE) != RESET) 
    {
        len = sizeof(dma_rx_buf_u6) - DMA_GetCurrDataCounter(USART6x_RX_DMA_STREAM);	/*获取DMA缓冲区内的有效数据长度 --------------------------------------*/
        DMA_Cmd(USART6x_RX_DMA_STREAM, DISABLE);  
        ring_buf_put(&ringbuf_recv_u6,dma_rx_buf_u6, len);    							/*将数据放入接收缓冲区*/   
        while (DMA_GetCmdStatus(USART6x_RX_DMA_STREAM) != DISABLE && retry++ < 100){}  
        DMA_SetCurrDataCounter(USART6x_RX_DMA_STREAM, sizeof(dma_rx_buf_u6)); 			/*复位DMA当前计数器值 ------------------------------------------------*/
        DMA_Cmd(USART6x_RX_DMA_STREAM, ENABLE);         
        DMA_ClearFlag(USART6x_RX_DMA_STREAM, USART6x_RX_DMA_FALG);    					/*清除传输完成标志,否则会进入传输完成中断*/
        len = USART6x->DR;                               								/*清除中断标志 -------*/
    }
    if (USART_GetITStatus(USART6x, USART_IT_TC) != RESET)
    {  
        if ((len = ring_buf_get(&ringbuf_send_u6, dma_tx_buf_u6, sizeof(dma_tx_buf_u6))))
        {                
            DMA_Cmd(USART6x_TX_DMA_STREAM, DISABLE);
            DMA_ClearFlag(USART6x_TX_DMA_STREAM, USART6x_TX_DMA_FALG);
            DMA_SetCurrDataCounter(USART6x_TX_DMA_STREAM, len);        
            DMA_Cmd(USART6x_TX_DMA_STREAM, ENABLE);
        }
        else 
        {
            USART_ITConfig(USART6x, USART_IT_TC, DISABLE);       
        }
    }
	#else
    unsigned char data;
    if (USART_GetITStatus(USART6x, USART_IT_RXNE) != RESET) 
    {
        data = USART_ReceiveData(USART6x);
        ring_buf_put(&ringbuf_recv_u6,&data, 1);           /*将数据放入接收缓冲区*/  
    }
    if (USART_GetITStatus(USART6x, USART_IT_TXE) != RESET) 
    {
        if (ring_buf_get(&ringbuf_send_u6, &data, 1))      /*从缓冲区中取出数据---*/
        {
            USART_SendData(USART6x, data);            
        }
        else
            USART_ITConfig(USART6x, USART_IT_TXE, DISABLE);    
    }
    if (USART_GetITStatus(USART6x, USART_IT_FE) != RESET)
    {
        data = USART_ReceiveData(USART6x);

    }    
	#endif
}

#endif

/*******************************************************************************
 * 函数名称：putstr
 * 功能描述：输出一个字符串
 * 输入参数：none
 * 返 回 值：none
 ******************************************************************************/
static void putstr(const char *str)
{
    send((void *)str, strlen(str));
}

static void putstr_u6(const char *str)
{
    send_u6((void *)str, strlen(str));
}

/*******************************************************************************
 * 函数名称：clear
 * 功能描述：清除接收缓冲区的数据
 * 输入参数：none
 * 返 回 值：none
 ******************************************************************************/
static void clear(void)
{
    ring_buf_clr(&ringbuf_recv);
}

static void clear_u6(void)
{
    ring_buf_clr(&ringbuf_recv_u6);
}

void clear_sendbuf(void)
{
    ring_buf_clr(&ringbuf_send);
}


/*******************************************************************************
 * 函数名称：buflen
 * 功能描述：清除接收缓冲区的数据
 * 输入参数：none
 * 返 回 值：none
 ******************************************************************************/
static unsigned int buflen(void)
{
    return ring_buf_len(&ringbuf_recv);
}

static unsigned int buflen_u6(void)
{
    return ring_buf_len(&ringbuf_recv_u6);
}


/*******************************************************************************
 * 函数名称：print
 * 功能描述：格式化打印输出
 * 输入参数：none
 * 返 回 值：none
 ******************************************************************************/
static void print(const char *format, ...)
{
    va_list args;
    char buf[256];
    va_start (args, format);
    vsprintf (buf, format, args);       
    va_end (args);    
    putstr(buf);
}


/*外部接口定义 --------------------------------------------------------------*/
extern const tty_t tty = 
{
    init,
    send,
    recv,
    putstr,
    clear,
    buflen,
    print
};

extern const tty_t tty_u6 = 
{
    init,
    send_u6,
    recv_u6,
    putstr_u6,
    clear_u6,
    buflen_u6,
    print
};



