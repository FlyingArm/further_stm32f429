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
#define TTY_DMA_TX_LEN        10                        /*DMA ���ͻ����� ----*/
#define TTY_DMA_RX_LEN        10                        /*DMA ���ջ����� ----*/

#define TTY_USE_DMA           1                         /*����DMA -----------*/

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


#endif

