#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "stm32f4xx.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
//#define  sFLASH_ID                       0xEF3015     //W25X16
//#define  sFLASH_ID                       0xEF4015	    //W25Q16
#define  sFLASH_ID                        0XEF4017     //W25Q64
//#define  sFLASH_ID                       0XEF4018     //W25Q128

//#define SPI_FLASH_PageSize            4096
#define SPI_FLASH_PageSize             256
#define SPI_FLASH_PerWritePageSize     256

/* Private define ------------------------------------------------------------*/
/*命令定义-开头*******************************/
#define W25X_WriteEnable		      0x06 
#define W25X_WriteDisable		      0x04 
#define W25X_ReadStatusReg		      0x05 
#define W25X_WriteStatusReg		  	  0x01 
#define W25X_ReadData			      0x03 
#define W25X_FastReadData		      0x0B 
#define W25X_FastReadDual		      0x3B 
#define W25X_PageProgram		      0x02 
#define W25X_BlockErase			      0xD8 
#define W25X_SectorErase		      0x20 
#define W25X_ChipErase			      0xC7 
#define W25X_PowerDown			      0xB9 
#define W25X_ReleasePowerDown	      0xAB 
#define W25X_DeviceID			      0xAB 
#define W25X_ManufactDeviceID   	  0x90 
#define W25X_JedecDeviceID		      0x9F 

#define WIP_Flag                      0x01  /* Write In Progress (WIP) flag */
#define Dummy_Byte                    0xFF
/*命令定义-结尾*******************************/


/*SPI接口定义-开头****************************/
#define FLASH_SPI                           SPI4
#define FLASH_SPI_CLK                       RCC_APB2Periph_SPI4
#define FLASH_SPI_CLK_INIT                  RCC_APB2PeriphClockCmd

#define FLASH_SPI_SCK_PIN                   GPIO_Pin_2                 
#define FLASH_SPI_SCK_GPIO_PORT             GPIOE                      
#define FLASH_SPI_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOE
#define FLASH_SPI_SCK_PINSOURCE             GPIO_PinSource2
#define FLASH_SPI_SCK_AF                    GPIO_AF_SPI4

#define FLASH_SPI_MISO_PIN                  GPIO_Pin_5                
#define FLASH_SPI_MISO_GPIO_PORT            GPIOE               
#define FLASH_SPI_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOE
#define FLASH_SPI_MISO_PINSOURCE            GPIO_PinSource5
#define FLASH_SPI_MISO_AF                   GPIO_AF_SPI4

#define FLASH_SPI_MOSI_PIN                  GPIO_Pin_6                
#define FLASH_SPI_MOSI_GPIO_PORT            GPIOE                     
#define FLASH_SPI_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOE
#define FLASH_SPI_MOSI_PINSOURCE            GPIO_PinSource6
#define FLASH_SPI_MOSI_AF                   GPIO_AF_SPI4

#define FLASH_CS_PIN                        GPIO_Pin_4            
#define FLASH_CS_GPIO_PORT                  GPIOE                    
#define FLASH_CS_GPIO_CLK                   RCC_AHB1Periph_GPIOE

#define SPI_FLASH_CS_LOW()      {FLASH_CS_GPIO_PORT->BSRRH=FLASH_CS_PIN;}
#define SPI_FLASH_CS_HIGH()     {FLASH_CS_GPIO_PORT->BSRRL=FLASH_CS_PIN;}
/*SPI接口定义-结尾****************************/

/*等待超时时间*/
#define SPIT_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define SPIT_LONG_TIMEOUT         ((uint32_t)(10 * SPIT_FLAG_TIMEOUT))

/*信息输出*/
#define FLASH_DEBUG_ON         1

#define FLASH_INFO(fmt,arg...)           printf("<<-FLASH-INFO->> "fmt"\n",##arg)
#define FLASH_ERROR(fmt,arg...)          printf("<<-FLASH-ERROR->> "fmt"\n",##arg)
#define FLASH_DEBUG(fmt,arg...)          do{\
                                          if(FLASH_DEBUG_ON)\
                                          printf("<<-FLASH-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                          }while(0)


//u8 TEXT_Buffer[]={"STM32 FLASH TEST"};
//#define TEXT_LENTH sizeof(TEXT_Buffer)	 		  
//#define SIZE (TEXT_LENTH/4+((TEXT_LENTH%4)?1:0))
										  
//#define countof(a)      (sizeof(a) / sizeof(*(a)))
#define  FLASH_WriteAddress     0x00000
#define  FLASH_ReadAddress      FLASH_WriteAddress
#define  FLASH_SectorToErase    FLASH_WriteAddress									  
/* 发送缓冲区初始化 */
//uint8_t Tx_Buffer[] = "STM32 FLASH TEST";
//#define  BufferSize (countof(Tx_Buffer)-1)
//uint8_t Rx_Buffer[BufferSize];
										  
#define PM2005_CAL_SAVE_ADDR_0  0x10000
#define TEMP_CAL_SAVE_ADDR_0    0x00000

																					
void SPI_FLASH_Init(void);
void SPI_FLASH_SectorErase(u32 SectorAddr);
void SPI_FLASH_BulkErase(void);
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
u32 SPI_FLASH_ReadID(void);
u32 SPI_FLASH_ReadDeviceID(void);
void SPI_FLASH_StartReadSequence(u32 ReadAddr);
void SPI_Flash_PowerDown(void);
void SPI_Flash_WAKEUP(void);


u8 SPI_FLASH_ReadByte(void);
u8 SPI_FLASH_SendByte(u8 byte);
u16 SPI_FLASH_SendHalfWord(u16 HalfWord);
void SPI_FLASH_WriteEnable(void);
void SPI_FLASH_WaitForWriteEnd(void);

#endif /* __SPI_FLASH_H */

