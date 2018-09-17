/**
  ******************************************************************************
  * @file    stm32f429i_discovery_lcd.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    28-October-2013
  * @brief   This file includes the LCD driver for ILI9341 Liquid Crystal 
  *          Display Modules of STM32F429I-DISCO kit (MB1075).
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f429i_discovery_lcd.h"
#include "includes.h"

/** @addtogroup Utilities
  * @{
  */ 

/** @addtogroup STM32F4_DISCOVERY
  * @{
  */ 

/** @addtogroup STM32F429I_DISCOVERY
  * @{
  */
    
/** @defgroup STM32F429I_DISCOVERY_LCD 
  * @brief This file includes the LCD driver for (ILI9341) 
  * @{
  */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Private_Defines
  * @{
  */

#define POLY_Y(Z)          ((int32_t)((Points + Z)->X))
#define POLY_X(Z)          ((int32_t)((Points + Z)->Y))   
/**
  * @}
  */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Private_Macros
  * @{
  */
#define ABS(X)  ((X) > 0 ? (X) : -(X))    
/**
  * @}
  */ 
  
/** @defgroup STM32F429I_DISCOVERY_LCD_Private_Variables
  * @{
  */ 
static sFONT *LCD_Currentfonts;
/* Global variables to set the written text color */
static uint16_t CurrentTextColor   = 0x0000;
static uint16_t CurrentBackColor   = 0xFFFF;
/* Default LCD configuration with LCD Layer 1 */
static uint32_t CurrentFrameBuffer = LCD_FRAME_BUFFER;
static uint32_t CurrentLayer = LCD_BACKGROUND_LAYER;


uint16_t HPW = 2;
uint16_t VS  = 2;
uint16_t HBP = 20;
uint16_t VBP = 20;
uint16_t HFP = 20;
uint16_t VFP = 20;
uint16_t WIDTH = 480;
uint16_t HEIGHT = 854;


/**
  * @}
  */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Private_FunctionPrototypes
  * @{
  */ 
#ifndef USE_Delay
static void delay(__IO uint32_t nCount);
#endif /* USE_Delay*/

static void PutPixel(int16_t x, int16_t y);
static void LCD_PolyLineRelativeClosed(pPoint Points, uint16_t PointCount, uint16_t Closed);
static void LCD_AF_GPIOConfig(void);
void ST7701_PanelInitialCode (void);
/**
  * @}
  */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Private_Functions
  * @{
  */ 

/**
  * @brief  DeInitializes the LCD.
  * @param  None
  * @retval None
  */
void LCD_DeInit(void)
{ 
  GPIO_InitTypeDef GPIO_InitStructure;

  /* LCD Display Off */
  LCD_DisplayOff();

  /* LCD_SPI disable */
  SPI_Cmd(LCD_SPI, DISABLE);
  
  /* LCD_SPI DeInit */
  SPI_I2S_DeInit(LCD_SPI);
   
  /* Disable SPI clock  */
  RCC_APB2PeriphClockCmd(LCD_SPI_CLK, DISABLE);
    
  /* Configure NCS in Output Push-Pull mode */
  GPIO_InitStructure.GPIO_Pin = LCD_NCS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(LCD_NCS_GPIO_PORT, &GPIO_InitStructure);
   
  /* Configure SPI pins: SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Pin = LCD_SPI_SCK_PIN;
  GPIO_Init(LCD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_SPI_MISO_PIN;
  GPIO_Init(LCD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = LCD_SPI_MOSI_PIN;
  GPIO_Init(LCD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /* GPIOA configuration */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_MCO);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6 |
                                GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* GPIOB configuration */
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_MCO);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_8    |
                                GPIO_Pin_9  |  GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /* GPIOC configuration */
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_MCO);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6  | GPIO_Pin_7  | GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  /* GPIOD configuration */
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_MCO);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3  | GPIO_Pin_6; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);  

  /* GPIOF configuration */
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource10, GPIO_AF_MCO);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOF, &GPIO_InitStructure);  

  /* GPIOG configuration */
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource6, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource7, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource10, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_MCO);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource12, GPIO_AF_MCO);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6  | GPIO_Pin_7  | GPIO_Pin_10    |
                                GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
}


/**
  * @brief  Initializes the LCD.
  * @param  None
  * @retval None
  */
void LCD_Init(void)
{ 
  LTDC_InitTypeDef       LTDC_InitStruct;
  
  /* Configure the LCD Control pins ------------------------------------------*/
  LCD_CtrlLinesConfig();
  LCD_ChipSelect(DISABLE);
  LCD_ChipSelect(ENABLE);

  /* Configure the LCD_SPI interface -----------------------------------------*/
  LCD_SPIConfig(); 
  /* Power on the LCD --------------------------------------------------------*/
  //LCD_PowerOn();
  //ili9806g_lcd_init();
	
  ST7701_PanelInitialCode();
  
  /* Enable the LTDC Clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_LTDC, ENABLE);
  
  /* Enable the DMA2D Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2D, ENABLE); 

  /* Configure the LCD Control pins */
  LCD_AF_GPIOConfig(); 
  
  
  /* Configure the FMC Parallel interface : SDRAM is used as Frame Buffer for LCD */
  /*SDRAM_Init();*/
  bsp_InitExtSDRAM();
  
  /* LTDC Configuration *********************************************************/  
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AL;     
  /* Initialize the vertical synchronization polarity as active low */  
  LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AL;     
  /* Initialize the data enable polarity as active low */
  LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AL;     
  /* Initialize the pixel clock polarity as input pixel clock */ 
  LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IPC;
  
  /* Configure R,G,B component values for LCD background color */                   
  LTDC_InitStruct.LTDC_BackgroundRedValue = 0;            
  LTDC_InitStruct.LTDC_BackgroundGreenValue = 0;          
  LTDC_InitStruct.LTDC_BackgroundBlueValue = 0;  
  
  /* Configure PLLSAI prescalers for LCD */
  /* Enable Pixel Clock */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAI_N = 192 Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAI_R = 192/4 = 48 Mhz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 48/8 = 6 Mhz */
  RCC_PLLSAIConfig(/*288*/ /* 320 */ 400, 7, 2); 
  RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);
  
  /* Enable PLLSAI Clock */
  RCC_PLLSAICmd(ENABLE);
  /* Wait for PLLSAI activation */
  while(RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET)
  {
  }
  
  /* Timing configuration */  
  /* Configure horizontal synchronization width */     
  LTDC_InitStruct.LTDC_HorizontalSync = HPW - 1;
  /* Configure vertical synchronization height */
  LTDC_InitStruct.LTDC_VerticalSync = VS;
  /* Configure accumulated horizontal back porch */
  LTDC_InitStruct.LTDC_AccumulatedHBP = HPW - 1 + HBP; // HBP 20
  /* Configure accumulated vertical back porch */
  LTDC_InitStruct.LTDC_AccumulatedVBP = VS + VBP; // VBP 80
  /* Configure accumulated active width */  
  LTDC_InitStruct.LTDC_AccumulatedActiveW = HPW - 1 + HBP + WIDTH;
  /* Configure accumulated active height */
  LTDC_InitStruct.LTDC_AccumulatedActiveH = VS + VBP + HEIGHT;
  /* Configure total width */
  LTDC_InitStruct.LTDC_TotalWidth = HPW - 1 + HBP + WIDTH + HFP; // HFP 20
  /* Configure total height */
  LTDC_InitStruct.LTDC_TotalHeigh = VS + VBP + HEIGHT + VFP; // VFP 80
  
  LTDC_Init(&LTDC_InitStruct);

}  

/**
  * @brief  Initializes the LCD Layers.
  * @param  None
  * @retval None
  */
void LCD_LayerInit(void)
{
  LTDC_Layer_InitTypeDef LTDC_Layer_InitStruct; 
  
  /* Windowing configuration */
  /* In this case all the active display area is used to display a picture then :
  Horizontal start = horizontal synchronization + Horizontal back porch = 30 
  Horizontal stop = Horizontal start + window width -1 = 30 + 240 -1
  Vertical start   = vertical synchronization + vertical back porch     = 4
  Vertical stop   = Vertical start + window height -1  = 4 + 320 -1      */      
  LTDC_Layer_InitStruct.LTDC_HorizontalStart = HPW + HBP;
  LTDC_Layer_InitStruct.LTDC_HorizontalStop = (LCD_PIXEL_WIDTH + HPW + HBP - 1); 
  LTDC_Layer_InitStruct.LTDC_VerticalStart = VS + VBP;
  LTDC_Layer_InitStruct.LTDC_VerticalStop = (LCD_PIXEL_HEIGHT + VS + VBP - 1);
  
  /* Pixel Format configuration*/
  LTDC_Layer_InitStruct.LTDC_PixelFormat = LTDC_Pixelformat_RGB565;
  /* Alpha constant (255 totally opaque) */
  LTDC_Layer_InitStruct.LTDC_ConstantAlpha = 255; 
  /* Default Color configuration (configure A,R,G,B component values) */          
  LTDC_Layer_InitStruct.LTDC_DefaultColorBlue = 0;        
  LTDC_Layer_InitStruct.LTDC_DefaultColorGreen = 0;       
  LTDC_Layer_InitStruct.LTDC_DefaultColorRed = 0;         
  LTDC_Layer_InitStruct.LTDC_DefaultColorAlpha = 0;
  /* Configure blending factors */       
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_CA;    
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_CA;
  
  /* the length of one line of pixels in bytes + 3 then :
  Line Lenth = Active high width x number of bytes per pixel + 3 
  Active high width         = LCD_PIXEL_WIDTH 
  number of bytes per pixel = 2    (pixel_format : RGB565) 
  */
  LTDC_Layer_InitStruct.LTDC_CFBLineLength = ((LCD_PIXEL_WIDTH * 2) + 3);
  /* the pitch is the increment from the start of one line of pixels to the 
  start of the next line in bytes, then :
  Pitch = Active high width x number of bytes per pixel */ 
  LTDC_Layer_InitStruct.LTDC_CFBPitch = (LCD_PIXEL_WIDTH * 2);
  
  /* Configure the number of lines */  
  LTDC_Layer_InitStruct.LTDC_CFBLineNumber = LCD_PIXEL_HEIGHT;
  
  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM */    
  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = LCD_FRAME_BUFFER;
  
  /* Initialize LTDC layer 1 */
  LTDC_LayerInit(LTDC_Layer1, &LTDC_Layer_InitStruct);
  
  /* Configure Layer2 */
  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset */     
  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = LCD_FRAME_BUFFER + BUFFER_OFFSET;
  
  /* Configure blending factors */       
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_PAxCA;    
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_PAxCA;
  
  /* Initialize LTDC layer 2 */
  LTDC_LayerInit(LTDC_Layer2, &LTDC_Layer_InitStruct); 
  
  /* LTDC configuration reload */  
  LTDC_ReloadConfig(LTDC_IMReload);
  
  /* Enable foreground & background Layers */
  LTDC_LayerCmd(LTDC_Layer1, ENABLE); 
  LTDC_LayerCmd(LTDC_Layer2, ENABLE); 
  
  /* LTDC configuration reload */  
  LTDC_ReloadConfig(LTDC_IMReload);
  
  /* Set default font */    
  //LCD_SetFont(&LCD_DEFAULT_FONT); 
  
  /* dithering activation */
  LTDC_DitherCmd(ENABLE);
}

#define HPW  2
#define VS   2
#define HBP  20
#define VBP  8
#define HFP  20
#define VFP  8
#define WIDTH  854
#define HEIGHT 480


void LCD_Config(void)
{
  LTDC_InitTypeDef               LTDC_InitStruct;
  LTDC_Layer_InitTypeDef         LTDC_Layer_InitStruct;
  GPIO_InitTypeDef GPIO_InitStructure;
	
 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  /* Configure NCS in Output Push-Pull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  /* Configure the LCD Control pins ------------------------------------------*/
  LCD_CtrlLinesConfig();
  LCD_ChipSelect(DISABLE);
  LCD_ChipSelect(ENABLE);

  /* Configure the LCD_SPI interface -----------------------------------------*/
  LCD_SPIConfig(); 
 
  /* Power on the LCD --------------------------------------------------------*/
  //LCD_PowerOn();
  ST7701_PanelInitialCode(); 
  //ili9806g_lcd_init();
	
  /* Enable the LTDC Clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_LTDC, ENABLE);
  
  /* Enable the DMA2D Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2D, ENABLE); 
  
  /* Configure the LCD Control pins */
  LCD_AF_GPIOConfig();  

  /* Configure the FMC Parallel interface : SDRAM is used as Frame Buffer for LCD */
  /*SDRAM_Init();*/
  bsp_InitExtSDRAM();

  /* Enable the DMA2D Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2D, ENABLE); 
  
/* Enable Pixel Clock --------------------------------------------------------*/

  /* Configure PLLSAI prescalers for LCD */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAI_N = 192 Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAI_R = 192/3 = 64 Mhz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 64/8 = 8 Mhz */
  RCC_PLLSAIConfig(288, 7, 2);
  RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);
  
  /* Enable PLLSAI Clock */
  RCC_PLLSAICmd(ENABLE);
  /* Wait for PLLSAI activation */
  while(RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET)
  {
  }

/* LTDC Initialization -------------------------------------------------------*/

  /* Initialize the horizontal synchronization polarity as active low*/
  LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AH;     
  /* Initialize the vertical synchronization polarity as active low */  
  LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AH;     
  /* Initialize the data enable polarity as active low */ 
  LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AH;     
  /* Initialize the pixel clock polarity as input pixel clock */ 
  LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IPC;
  
  /* Timing configuration */
    /* Configure horizontal synchronization width */     
  LTDC_InitStruct.LTDC_HorizontalSync = /*9*/HPW - 1;
  /* Configure vertical synchronization height */
  LTDC_InitStruct.LTDC_VerticalSync = /*1*/VS;
  /* Configure accumulated horizontal back porch */
  LTDC_InitStruct.LTDC_AccumulatedHBP = /*29*/HPW - 1 + HBP; 
  /* Configure accumulated vertical back porch */
  LTDC_InitStruct.LTDC_AccumulatedVBP = /*3*/VS + VBP;  
  /* Configure accumulated active width */  
  LTDC_InitStruct.LTDC_AccumulatedActiveW = /*269*/HPW - 1 + HBP + WIDTH;
  /* Configure accumulated active height */
  LTDC_InitStruct.LTDC_AccumulatedActiveH = /*323*/VS + VBP + HEIGHT;
  /* Configure total width */
  LTDC_InitStruct.LTDC_TotalWidth = /*279*/HPW - 1 + HBP + WIDTH + HFP; 
  /* Configure total height */
  LTDC_InitStruct.LTDC_TotalHeigh = /*327*/VS + VBP + HEIGHT + VFP;
  
  LTDC_Init(&LTDC_InitStruct);
  
  /* Configure R,G,B component values for LCD background color */                   
  LTDC_InitStruct.LTDC_BackgroundRedValue = 0;            
  LTDC_InitStruct.LTDC_BackgroundGreenValue = 0;          
  LTDC_InitStruct.LTDC_BackgroundBlueValue = 0; 
            
  LTDC_Init(&LTDC_InitStruct);
  
/* LTDC initialization end ---------------------------------------------------*/

/* Layer1 Configuration ------------------------------------------------------*/  
    
  /* Windowing configuration */ 
  /* In this case all the active display area is used to display a picture then :
     Horizontal start = horizontal synchronization + Horizontal back porch = 30 
     Horizontal stop = Horizontal start + window width -1 = 30 + 240 -1
     Vertical start   = vertical synchronization + vertical back porch     = 4
     Vertical stop   = Vertical start + window height -1  = 4 + 320 -1      */ 
  LTDC_Layer_InitStruct.LTDC_HorizontalStart = /*30*/HPW + HBP;
  LTDC_Layer_InitStruct.LTDC_HorizontalStop = /*(240 + 30 - 1)*/WIDTH + HPW + HBP - 1; 
  LTDC_Layer_InitStruct.LTDC_VerticalStart = /*4*/VS + VBP;
  LTDC_Layer_InitStruct.LTDC_VerticalStop = /*(320 + 4 - 1)*/HEIGHT + VS + VBP - 1;
  
  /* Pixel Format configuration*/           
  LTDC_Layer_InitStruct.LTDC_PixelFormat = LTDC_Pixelformat_RGB565;
  
  /* Alpha constant (255 totally opaque) */
  LTDC_Layer_InitStruct.LTDC_ConstantAlpha = 255; 
  
  /* Configure blending factors */       
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_CA;    
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_CA;  
  
  /* Default Color configuration (configure A,R,G,B component values) */          
  LTDC_Layer_InitStruct.LTDC_DefaultColorBlue = 0;        
  LTDC_Layer_InitStruct.LTDC_DefaultColorGreen = 0;       
  LTDC_Layer_InitStruct.LTDC_DefaultColorRed = 0;         
  LTDC_Layer_InitStruct.LTDC_DefaultColorAlpha = 0;   
   
  /* Input Address configuration */    
  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = EXT_SDRAM_ADDR;
 
  /* the length of one line of pixels in bytes + 3 then :
     Line Lenth = Active high width x number of bytes per pixel + 3 
     Active high width         = 240 
     number of bytes per pixel = 2    (pixel_format : RGB565) 
  */
  LTDC_Layer_InitStruct.LTDC_CFBLineLength = ((800 * 2) + 3);
  
  /*  the pitch is the increment from the start of one line of pixels to the 
      start of the next line in bytes, then :
      Pitch = Active high width x number of bytes per pixel     
  */
  LTDC_Layer_InitStruct.LTDC_CFBPitch = (800 * 2);  
  
  /* configure the number of lines */
  LTDC_Layer_InitStruct.LTDC_CFBLineNumber = 480;
   
  LTDC_LayerInit(LTDC_Layer1, &LTDC_Layer_InitStruct);

  LTDC_DitherCmd(ENABLE);
}

/**
  * @brief  Controls LCD Chip Select (CS) pin
  * @param  NewState CS pin state
  * @retval None
  */
void LCD_ChipSelect(FunctionalState NewState)
{
  if (NewState == DISABLE)
  {
  	#if 1
    GPIO_ResetBits(LCD_NCS_GPIO_PORT, LCD_NCS_PIN); /* CS pin low: LCD disabled */
	#else
	GPIO_ResetBits(LCD_NCSX_GPIO_PORT, LCD_NCSX_PIN);
	#endif
  }
  else
  {
  	#if 1
    GPIO_SetBits(LCD_NCS_GPIO_PORT, LCD_NCS_PIN); /* CS pin high: LCD enabled */
	#else
	GPIO_SetBits(LCD_NCSX_GPIO_PORT, LCD_NCSX_PIN);
	#endif
  }
}

/**
  * @brief  Sets the LCD Layer.
  * @param  Layerx: specifies the Layer foreground or background.
  * @retval None
  */
void LCD_SetLayer(uint32_t Layerx)
{
  if (Layerx == LCD_BACKGROUND_LAYER)
  {
    CurrentFrameBuffer = LCD_FRAME_BUFFER; 
    CurrentLayer = LCD_BACKGROUND_LAYER;
  }
  else
  {
    CurrentFrameBuffer = LCD_FRAME_BUFFER + BUFFER_OFFSET;
    CurrentLayer = LCD_FOREGROUND_LAYER;
  }
}  

/**
  * @brief  Sets the LCD Text and Background colors.
  * @param  TextColor: specifies the Text Color.
  * @param  BackColor: specifies the Background Color.
  * @retval None
  */
void LCD_SetColors(uint16_t TextColor, uint16_t BackColor)
{
  CurrentTextColor = TextColor; 
  CurrentBackColor = BackColor;
}

/**
  * @brief  Gets the LCD Text and Background colors.
  * @param  TextColor: pointer to the variable that will contain the Text 
            Color.
  * @param  BackColor: pointer to the variable that will contain the Background 
            Color.
  * @retval None
  */
void LCD_GetColors(uint16_t *TextColor, uint16_t *BackColor)
{
  *TextColor = CurrentTextColor;
  *BackColor = CurrentBackColor;
}

/**
  * @brief  Sets the Text color.
  * @param  Color: specifies the Text color code RGB(5-6-5).
  * @retval None
  */
void LCD_SetTextColor(uint16_t Color)
{
  CurrentTextColor = Color;
}

/**
  * @brief  Sets the Background color.
  * @param  Color: specifies the Background color code RGB(5-6-5).
  * @retval None
  */
void LCD_SetBackColor(uint16_t Color)
{
  CurrentBackColor = Color;
}

/**
  * @brief  Sets the Text Font.
  * @param  fonts: specifies the font to be used.
  * @retval None
  */
void LCD_SetFont(sFONT *fonts)
{
  LCD_Currentfonts = fonts;
}

/**
  * @brief  Configure the transparency.
  * @param  transparency: specifies the transparency, 
  *         This parameter must range from 0x00 to 0xFF.
  * @retval None
  */
void LCD_SetTransparency(uint8_t transparency)
{
  if (CurrentLayer == LCD_BACKGROUND_LAYER)
  {
    LTDC_LayerAlpha(LTDC_Layer1, transparency);
  }
  else
  {     
    LTDC_LayerAlpha(LTDC_Layer2, transparency);
  }
  LTDC_ReloadConfig(LTDC_IMReload);
}

/**
  * @brief  Gets the Text Font.
  * @param  None.
  * @retval the used font.
  */
sFONT *LCD_GetFont(void)
{
  return LCD_Currentfonts;
}

/**
  * @brief  Clears the selected line.
  * @param  Line: the Line to be cleared.
  *   This parameter can be one of the following values:
  *     @arg LCD_LINE_x: where x can be: 0..13 if LCD_Currentfonts is Font16x24
  *                                      0..26 if LCD_Currentfonts is Font12x12 or Font8x12
  *                                      0..39 if LCD_Currentfonts is Font8x8
  * @retval None
  */
void LCD_ClearLine(uint16_t Line)
{
  uint16_t refcolumn = 0;
  /* Send the string character by character on lCD */
  while ((refcolumn < LCD_PIXEL_WIDTH) && (((refcolumn + LCD_Currentfonts->Width)& 0xFFFF) >= LCD_Currentfonts->Width))
  {
    /* Display one character on LCD */
    LCD_DisplayChar(Line, refcolumn, ' ');
    /* Decrement the column position by 16 */
    refcolumn += LCD_Currentfonts->Width;
  }
}

/**
  * @brief  Clears the hole LCD.
  * @param  Color: the color of the background.
  * @retval None
  */
void LCD_Clear(uint16_t Color)
{
  uint32_t index = 0;
  
  /* erase memory */
  for (index = 0x00; index < BUFFER_OFFSET; index++)
  {
    *(__IO uint16_t*)(CurrentFrameBuffer + (2*index)) = Color;
  } 
}

/**
  * @brief  Sets the cursor position.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position. 
  * @retval Display Address
  */
uint32_t LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{  
  return CurrentFrameBuffer + 2*(Xpos + (LCD_PIXEL_WIDTH*Ypos));
}

/**
  * @brief  Config and Sets the color Keying.
  * @param  RGBValue: Specifies the Color reference. 
  * @retval None
  */
void LCD_SetColorKeying(uint32_t RGBValue)
{  
  LTDC_ColorKeying_InitTypeDef   LTDC_colorkeying_InitStruct;
  
  /* configure the color Keying */
  LTDC_colorkeying_InitStruct.LTDC_ColorKeyBlue = 0x0000FF & RGBValue;
  LTDC_colorkeying_InitStruct.LTDC_ColorKeyGreen = (0x00FF00 & RGBValue) >> 8;
  LTDC_colorkeying_InitStruct.LTDC_ColorKeyRed = (0xFF0000 & RGBValue) >> 16;  

  if (CurrentLayer == LCD_BACKGROUND_LAYER)
  {   
    /* Enable the color Keying for Layer1 */
    LTDC_ColorKeyingConfig(LTDC_Layer1, &LTDC_colorkeying_InitStruct, ENABLE);
    LTDC_ReloadConfig(LTDC_IMReload);
  }
  else
  {
    /* Enable the color Keying for Layer2 */
    LTDC_ColorKeyingConfig(LTDC_Layer2, &LTDC_colorkeying_InitStruct, ENABLE);
    LTDC_ReloadConfig(LTDC_IMReload);
  }
}

/**
  * @brief  Disable the color Keying.
  * @param  RGBValue: Specifies the Color reference. 
  * @retval None
  */
void LCD_ReSetColorKeying(void)
{
  LTDC_ColorKeying_InitTypeDef   LTDC_colorkeying_InitStruct;
  
  if (CurrentLayer == LCD_BACKGROUND_LAYER)
  {   
    /* Disable the color Keying for Layer1 */
    LTDC_ColorKeyingConfig(LTDC_Layer1, &LTDC_colorkeying_InitStruct, DISABLE);
    LTDC_ReloadConfig(LTDC_IMReload);
  }
  else
  {
    /* Disable the color Keying for Layer2 */
    LTDC_ColorKeyingConfig(LTDC_Layer2, &LTDC_colorkeying_InitStruct, DISABLE);
    LTDC_ReloadConfig(LTDC_IMReload);
  }
} 

/**
  * @brief  Draws a character on LCD.
  * @param  Xpos: the Line where to display the character shape.
  * @param  Ypos: start column address.
  * @param  c: pointer to the character data.
  * @retval None
  */
void LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c)
{
  uint32_t index = 0, counter = 0, xpos =0;
  uint32_t  Xaddress = 0;
  
  xpos = Xpos*LCD_PIXEL_WIDTH*2;
  Xaddress += Ypos;
  
  for(index = 0; index < LCD_Currentfonts->Height; index++)
  {
    
    for(counter = 0; counter < LCD_Currentfonts->Width; counter++)
    {
          
      if((((c[index] & ((0x80 << ((LCD_Currentfonts->Width / 12 ) * 8 ) ) >> counter)) == 0x00) &&(LCD_Currentfonts->Width <= 12))||
        (((c[index] & (0x1 << counter)) == 0x00)&&(LCD_Currentfonts->Width > 12 )))
      {
          /* Write data value to all SDRAM memory */
         *(__IO uint16_t*) (CurrentFrameBuffer + (2*Xaddress) + xpos) = CurrentBackColor;
      }
      else
      {
          /* Write data value to all SDRAM memory */
         *(__IO uint16_t*) (CurrentFrameBuffer + (2*Xaddress) + xpos) = CurrentTextColor;         
      }
      Xaddress++;
    }
      Xaddress += (LCD_PIXEL_WIDTH - LCD_Currentfonts->Width);
  }
}

/**
  * @brief  Displays one character (16dots width, 24dots height).
  * @param  Line: the Line where to display the character shape .
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..29
  * @param  Column: start column address.
  * @param  Ascii: character ascii code, must be between 0x20 and 0x7E.
  * @retval None
  */
void LCD_DisplayChar(uint16_t Line, uint16_t Column, uint8_t Ascii)
{
  Ascii -= 32;

  LCD_DrawChar(Line, Column, &LCD_Currentfonts->table[Ascii * LCD_Currentfonts->Height]);
}

/**
  * @brief  Displays a maximum of 20 char on the LCD.
  * @param  Line: the Line where to display the character shape .
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..9
  * @param  *ptr: pointer to string to display on LCD.
  * @retval None
  */
void LCD_DisplayStringLine(uint16_t Line, uint8_t *ptr)
{  
  uint16_t refcolumn = 0;
  /* Send the string character by character on lCD */
  while ((refcolumn < LCD_PIXEL_WIDTH) && ((*ptr != 0) & (((refcolumn + LCD_Currentfonts->Width) & 0xFFFF) >= LCD_Currentfonts->Width)))
  {
    /* Display one character on LCD */
    LCD_DisplayChar(Line, refcolumn, *ptr);
    /* Decrement the column position by width */
    refcolumn += LCD_Currentfonts->Width;
    /* Point on the next character */
    ptr++;
  }
}

/**
  * @brief  Sets a display window
  * @param  Xpos: specifies the X bottom left position from 0 to 240.
  * @param  Ypos: specifies the Y bottom left position from 0 to 320.
  * @param  Height: display window height, can be a value from 0 to 320.
  * @param  Width: display window width, can be a value from 0 to 240.
  * @retval None
  */
void LCD_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width)
{

  if (CurrentLayer == LCD_BACKGROUND_LAYER)
  { 
    /* reconfigure the layer1 position */
    LTDC_LayerPosition(LTDC_Layer1, Xpos, Ypos);
    LTDC_ReloadConfig(LTDC_IMReload);
    
    /* reconfigure the layer1 size */
    LTDC_LayerSize(LTDC_Layer1, Width, Height);
    LTDC_ReloadConfig(LTDC_IMReload);
 }
 else
 {   
    /* reconfigure the layer2 position */
    LTDC_LayerPosition(LTDC_Layer2, Xpos, Ypos);
    LTDC_ReloadConfig(LTDC_IMReload); 
   
   /* reconfigure the layer2 size */
    LTDC_LayerSize(LTDC_Layer2, Width, Height);
    LTDC_ReloadConfig(LTDC_IMReload);
  }
}

/**
  * @brief  Disables LCD Window mode.
  * @param  None
  * @retval None
  */
void LCD_WindowModeDisable(void)
{
  LCD_SetDisplayWindow(0, 0, LCD_PIXEL_HEIGHT, LCD_PIXEL_WIDTH); 
}

/**
  * @brief  Displays a line.
  * @param Xpos: specifies the X position, can be a value from 0 to 240.
  * @param Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param Length: line length.
  * @param Direction: line direction.
  *   This parameter can be one of the following values: LCD_DIR_HORIZONTAL or LCD_DIR_VERTICAL.
  * @retval None
  */
void LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction)
{
  DMA2D_InitTypeDef      DMA2D_InitStruct;
  
  uint32_t  Xaddress = 0;
  uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0;
  
  Xaddress = CurrentFrameBuffer + 2*(LCD_PIXEL_WIDTH*Ypos + Xpos);
 
  Red_Value = (0xF800 & CurrentTextColor) >> 11;
  Blue_Value = 0x001F & CurrentTextColor;
  Green_Value = (0x07E0 & CurrentTextColor) >> 5;

  /* Configure DMA2D */    
  DMA2D_DeInit();  
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;       
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;      
  DMA2D_InitStruct.DMA2D_OutputGreen = Green_Value;      
  DMA2D_InitStruct.DMA2D_OutputBlue = Blue_Value;     
  DMA2D_InitStruct.DMA2D_OutputRed = Red_Value;                
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;                  
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = Xaddress;                  
  
  if(Direction == LCD_DIR_HORIZONTAL)
  {                                                      
    DMA2D_InitStruct.DMA2D_OutputOffset = 0;                
    DMA2D_InitStruct.DMA2D_NumberOfLine = 1;            
    DMA2D_InitStruct.DMA2D_PixelPerLine = Length; 
  }
  else
  {                                                            
    DMA2D_InitStruct.DMA2D_OutputOffset = LCD_PIXEL_WIDTH - 1;                
    DMA2D_InitStruct.DMA2D_NumberOfLine = Length;            
    DMA2D_InitStruct.DMA2D_PixelPerLine = 1;  
  }
  
  DMA2D_Init(&DMA2D_InitStruct);  
  /* Start Transfer */ 
  DMA2D_StartTransfer();  
  /* Wait for CTC Flag activation */
  while(DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET)
  {
  }
  
}

/**
  * @brief  Displays a rectangle.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Height: display rectangle height, can be a value from 0 to 320.
  * @param  Width: display rectangle width, can be a value from 0 to 240.
  * @retval None
  */
void LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width)
{
  /* draw horizontal lines */
  LCD_DrawLine(Xpos, Ypos, Width, LCD_DIR_HORIZONTAL);
  LCD_DrawLine(Xpos, (Ypos+ Height), Width, LCD_DIR_HORIZONTAL);
  
  /* draw vertical lines */
  LCD_DrawLine(Xpos, Ypos, Height, LCD_DIR_VERTICAL);
  LCD_DrawLine((Xpos + Width), Ypos, Height, LCD_DIR_VERTICAL);
}

/**
  * @brief  Draw a circle.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Radius: radius of the circle.
  * @retval None
  */
void LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
    int x = -Radius, y = 0, err = 2-2*Radius, e2;
    do {
        *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos+y)))) = CurrentTextColor; 
        *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos+y)))) = CurrentTextColor;
        *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos-y)))) = CurrentTextColor;
        *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos-y)))) = CurrentTextColor;
      
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    }
    while (x <= 0);
}

/**
  * @brief  Draw a full ellipse.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Radius: minor radius of ellipse.
  * @param  Radius2: major radius of ellipse.  
  * @retval None
  */
void LCD_DrawFullEllipse(int Xpos, int Ypos, int Radius, int Radius2)
{
  int x = -Radius, y = 0, err = 2-2*Radius, e2;
  float K = 0, rad1 = 0, rad2 = 0;
  
  rad1 = Radius;
  rad2 = Radius2;
  
  if (Radius > Radius2)
  { 
    do 
    {
      K = (float)(rad1/rad2);
      LCD_DrawLine((Xpos+x), (Ypos-(uint16_t)(y/K)), (2*(uint16_t)(y/K) + 1), LCD_DIR_VERTICAL);
      LCD_DrawLine((Xpos-x), (Ypos-(uint16_t)(y/K)), (2*(uint16_t)(y/K) + 1), LCD_DIR_VERTICAL);
      
      e2 = err;
      if (e2 <= y) 
      {
        err += ++y*2+1;
        if (-x == y && e2 <= x) e2 = 0;
      }
      if (e2 > x) err += ++x*2+1;
      
    }
    while (x <= 0);
  }
  else
  {
    y = -Radius2; 
    x = 0;
    do 
    { 
      K = (float)(rad2/rad1);       
      LCD_DrawLine((Xpos-(uint16_t)(x/K)), (Ypos+y), (2*(uint16_t)(x/K) + 1), LCD_DIR_HORIZONTAL);
      LCD_DrawLine((Xpos-(uint16_t)(x/K)), (Ypos-y), (2*(uint16_t)(x/K) + 1), LCD_DIR_HORIZONTAL);
      
      e2 = err;
      if (e2 <= x) 
      {
        err += ++x*2+1;
        if (-y == x && e2 <= y) e2 = 0;
      }
      if (e2 > y) err += ++y*2+1;
    }
    while (y <= 0);
  }
}

/**
  * @brief  Displays an Ellipse.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Radius: specifies Radius.
  * @param  Radius2: specifies Radius2.
  * @retval None
  */
void LCD_DrawEllipse(int Xpos, int Ypos, int Radius, int Radius2)
{
  int x = -Radius, y = 0, err = 2-2*Radius, e2;
  float K = 0, rad1 = 0, rad2 = 0;
   
  rad1 = Radius;
  rad2 = Radius2;
  
  if (Radius > Radius2)
  { 
    do {
      K = (float)(rad1/rad2);
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos+(uint16_t)(y/K))))) = CurrentTextColor; 
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos+(uint16_t)(y/K))))) = CurrentTextColor;
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos-(uint16_t)(y/K))))) = CurrentTextColor;
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos-(uint16_t)(y/K))))) = CurrentTextColor;
            
      e2 = err;
      if (e2 <= y) {
        err += ++y*2+1;
        if (-x == y && e2 <= x) e2 = 0;
      }
      if (e2 > x) err += ++x*2+1;
    }
    while (x <= 0);
  }
  else
  {
    y = -Radius2; 
    x = 0;
    do { 
      K = (float)(rad2/rad1);
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-(uint16_t)(x/K)) + LCD_PIXEL_WIDTH*(Ypos+y)))) = CurrentTextColor; 
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+(uint16_t)(x/K)) + LCD_PIXEL_WIDTH*(Ypos+y)))) = CurrentTextColor;
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+(uint16_t)(x/K)) + LCD_PIXEL_WIDTH*(Ypos-y)))) = CurrentTextColor;
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-(uint16_t)(x/K)) + LCD_PIXEL_WIDTH*(Ypos-y)))) = CurrentTextColor;
      
      e2 = err;
      if (e2 <= x) {
        err += ++x*2+1;
        if (-y == x && e2 <= y) e2 = 0;
      }
      if (e2 > y) err += ++y*2+1;     
    }
    while (y <= 0);
  }
}

/**
  * @brief  Displays a mono-color picture.
  * @param  Pict: pointer to the picture array.
  * @retval None
  */
void LCD_DrawMonoPict(const uint32_t *Pict)
{
  uint32_t index = 0, counter = 0;
  
   
  for(index = 0; index < 2400; index++)
  {
    for(counter = 0; counter < 32; counter++)
    {
      if((Pict[index] & (1 << counter)) == 0x00)
      {
        *(__IO uint16_t*)(CurrentFrameBuffer) = CurrentBackColor;
      }
      else
      {
        *(__IO uint16_t*)(CurrentFrameBuffer) = CurrentTextColor;
      }
    }
  }
}

/**
  * @brief  Displays a bitmap picture loaded in the internal Flash.
  * @param  BmpAddress: Bmp picture address in the internal Flash.
  * @retval None
  */
void LCD_WriteBMP(uint32_t BmpAddress)
{
  uint32_t index = 0, size = 0, width = 0, height = 0, bit_pixel = 0;
  uint32_t Address;
  uint32_t currentline = 0, linenumber = 0;
 
  Address = CurrentFrameBuffer;

  /* Read bitmap size */
  size = *(__IO uint16_t *) (BmpAddress + 2);
  size |= (*(__IO uint16_t *) (BmpAddress + 4)) << 16;
  
  /* Get bitmap data address offset */
  index = *(__IO uint16_t *) (BmpAddress + 10);
  index |= (*(__IO uint16_t *) (BmpAddress + 12)) << 16;

  /* Read bitmap width */
  width = *(uint16_t *) (BmpAddress + 18);
  width |= (*(uint16_t *) (BmpAddress + 20)) << 16;

  /* Read bitmap height */
  height = *(uint16_t *) (BmpAddress + 22);
  height |= (*(uint16_t *) (BmpAddress + 24)) << 16;

  /* Read bit/pixel */
  bit_pixel = *(uint16_t *) (BmpAddress + 28);  
 
  if (CurrentLayer == LCD_BACKGROUND_LAYER)
  {
    /* reconfigure layer size in accordance with the picture */
    LTDC_LayerSize(LTDC_Layer1, width, height);
    LTDC_ReloadConfig(LTDC_VBReload);

    /* Reconfigure the Layer pixel format in accordance with the picture */    
    if ((bit_pixel/8) == 4)
    {
      LTDC_LayerPixelFormat(LTDC_Layer1, LTDC_Pixelformat_ARGB8888);
      LTDC_ReloadConfig(LTDC_VBReload);
    }
    else if ((bit_pixel/8) == 2)
    {
      LTDC_LayerPixelFormat(LTDC_Layer1, LTDC_Pixelformat_RGB565);
      LTDC_ReloadConfig(LTDC_VBReload);
    }
    else 
    {
      LTDC_LayerPixelFormat(LTDC_Layer1, LTDC_Pixelformat_RGB888);
      LTDC_ReloadConfig(LTDC_VBReload);
    } 
  }
  else
  {
    /* reconfigure layer size in accordance with the picture */
    LTDC_LayerSize(LTDC_Layer2, width, height);
    LTDC_ReloadConfig(LTDC_VBReload); 
    
    /* Reconfigure the Layer pixel format in accordance with the picture */
    if ((bit_pixel/8) == 4)
    {
      LTDC_LayerPixelFormat(LTDC_Layer2, LTDC_Pixelformat_ARGB8888);
      LTDC_ReloadConfig(LTDC_VBReload);
    }
    else if ((bit_pixel/8) == 2)
    {
      LTDC_LayerPixelFormat(LTDC_Layer2, LTDC_Pixelformat_RGB565);
      LTDC_ReloadConfig(LTDC_VBReload);
    }
    else
    {
      LTDC_LayerPixelFormat(LTDC_Layer2, LTDC_Pixelformat_RGB888);
      LTDC_ReloadConfig(LTDC_VBReload);  
    }
  }
 
  /* compute the real size of the picture (without the header)) */  
  size = (size - index); 

  /* bypass the bitmap header */
  BmpAddress += index;

  /* start copie image from the bottom */
  Address += width*(height-1)*(bit_pixel/8);
  
  for(index = 0; index < size; index++)
  {
    *(__IO uint8_t*) (Address) = *(__IO uint8_t *)BmpAddress;
    
    /*jump on next byte */   
    BmpAddress++;
    Address++;
    currentline++;
    
    if((currentline/(bit_pixel/8)) == width)
    {
      if(linenumber < height)
      {
        linenumber++;
        Address -=(2*width*(bit_pixel/8));
        currentline = 0;
      }
    }
  }
}

/**
  * @brief  Displays a full rectangle.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Height: rectangle height.
  * @param  Width: rectangle width.
  * @retval None
  */
void LCD_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  DMA2D_InitTypeDef      DMA2D_InitStruct;
  
  uint32_t  Xaddress = 0; 
  uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0;
 
  Red_Value = (0xF800 & CurrentTextColor) >> 11;
  Blue_Value = 0x001F & CurrentTextColor;
  Green_Value = (0x07E0 & CurrentTextColor) >> 5;
  
  Xaddress = CurrentFrameBuffer + 2*(LCD_PIXEL_WIDTH*Ypos + Xpos);
  
  /* configure DMA2D */
  DMA2D_DeInit();
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;       
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;      
  DMA2D_InitStruct.DMA2D_OutputGreen = Green_Value;      
  DMA2D_InitStruct.DMA2D_OutputBlue = Blue_Value;     
  DMA2D_InitStruct.DMA2D_OutputRed = Red_Value;                
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;                  
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = Xaddress;                
  DMA2D_InitStruct.DMA2D_OutputOffset = (LCD_PIXEL_WIDTH - Width);                
  DMA2D_InitStruct.DMA2D_NumberOfLine = Height;            
  DMA2D_InitStruct.DMA2D_PixelPerLine = Width;
  DMA2D_Init(&DMA2D_InitStruct); 
  
  /* Start Transfer */ 
  DMA2D_StartTransfer();
  
  /* Wait for CTC Flag activation */
  while(DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET)
  {
  } 

  LCD_SetTextColor(CurrentTextColor);
}

/**
  * @brief  Displays a full circle.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Radius
  * @retval None
  */
void LCD_DrawFullCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  int32_t  D;    /* Decision Variable */ 
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */ 
  
  D = 3 - (Radius << 1);
  
  CurX = 0;
  CurY = Radius;
  
  while (CurX <= CurY)
  {
    if(CurY > 0) 
    {
      LCD_DrawLine(Xpos - CurX, Ypos - CurY, 2*CurY, LCD_DIR_VERTICAL);
      LCD_DrawLine(Xpos + CurX, Ypos - CurY, 2*CurY, LCD_DIR_VERTICAL);
    }
    
    if(CurX > 0) 
    {
      LCD_DrawLine(Xpos - CurY, Ypos - CurX, 2*CurX, LCD_DIR_VERTICAL);
      LCD_DrawLine(Xpos + CurY, Ypos - CurX, 2*CurX, LCD_DIR_VERTICAL);
    }
    if (D < 0)
    { 
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }
  
  LCD_DrawCircle(Xpos, Ypos, Radius);  
}

/**
  * @brief  Displays an uni-line (between two points).
  * @param  x1: specifies the point 1 x position.
  * @param  y1: specifies the point 1 y position.
  * @param  x2: specifies the point 2 x position.
  * @param  y2: specifies the point 2 y position.
  * @retval None
  */
void LCD_DrawUniLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
  curpixel = 0;
  
  deltax = ABS(x2 - x1);        /* The difference between the x's */
  deltay = ABS(y2 - y1);        /* The difference between the y's */
  x = x1;                       /* Start x off at the first pixel */
  y = y1;                       /* Start y off at the first pixel */
  
  if (x2 >= x1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }
  
  if (y2 >= y1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }
  
  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    numadd = deltax;
    numpixels = deltay;         /* There are more y-values than x-values */
  }
  
  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    PutPixel(x, y);             /* Draw the current pixel */
    num += numadd;              /* Increase the numerator by the top of the fraction */
    if (num >= den)             /* Check if numerator >= denominator */
    {
      num -= den;               /* Calculate the new numerator value */
      x += xinc1;               /* Change the x as appropriate */
      y += yinc1;               /* Change the y as appropriate */
    }
    x += xinc2;                 /* Change the x as appropriate */
    y += yinc2;                 /* Change the y as appropriate */
  }
}

/**
  * @brief  Displays an triangle.
  * @param  Points: pointer to the points array.
  * @retval None
  */
void LCD_Triangle(pPoint Points, uint16_t PointCount)
{
  int16_t X = 0, Y = 0;
  pPoint First = Points;

  if(PointCount != 3)
  {
    return;
  }

  while(--PointCount)
  {
    X = Points->X;
    Y = Points->Y;
    Points++;
    LCD_DrawUniLine(X, Y, Points->X, Points->Y);
  }
  LCD_DrawUniLine(First->X, First->Y, Points->X, Points->Y);
}

/**
  * @brief  Fill an triangle (between 3 points).
  * @param  x1..3: x position of triangle point 1..3.
  * @param  y1..3: y position of triangle point 1..3.
  * @retval None
  */
void LCD_FillTriangle(uint16_t x1, uint16_t x2, uint16_t x3, uint16_t y1, uint16_t y2, uint16_t y3)
{ 
  
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
  curpixel = 0;
  
  deltax = ABS(x2 - x1);        /* The difference between the x's */
  deltay = ABS(y2 - y1);        /* The difference between the y's */
  x = x1;                       /* Start x off at the first pixel */
  y = y1;                       /* Start y off at the first pixel */
  
  if (x2 >= x1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }
  
  if (y2 >= y1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }
  
  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    numadd = deltax;
    numpixels = deltay;         /* There are more y-values than x-values */
  }
  
  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    LCD_DrawUniLine(x, y, x3, y3);
    
    num += numadd;              /* Increase the numerator by the top of the fraction */
    if (num >= den)             /* Check if numerator >= denominator */
    {
      num -= den;               /* Calculate the new numerator value */
      x += xinc1;               /* Change the x as appropriate */
      y += yinc1;               /* Change the y as appropriate */
    }
    x += xinc2;                 /* Change the x as appropriate */
    y += yinc2;                 /* Change the y as appropriate */
  }  
  
  
}
/**
  * @brief  Displays an poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_PolyLine(pPoint Points, uint16_t PointCount)
{
  int16_t X = 0, Y = 0;

  if(PointCount < 2)
  {
    return;
  }

  while(--PointCount)
  {
    X = Points->X;
    Y = Points->Y;
    Points++;
    LCD_DrawUniLine(X, Y, Points->X, Points->Y);
  }
}

/**
  * @brief  Displays an relative poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @param  Closed: specifies if the draw is closed or not.
  *           1: closed, 0 : not closed.
  * @retval None
  */
static void LCD_PolyLineRelativeClosed(pPoint Points, uint16_t PointCount, uint16_t Closed)
{
  int16_t X = 0, Y = 0;
  pPoint First = Points;

  if(PointCount < 2)
  {
    return;
  }  
  X = Points->X;
  Y = Points->Y;
  while(--PointCount)
  {
    Points++;
    LCD_DrawUniLine(X, Y, X + Points->X, Y + Points->Y);
    X = X + Points->X;
    Y = Y + Points->Y;
  }
  if(Closed)
  {
    LCD_DrawUniLine(First->X, First->Y, X, Y);
  }  
}

/**
  * @brief  Displays a closed poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_ClosedPolyLine(pPoint Points, uint16_t PointCount)
{
  LCD_PolyLine(Points, PointCount);
  LCD_DrawUniLine(Points->X, Points->Y, (Points+PointCount-1)->X, (Points+PointCount-1)->Y);
}

/**
  * @brief  Displays a relative poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_PolyLineRelative(pPoint Points, uint16_t PointCount)
{
  LCD_PolyLineRelativeClosed(Points, PointCount, 0);
}

/**
  * @brief  Displays a closed relative poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_ClosedPolyLineRelative(pPoint Points, uint16_t PointCount)
{
  LCD_PolyLineRelativeClosed(Points, PointCount, 1);
}

/**
  * @brief  Displays a  full poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_FillPolyLine(pPoint Points, uint16_t PointCount)
{
 
  int16_t X = 0, Y = 0, X2 = 0, Y2 = 0, X_center = 0, Y_center = 0, X_first = 0, Y_first = 0, pixelX = 0, pixelY = 0, counter = 0;
  uint16_t  IMAGE_LEFT = 0, IMAGE_RIGHT = 0, IMAGE_TOP = 0, IMAGE_BOTTOM = 0;  

  IMAGE_LEFT = IMAGE_RIGHT = Points->X;
  IMAGE_TOP= IMAGE_BOTTOM = Points->Y;

  for(counter = 1; counter < PointCount; counter++)
  {
    pixelX = POLY_X(counter);
    if(pixelX < IMAGE_LEFT)
    {
      IMAGE_LEFT = pixelX;
    }
    if(pixelX > IMAGE_RIGHT)
    {
      IMAGE_RIGHT = pixelX;
    }
    
    pixelY = POLY_Y(counter);
    if(pixelY < IMAGE_TOP)
    { 
      IMAGE_TOP = pixelY;
    }
    if(pixelY > IMAGE_BOTTOM)
    {
      IMAGE_BOTTOM = pixelY;
    }
  }  
  
  if(PointCount < 2)
  {
    return;
  }
  
  X_center = (IMAGE_LEFT + IMAGE_RIGHT)/2;
  Y_center = (IMAGE_BOTTOM + IMAGE_TOP)/2;
 
  X_first = Points->X;
  Y_first = Points->Y;
  
  while(--PointCount)
  {
    X = Points->X;
    Y = Points->Y;
    Points++;
    X2 = Points->X;
    Y2 = Points->Y;    
  
    LCD_FillTriangle(X, X2, X_center, Y, Y2, Y_center);
    LCD_FillTriangle(X, X_center, X2, Y, Y_center, Y2);
    LCD_FillTriangle(X_center, X2, X, Y_center, Y2, Y);   
  }
  
  LCD_FillTriangle(X_first, X2, X_center, Y_first, Y2, Y_center);
  LCD_FillTriangle(X_first, X_center, X2, Y_first, Y_center, Y2);
  LCD_FillTriangle(X_center, X2, X_first, Y_center, Y2, Y_first); 
}

/**
  * @brief  Writes command to select the LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */
void LCD_WriteCommand(uint8_t LCD_Reg)
{
  /* Reset WRX to send command */
  LCD_CtrlLinesWrite(LCD_WRX_GPIO_PORT, LCD_WRX_PIN, Bit_RESET);

  /* Reset LCD control line(/CS) and Send command */
  LCD_ChipSelect(DISABLE);
  
  SPI_I2S_SendData(LCD_SPI, LCD_Reg);
  
  /* Wait until a data is sent(not busy), before config /CS HIGH */
  
  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET);
  
  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET);

  LCD_ChipSelect(ENABLE);
}

/**
  * @brief  Writes data to select the LCD register.
  *         This function must be used after LCD_WriteCommand() function
  * @param  value: data to write to the selected register.
  * @retval None
  */
void LCD_WriteData(uint8_t value)
{
    /* Set WRX to send data */
  LCD_CtrlLinesWrite(LCD_WRX_GPIO_PORT, LCD_WRX_PIN, Bit_SET);
  
  /* Reset LCD control line(/CS) and Send data */  
  LCD_ChipSelect(DISABLE);
	
  SPI_I2S_SendData(LCD_SPI, value);
  
  /* Wait until a data is sent(not busy), before config /CS HIGH */
  
  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET) ;
  
  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET);
  
  LCD_ChipSelect(ENABLE);
}

void bsp_spiDelay(void)
{
#if 1
	uint32_t i;

	/*
		�ӳ�5ʱ�� F407 (168MHz��Ƶ�� GPIOģ�⣬ʵ�� SCK ���� = 480ns (��Լ2M)
	*/
	for (i = 0; i < 15; i++);
#else
	/*
		������ӳ���䣬 F407 (168MHz��Ƶ�� GPIOģ�⣬ʵ�� SCK ���� = 200ns (��Լ5M)
	*/
#endif
}

void write_command(uint8_t addr) 
{
  uint8_t i = 0;
  
  //SCK_0();
  LCD_ChipSelect(DISABLE);                   //Ƭѡ������Ч 
  									//ʱ��������
  MOSI_0(); //D/Cλ���ͣ�д����							
  SCK_0();  //ʱ��������
  bsp_spiDelay();
  SCK_1();
  for(i = 0;i < 8;i++)            			//ģ��ʱ�ӵ�������
  { 
     if (addr & 0x80)
     {
         //��Ϊ1������
          MOSI_1();          				//���Ϊ8λ��ÿһλ������������
     }
         //��Ϊ0������
     else
     {
          MOSI_0();           				//WR����
     }
	 addr <<= 1;
	 SCK_0();              				//ʱ��������	
	 bsp_spiDelay();
     SCK_1();							 	//ʱ������
  }
  
  LCD_ChipSelect(ENABLE);
}

void write_data(uint8_t data) 
{
  uint8_t i = 0;
  
  //SCK_0();
  LCD_ChipSelect(DISABLE);
  
  MOSI_1();                       //D/Cλ(WR)�� �͸ߣ�д����   
  SCK_0();
  bsp_spiDelay();
  SCK_1();

  for(i=0;i<8;i++)
  {
    if (data & 0x80)
    {
      MOSI_1();
    }
    else
    {
      MOSI_0();
    }
	data <<= 1;
	SCK_0();
	bsp_spiDelay();
	SCK_1();
  }
  
  LCD_ChipSelect(ENABLE);
}

#define LCD_WriteCommand write_command
#define LCD_WriteData write_data

/**
  * @brief  Configure the LCD controller (Power On sequence as described in ILI9341 Datasheet)
  * @param  None
  * @retval None
  */
void LCD_PowerOn(void)
{
  #if 0
  LCD_WriteCommand(0xCA);
  LCD_WriteData(0xC3);
  LCD_WriteData(0x08);
  LCD_WriteData(0x50);
  LCD_WriteCommand(LCD_POWERB);
  LCD_WriteData(0x00);
  LCD_WriteData(0xC1);
  LCD_WriteData(0x30);
  LCD_WriteCommand(LCD_POWER_SEQ);
  LCD_WriteData(0x64);
  LCD_WriteData(0x03);
  LCD_WriteData(0x12);
  LCD_WriteData(0x81);
  LCD_WriteCommand(LCD_DTCA);
  LCD_WriteData(0x85);
  LCD_WriteData(0x00);
  LCD_WriteData(0x78);
  LCD_WriteCommand(LCD_POWERA);
  LCD_WriteData(0x39);
  LCD_WriteData(0x2C);
  LCD_WriteData(0x00);
  LCD_WriteData(0x34);
  LCD_WriteData(0x02);
  LCD_WriteCommand(LCD_PRC);
  LCD_WriteData(0x20);
  LCD_WriteCommand(LCD_DTCB);
  LCD_WriteData(0x00);
  LCD_WriteData(0x00);
  LCD_WriteCommand(LCD_FRC);
  LCD_WriteData(0x00);
  LCD_WriteData(0x1B);
  LCD_WriteCommand(LCD_DFC);
  LCD_WriteData(0x0A);
  LCD_WriteData(0xA2);
  LCD_WriteCommand(LCD_POWER1);
  LCD_WriteData(0x10);
  LCD_WriteCommand(LCD_POWER2);
  LCD_WriteData(0x10);
  LCD_WriteCommand(LCD_VCOM1);
  LCD_WriteData(0x45);
  LCD_WriteData(0x15);
  LCD_WriteCommand(LCD_VCOM2);
  LCD_WriteData(0x90);
  LCD_WriteCommand(LCD_MAC);
  LCD_WriteData(0xC8);
  LCD_WriteCommand(LCD_3GAMMA_EN);
  LCD_WriteData(0x00);
  LCD_WriteCommand(LCD_RGB_INTERFACE);
  LCD_WriteData(0xC2);
  LCD_WriteCommand(LCD_DFC);
  LCD_WriteData(0x0A);
  LCD_WriteData(0xA7);
  LCD_WriteData(0x27);
  LCD_WriteData(0x04);

  /* colomn address set */
  LCD_WriteCommand(LCD_COLUMN_ADDR);
  LCD_WriteData(0x00);
  LCD_WriteData(0x00);
  LCD_WriteData(0x00);
  LCD_WriteData(0xEF);
  /* Page Address Set */
  LCD_WriteCommand(LCD_PAGE_ADDR);
  LCD_WriteData(0x00);
  LCD_WriteData(0x00);
  LCD_WriteData(0x01);
  LCD_WriteData(0x3F);
  LCD_WriteCommand(LCD_INTERFACE);
  LCD_WriteData(0x01);
  LCD_WriteData(0x00);
  LCD_WriteData(0x06);
  
  LCD_WriteCommand(LCD_GRAM);
  delay(200);
  
  LCD_WriteCommand(LCD_GAMMA);
  LCD_WriteData(0x01);
  
  LCD_WriteCommand(LCD_PGAMMA);
  LCD_WriteData(0x0F);
  LCD_WriteData(0x29);
  LCD_WriteData(0x24);
  LCD_WriteData(0x0C);
  LCD_WriteData(0x0E);
  LCD_WriteData(0x09);
  LCD_WriteData(0x4E);
  LCD_WriteData(0x78);
  LCD_WriteData(0x3C);
  LCD_WriteData(0x09);
  LCD_WriteData(0x13);
  LCD_WriteData(0x05);
  LCD_WriteData(0x17);
  LCD_WriteData(0x11);
  LCD_WriteData(0x00);
  LCD_WriteCommand(LCD_NGAMMA);
  LCD_WriteData(0x00);
  LCD_WriteData(0x16);
  LCD_WriteData(0x1B);
  LCD_WriteData(0x04);
  LCD_WriteData(0x11);
  LCD_WriteData(0x07);
  LCD_WriteData(0x31);
  LCD_WriteData(0x33);
  LCD_WriteData(0x42);
  LCD_WriteData(0x05);
  LCD_WriteData(0x0C);
  LCD_WriteData(0x0A);
  LCD_WriteData(0x28);
  LCD_WriteData(0x2F);
  LCD_WriteData(0x0F);
  
  LCD_WriteCommand(LCD_SLEEP_OUT);
  delay(200);
  LCD_WriteCommand(LCD_DISPLAY_ON);
  /* GRAM start writing */
  LCD_WriteCommand(LCD_GRAM);
  #endif
 }

#if 0
#define SPIT_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define SPIT_LONG_TIMEOUT         ((uint32_t)(10 * SPIT_FLAG_TIMEOUT))
#define LCD_ERROR(fmt,arg...)    printf("<<-FLASH-ERROR->> "fmt"\n",##arg)
static __IO unsigned int  SPITimeout = SPIT_LONG_TIMEOUT;	 
//static unsigned int SPI_TIMEOUT_UserCallback(uint8_t errorCode);

 /**
   * @brief  
   * @param  None.
   * @retval None.
   */
 static  uint16_t SPI_TIMEOUT_UserCallback(uint8_t errorCode)
 {
   LCD_ERROR("\r\nSPI timeout!errorCode = %d",errorCode);
   return 0;
 }

 /**
  * @brief  
  * @param  
  * @retval 
  */
static uint8_t SPI_LCD_SendByte(u8 byte)
{
  SPITimeout = SPIT_FLAG_TIMEOUT;

  while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET)
  {
    if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(0);
  }

  SPI_I2S_SendData(LCD_SPI, byte);

  SPITimeout = SPIT_FLAG_TIMEOUT;

  
  while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  {
    if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
  }

  return SPI_I2S_ReceiveData(LCD_SPI);
}

/**
  * @brief  
  * @param 
  * @retval 
  */
static uint8_t SPI_LCD_ReadByte(void)
{
  return (SPI_LCD_SendByte(0xFF));
}
#endif


/**
  * @brief  ST7701_PanelInitialCode
  * @param  None
  * @retval None
  */

#define LCD_Nreset_On   (GPIO_SetBits(LCD_NRESET_GPIO_PORT, LCD_NRESET_PIN))
#define LCD_Nreset_Off  (GPIO_ResetBits(LCD_NRESET_GPIO_PORT, LCD_NRESET_PIN))
#define LCD_BKL_On   (GPIO_SetBits(LED_BKL_GPIO_PORT, LED_BKL_PIN))
#define LCD_BKL_Off  (GPIO_ResetBits(LED_BKL_GPIO_PORT, LED_BKL_PIN))

#if 0

#define W_C    	write_command
#define W_D		write_data

void ili9806g_lcd_init(void)
{
	OS_ERR      err;
	
	OSTimeDly(50, OS_OPT_TIME_DLY, &err); 						//��ʱһ�� 
	LCD_Nreset_On;
	OSTimeDly(110, OS_OPT_TIME_DLY, &err); 
	LCD_Nreset_Off;						//Һ������λ
	OSTimeDly(150, OS_OPT_TIME_DLY, &err); 					//��ʱһ�� 
	LCD_Nreset_On;
	OSTimeDly(200, OS_OPT_TIME_DLY, &err); 	//��ʱһ�� 
	/****LCD�����ṩ�ĳ�ʼ������(ILI9806G�Ĵ���оƬ)****/
	W_C(0xFF); // EXTC Command Set enable register
	W_D(0xFF);
	W_D(0x98);
	W_D(0x06);

	W_C(0xBA); // SPI Interface Setting
	W_D(0xE0);

	W_C(0xB0);
	W_D(0x0C);

	W_C(0xB1);
	W_D(0x01);
	W_D(0x0F);
	W_D(0x14);

	W_C(0xB8);
	W_D(0x00);

	W_C(0xBC); // GIP 1
	W_D(0x01);
	W_D(0x0E);
	W_D(0x61);
	W_D(0xFB);
	W_D(0x10);
	W_D(0x10);
	W_D(0x0B);
	W_D(0x0F);
	W_D(0x2E);
	W_D(0x73);
	W_D(0xFF);
	W_D(0xFF);
	W_D(0x0E);
	W_D(0x0E);
	W_D(0x00);
	W_D(0x03);
	W_D(0x66);
	W_D(0x63);
	W_D(0x01);
	W_D(0x00);
	W_D(0x00);

	W_C(0xBD); // GIP 2
	W_D(0x01);
	W_D(0x23);
	W_D(0x45);
	W_D(0x67);
	W_D(0x01);
	W_D(0x23);
	W_D(0x45);
	W_D(0x67);
	W_C(0xBE); // GIP 3
	W_D(0x00);
	W_D(0x21);
	W_D(0xAB);
	W_D(0x60);
	W_D(0x22);
	W_D(0x22);
	W_D(0x22);
	W_D(0x22);
	W_D(0x22);
	W_C(0xC7); // Vcom
	W_D(0x47);

	W_C(0xED); // EN_volt_reg
	W_D(0x7F);
	W_D(0x0F);
	W_D(0x00);

	W_C(0xB6); // Display Function Control
	W_D(0x22); // 20

	W_C(0xC0); // Power Control 1
	W_D(0x37);
	W_D(0x0B);
	W_D(0x0A);
	W_C(0xFC); // LVGL
	W_D(0x0A);
	W_C(0xDF); // Engineering Setting
	W_D(0x00);
	W_D(0x00);
	W_D(0x00);
	W_D(0x00);
	W_D(0x00);
	W_D(0x20);
	W_C(0xF3); // DVDD Voltage Setting
	W_D(0x74);
	W_C(0xB4); // Display Inversion Control
	W_D(0x00);
	W_D(0x00);
	W_D(0x00);
	W_C(0xF7); // 480x800
	W_D(0x82);
	W_C(0xB1); // Frame Rate
	W_D(0x00);
	W_D(0x12);
	W_D(0x12);
	W_C(0xF2); // CR/EQ/PC
	W_D(0x80);
	W_D(0x5B);
	W_D(0x40);
	W_D(0x28);

	W_C(0xC1); // Power Control 2
	W_D(0x07);
	W_D(0x9F);
	W_D(0x71);
	W_D(0x20);
	W_C(0xE0); //Gamma
	W_D(0x00); //P1
	W_D(0x11); //P2
	W_D(0x18); //P3
	W_D(0x0C); //P4
	W_D(0x0F); //P5
	W_D(0x0D); //P6
	W_D(0x09); //P7
	W_D(0x08); //P8
	W_D(0x02); //P9
	W_D(0x06); //P10
	W_D(0x0F); //P11
	W_D(0x0E); //P12
	W_D(0x10); //P13
	W_D(0x18); //P14
	W_D(0x14); //P15
	W_D(0x00); //P16
	W_C(0xE1); //Gamma
	W_D(0x00); //P1
	W_D(0x05); //P2
	W_D(0x0D); //P3
	W_D(0x0B); //P4
	W_D(0x0D); //P5
	W_D(0x0B); //P6
	W_D(0x05); //P7
	W_D(0x03); //P8
	W_D(0x09); //P9
	W_D(0x0D); //P10
	W_D(0x0C); //P11
	W_D(0x10); //P12
	W_D(0x0B); //P13
	W_D(0x13); //P14

	W_D(0x09); //P15
	W_D(0x00); //P16
	W_C(0x35); //Tearing Effect ON
	W_D(0x00);

	W_C(0x3A); // 18bit
	W_D(0x66); 

	W_C(0x36); // Memory Access Control
	W_D(0xA0); 
	
	W_C(0x2A); // 
	W_D(0x00);
	W_D(0x00);
	W_D(0x03);
	W_D(0x20);
 
	W_C(0x2B); // 
	W_D(0x00);
	W_D(0x00);
	W_D(0x01);
	W_D(0xE0);
	
	W_C(0x11); //Exit Sleep
	OSTimeDly(120, OS_OPT_TIME_DLY, &err);
	W_C(0x29); // Display On
	W_C(0x2C);
	W_C(0x3C); 
	OSTimeDly(120, OS_OPT_TIME_DLY, &err);
	
	LCD_BKL_On;
}

#endif
//char buf[30] = {0};
#if 1

#define WriteComm    	write_command
#define WriteData		write_data

void ST7701_PanelInitialCode (void)
{
	OS_ERR  err;
	//--------------------------------------ST7701 Reset Sequence---------------------------------------//
	LCD_Nreset_On;
	OSTimeDly(1, OS_OPT_TIME_DLY, &err);
	LCD_Nreset_Off;
	OSTimeDly(1, OS_OPT_TIME_DLY, &err);
	LCD_Nreset_On;
	OSTimeDly(120, OS_OPT_TIME_DLY, &err);
	WriteComm (0x11);
	OSTimeDly(20, OS_OPT_TIME_DLY, &err);
	WriteComm (0x36);
	WriteData(0x80);
	WriteComm (0x3A);
	WriteData(0x66);
	//--------------------------------------- Bank0 Setting------------------------------------------------- //
//------------------------------------ Display Control setting----------------------------------- -----------//
	WriteComm (0xFF);
	WriteData (0x77);
	WriteData (0x01);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x10);
	
	WriteComm (0xC0);
	WriteData (0xe9);
	WriteData (0x03);
	//WriteData (0x6B);
	//WriteData (0x00);
	
	WriteComm (0xC1);
	WriteData (0x12);
	WriteData (0x02);
	WriteComm (0xC2);
	WriteData (0x37);
	WriteData (0x08);
	WriteComm (0xCC);
	WriteData (0x10);
	//----------------------------------Gamma Cluster Setting---------------------------------------- //
	WriteComm (0xB0);
	WriteData (0x00);
	WriteData (0x0E);
	WriteData (0x15);
	WriteData (0x0F);
	WriteData (0x11);
	WriteData (0x08);
	WriteData (0x08);
	WriteData (0x08);
	WriteData (0x08);
	WriteData (0x23);
	WriteData (0x04);
	WriteData (0x13);
	WriteData (0x12);
	WriteData (0x2B);
	WriteData (0x34);
	WriteData (0x1F);
	WriteComm (0xB1);
	WriteData (0x00);
	WriteData (0x0E);
	WriteData (0x95);
	WriteData (0x0F);
	WriteData (0x13);
	WriteData (0x07);
	WriteData (0x09);
	WriteData (0x08);
	WriteData (0x08);
	WriteData (0x22);
	WriteData (0x04);
	WriteData (0x10);
	WriteData (0x0E);
	WriteData (0x2C);
	WriteData (0x34);
	WriteData (0x1F);
	//------------------------------------End Gamma Setting------------------------------------------- //
	//---------------------------------End Display Control setting-------------------------------------//
	//--------------------------------------Bank0 Setting End------------------------------------------ //
	//---------------------------------------- Bank1 Setting------------------------------------------------ //
	//----------------------------- Power Control Registers Initial -----------------------------------//
	WriteComm (0xFF);
	WriteData (0x77);
	WriteData (0x01);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x11);
	WriteComm (0xB0);
	WriteData (0x4D);
	//----------------------------------------Vcom Setting------------------------------------------------ //
	WriteComm (0xB1);
	WriteData (0x13);
	//--------------------------------------End Vcom Setting-------------------------------------------- //
	WriteComm (0xB2);
	WriteData (0x07);
	WriteComm (0xB3);
	WriteData (0x80);
	WriteComm (0xB5);
	WriteData (0x47);
	WriteComm (0xB7);
	WriteData (0x85);
	WriteComm (0xB8);
	WriteData (0x20);
	WriteComm (0xB9);
	WriteData (0x10);
	WriteComm (0xC1);
	WriteData (0x78);
	WriteComm (0xC2);
	WriteData (0x78);

	//WriteComm (0xC3); // RGBCTRL high activ
	//WriteData (0xC0);

	WriteComm (0xD0);
	WriteData (0x88);
	//------------------------------End Power Control Registers Initial ----------------------------//
	OSTimeDly(100, OS_OPT_TIME_DLY, &err);
	//------------------------------------------ GIP Setting----------------------------------------- --------//
	WriteComm (0xE0);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x02);
	WriteComm (0xE1);
	WriteData (0x0B);
	WriteData (0x00);
	WriteData (0x0D);
	WriteData (0x00);
	WriteData (0x0C);
	WriteData (0x00);
	WriteData (0x0E);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x44);
	WriteData (0x44);
	WriteComm (0xE2);
	WriteData (0x33);
	WriteData (0x33);
	WriteData (0x44);
	WriteData (0x44);
	WriteData (0x64);
	WriteData (0x00);
	WriteData (0x66);
	WriteData (0x00);
	WriteData (0x65);
	WriteData (0x00);
	WriteData (0x67);
	WriteData (0x00);
	WriteData (0x00);
	WriteComm (0xE3);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x33);
	WriteData (0x33);
	WriteComm (0xE4);
	WriteData (0x44);
	WriteData (0x44);
	WriteComm (0xE5);
	WriteData (0x0C);
	WriteData (0x78);
	WriteData (0x3C);
	WriteData (0xA0);
	WriteData (0x0E);
	WriteData (0x78);
	WriteData (0x3C);
	WriteData (0xA0);
	WriteData (0x10);
	WriteData (0x78);
	WriteData (0x3C);
	WriteData (0xA0);
	WriteData (0x12);
	WriteData (0x78);
	WriteData (0x3C);
	WriteData (0xA0); 
	WriteComm (0xE6);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x33);
	WriteData (0x33);
	WriteComm (0xE7);
	WriteData (0x44);
	WriteData (0x44);
	WriteComm (0xE8);
	WriteData (0x0d);
	WriteData (0x78);
	WriteData (0x3C);
	WriteData (0xA0);
	WriteData (0x0F);
	WriteData (0x78);
	WriteData (0x3C);
	WriteData (0xA0);
	WriteData (0x11);
	WriteData (0x78);
	WriteData (0x3C);
	WriteData (0xA0);
	WriteData (0x13);
	WriteData (0x78);
	WriteData (0x3C);
	WriteData (0xA0);
	WriteComm (0xEB);
	WriteData (0x02);
	WriteData (0x02);
	WriteData (0x39);
	WriteData (0x39);
	WriteData (0xEE);
	WriteData (0x44);
	WriteData (0x00);
	WriteComm (0xEC);
	WriteData (0x00);
	WriteData (0x00);
	WriteComm (0xED);
	WriteData (0xFF);
	WriteData (0xF1);
	WriteData (0x04);
	WriteData (0x56);
	WriteData (0x72);
	WriteData (0x3F);
	WriteData (0xFF);
	WriteData (0xFF);
	WriteData (0xFF);
	WriteData (0xFF);
	WriteData (0xF3);
	WriteData (0x27);
	WriteData (0x65);
	WriteData (0x40);
	WriteData (0x1F);
	WriteData (0xFF);
	//------------------------------------------ End GIP Setting-------------------------------------------- //
	//--------------------------- Power Control Registers Initial End--------------------------------//
	//---------------------------------------Bank1 Setting------------------------------------------------- //
	WriteComm (0xFF);
	WriteData (0x77);
	WriteData (0x01);
	WriteData (0x00);
	WriteData (0x00);
	WriteData (0x00);
	WriteComm (0x29);

	LCD_BKL_Off;
}
#endif	                                                   

/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void LCD_DisplayOn(void)
{
  LCD_WriteCommand(LCD_DISPLAY_ON);
}

/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void LCD_DisplayOff(void)
{
    /* Display Off */
    LCD_WriteCommand(LCD_DISPLAY_OFF);
}

/**
  * @brief  Configures LCD control lines in Output Push-Pull mode.
  * @note   The LCD_NCS line can be configured in Open Drain mode  
  *         when VDDIO is lower than required LCD supply.
  * @param  None
  * @retval None
  */

////////////////////////////////////////////////////////////////////
/**
 * @brief Wakeup push-button
 */
#define USER_BUTTON_ENT_PIN                		GPIO_Pin_7
#define USER_BUTTON_ENT_GPIO_PORT          		GPIOD
#define USER_BUTTON_ENT_GPIO_CLK           		RCC_AHB1Periph_GPIOD

#define USER_BUTTON_PSW_PIN                 	GPIO_Pin_8
#define USER_BUTTON_PSW_GPIO_PORT           	GPIOC
#define USER_BUTTON_PSW_GPIO_CLK            	RCC_AHB1Periph_GPIOC

#define USER_LED_PIN                 			GPIO_Pin_3
#define USER_LED_GPIO_PORT           			GPIOE
#define USER_LED_GPIO_CLK            			RCC_AHB1Periph_GPIOE

//
//
//
void InitTouchKey(void)
{
  	GPIO_InitTypeDef GPIO_InitStructure;
  
  	/* Enable the BUTTON Clock */
  	RCC_AHB1PeriphClockCmd(USER_BUTTON_ENT_GPIO_CLK  | USER_LED_GPIO_CLK | USER_BUTTON_PSW_GPIO_CLK, ENABLE);

  	/* Configure Button pin as input */
  	GPIO_InitStructure.GPIO_Pin = USER_BUTTON_ENT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  	GPIO_Init(USER_BUTTON_ENT_GPIO_PORT, &GPIO_InitStructure);

	/* Configure NCS in Output Push-Pull mode */
	GPIO_InitStructure.GPIO_Pin = USER_BUTTON_PSW_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(USER_BUTTON_PSW_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = USER_LED_PIN;
	GPIO_Init(USER_LED_GPIO_PORT, &GPIO_InitStructure);
}


void LCD_CtrlLinesConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOs clock*/
  RCC_AHB1PeriphClockCmd(LCD_NCS_GPIO_CLK | LCD_WRX_GPIO_CLK | LCD_NRESET_GPIO_CLK | LCD_NCSX_GPIO_CLK | LED_BKL_GPIO_CLK, ENABLE);

  /* Configure NCS in Output Push-Pull mode */
  GPIO_InitStructure.GPIO_Pin = LCD_NCS_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(LCD_NCS_GPIO_PORT, &GPIO_InitStructure);
  
  /* Configure WRX in Output Push-Pull mode */
  GPIO_InitStructure.GPIO_Pin = LCD_WRX_PIN;
  GPIO_Init(LCD_WRX_GPIO_PORT, &GPIO_InitStructure);

   /* Configure RESET in Output Push-Pull mode */
  GPIO_InitStructure.GPIO_Pin = LCD_NRESET_PIN;
  GPIO_Init(LCD_NRESET_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_NCSX_PIN;
  GPIO_Init(LCD_NCSX_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LED_BKL_PIN;
  GPIO_Init(LED_BKL_GPIO_PORT, &GPIO_InitStructure);

  /* Set chip select pin high */
  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
}

/**
  * @brief  Returns the selected Button state.
  * @param  Button: Specifies the Button to be checked.
  *   This parameter should be: BUTTON_USER  
  * @retval The Button GPIO pin value.
  */
uint32_t GetTouchKeyState(void)
{
  	return GPIO_ReadInputDataBit(USER_BUTTON_ENT_GPIO_PORT, USER_BUTTON_ENT_PIN);
}

void PSWOutputHigh(void)
{
  	GPIO_SetBits(USER_BUTTON_PSW_GPIO_PORT, USER_BUTTON_PSW_PIN);

}

void PSWOutputLow(void)
{
	GPIO_ResetBits(USER_BUTTON_PSW_GPIO_PORT, USER_BUTTON_PSW_PIN);
}

void LEDOutputHigh(void)
{
  	GPIO_SetBits(USER_LED_GPIO_PORT, USER_LED_PIN);
}

void LEDOutputLow(void)
{
	GPIO_ResetBits(USER_LED_GPIO_PORT, USER_LED_PIN);
}

static uint8_t key_pullup = 0;
static uint8_t key_pulldown = 0;

typedef enum {
	
	KEY_SWICH_ON  = 0,
	KEY_SWICH_OFF = 1,
	KEY_IDLE      = -1,
	
}HARD_KEY_STATE_t;

HARD_KEY_STATE_t HARD_KEY_STATE = KEY_SWICH_ON;

#if 0
void HardKeyProc(void)
{
	U32 t0, t1;
	OS_ERR  err;

	if(GetTouchKeyState() == Bit_RESET)
	{
		t0 = GUI_GetTime();

		while(GetTouchKeyState() == Bit_RESET)
		{
			t1 = GUI_GetTime() - t0;
			
			if(t1 > 20)
				break;
		}

		switch((int)HARD_KEY_STATE)  
		{  
    		case KEY_SWICH_ON: 

				PSWOutputHigh();
				LEDOutputLow();

				HARD_KEY_STATE = KEY_IDLE;
				key_pulldown = 0;
				
				break;
			case KEY_SWICH_OFF: 
				
				PSWOutputLow();
				LEDOutputHigh();

				HARD_KEY_STATE = KEY_IDLE;
				key_pullup = 0;
					
				break;
			case KEY_IDLE: 
				
				if(key_pulldown)
					HARD_KEY_STATE = KEY_SWICH_OFF;
				else if(key_pullup)
					HARD_KEY_STATE = KEY_SWICH_ON;

				break;
			default:  
        		break;
		}
	}
	else
	{

		if(HARD_KEY_STATE == KEY_IDLE && !key_pulldown)
			key_pulldown = 1;
		else if(HARD_KEY_STATE == KEY_IDLE && !key_pullup)
			key_pullup = 1;
	}

	OSTimeDly(10, OS_OPT_TIME_DLY, &err);
}
#else

static unsigned char Trgg_sta = 0;  // �̰�״̬
static unsigned char cntu_sta = 0;  // ����״̬

static void KeyRead( void )
{
    unsigned char ReadData = !(GetTouchKeyState());

    Trgg_sta = ReadData & (ReadData ^ cntu_sta);      
    cntu_sta = ReadData;                               
}

void HardKeyProc(void)
{
	KeyRead();
	
	if(Trgg_sta == 1 && HARD_KEY_STATE == KEY_SWICH_ON)
	{
		HARD_KEY_STATE = KEY_SWICH_OFF;

		PSWOutputHigh();
		LEDOutputLow();
	}
	else if(Trgg_sta == 1 && HARD_KEY_STATE == KEY_SWICH_OFF)
	{
		HARD_KEY_STATE = KEY_SWICH_ON;

		PSWOutputLow();
		LEDOutputHigh();
	}
}
#endif

/**
  * @brief  Sets or reset LCD control lines.
  * @param  GPIOx: where x can be B or D to select the GPIO peripheral.
  * @param  CtrlPins: the Control line.
  *   This parameter can be:
  *     @arg LCD_NCS_PIN: Chip Select pin
  *     @arg LCD_NWR_PIN: Read/Write Selection pin
  *     @arg LCD_RS_PIN: Register/RAM Selection pin
  * @param  BitVal: specifies the value to be written to the selected bit.
  *   This parameter can be:
  *     @arg Bit_RESET: to clear the port pin
  *     @arg Bit_SET: to set the port pin
  * @retval None
  */
void LCD_CtrlLinesWrite(GPIO_TypeDef* GPIOx, uint16_t CtrlPins, BitAction BitVal)
{
  /* Set or Reset the control line */
  GPIO_WriteBit(GPIOx, (uint16_t)CtrlPins, (BitAction)BitVal);
}

/**
  * @brief  Configures the LCD_SPI interface.
  * @param  None
  * @retval None
  */
void LCD_SPIConfig(void)
{
  #if 1
  	GPIO_InitTypeDef  GPIO_InitStructure;

	/* ��GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_SCK | RCC_MOSI | RCC_MISO, ENABLE);

	/* ���ü����������IO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO������ٶ� */

	GPIO_InitStructure.GPIO_Pin = PIN_SCK;
	GPIO_Init(PORT_SCK, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_MOSI;
	GPIO_Init(PORT_MOSI, &GPIO_InitStructure);

	/* ����GPIOΪ��������ģʽ(ʵ����CPU��λ���������״̬) */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* �������������� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO������ٶ� */

	GPIO_InitStructure.GPIO_Pin = PIN_MISO;
	GPIO_Init(PORT_MISO, &GPIO_InitStructure);
	
  #else	
  SPI_InitTypeDef    SPI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Enable LCD_SPI_SCK_GPIO_CLK, LCD_SPI_MISO_GPIO_CLK and LCD_SPI_MOSI_GPIO_CLK clock */
  RCC_AHB1PeriphClockCmd(LCD_SPI_SCK_GPIO_CLK | LCD_SPI_MISO_GPIO_CLK | LCD_SPI_MOSI_GPIO_CLK, ENABLE);

  /* Enable LCD_SPI and SYSCFG clock  */
  RCC_APB2PeriphClockCmd(LCD_SPI_CLK, ENABLE);
  
  /* Configure LCD_SPI SCK pin */
  GPIO_InitStructure.GPIO_Pin = LCD_SPI_SCK_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(LCD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /* Configure LCD_SPI MISO pin */
  GPIO_InitStructure.GPIO_Pin = LCD_SPI_MISO_PIN;
  GPIO_Init(LCD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /* Configure LCD_SPI MOSI pin */
  GPIO_InitStructure.GPIO_Pin = LCD_SPI_MOSI_PIN;
  GPIO_Init(LCD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /* Connect SPI SCK */
  GPIO_PinAFConfig(LCD_SPI_SCK_GPIO_PORT, LCD_SPI_SCK_SOURCE, LCD_SPI_SCK_AF);

  /* Connect SPI MISO */
  GPIO_PinAFConfig(LCD_SPI_MISO_GPIO_PORT, LCD_SPI_MISO_SOURCE, LCD_SPI_MISO_AF);

  /* Connect SPI MOSI */
  GPIO_PinAFConfig(LCD_SPI_MOSI_GPIO_PORT, LCD_SPI_MOSI_SOURCE, LCD_SPI_MOSI_AF);
  
  SPI_I2S_DeInit(LCD_SPI);

  /* SPI configuration -------------------------------------------------------*/
  /* If the SPI peripheral is already enabled, don't reconfigure it */
  if ((LCD_SPI->CR1 & SPI_CR1_SPE) == 0)
  {    
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    /* SPI baudrate is set to 5.6 MHz (PCLK2/SPI_BaudRatePrescaler = 90/16 = 5.625 MHz) 
       to verify these constraints:
          - ILI9341 LCD SPI interface max baudrate is 10MHz for write and 6.66MHz for read
          - l3gd20 SPI interface max baudrate is 10MHz for write/read
          - PCLK2 frequency is set to 90 MHz 
       */
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(LCD_SPI, &SPI_InitStructure);

    /* Enable L3GD20_SPI  */
    SPI_Cmd(LCD_SPI, ENABLE);
  }
  #endif
}

/**
  * @brief  GPIO config for LTDC.
  * @param  None
  * @retval None
  */
static  void LCD_AF_GPIOConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  /* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOF, GPIOG AHB Clocks */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | \
                         RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | \
                         RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);

/* GPIOs Configuration */
/*
 +------------------------+-----------------------+----------------------------+
 +                       LCD pins assignment                                   +
 +------------------------+-----------------------+----------------------------+
 |  LCD_TFT R2 <-> PC.10  |  LCD_TFT G2 <-> PA.06 |  LCD_TFT B2 <-> PD.06      |
 |  LCD_TFT R3 <-> PB.00  |  LCD_TFT G3 <-> PG.10 |  LCD_TFT B3 <-> PG.11      |
 |  LCD_TFT R4 <-> PA.11  |  LCD_TFT G4 <-> PB.10 |  LCD_TFT B4 <-> PG.12      |
 |  LCD_TFT R5 <-> PA.12  |  LCD_TFT G5 <-> PB.11 |  LCD_TFT B5 <-> PA.03      |
 |  LCD_TFT R6 <-> PB.01(PA8)  |  LCD_TFT G6 <-> PC.07 |  LCD_TFT B6 <-> PB.08      |
 |  LCD_TFT R7 <-> PG.06  |  LCD_TFT G7 <-> PD.03 |  LCD_TFT B7 <-> PB.09      |
 -------------------------------------------------------------------------------
          |  LCD_TFT HSYNC <-> PC.06  | LCDTFT VSYNC <->  PA.04 |
          |  LCD_TFT CLK   <-> PG.07  | LCD_TFT DE   <->  PF.10 |
           -----------------------------------------------------

*/
 #if 0
 /* GPIOA configuration */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_LTDC);
  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_LTDC); // PA8
  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_LTDC);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6 | \
                             GPIO_Pin_8 | GPIO_Pin_11 | GPIO_Pin_12;
                             
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

 /* GPIOB configuration */  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_LTDC);
  //GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_LTDC); // PB1
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_LTDC);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | /*GPIO_Pin_1 |*/ GPIO_Pin_8 | \
                             GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
  
  GPIO_Init(GPIOB, &GPIO_InitStruct);

 /* GPIOC configuration */
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_LTDC);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10;
                             
  GPIO_Init(GPIOC, &GPIO_InitStruct);

 /* GPIOD configuration */
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_LTDC);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_6;
                             
  GPIO_Init(GPIOD, &GPIO_InitStruct);
  
 /* GPIOF configuration */
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource10, GPIO_AF_LTDC);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
                             
  GPIO_Init(GPIOF, &GPIO_InitStruct);     

 /* GPIOG configuration */  
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource7, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource10, 0x09);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource12, 0x09);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10 | \
                             GPIO_Pin_11 | GPIO_Pin_12;
  
  GPIO_Init(GPIOG, &GPIO_InitStruct);
  #else
  /* GPIOA configuration */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_LTDC);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6 | \
                             GPIO_Pin_11 | GPIO_Pin_12;
                             
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  
 /* GPIOB configuration */  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, 0x09);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, 0x09);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_LTDC);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | \
                             GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
  
  GPIO_Init(GPIOB, &GPIO_InitStruct);

 /* GPIOC configuration */
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_LTDC);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10;
                             
  GPIO_Init(GPIOC, &GPIO_InitStruct);

 /* GPIOD configuration */
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_LTDC);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_6;
                             
  GPIO_Init(GPIOD, &GPIO_InitStruct);
  
 /* GPIOF configuration */
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource10, GPIO_AF_LTDC);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
                             
  GPIO_Init(GPIOF, &GPIO_InitStruct);     

 /* GPIOG configuration */  
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource7, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource10, 0x09);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource12, 0x09);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10 | \
                             GPIO_Pin_11 | GPIO_Pin_12;
  
  GPIO_Init(GPIOG, &GPIO_InitStruct);
  #endif

}

/**
  * @brief  Displays a pixel.
  * @param  x: pixel x.
  * @param  y: pixel y.  
  * @retval None
  */
static void PutPixel(int16_t x, int16_t y)
{ 
  if(x < 0 || x > 239 || y < 0 || y > 319)
  {
    return;  
  }
  LCD_DrawLine(x, y, 1, LCD_DIR_HORIZONTAL);
}

#ifndef USE_Delay
/**
  * @brief  Inserts a delay time.
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
static void delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0; 
  for(index = nCount; index != 0; index--)
  {
  }
}
#endif /* USE_Delay*/
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
