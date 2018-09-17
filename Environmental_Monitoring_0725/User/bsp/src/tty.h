/******************************************************************************
 * Copyright (C) 2016, roger
 * All rights reserved.
 *
 * �ļ�����: tty.h
 * ժ    Ҫ������̨����

 *             
 * ��ǰ�汾: 3.0
 * ��    ��: roger
 * �������: 2016-09-24
 *             
 * ȡ���汾: 2.0
 * ԭ����  : roger
 * �������: 2015-07-08
 ******************************************************************************/
#ifndef _TTY_H_
#define _TTY_H_

#include "stm32f4xx.h"

#define TTY_BAUDRATE          115200                    /*������ ------------*/
#define TTY_TXBUF_SIZE        1024                       /*���ͻ��������� -----*/
#define TTY_RXBUF_SIZE        1536                       /*���ջ��������� -----*/
#define TTY_DMA_TX_LEN        1024                        /*DMA ���ͻ����� ----*/
#define TTY_DMA_RX_LEN        1536                        /*DMA ���ջ����� ----*/

#define TTY_TXBUF_SIZE_U6        128                       /*���ͻ��������� -----*/
#define TTY_RXBUF_SIZE_U6        256                       /*���ջ��������� -----*/
#define TTY_DMA_TX_LEN_U6        128                        /*DMA ���ͻ����� ----*/
#define TTY_DMA_RX_LEN_U6        256                        /*DMA ���ջ����� ----*/
              
#define TTY_USE_DMA           1                         /*����DMA -----------*/

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
    void (*init)(void);                                     /*��ʼ�� --------*/    
    unsigned int (*write)(void *buf, unsigned int len);     /*����д --------*/
    unsigned int (*read) (void *buf, unsigned int len);     /*������ --------*/
    void (*puts)(const char *str);                          /*����һ���ַ��� */
    void (*clr)(void);                                      /*������ջ����� */
    unsigned int (*buflen)(void);                           /*���ջ������ĳ���*/
    void (*printf)(const char *format, ...);                /*��ʽ����ӡ ----*/
}tty_t;

/* Exported variables ------------------------------------------------------- */
extern const tty_t tty;
extern const tty_t tty_u6;

#endif



