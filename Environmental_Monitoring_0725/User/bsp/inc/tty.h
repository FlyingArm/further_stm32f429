/******************************************************************************
 * Copyright (C) 2016, roger
 * All rights reserved.
 *
 * 文件名称: tty.h
 * 摘    要：控制台驱动

 *             
 * 当前版本: 3.0
 * 作    者: roger
 * 完成日期: 2016-09-24
 *             
 * 取代版本: 2.0
 * 原作者  : roger
 * 完成日期: 2015-07-08
 ******************************************************************************/


#ifndef _TTY_H_
#define _TTY_H_

#include "stm32f4xx.h"

#define TTY_BAUDRATE          115200                    /*波特率 ------------*/
#define TTY_TXBUF_SIZE        1024                       /*发送缓冲区长度 -----*/
#define TTY_RXBUF_SIZE        1536                       /*接收缓冲区长度 -----*/
#define TTY_DMA_TX_LEN        10                        /*DMA 发送缓冲区 ----*/
#define TTY_DMA_RX_LEN        10                        /*DMA 接收缓冲区 ----*/

#define TTY_USE_DMA           1                         /*启用DMA -----------*/

#define USARTx                           USART3

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_Pin_10
#define USARTx_TX_GPIO_PORT              GPIOB
#define USARTx_TX_AF                     GPIO_AF_USART3
#define USARTx_TX_GPIO_PinSource         GPIO_PinSource10
#define USARTx_RX_PIN                    GPIO_Pin_11
#define USARTx_RX_GPIO_PORT              GPIOB
#define USARTx_RX_AF                     GPIO_AF_USART3
#define USARTx_RX_GPIO_PinSource		 GPIO_PinSource11

/* Definition for USARTx's DMA */
#define USARTx_TX_DMA_CHANNEL            DMA_Channel_4
#define USARTx_TX_DMA_STREAM             DMA1_Stream3
#define USARTx_RX_DMA_CHANNEL            DMA_Channel_4
#define USARTx_RX_DMA_STREAM             DMA1_Stream1

/* Definition for USARTx's NVIC */
#define USARTx_DMA_TX_IRQn               DMA1_Stream3_IRQn
#define USARTx_DMA_RX_IRQn               DMA1_Stream1_IRQn
#define USARTx_DMA_TX_IRQHandler         DMA1_Stream3_IRQHandler
#define USARTx_DMA_RX_IRQHandler         DMA1_Stream1_IRQHandler
#define USARTx_IRQn                      USART3_IRQn
#define USARTx_IRQHandler                USART3_IRQHandler

#define USARTx_RCC_DMA				     RCC_AHB1Periph_DMA1
#define USARTx_RCC_APB1_AF				 RCC_APB1Periph_USART3
#define USARTx_RCC_APB2_AF				 RCC_APB2Periph_USART6

/* Exported Structs ---------------------------------------------------------*/

typedef struct          
{
    void (*init)(void);                                     /*初始化 --------*/    
    unsigned int (*write)(void *buf, unsigned int len);     /*数据写 --------*/
    unsigned int (*read) (void *buf, unsigned int len);     /*读数据 --------*/
    void (*puts)(const char *str);                          /*输入一个字符串 */
    void (*clr)(void);                                      /*清除接收缓冲区 */
    unsigned int (*buflen)(void);                           /*接收缓冲区的长度*/
    void (*printf)(const char *format, ...);                /*格式化打印 ----*/
}tty_t;


/* Exported variables ------------------------------------------------------- */
extern const tty_t tty;


#endif

