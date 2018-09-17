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
#define TTY_DMA_TX_LEN        1024                        /*DMA 发送缓冲区 ----*/
#define TTY_DMA_RX_LEN        1536                        /*DMA 接收缓冲区 ----*/

#define TTY_TXBUF_SIZE_U6        128                       /*发送缓冲区长度 -----*/
#define TTY_RXBUF_SIZE_U6        256                       /*接收缓冲区长度 -----*/
#define TTY_DMA_TX_LEN_U6        128                        /*DMA 发送缓冲区 ----*/
#define TTY_DMA_RX_LEN_U6        256                        /*DMA 接收缓冲区 ----*/
              
#define TTY_USE_DMA           1                         /*启用DMA -----------*/

#if 1

/* Definition for USARTx Pins */
#define USARTx                           UART7
#define USARTx_TX_GPIO_CLK               RCC_AHB1Periph_GPIOF
#define USARTx_TX_PIN                    GPIO_Pin_7
#define USARTx_TX_GPIO_PORT              GPIOF
#define USARTx_TX_AF                     GPIO_AF_UART7
#define USARTx_TX_GPIO_PinSource         GPIO_PinSource7
#define USARTx_RX_PIN                    GPIO_Pin_6
#define USARTx_RX_GPIO_PORT              GPIOF
#define USARTx_RX_AF                     GPIO_AF_UART7
#define USARTx_RX_GPIO_PinSource		 GPIO_PinSource6

/* Definition for USARTx's DMA */
#define USARTx_TX_DMA_CHANNEL            DMA_Channel_5
#define USARTx_TX_DMA_STREAM             DMA1_Stream1
#define USARTx_TX_DMA_FALG               DMA_IT_TCIF1
#define USARTx_RX_DMA_CHANNEL            DMA_Channel_5
#define USARTx_RX_DMA_STREAM             DMA1_Stream3
#define USARTx_RX_DMA_FALG               DMA_IT_TCIF3

/* Definition for USARTx's NVIC */
#define USARTx_DMA_TX_IRQn               DMA1_Stream1_IRQn
#define USARTx_DMA_RX_IRQn               DMA1_Stream3_IRQn
#define USARTx_DMA_TX_IRQHandler         DMA1_Stream1_IRQHandler
#define USARTx_DMA_RX_IRQHandler         DMA1_Stream3_IRQHandler
#define USARTx_IRQn                      UART7_IRQn
#define USARTx_IRQHandler                UART7_IRQHandler

#define USARTx_RCC_DMA				     RCC_AHB1Periph_DMA1
#define USARTx_RCC_APB1_AF				 RCC_APB1Periph_UART7

#else

#define USARTx                           USART6
/* Definition for USARTx Pins */
#define USARTx_TX_GPIO_CLK               RCC_AHB1Periph_GPIOG
#define USARTx_TX_PIN                    GPIO_Pin_14
#define USARTx_TX_GPIO_PORT              GPIOG
#define USARTx_TX_AF                     GPIO_AF_USART6
#define USARTx_TX_GPIO_PinSource         GPIO_PinSource14
#define USARTx_RX_PIN                    GPIO_Pin_9
#define USARTx_RX_GPIO_PORT              GPIOG
#define USARTx_RX_AF                     GPIO_AF_USART6
#define USARTx_RX_GPIO_PinSource		 GPIO_PinSource9

/* Definition for USARTx's DMA */
#define USARTx_TX_DMA_CHANNEL            DMA_Channel_5
#define USARTx_TX_DMA_STREAM             DMA2_Stream6
#define USARTx_TX_DMA_FALG               DMA_IT_TCIF6
#define USARTx_RX_DMA_CHANNEL            DMA_Channel_5
#define USARTx_RX_DMA_STREAM             DMA2_Stream1
#define USARTx_RX_DMA_FALG               DMA_IT_TCIF1

/* Definition for USARTx's NVIC */
#define USARTx_DMA_TX_IRQn               DMA2_Stream6_IRQn
#define USARTx_DMA_RX_IRQn               DMA2_Stream1_IRQn
#define USARTx_DMA_TX_IRQHandler         DMA2_Stream6_IRQHandler
#define USARTx_DMA_RX_IRQHandler         DMA2_Stream1_IRQHandler
#define USARTx_IRQn                      USART6_IRQn
#define USARTx_IRQHandler                USART6_IRQHandler

#define USARTx_RCC_DMA				     RCC_AHB1Periph_DMA2
#define USARTx_RCC_APB2_AF				 RCC_APB2Periph_USART6

#endif

#if 1
#define USART6x                           USART6
/* Definition for USARTx Pins */
#define USART6x_TX_GPIO_CLK               RCC_AHB1Periph_GPIOG
#define USART6x_TX_PIN                    GPIO_Pin_14
#define USART6x_TX_GPIO_PORT              GPIOG
#define USART6x_TX_AF                     GPIO_AF_USART6
#define USART6x_TX_GPIO_PinSource         GPIO_PinSource14
#define USART6x_RX_PIN                    GPIO_Pin_9
#define USART6x_RX_GPIO_PORT              GPIOG
#define USART6x_RX_AF                     GPIO_AF_USART6
#define USART6x_RX_GPIO_PinSource		  GPIO_PinSource9

/* Definition for USARTx's DMA */
#define USART6x_TX_DMA_CHANNEL            DMA_Channel_5
#define USART6x_TX_DMA_STREAM             DMA2_Stream6
#define USART6x_TX_DMA_FALG               DMA_IT_TCIF6
#define USART6x_RX_DMA_CHANNEL            DMA_Channel_5
#define USART6x_RX_DMA_STREAM             DMA2_Stream1
#define USART6x_RX_DMA_FALG               DMA_IT_TCIF1


/* Definition for USARTx's NVIC */
#define USART6x_DMA_TX_IRQn               DMA2_Stream6_IRQn
#define USART6x_DMA_RX_IRQn               DMA2_Stream1_IRQn
#define USART6x_DMA_TX_IRQHandler         DMA2_Stream6_IRQHandler
#define USART6x_DMA_RX_IRQHandler         DMA2_Stream1_IRQHandler
#define USART6x_IRQn                      USART6_IRQn
#define USART6x_IRQHandler                USART6_IRQHandler

#define USART6x_RCC_DMA				     RCC_AHB1Periph_DMA2
#define USART6x_RCC_APB2_AF				 RCC_APB2Periph_USART6
#endif

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
extern const tty_t tty_u6;

#endif



