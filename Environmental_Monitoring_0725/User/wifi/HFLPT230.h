/******************** Software  Recycle ************************
	* @1  £∫TBD
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
********************Software  Recycle *************************/
#ifndef _HFLPB100_H_
#define _HFLPB100_H_

#include "stm32f4xx.h"

typedef enum{
	WIFI_CHANGING = 0,	//:ƒ£ Ω«–ªªπ˝≥Ã÷–
	WIFI_AP = 1,        //:”√◊˜wifi∑˛ŒÒ∆˜ π”√£¨ø…“‘»√…Ë±∏¿¥¡¨Ω”
	WIFI_STA = 2,       //:”√◊˜wifi…Ë±∏¿¥¡¨Ω”£¨”√”⁄¡¨Ω”wifi∑˛ŒÒ∆˜
	WIFI_APSTA =3,      //£∫◊€…œ
	WIFI_TOSMART_LINK = 4,  //3S“‘œ¬µÕµÁ∆Ω
	WIFI_TO_RESET = 5,      //3S“‘…œµÕµÁ∆Ω
}EWifiMode,*pEWifiMode;

extern EWifiMode gWifiState ;  //≥ı º◊¥Ã¨±Í÷æ

typedef enum
{
	Task_01             = 0x01,                        // 01 »ŒŒÒ‘›Œ¥ π”√
	Task_POST_With_Body = 0x02,
	Task_Post           = 0x04,                     	// 03 Azure Post »ŒŒÒ
	Task_Delete         = 0x08,                   		// 04 Azure Deletezu»ŒŒÒ                		
	Task_POST_Login     = 0x10,
	Task_WifiConfig     = 0x20,               			// 06 ÷ÿ–¬≈‰÷√WiFiƒ£øÈ 
	Task_Wifitest       = 0x40,                 		// 07 ≥ı ºªØ∂¡»° …Ë÷√WiFiƒ£øÈ ≤Œ ˝
	Task_WifiRESET      = 0x80,                  		// 08 »ŒŒÒ‘›Œ¥ π”
	Task_GetState       = 0x200,                 		// 02 ≤È—Øƒ£øÈ◊¥Ã¨
	Task_InitWifi       = 0x100,
}TASK_LIST;

typedef struct{
	unsigned char  tm_sec;   /* seconds after the minute, 0 to 60
							(0 - 60 allows for the occasional leap second) */
	unsigned char  tm_min;   /* minutes after the hour, 0 to 59 */
	unsigned char  tm_hour;  /* hours since midnight, 0 to 23 */
	unsigned char  tm_mday;  /* day of the month, 1 to 31 */
	unsigned char  tm_mon;   /* months since January, 0 to 11 */
	unsigned short tm_year;  /* years since 1900 */
	
}GMT_TIME;

typedef struct
{
	char Wifi_mac[16];
	char Wifi_deviceid[32];
	unsigned int  Wifi_devicestate;
	unsigned char Wifi_signalquality;
	unsigned char Wifi_Configtimeout;                 // 5 ∑÷÷”
}WIFI_Para;

typedef struct
{
	unsigned char timer_order[505];
	unsigned char current_mode;
	unsigned char mode;
	unsigned char start_date[14];
	unsigned char end_date[14];
}Azure_ModeControl;

typedef struct
{
	WIFI_Para wifi_state;
	GMT_TIME gmt_time;
	unsigned int expiry_time;
	Azure_ModeControl mode_Control;
}WIFI_State;

typedef void (*encoding)(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State);
typedef unsigned char (*decoding)(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State);

typedef struct
{
	encoding  Encode;
	decoding  Decode;
	
}WIFI_CMD;

typedef struct
{
	unsigned short wifi_taskqueue;
	WIFI_CMD *CMD;
	unsigned char wifi_CmdCount;
	unsigned char wifi_TaskType;
	volatile unsigned int Response_TimeOut;
	volatile unsigned int Response_TimeOut_POST;
	WIFI_State wifi_hflpb100;
	unsigned char CMD_SEND_CTR;
	unsigned char CMD_COM_STATE_CTR;
	
}WIFI_TASK;

typedef struct BUFFER_TAG
  {
	  unsigned char* buffer;
	  size_t size;
  } BUFFER;

  typedef struct STRING_TAG
  {
	  char* s;
  } STRING;

  typedef struct BUFFER_TAG_
{
    unsigned char buffer[128];
    size_t size;
} BUFFER_;

 typedef struct STRING_TAG_
{
    char s[128];
} STRING_;

typedef struct BUFFER_TAG* BUFFER_HANDLE;
typedef struct STRING_TAG* STRING_HANDLE;
typedef struct BUFFER_TAG_* BUFFER_HANDLE_;
typedef struct STRING_TAG_* STRING_HANDLE_;


typedef struct
{
	char *DeviceId;
	char *DeviceType;
	char primaryKey[32];
}Login_Body_Req;

typedef struct
{
	char SSID[128];
	char PASS[64];
	char *ACCOUNTID;
	char *ID;
	unsigned char BL_CONF_FLAG;
	unsigned char WIFI_SSID_LEN;
	unsigned char WIFI_PASS_LEN;
	unsigned char wifi_connect_status;
}Bluetooth_Login;

typedef struct
{
	unsigned char device_error;
	unsigned char Network_OK;
	unsigned char Send_OK;
	unsigned char Login_OK;
	unsigned char Post_Allow;
	unsigned char Reset_WIFI;
	unsigned char Enter_ENTM;
	unsigned char Tmr5_Ctl;
	unsigned char WIFI_Reboot;
	unsigned char GET_PARA;
	unsigned char Set_Timer;
	unsigned char PostTimes;
}WIFI_CTR;

typedef struct _AzureState_MODE
{
	unsigned int state;
}AzureState_MODE;

void Uart_TxRxTask(WIFI_TASK * task);
/******************	    API     ************************/																			 
//Old Program
void WifiHFInit(void);
//New Program
void WifiInit(void);
void Wifi_Program(unsigned char msTick);
void WifiConfigTest(void);
/******************	 API     ************************/

void Uart_TxRxTask(WIFI_TASK * task);
void Wifi_ConfigReset(WIFI_TASK * task);
unsigned int RTC_Set(u16 syear,unsigned char smon,unsigned char sday,unsigned char hour,unsigned char min,unsigned char sec);
u8 Is_Leap_Year(u16 year);
unsigned char RTC_GetHour(WIFI_State *wifi_State);
void wifi_TimeCount(WIFI_TASK * task);
void wifi_timeout_count(WIFI_TASK * task);


#endif


