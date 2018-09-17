#include "time.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "includes.h"
#include "cJSON_APP.h"
#include "GUI.h"

////////////////////////////////////////////////////////////////
extern OS_TMR 	tmr1;		//timer1
extern OS_TMR	tmr2;		//timer2

extern void  WIFITaskPend ( void );
extern void  WIFITaskPost ( void);
extern void  Uart3RecTimerStart (void);

//////////////////////////////////////////////////////////////////

WIFI_CTR 				WiFi_State;
Bluetooth_Login 		Bluetooth_State;
static unsigned char 	Uart_CommState = 0;

//////////////////////////////////////////////////////////////////
static void ReadStrUnit(char * str,char *temp_str,int idx,int len)  
{
    int index = 0;
    for(index = 0; index < len; index++)
    {
        temp_str[index] = str[idx+index];
    }
    temp_str[index] = '\0';
}

int GetSubStrPos(char *str1,char *str2)
{
    int idx = 0;
	char temp_str[30];    
	
    int len1 = strlen(str1);
    int len2 = strlen(str2);

    if( len1 < len2)
    {
        printf("error 1 \n"); 
        return -1;
    }

    while(1)
    {
        ReadStrUnit(str1,temp_str,idx,len2);    
        if(strcmp(str2,temp_str)==0)break;     
        idx++;                                 
        if(idx>=len1)return -1;                 
    }

    return idx;    
}

void wifi_TimeCount(WIFI_TASK * task)
{
	task->Response_TimeOut = 0;
}

void wifi_timeout_count(WIFI_TASK * task)
{
	task->Response_TimeOut_POST = 0;
}

EWifiMode gWifiState;  
volatile unsigned char Rx_Buffer[1537];           // 接收缓冲区
volatile unsigned int Rx_Length= 0;
static unsigned char Tx_Buffer[1024] = { 0 };
static unsigned int Tx_Length = 0;


/** 
  */
static void WifiGPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOF, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	         	     // nReset
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	         	     
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
    GPIO_Init(GPIOF, &GPIO_InitStructure);
}

void WifiInit(void)
{
	WifiGPIOInit();
}


u8 Is_Leap_Year(u16 year)
{			  
	if(year%4==0) 
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;
			else return 0;   
		}else return 1;   
	}else return 0;	
}	

#if 1

u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; 
const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

unsigned int RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
	u16 t;
	u32 seccount=0;
	if(syear<1970||syear>2099)return 0;	   
	for(t=1970;t<syear;t++)	
	{
		if(Is_Leap_Year(t))seccount+=31622400;
		else seccount+=31536000;			 
	}
	smon-=1;
	for(t=0;t<smon;t++)	
	{
		seccount+=(u32)mon_table[t]*86400;
		if(Is_Leap_Year(syear)&&t==1)seccount+=86400;   
	}
	seccount+=(u32)(sday-1)*86400;
	seccount+=(u32)hour*3600;
    seccount+=(u32)min*60;	
	seccount+=sec;

//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	
//	PWR_BackupAccessCmd(ENABLE);	
//	RTC_SetCounter(seccount);	

//	RTC_WaitForLastTask();		
	return seccount;	    
}

#endif

void Send_Plus(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "+++";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);
}

void Send_Char_a(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "a";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}

void Send_Cmd_Mac(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+WSMAC\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}

void Send_Cmd_Connect(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+WSLK\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}
void Send_Cmd_State(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+WSLQ\r\n";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}

void Send_Cmd_Config(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+SMTLK\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
	
	wifi_State->wifi_state.Wifi_devicestate |= 0x0040;    // ????
	wifi_State->wifi_state.Wifi_Configtimeout = 240;          // ????? ??
	
	// ??
	wifi_State->wifi_state.Wifi_devicestate &= 0xF9FF;    // 
	
}
void Send_Cmd_SSLADDR(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+SSLADDR=aldesiotsuiterecette.azure-devices.net\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	

}
void Send_Cmd_NTPSER(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+NTPSER=115.29.164.59\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	

}
void Send_Cmd_NTPON(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+NTPEN=on\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	

}
void Send_Cmd_REBOOT(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+Z\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}


void Send_Cmd_Exit(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+ENTM\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}
//zyn
static unsigned char ssid_plus[64]= {"AT+WSSSID=RD-2\r"};

void Send_Cmd_name(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned int i;
	
	#ifdef SPI_FLASH_WIFI
	if(Bluetooth_State.BL_CONF_FLAG)
	{  
		sf_ReadBuffer((u8 *)Bluetooth_State.SSID, SSID_ADDR, Bluetooth_State.WIFI_SSID_LEN);
		
		memset(ssid_plus + 10,NULL,54);
		memcpy(ssid_plus + 10,Bluetooth_State.SSID,Bluetooth_State.WIFI_SSID_LEN);
		strncat(ssid_plus,"\r",1);
		printf("Send_Cmd_name SSID %s\r\n",ssid_plus);
	}
	#endif
	(* data_length) = strlen((const char *)ssid_plus);
	memcpy(data_temp,ssid_plus,strlen((const char *)ssid_plus));
	#if 0
	(* data_length) = strlen((const char *)ssid_plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(ssid_plus + i);
	#endif
}

static unsigned char pass_plus[64]= {"AT+WSKEY=wpa2psk,aes,301301301\r"};

void Send_Cmd_Secret(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned int i;

	#ifdef SPI_FLASH_WIFI
	if(Bluetooth_State.BL_CONF_FLAG)
	{  
		Bluetooth_State.BL_CONF_FLAG = 0;

		sf_ReadBuffer((u8 *)Bluetooth_State.PASS, PASS_ADDR, Bluetooth_State.WIFI_PASS_LEN);
		
		memset(pass_plus + 21,NULL,43);
		memcpy(pass_plus + 21,Bluetooth_State.PASS,Bluetooth_State.WIFI_PASS_LEN);
		strncat(pass_plus,"\r",1);
		printf("Send_Cmd_Secret PASS %s\r\n",pass_plus);
	}
	#endif
	(* data_length) = strlen((const char *)pass_plus);
	memcpy(data_temp,pass_plus,strlen((const char *)pass_plus));
	#if 0
	(* data_length) = strlen((const char *)pass_plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(pass_plus + i);
	#endif
}

void Send_Cmd_sta(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+WMODE=sta\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}
void Send_Cmd_reset(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+WSSSID= \r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}


const unsigned char Head_Host[]= "Host: aldesnamespace.servicebus.chinacloudapi.cn\r\n";
const unsigned char Head_ContentType[]= "Content-Type: application/json\r\n";
const unsigned char Head_UserAgent[]= "User-Agent: Aldes-Modem/1.0\r\n";
const unsigned char Head_Connection[]= "Connection: keep-alive\r\n";
const unsigned char Head_BlankRow[]= "\r\n";
const unsigned char Send_PostCmd[]= "POST /aldeseh/messages HTTP/1.1\r\n";
const unsigned char Head_PostAuthorization[]= {
"Authorization: SharedAccessSignature sr=https%3a%2f%2faldesnamespace.servicebus.chinacloudapi.cn%2faldeseh%2f&sig=5mQrXrUzjLOB9LL0GbFlJwMyKGkUeC%2fDQoVrBqvNSyA%3d&se=1503279326&skn=senderkey\r\n",};
const unsigned char Head_ContentLength[]= "Content-Length: 358\r\n";

#if 1
const unsigned char* Load_Data[23][2]={
	
	/*
	1{ "modemid", "ACCF2361E133"},
	2{ "productid", "ACCF2361E133_AIR"},
	3{ "outside_tpt", "-4.0"},
	4{ "reject_tpt", "-4.0"},
	5{ "tii_est", "0"},
	6{ "extf_spd", "0"},
	7{ "extf_tsn", "0"},
	8{ "vi_spd", "0"},
	9{ "vv_tsn", "0"},
	10{ "ff_cpt", "0"},
	11{ "ffe_flw", "42"},
	12{ "extf_flw", "46"},
	13{ "dep_ind", "0"},
	14{ "ext_tpt", "-4.0"},
	15{ "cve_csn", "0"},
	16{ "vi_csn", "0"},
	17{ "tte_csn", "0"},
	18{ "DtB_ind", "0"},
	19{ "echange_pwr", "0"},
	20{ "set_spd", "1"},
	21{ "exch_eng", "0"},
	22{ "Co2_vmc", "0.0"},
	23{ "pm25_vmc", "0"},
	24{ "voc_vmc", "1"}
	*/
	{ "modemid", "ACCF2361E133"},                   // 1
	{ "productid", "ACCF2361E133_AIR"},             // 2
	{ "outside_tpt", "-8.0"},                       // 3
	{ "reject_tpt", "-8.0"},                        // 4
	{ "extf_spd", "0"},
	{ "vi_spd", "0"},
	{ "ff_cpt", "0"},
	{ "ffe_flw", "42"},
	{ "extf_flw", "46"},
	{ "dep_ind", "0"},
	{ "ext_tpt", "-8.0"},
	{ "cve_csn", "0"},
	{ "vi_csn", "0"},
	{ "tte_csn", "0"},
	{ "dtb_ind", "0"},
	{ "echange_pwr", "0"},
	{ "set_spd", "1"},
	{ "Start_date_mode", ""},
	{ "End_date_mode", ""},
	//{ "week_planning", ""},
	{ "current_mode", "V"},
	{ "Co2_vmc", "0.0"},
	{ "Pm25_Vmc", "0.0"},
	{ "Voc_Vmc", "1"}
	
};

typedef struct
{
	unsigned char value_length[22];
	unsigned char value[22][5];
}WIFI_SENDDATA;

static WIFI_SENDDATA device_data;

unsigned short int Add_DeviceState(unsigned char *data_temp,unsigned int *data_length,WIFI_State *wifi_State)
{
	unsigned char i;
	unsigned short int length = 0;
	unsigned short  temp;
	
	length = 0;
	*(data_temp + *data_length)= '{';
	*data_length += 1;
	length  += 1;
	
	for(i = 0; i < 23; i ++)
	{
		*(data_temp + *data_length)= '"';
		*data_length += 1;
		length  += 1;
		
		temp = strlen(Load_Data[i][0]);
		memcpy(data_temp+ *data_length,Load_Data[i][0],temp);
		*data_length += temp;
		length += temp;
		
		*(data_temp + *data_length)= '"';
		*data_length += 1;
		length  += 1;
		
		*(data_temp + *data_length)= ':';
		*data_length += 1;
		length  += 1;
		
		
		// ??
		
		if( i > 1 )
		{
			if( i > 19 || i < 17 )
			{
				
				memcpy(data_temp+ *data_length,device_data.value[i-2],device_data.value_length[i-2]);
				*data_length += device_data.value_length[i-2];
				length += device_data.value_length[i-2];
				
				
				
			}
			else if( i == 17)
			{
				*(data_temp + *data_length)= '"';
				*data_length += 1;
				length  += 1;
				
				
				*(data_temp + *data_length)= '"';
				*data_length += 1;
				length  += 1;
			}
			else if( i == 18)
			{
				*(data_temp + *data_length)= '"';
				*data_length += 1;
				length  += 1;
			
				*(data_temp + *data_length)= '"';
				*data_length += 1;
				length  += 1;
			}
			else if( i == 19)
			{
				*(data_temp + *data_length)= '"';
				*data_length += 1;
				length  += 1;
				
				if(wifi_State->mode_Control.current_mode)
				{
					temp = 1;
					memcpy(data_temp+ *data_length,&(wifi_State->mode_Control.current_mode),temp);
					*data_length += temp;
					length += temp;
				}
				*(data_temp + *data_length)= '"';
				*data_length += 1;
				length  += 1;
			}
			
			
		}
		else
		{
			*(data_temp + *data_length)= '"';
			*data_length += 1;
			length  += 1;
			
			temp = strlen(Load_Data[i][1]);
			memcpy(data_temp+ *data_length,Load_Data[i][1],temp);
			
			if(strtol(wifi_State->wifi_state.Wifi_mac,NULL,16) != 0)
				memcpy(data_temp+ *data_length,wifi_State->wifi_state.Wifi_mac,12);
			
			*data_length += temp;
			length += temp;
			
			*(data_temp + *data_length)= '"';
			*data_length += 1;
			length  += 1;
		}
		/// 
		
		
		if( i != 22)
		{
			*(data_temp + *data_length)= ',';
			*data_length += 1;
			length  += 1;
		}
	}
	memcpy(data_temp+ *data_length,"}\r\n",3);
	*data_length += 3;
	//length  += 1;   
	
	return length;                   // 
}
#endif


void Send_Post(unsigned char *data_temp,unsigned int *data_length,WIFI_State *wifi_State)
{
	
	unsigned int i;
	unsigned int temp;
	unsigned char expiry_temp[12];
	unsigned char * addr_temp;
	
	*data_length = 0;
	
	temp = strlen(Send_PostCmd);
	memcpy(data_temp + *data_length,Send_PostCmd,temp);
	(* data_length) = temp;
	
	temp = strlen(Head_Host);
	memcpy(data_temp + *data_length,Head_Host,temp);
	(* data_length) += temp;
	
	temp = strlen(Head_PostAuthorization);
	memcpy(data_temp+ *data_length,Head_PostAuthorization,strlen(Head_PostAuthorization));
	
	for(i = 0; i < temp; i++)
	{
		if((memcmp(data_temp+ *data_length + i,"&se=",4) == 0) && (wifi_State->expiry_time))
		{
			sprintf(expiry_temp,"%d",(wifi_State->expiry_time + 120));
			memcpy(data_temp+ *data_length + i + 4,expiry_temp,10);  
		}
	}
	(* data_length) += temp;
	
	temp = strlen(Head_ContentType);
	memcpy(data_temp+ *data_length,Head_ContentType,strlen(Head_ContentType));
	(* data_length) += temp;
	
	temp = strlen(Head_UserAgent);
	memcpy(data_temp+ *data_length,Head_UserAgent,strlen(Head_UserAgent));
	(* data_length) += temp;
	
	temp = strlen(Head_Connection);
	memcpy(data_temp+ *data_length,Head_Connection,strlen(Head_Connection));
	(* data_length) += temp;
	
	temp = strlen(Head_ContentLength);
	memcpy(data_temp+ *data_length,Head_ContentLength,strlen(Head_ContentLength));
	(* data_length) += temp;
	
	addr_temp = data_temp + *data_length - 5;
	
	temp = strlen(Head_BlankRow);
	memcpy(data_temp + *data_length,Head_BlankRow,temp);
	(* data_length) += temp;
	
	
	temp = Add_DeviceState(data_temp,data_length,wifi_State);
	
	sprintf(expiry_temp,"%d",temp);
	memcpy(addr_temp,expiry_temp,3);

	wifi_State->wifi_state.Wifi_devicestate |= 0x0001;    // ????
}


const unsigned char Send_DeleteCmd[]= "DELETE /topicpilotage8964/Subscriptions/ACCF23924FB4_AIR/messages/head HTTP/1.1\r\n";
const unsigned char Head_DeleteAuthorization[]= {
		"Authorization:SharedAccessSignature sr=https%3a%2f%2faldesnamespace.servicebus.chinacloudapi.cn%2f&sig=8Uap3h%2bV%2bLiAB%2bmY%2bmusicp20R1dwtTdCunOV0wIteY%3d&se=1503297514&skn=RootManageSharedAccessKey\r\n"
	};

void Send_Delete(unsigned char *data_temp,unsigned int *data_length,WIFI_State *wifi_State)
{
	unsigned int i;
	unsigned int temp;
	unsigned char expiry_mac_temp[20];
	unsigned long  long mac_temp;
	unsigned char mac_width;
	
	*data_length = 0;
	temp = 0;
	//Mac[12] = '\r';
	//Mac[13] = '\n';
	
	mac_temp = strtoll((const char *)wifi_State->wifi_state.Wifi_mac,NULL,16);

	if(mac_temp)
	{
		memcpy(data_temp,Send_DeleteCmd,21);
		temp += 21;

		mac_temp %= 10000;
		
		sprintf(expiry_mac_temp,"%d",mac_temp);
		if(mac_temp > 999)
			mac_width = 4;
		else if(mac_temp > 99)
			mac_width = 3;
		else if(mac_temp > 9)
			mac_width = 2;
		else
			mac_width = 1;
		
		memcpy(data_temp + 21,expiry_mac_temp,mac_width);
		temp += mac_width;
		memcpy(data_temp + 21 + mac_width,Send_DeleteCmd + 25,strlen(Send_DeleteCmd + 25));
		memcpy(data_temp + 21 + mac_width + 15 ,wifi_State->wifi_state.Wifi_mac,12);
		temp += strlen(Send_DeleteCmd + 25);
		(* data_length) += temp;
	}
	else
	{
		temp = strlen(Send_DeleteCmd);
		memcpy(data_temp + *data_length,Send_DeleteCmd,temp);
		(* data_length) += temp;
	}
	
	
	temp = strlen(Head_Host);
	memcpy(data_temp + *data_length,Head_Host,temp);
	(* data_length) += temp;
	
	
	temp = strlen(Head_DeleteAuthorization);
	memcpy(data_temp + *data_length,Head_DeleteAuthorization,temp);
	for(i = 0; i < temp; i++)
	{
		if((memcmp(data_temp+ *data_length + i,"&se=",4) == 0) && (wifi_State->expiry_time))
		{
			sprintf(expiry_mac_temp,"%d",(wifi_State->expiry_time + 120));
			memcpy(data_temp+ *data_length + i + 4,expiry_mac_temp,10);  
		}
	}
	(* data_length) += temp;
	
	temp = strlen(Head_ContentType);
	memcpy(data_temp + *data_length,Head_ContentType,temp);
	(* data_length) += temp;
	
	temp = strlen(Head_UserAgent);
	memcpy(data_temp + *data_length,Head_UserAgent,temp);
	(* data_length) += temp;
	
	
	temp = strlen(Head_Connection);
	memcpy(data_temp + *data_length,Head_Connection,temp);
	(* data_length) += temp;
	
	temp = strlen(Head_BlankRow);
	memcpy(data_temp + *data_length,Head_BlankRow,temp);
	(* data_length) += temp;
	
	temp = strlen(Head_BlankRow);
	memcpy(data_temp + *data_length,Head_BlankRow,temp);
	(* data_length) += temp;
	
	wifi_State->wifi_state.Wifi_devicestate |= 0x0008;    // ????
}



// ????
// ?    ?????????        - 3
// ?   ??? ?????          - 2
// ?   ???????  ????   - 1
// ?   ??????? ????    - 0

typedef enum
{
	RECV_CHECKERROR = 0, 
	RECV_CHECKOK    = 1,
	RECV_ONGOING    = 2,
	RECV_NODATA     = 3,
	GET_ERROR       = 0,
	GET_OK          = 1,
	POST_With_Body_Error = 4,
	Rec_Data_Error = 5,
	Reset_WiFi,
	Http_Req_Error,
}RECV_CHECKSTATE;

typedef enum
{
	HTTP_CMD                = 0,                    // HTTP -CMD
	
	// POST STATE
	HTTP_TRANSCHUNKE_POST   = 11,                    // Transfer-Encoding  CHUNK 401 201 
	HTTP_CONTENT_POST       = 12,                    // Content-Type
	HTTP_SERVER_POST        = 13,                    // Server
	HTTP_STS_POST           = 14,                    // Strict-Transport-Security
	HTTP_DATE_POST          = 15,
	HTTP_BLANK_POST         = 16,	
	HTTP_DATALENGTH_POST    = 17,
	HTTP_DATA_POST          = 18,
	
	// DEL STATE
	HTTP_TRANSCHUNKE_DEL   = 21,                    // Transfer-Encoding  CHUNK 401 201 
	HTTP_CONTENT_DEL       = 22,                    // Content-Type
	HTTP_SERVER_DEL        = 23,                    // Server
	HTTP_BKP_DEL           = 24,                    // BrokerProperties 
	HTTP_PID_DEL           = 25,                    // BrokerProperties 
	HTTP_STS_DEL           = 26,                    // BrokerProperties 
	HTTP_DATE_DEL          = 27,
	HTTP_BLANK_DEL         = 28,	
	HTTP_DATALENGTH_DEL    = 29,
	HTTP_DATA_DEL          = 30,
	
	//
	HTTP_DATALENGTH_204    = 41,                    // 
	HTTP_CONTENT_204       = 42,                    // Content-Type
	HTTP_SERVER_204        = 43,                    // Server
	HTTP_STS_204           = 44,                    // Strict-Transport-Security
	HTTP_DATE_204          = 45,
	HTTP_BLANK_204         = 46,	

	// http state
	HTTP_CONTENT        = 1,                    // Content-Type
	HTTP_SERVER         = 2,                    // Server
	HTTP_DATE           = 3,
	HTTP_CONNECT        = 4,
	HTTP_DATALENGTH     = 5,
	HTTP_BLANK          = 6,	
	HTTP_DATA           = 7,
	HTTP_END            = 8
}HTTP_ANALYSIS;


unsigned char Check_PostResponse(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	unsigned char time_temp[6];
	unsigned short length;
	
	unsigned int i;
	unsigned int j;
	
	static unsigned int temp;
	static unsigned char Analysis_State;
	
	if(data_length > 3)
	{
		
		for(i = temp;i < data_length;i++)
		{
			
			if((memcmp(data_temp + i,"\r\n",2) == 0))
			{
				switch(Analysis_State)
				{
					case HTTP_CMD:
						
						for(j = temp;j < i; j++)
						{
							if((memcmp(data_temp  +j,"HTTP/",5) == 0))
							{
								if((memcmp(data_temp  +j + 9,"201",3) == 0))
									Analysis_State = HTTP_TRANSCHUNKE_POST;
								else if((memcmp(data_temp  +j + 9,"400",3) == 0))
									Analysis_State = HTTP_CONTENT;
								else if((memcmp(data_temp  +j + 9,"401",3) == 0))
									Analysis_State = HTTP_TRANSCHUNKE_POST;
								else 
									Analysis_State = HTTP_CONTENT;
								break;
							}
						}
					break;
					case HTTP_TRANSCHUNKE_POST:
						
						Analysis_State = HTTP_CONTENT_POST;
					break;		
					case HTTP_CONTENT_POST:
						
						Analysis_State = HTTP_SERVER_POST;
					break;				
					case HTTP_SERVER_POST:
						
						Analysis_State = HTTP_STS_POST;
					break;	
					case HTTP_STS_POST:
						
						Analysis_State = HTTP_DATE_POST;
					break;	
					case HTTP_DATE_POST:
						for(j = temp;j < i; j++)
						{
							if((memcmp(data_temp + j,"Date",4) == 0))
							{
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 11),2);
								wifi_State->gmt_time.tm_mday  = atoi(time_temp);
						
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 14),3);
								wifi_State->gmt_time.tm_mon  = atoi(time_temp);

								if(memcmp(time_temp,"Jan",3) == 0)
									wifi_State->gmt_time.tm_mon = 1;
								else if(memcmp(time_temp,"Feb",3) == 0)
									wifi_State->gmt_time.tm_mon = 2;
								else if(memcmp(time_temp,"Mar",3) == 0)
									wifi_State->gmt_time.tm_mon = 3;
								else if(memcmp(time_temp,"Apr",3) == 0)
									wifi_State->gmt_time.tm_mon = 4;
								else if(memcmp(time_temp,"May",3) == 0)
									wifi_State->gmt_time.tm_mon = 5;
								else if(memcmp(time_temp,"Jun",3) == 0)
									wifi_State->gmt_time.tm_mon = 6;
								else if(memcmp(time_temp,"Jul",3) == 0)
									wifi_State->gmt_time.tm_mon = 7;
								else if(memcmp(time_temp,"Aug",3) == 0)
									wifi_State->gmt_time.tm_mon = 8;
								else if(memcmp(time_temp,"Sep",3) == 0)
									wifi_State->gmt_time.tm_mon = 9;
								else if(memcmp(time_temp,"Oct",3) == 0)
									wifi_State->gmt_time.tm_mon = 10;
								else if(memcmp(time_temp,"Nov",3) == 0)
									wifi_State->gmt_time.tm_mon = 11;
								else if(memcmp(time_temp,"Dec",3) == 0)
									wifi_State->gmt_time.tm_mon = 12;								
								
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 18),4);
								wifi_State->gmt_time.tm_year  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 23),2);
								wifi_State->gmt_time.tm_hour  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 26),2);
								wifi_State->gmt_time.tm_min  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 29),2);
								wifi_State->gmt_time.tm_sec = atoi(time_temp);
								
								wifi_State->expiry_time = RTC_Set(wifi_State->gmt_time.tm_year,wifi_State->gmt_time.tm_mon,wifi_State->gmt_time.tm_mday,wifi_State->gmt_time.tm_hour,wifi_State->gmt_time.tm_min,wifi_State->gmt_time.tm_sec);
								if(wifi_State->expiry_time)
									wifi_State->expiry_time += 28800;                 // ????  GMT + 8    8*3600
								break;
								
							}
						}
						Analysis_State = HTTP_BLANK_POST;
					break;	
					case HTTP_BLANK_POST:
						Analysis_State = HTTP_DATALENGTH_POST;
					break;	
					case HTTP_DATALENGTH_POST:
						
							memset(time_temp,0,6);
							memcpy(time_temp,(data_temp + temp), (i-temp));
							//length = atoi(time_temp);
						
							length = (unsigned short)strtol(time_temp,NULL,16);    	// ??? ????? ???
					
							Analysis_State = HTTP_DATA_POST;
					
						if((data_length - i) == 2)
						{
							temp = 0;
							Analysis_State = 0;
							return RECV_CHECKOK;
						}
					break;	
					case HTTP_DATA_POST:
						if(length)
							Analysis_State = HTTP_DATALENGTH_POST;
						else
						{
							if(( data_length  - i ) < 3)
							{
								temp = 0;
								Analysis_State = 0;
								return RECV_CHECKOK;
							}
							else
								Analysis_State = HTTP_END;
							//Analysis_State = HTTP_END;
						}
					break;
					case HTTP_CONTENT:
						Analysis_State = HTTP_SERVER;
					break;
					case HTTP_SERVER:
						Analysis_State = HTTP_DATE;
					break;
					case HTTP_DATE:
						for(j = temp;j < i; j++)
						{
							if((memcmp(data_temp + j,"Date",4) == 0))
							{
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 11),2);
								wifi_State->gmt_time.tm_mday  = atoi(time_temp);
						
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 14),3);
								wifi_State->gmt_time.tm_mon  = atoi(time_temp);

								if(memcmp(time_temp,"Jan",3) == 0)
									wifi_State->gmt_time.tm_mon = 1;
								else if(memcmp(time_temp,"Feb",3) == 0)
									wifi_State->gmt_time.tm_mon = 2;
								else if(memcmp(time_temp,"Mar",3) == 0)
									wifi_State->gmt_time.tm_mon = 3;
								else if(memcmp(time_temp,"Apr",3) == 0)
									wifi_State->gmt_time.tm_mon = 4;
								else if(memcmp(time_temp,"May",3) == 0)
									wifi_State->gmt_time.tm_mon = 5;
								else if(memcmp(time_temp,"Jun",3) == 0)
									wifi_State->gmt_time.tm_mon = 6;
								else if(memcmp(time_temp,"Jul",3) == 0)
									wifi_State->gmt_time.tm_mon = 7;
								else if(memcmp(time_temp,"Aug",3) == 0)
									wifi_State->gmt_time.tm_mon = 8;
								else if(memcmp(time_temp,"Sep",3) == 0)
									wifi_State->gmt_time.tm_mon = 9;
								else if(memcmp(time_temp,"Oct",3) == 0)
									wifi_State->gmt_time.tm_mon = 10;
								else if(memcmp(time_temp,"Nov",3) == 0)
									wifi_State->gmt_time.tm_mon = 11;
								else if(memcmp(time_temp,"Dec",3) == 0)
									wifi_State->gmt_time.tm_mon = 12;								
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 18),4);
								wifi_State->gmt_time.tm_year  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 23),2);
								wifi_State->gmt_time.tm_hour  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 26),2);
								wifi_State->gmt_time.tm_min  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 29),2);
								wifi_State->gmt_time.tm_sec = atoi(time_temp);
								
								wifi_State->expiry_time = RTC_Set(wifi_State->gmt_time.tm_year,wifi_State->gmt_time.tm_mon,wifi_State->gmt_time.tm_mday,wifi_State->gmt_time.tm_hour,wifi_State->gmt_time.tm_min,wifi_State->gmt_time.tm_sec);
								if(wifi_State->expiry_time)
									wifi_State->expiry_time += 28800;                 // ????  GMT + 8    8*3600
								break;
							}
						}
						Analysis_State = HTTP_CONNECT;
					break;
					case HTTP_CONNECT:
						Analysis_State = HTTP_DATALENGTH;
					break;
					case HTTP_DATALENGTH:
						
						if((memcmp(data_temp + temp ,"Con",3) == 0))
						{
							memset(time_temp,0,6);
							memcpy(time_temp,(data_temp + temp + 16), 3);
							length = atoi(time_temp);
							Analysis_State = HTTP_BLANK;
						}
					break;	
					case HTTP_BLANK:
						Analysis_State = HTTP_DATA;
					break;	
					case HTTP_DATA:
						Analysis_State = HTTP_END;
					break;
					case HTTP_END:
						temp = 0;
						Analysis_State = 0;
						return RECV_CHECKOK;
					break;
					default:					
					break;
				}
				temp = i + 2;
			}
			else if((data_length - i) == 1)
				return RECV_ONGOING;
		}
		return RECV_ONGOING;
	}
	else
	{
		temp = 0;
		Analysis_State = 0;
		return RECV_NODATA;
	}
}

unsigned char Check_DeleteResponse(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	unsigned char time_temp[6];
	unsigned short length;
	
	unsigned int i;
	unsigned int j;
	
	static unsigned int temp;
	static unsigned char Analysis_State;
	
	if(data_length > 3)
	{
		
		for(i = temp;i < data_length;i++)
		{
			if((memcmp(data_temp + i,"\r\n",2) == 0))
			{
				switch(Analysis_State)
				{
					case HTTP_CMD:
						
						for(j = temp;j < i; j++)
						{
							if((memcmp(data_temp  +j,"HTTP/",5) == 0))
							{
								if((memcmp(data_temp  +j + 9,"200",3) == 0))
									Analysis_State = HTTP_TRANSCHUNKE_DEL;
								else if((memcmp(data_temp  +j + 9,"204",3) == 0))
									Analysis_State = HTTP_DATALENGTH_204;
								else if((memcmp(data_temp  +j + 9,"400",3) == 0))
									Analysis_State = HTTP_CONTENT;
								else if((memcmp(data_temp  +j + 9,"401",3) == 0))
									Analysis_State = HTTP_TRANSCHUNKE_POST;
								else if((memcmp(data_temp  +j + 9,"404",3) == 0))
									Analysis_State = HTTP_TRANSCHUNKE_POST;
								else 
									Analysis_State = HTTP_CONTENT;

								break;
							}
						}
					break;
					case HTTP_TRANSCHUNKE_POST:
						
						Analysis_State = HTTP_CONTENT_POST;
					break;		
					case HTTP_CONTENT_POST:
						
						Analysis_State = HTTP_SERVER_POST;
					break;				
					case HTTP_SERVER_POST:
						
						Analysis_State = HTTP_STS_POST;
					break;	
					case HTTP_STS_POST:
						
						Analysis_State = HTTP_DATE_POST;
					break;	
					case HTTP_DATE_POST:
						for(j = temp;j < i; j++)
						{
							if((memcmp(data_temp + j,"Date",4) == 0))
							{
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 11),2);
								wifi_State->gmt_time.tm_mday  = atoi(time_temp);
						
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 14),3);
								wifi_State->gmt_time.tm_mon  = atoi(time_temp);

								if(memcmp(time_temp,"Jan",3) == 0)
									wifi_State->gmt_time.tm_mon = 1;
								else if(memcmp(time_temp,"Feb",3) == 0)
									wifi_State->gmt_time.tm_mon = 2;
								else if(memcmp(time_temp,"Mar",3) == 0)
									wifi_State->gmt_time.tm_mon = 3;
								else if(memcmp(time_temp,"Apr",3) == 0)
									wifi_State->gmt_time.tm_mon = 4;
								else if(memcmp(time_temp,"May",3) == 0)
									wifi_State->gmt_time.tm_mon = 5;
								else if(memcmp(time_temp,"Jun",3) == 0)
									wifi_State->gmt_time.tm_mon = 6;
								else if(memcmp(time_temp,"Jul",3) == 0)
									wifi_State->gmt_time.tm_mon = 7;
								else if(memcmp(time_temp,"Aug",3) == 0)
									wifi_State->gmt_time.tm_mon = 8;
								else if(memcmp(time_temp,"Sep",3) == 0)
									wifi_State->gmt_time.tm_mon = 9;
								else if(memcmp(time_temp,"Oct",3) == 0)
									wifi_State->gmt_time.tm_mon = 10;
								else if(memcmp(time_temp,"Nov",3) == 0)
									wifi_State->gmt_time.tm_mon = 11;
								else if(memcmp(time_temp,"Dec",3) == 0)
									wifi_State->gmt_time.tm_mon = 12;								
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 18),4);
								wifi_State->gmt_time.tm_year  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 23),2);
								wifi_State->gmt_time.tm_hour  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 26),2);
								wifi_State->gmt_time.tm_min  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 29),2);
								wifi_State->gmt_time.tm_sec = atoi(time_temp);
								
								wifi_State->expiry_time = RTC_Set(wifi_State->gmt_time.tm_year,wifi_State->gmt_time.tm_mon,wifi_State->gmt_time.tm_mday,wifi_State->gmt_time.tm_hour,wifi_State->gmt_time.tm_min,wifi_State->gmt_time.tm_sec);
								if(wifi_State->expiry_time)
									wifi_State->expiry_time += 28800;                 // ????  GMT + 8    8*3600
								
								break;
							}
						}
						Analysis_State = HTTP_BLANK_POST;
					break;	
					case HTTP_BLANK_POST:
						
						Analysis_State = HTTP_DATALENGTH_POST;
					break;	
					case HTTP_DATALENGTH_POST:
						
							memset(time_temp,0,6);
							memcpy(time_temp,(data_temp + temp), (i-temp));
							//length = atoi(time_temp);
						
							length = (unsigned short)strtol(time_temp,NULL,16);    	// ??? ????? ???
					
							Analysis_State = HTTP_DATA_POST;
					break;	
					case HTTP_DATA_POST:
						
						if(length)
							Analysis_State = HTTP_DATALENGTH_POST;
						else
							Analysis_State = HTTP_END;
						
					break;
					case HTTP_DATALENGTH_204:
						
						Analysis_State = HTTP_CONTENT_204;
					break;		
					case HTTP_CONTENT_204:
						
						Analysis_State = HTTP_SERVER_204;
					break;				
					case HTTP_SERVER_204:
						
						Analysis_State = HTTP_STS_204;
					break;	
					case HTTP_STS_204:
						
						Analysis_State = HTTP_DATE_204;
					break;	
					case HTTP_DATE_204:
						for(j = temp;j < i; j++)
						{
							if((memcmp(data_temp + j,"Date",4) == 0))
							{
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 11),2);
								wifi_State->gmt_time.tm_mday  = atoi(time_temp);
						
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 14),3);
								wifi_State->gmt_time.tm_mon  = atoi(time_temp);

								if(memcmp(time_temp,"Jan",3) == 0)
									wifi_State->gmt_time.tm_mon = 1;
								else if(memcmp(time_temp,"Feb",3) == 0)
									wifi_State->gmt_time.tm_mon = 2;
								else if(memcmp(time_temp,"Mar",3) == 0)
									wifi_State->gmt_time.tm_mon = 3;
								else if(memcmp(time_temp,"Apr",3) == 0)
									wifi_State->gmt_time.tm_mon = 4;
								else if(memcmp(time_temp,"May",3) == 0)
									wifi_State->gmt_time.tm_mon = 5;
								else if(memcmp(time_temp,"Jun",3) == 0)
									wifi_State->gmt_time.tm_mon = 6;
								else if(memcmp(time_temp,"Jul",3) == 0)
									wifi_State->gmt_time.tm_mon = 7;
								else if(memcmp(time_temp,"Aug",3) == 0)
									wifi_State->gmt_time.tm_mon = 8;
								else if(memcmp(time_temp,"Sep",3) == 0)
									wifi_State->gmt_time.tm_mon = 9;
								else if(memcmp(time_temp,"Oct",3) == 0)
									wifi_State->gmt_time.tm_mon = 10;
								else if(memcmp(time_temp,"Nov",3) == 0)
									wifi_State->gmt_time.tm_mon = 11;
								else if(memcmp(time_temp,"Dec",3) == 0)
									wifi_State->gmt_time.tm_mon = 12;								
								
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 18),4);
								wifi_State->gmt_time.tm_year  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 23),2);
								wifi_State->gmt_time.tm_hour  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 26),2);
								wifi_State->gmt_time.tm_min  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 29),2);
								wifi_State->gmt_time.tm_sec = atoi(time_temp);
								
								wifi_State->expiry_time = RTC_Set(wifi_State->gmt_time.tm_year,wifi_State->gmt_time.tm_mon,wifi_State->gmt_time.tm_mday,wifi_State->gmt_time.tm_hour,wifi_State->gmt_time.tm_min,wifi_State->gmt_time.tm_sec);
								if(wifi_State->expiry_time)
									wifi_State->expiry_time += 28800;                 // ????  GMT + 8    8*3600
								break;
								
							}
						}
						Analysis_State = HTTP_BLANK_204;
					break;	
					case HTTP_BLANK_204:
						if(( data_length - i) < 3)
						{
							temp = 0;
							Analysis_State = 0;
							return RECV_CHECKOK;
						}
						else
							Analysis_State = HTTP_END;
						
					break;	
					case HTTP_TRANSCHUNKE_DEL:
						Analysis_State = HTTP_CONTENT_DEL;
					break;		
					case HTTP_CONTENT_DEL:
						
						Analysis_State = HTTP_SERVER_DEL;
					break;				
					case HTTP_SERVER_DEL:
						
						Analysis_State = HTTP_BKP_DEL;
					break;	
					case HTTP_BKP_DEL:
						
						Analysis_State = HTTP_PID_DEL;
					break;	
					case HTTP_PID_DEL:
						
						Analysis_State = HTTP_STS_DEL;
					break;
					
					case HTTP_STS_DEL:
						
						Analysis_State = HTTP_DATE_DEL;
					break;
					case HTTP_DATE_DEL:
						for(j = temp;j < i; j++)
						{
							if((memcmp(data_temp + j,"Date",4) == 0))
							{
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 11),2);
								wifi_State->gmt_time.tm_mday  = atoi(time_temp);
						
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 14),3);
								wifi_State->gmt_time.tm_mon  = atoi(time_temp);

								if(memcmp(time_temp,"Jan",3) == 0)
									wifi_State->gmt_time.tm_mon = 1;
								else if(memcmp(time_temp,"Feb",3) == 0)
									wifi_State->gmt_time.tm_mon = 2;
								else if(memcmp(time_temp,"Mar",3) == 0)
									wifi_State->gmt_time.tm_mon = 3;
								else if(memcmp(time_temp,"Apr",3) == 0)
									wifi_State->gmt_time.tm_mon = 4;
								else if(memcmp(time_temp,"May",3) == 0)
									wifi_State->gmt_time.tm_mon = 5;
								else if(memcmp(time_temp,"Jun",3) == 0)
									wifi_State->gmt_time.tm_mon = 6;
								else if(memcmp(time_temp,"Jul",3) == 0)
									wifi_State->gmt_time.tm_mon = 7;
								else if(memcmp(time_temp,"Aug",3) == 0)
									wifi_State->gmt_time.tm_mon = 8;
								else if(memcmp(time_temp,"Sep",3) == 0)
									wifi_State->gmt_time.tm_mon = 9;
								else if(memcmp(time_temp,"Oct",3) == 0)
									wifi_State->gmt_time.tm_mon = 10;
								else if(memcmp(time_temp,"Nov",3) == 0)
									wifi_State->gmt_time.tm_mon = 11;
								else if(memcmp(time_temp,"Dec",3) == 0)
									wifi_State->gmt_time.tm_mon = 12;								
								
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 18),4);
								wifi_State->gmt_time.tm_year  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 23),2);
								wifi_State->gmt_time.tm_hour  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 26),2);
								wifi_State->gmt_time.tm_min  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 29),2);
								wifi_State->gmt_time.tm_sec = atoi(time_temp);
								
								wifi_State->expiry_time = RTC_Set(wifi_State->gmt_time.tm_year,wifi_State->gmt_time.tm_mon,wifi_State->gmt_time.tm_mday,wifi_State->gmt_time.tm_hour,wifi_State->gmt_time.tm_min,wifi_State->gmt_time.tm_sec);
								if(wifi_State->expiry_time)
									wifi_State->expiry_time += 28800;                 // ????  GMT + 8    8*3600
								
								break;
								
							}
						}
						Analysis_State = HTTP_BLANK_DEL;
					break;	
					case HTTP_BLANK_DEL:
						
						Analysis_State = HTTP_DATALENGTH_DEL;
					break;	
					case HTTP_DATALENGTH_DEL:
						
							memset(time_temp,0,6);
							memcpy(time_temp,(data_temp + temp), (i-temp));
							//length = atoi(time_temp);
						
							length = (unsigned short)strtol(time_temp,NULL,16);    	// ??? ????? ???
					
							Analysis_State = HTTP_DATA_DEL;
					break;	
					case HTTP_DATA_DEL:
						
						if(length)
						{
							Analysis_State = HTTP_DATALENGTH_DEL;
							
//							memset(wifi_State->mode_Control.timer_order,0,505);
//							wifi_State->mode_Control.current_mode = 0;
							
							for(j = temp; j < i; j ++ )
							{
								if(memcmp(data_temp + j,":[\"",3) == 0)
								{
									if(*(data_temp + j + 3) == 'V')
									{
										//memcpy(wifi_State->mode_Control.modeControl,data_temp + j + 3,1);

									//	wifi_State->mode_Control.current_mode  = 'V';
										wifi_State->mode_Control.mode = 3;
										break;
									}
									else if(*(data_temp + j + 3) == 'W')
									{
										memcpy(wifi_State->mode_Control.start_date,data_temp + j +  4 ,14);
										memcpy(wifi_State->mode_Control.end_date,data_temp + j +  19 ,14);
										
										//wifi_State->mode_Control.current_mode = 'W';
										wifi_State->mode_Control.mode = 1;
										break;
									}										
									else if(*(data_temp + j + 3) == '0')
									{
										memcpy(wifi_State->mode_Control.timer_order,data_temp + j + 3,504);
										//wifi_State->mode_Control.current_mode = 0x00;
										//wifi_State->mode_Control.current_mode = 2;
										wifi_State->mode_Control.mode = 2;
										break;
									}	
									else if(*(data_temp + j + 3) == 'X')
									{
									//	wifi_State->mode_Control.current_mode = 'X';
										wifi_State->mode_Control.mode = 4;
										break;
									}	
									else if(*(data_temp + j + 3) == 'Y')
									{
										//wifi_State->mode_Control.current_mode = 'Y';
										wifi_State->mode_Control.mode = 5;
										break;
									}	
									else if(*(data_temp + j + 3) == 'Z')
									{
										//wifi_State->mode_Control.current_mode = 'Z';
										wifi_State->mode_Control.mode = 6;
										break;
									}	
								}
							}
							
							Analysis_State = HTTP_END;
						}
						else
							Analysis_State = HTTP_END;
						
						if(( data_length - i) < 3)
						{
							temp = 0;
							Analysis_State = 0;
							return RECV_CHECKOK;
						}
						else
							Analysis_State = HTTP_END;
						
					break;

					case HTTP_CONTENT:
						Analysis_State = HTTP_SERVER;
					break;
					case HTTP_SERVER:
						Analysis_State = HTTP_DATE;
					break;
					case HTTP_DATE:
						for(j = temp;j < i; j++)
						{
							if((memcmp(data_temp + j,"Date",4) == 0))
							{
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 11),2);
								wifi_State->gmt_time.tm_mday  = atoi(time_temp);
						
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 14),3);
								wifi_State->gmt_time.tm_mon  = atoi(time_temp);

								if(memcmp(time_temp,"Jan",3) == 0)
									wifi_State->gmt_time.tm_mon = 1;
								else if(memcmp(time_temp,"Feb",3) == 0)
									wifi_State->gmt_time.tm_mon = 2;
								else if(memcmp(time_temp,"Mar",3) == 0)
									wifi_State->gmt_time.tm_mon = 3;
								else if(memcmp(time_temp,"Apr",3) == 0)
									wifi_State->gmt_time.tm_mon = 4;
								else if(memcmp(time_temp,"May",3) == 0)
									wifi_State->gmt_time.tm_mon = 5;
								else if(memcmp(time_temp,"Jun",3) == 0)
									wifi_State->gmt_time.tm_mon = 6;
								else if(memcmp(time_temp,"Jul",3) == 0)
									wifi_State->gmt_time.tm_mon = 7;
								else if(memcmp(time_temp,"Aug",3) == 0)
									wifi_State->gmt_time.tm_mon = 8;
								else if(memcmp(time_temp,"Sep",3) == 0)
									wifi_State->gmt_time.tm_mon = 9;
								else if(memcmp(time_temp,"Oct",3) == 0)
									wifi_State->gmt_time.tm_mon = 10;
								else if(memcmp(time_temp,"Nov",3) == 0)
									wifi_State->gmt_time.tm_mon = 11;
								else if(memcmp(time_temp,"Dec",3) == 0)
									wifi_State->gmt_time.tm_mon = 12;								
								
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 18),4);
								wifi_State->gmt_time.tm_year  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 23),2);
								wifi_State->gmt_time.tm_hour  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 26),2);
								wifi_State->gmt_time.tm_min  = atoi(time_temp);
								memset(time_temp,0,6);
								memcpy(time_temp,(data_temp  +j + 29),2);
								wifi_State->gmt_time.tm_sec = atoi(time_temp);
								
								wifi_State->expiry_time = RTC_Set(wifi_State->gmt_time.tm_year,wifi_State->gmt_time.tm_mon,wifi_State->gmt_time.tm_mday,wifi_State->gmt_time.tm_hour,wifi_State->gmt_time.tm_min,wifi_State->gmt_time.tm_sec);
								if(wifi_State->expiry_time)
									wifi_State->expiry_time += 28800;                 // ????  GMT + 8    8*3600
								
								break;
								
							}
						}
						Analysis_State = HTTP_CONNECT;
					break;
					case HTTP_CONNECT:
						Analysis_State = HTTP_DATALENGTH;
					break;
					case HTTP_DATALENGTH:
						
						if((memcmp(data_temp + temp ,"Con",3) == 0))
						{
							memset(time_temp,0,6);
							memcpy(time_temp,(data_temp + temp + 16), 3);
							length = atoi(time_temp);
							
							Analysis_State = HTTP_BLANK;
						}

					break;	
					case HTTP_BLANK:
						Analysis_State = HTTP_DATA;
					break;	
					case HTTP_DATA:
						
						if((data_length - i) < 3)
						{
							temp = 0;
							Analysis_State = 0;
							return RECV_CHECKOK;
						}
						else
							Analysis_State = HTTP_END;
					
						Analysis_State = HTTP_END;
					break;
					case HTTP_END:
						temp = 0;
						Analysis_State = 0;
						return RECV_CHECKOK;
					//break;
					default:					
					break;

				}
				
				temp = i + 2;
			}
			else if((data_length - i) == 1)
				return RECV_ONGOING;
		}
		return RECV_ONGOING;
	}
	else
	{
		temp = 0;
		Analysis_State = 0;
		return RECV_NODATA;
	}
}

unsigned char check_a(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	if(data_length)
	{
		if(memcmp(data_temp,"a",1) == 0)
			return RECV_CHECKOK;
		else
			return RECV_CHECKERROR;
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char check_ok(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{

	if(data_length)
	{
		if(data_length > 6)
		{
			if(memcmp(data_temp,"+ok\r\n\r\n",7) == 0)
				return RECV_CHECKOK;
			else
				return RECV_CHECKERROR;			
		}
		else
			return RECV_ONGOING;
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Get_Mac(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	unsigned int i;
	
	if(data_length > 3)
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok=",4) == 0)
				{
					memcpy(wifi_State->wifi_state.Wifi_mac,data_temp + i +  4,12);
					return GET_OK;
				}
				
			}
			return GET_ERROR;

		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Get_Connect(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	unsigned int i;
	
	if(data_length > 3)
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if((memcmp((data_temp + i),"+ok=",4) ==0 ) && (memcmp(data_temp  + i + 4,"Disconnect",10) == 0))
				{
					wifi_State->wifi_state.Wifi_devicestate &= ~0x0600;
					wifi_State->wifi_state.Wifi_devicestate |= 0x0200;
					
					WiFi_State.Network_OK = 0;
					WiFi_State.Login_OK = 0;
					
					Bluetooth_State.wifi_connect_status = ble_wifi_connect_error_599;
					
					return GET_OK;
				}
				else if(memcmp((data_temp + i),"+ok=",4) ==0)
				{
					wifi_State->wifi_state.Wifi_devicestate &= ~0x0600;
					wifi_State->wifi_state.Wifi_devicestate |= 0x0400;  
					
					WiFi_State.Network_OK = 1;

					return GET_OK;
				}
					
			}
			return GET_ERROR;

		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}


unsigned char Get_State(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok=",4) == 0)
				{

					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Check_Config(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	return NULL;
}
unsigned char Check_SSLADDR(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{

					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}
unsigned char Check_NTPSER(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{

					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Check_NTPON(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{

					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}
unsigned char Check_REBOOT(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	WiFi_State.WIFI_Reboot = 1;
	return Reset_WiFi;
}

unsigned char Check_Exit(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	unsigned int i;
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok=",4) == 0)
				{

					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
		
		
	}
	else
	{
		return RECV_NODATA;
	}
}
unsigned char Check_name(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{
					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Check_Secret(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{
				
					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Check_sta(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{
					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Check_RESET(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{

					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}
WIFI_CMD Get_wifistate[8];

/** @defgroup TASK_TYPE
  * @{
  */
typedef enum
{
	TASK_IDLE               = 0,                 
	TYPE_WifiState          = 1,                 
	TYPE_WifiConfig         = 2,                  
	TYPE_Azure              = 10,                 
	TYPE_AzurePost          = 11,                 
	TYPE_AzureDelete        = 12,                 
	TYPE_AzurePOST_With_Body, 
	TYPE_AzureOTA           ,                 	  
}TASK_TYPE;


/** @defgroup WIFI_ATCmdState
  * @{
  */
typedef enum
{
	WIFI_Idle                = 0x00,                       
	WIFI_SendChar_plus	     = 0x01,                        
	WIFI_WaitResponse_a	     = 0x02,                       
	WIFI_SendChar_a	         = 0x03,                      
	WIFI_WaitResponse_ok	 = 0x04,                        
	WIFI_SendCmd             = 0x05,                        
	WIFI_WaitCmdResponse     = 0x06,                       
	WIFI_ExitCmd             = 0x07,                        
	WIFI_WaitOk              = 0x08,                       
}WIFI_ATCmdState;


// wifi to enter Cmd mode and exec AT cmd
// function   1,to get or set wifi module parameter(Mac,time,link_state)
//            2,to set the wifi module into SmarteLink mode for app config
static unsigned char Wifi_ATCmdTask(unsigned char * data_temp,unsigned int * data_length,WIFI_TASK * task)
{
	OS_ERR err;
	static unsigned char PlusTransCount = 0;
	static unsigned char a_ResponseCount = 0;
	static unsigned char CmdTransCount = 0;
	int response_state = -1u;

	switch(task->CMD_SEND_CTR)
	{
		case WIFI_Idle:
			PlusTransCount = 0;
			a_ResponseCount = 0;
			CmdTransCount = 0;
			response_state = 0;
			/*break;*/		
		case WIFI_SendChar_plus:
			OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err); 
			Send_Plus(data_temp,data_length,&task->wifi_hflpb100);
			PlusTransCount ++;
			OSTmrStart(&tmr1,&err);
			task->Response_TimeOut = 1;
			task->CMD_SEND_CTR = WIFI_WaitResponse_a;
			
			//printf("send plus(%s)\r\n",data_temp);		// for testing //
			
			break;
		case WIFI_WaitResponse_a:
			response_state = check_a(data_temp,*data_length,&task->wifi_hflpb100);
			
			if(response_state == RECV_CHECKOK)
			{
				task->CMD_SEND_CTR = WIFI_SendChar_a;
				OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err); 
			}
			else if(response_state == RECV_CHECKERROR)
			{
				task->CMD_SEND_CTR = WIFI_SendChar_plus;
				
				if(PlusTransCount > 5)
				{
					PlusTransCount = 0;
					task->CMD_SEND_CTR = WIFI_Idle;
					task->wifi_TaskType = TASK_IDLE;
					task->wifi_CmdCount = 0;
					return 0;
				}
			}
			else
			{
				task->CMD_SEND_CTR = WIFI_SendChar_plus;
				
				//printf("send plus (%s %d) nodata,send again\r\n",data_temp,task->CMD_SEND_CTR);		// for testing //
				
				if(!task->Response_TimeOut)
				{
					//printf("send plus timeout,send again\r\n");
					
					if(PlusTransCount > 5)
					{
						PlusTransCount = 0;
						task->CMD_SEND_CTR = WIFI_Idle;
						task->wifi_TaskType = TASK_IDLE;
						task->wifi_CmdCount = 0;
						return 0;
					}	
				}		
			}
			break;
		case WIFI_SendChar_a:
			PlusTransCount = 0;
			a_ResponseCount ++;
			OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);
			Send_Char_a(data_temp,data_length,&task->wifi_hflpb100);
			OSTmrStart(&tmr1,&err);
			task->Response_TimeOut = 1;
			task->CMD_SEND_CTR = WIFI_WaitResponse_ok;

			//printf("send a (%s %d)\r\n",data_temp,task->CMD_SEND_CTR);		// for testing //
			
			break;
		case WIFI_WaitResponse_ok:
			response_state = check_ok(data_temp,*data_length,&task->wifi_hflpb100);
		
			if(response_state == RECV_CHECKOK)
			{
				task->CMD_SEND_CTR = WIFI_SendCmd; 
				OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);
			}
			else if(response_state == RECV_CHECKERROR)
			{
				task->CMD_SEND_CTR = WIFI_SendChar_plus; 
				
				if(a_ResponseCount > 5)
				{
					a_ResponseCount = 0;
					task->CMD_SEND_CTR = WIFI_Idle;
					task->wifi_TaskType = TASK_IDLE;
					task->wifi_CmdCount = 0;					
					return 0;
				}   
			}
			else
			{
				task->CMD_SEND_CTR = WIFI_SendChar_plus; 

				//printf("send a (%s) nodata,send again\r\n",data_temp);		// for testing //
					
				if(!task->Response_TimeOut)
				{
					//printf("send a timeout,send again\r\n");		// for testing //
					
					if(a_ResponseCount > 5)
					{
						a_ResponseCount = 0;
						task->CMD_SEND_CTR = WIFI_Idle;
						task->wifi_TaskType = TASK_IDLE;
						task->wifi_CmdCount = 0;					
						return 0;
					}
				}
			}
			break;
		case WIFI_SendCmd:
			OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);
			task->CMD[CmdTransCount].Encode(data_temp,data_length,&task->wifi_hflpb100);
			OSTmrStart(&tmr1,&err);
			task->Response_TimeOut = 1;
			task->CMD_SEND_CTR = WIFI_WaitCmdResponse;

			//printf("send cmd(%s)\r\n",data_temp);		// for testing //
			
			break;
		case WIFI_WaitCmdResponse:
			response_state = task->CMD[CmdTransCount].Decode(data_temp,*data_length,&task->wifi_hflpb100);
		
			if(response_state == GET_OK)
			{
				CmdTransCount ++;
				if(task->wifi_CmdCount == CmdTransCount)
				{
					task->CMD_SEND_CTR = WIFI_ExitCmd;	
					OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err); 
				}
				else
					task->CMD_SEND_CTR = WIFI_SendCmd;	
			}
			else if(response_state == GET_ERROR)
			{
					task->CMD_SEND_CTR = WIFI_SendCmd;	
			}
			else if(response_state == Reset_WiFi)
			{
				task->CMD_SEND_CTR = WIFI_Idle;
				task->wifi_TaskType = TASK_IDLE;
				task->wifi_CmdCount = 0;
				OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);

			}
			else
			{
				if(!task->Response_TimeOut)
				{
					//printf("send cmd(%s) timeout,send again\r\n",data_temp);		// for testing //
					
					task->CMD_SEND_CTR = WIFI_SendCmd;	
				}
			}
			break;	
		case WIFI_ExitCmd:
			CmdTransCount = 0;
			OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);
			Send_Cmd_Exit(data_temp,data_length,&task->wifi_hflpb100);
			OSTmrStart(&tmr1,&err);
			task->Response_TimeOut = 1;
			task->CMD_SEND_CTR = WIFI_WaitOk;

			//printf("send exit cmd(%s)\r\n",data_temp);
			
			break;		
		case WIFI_WaitOk:
			response_state = Check_Exit(data_temp,*data_length,&task->wifi_hflpb100);
			
			WiFi_State.Enter_ENTM = 1;
			WiFi_State.Tmr5_Ctl = 1;
			
			OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);
			
			if(response_state == GET_OK)
			{
				task->CMD_SEND_CTR = WIFI_Idle;
				task->wifi_TaskType = TASK_IDLE;
				task->wifi_CmdCount = 0;
				
				/*OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);*/
			}
			else if(response_state == GET_ERROR)
			{
				task->CMD_SEND_CTR = WIFI_Idle;
				task->wifi_TaskType = TASK_IDLE;
				task->wifi_CmdCount = 0;
			}
			else
			{
				if(!task->Response_TimeOut)
				{
					//printf("send exit cmd(%s) timeout,send again\r\n",data_temp);		// for testing //
					
					task->CMD_SEND_CTR = WIFI_Idle;
					task->wifi_TaskType = TASK_IDLE;
					task->wifi_CmdCount = 0;
				}
			}
			break;		
		default:
			break;
	}

	return task->CMD_SEND_CTR;
}

/** @defgroup WIFI_toAzureState
  * @{
  */
typedef enum
{
	toAzureIdle					= 0x00,                        // 
	toAzureCmd					= 0x01,                        // 
	toAzureResponse				= 0x02,                        // 
	toAzureDelete				= 0x03,                        // 
	toAzureDeleteResponse		= 0x04,                        // 
}DEVICE_ToAzureState;


AzureState_MODE  AzureState = { toAzureIdle };
extern volatile unsigned char WIFIRunModeLoop;

// wifi communication with Azure cloud
// function   1, post device state to Azure  cloud
//            2, get parameter from Azure cloud

static unsigned char  Device_toAzureTask(unsigned char *data_temp,unsigned int *data_length,WIFI_TASK * task)
{
	OS_ERR err;
	unsigned char response_state = 0;
	static unsigned char AzureTransCount = 0;
	
	switch(AzureState.state)
	{
		case toAzureIdle:
			AzureTransCount = 0;
			OSTmrStop(&tmr2,OS_OPT_TMR_NONE,0,&err); 
			task->Response_TimeOut_POST = 1; // timeout flag
			AzureState.state = toAzureCmd;
		//break;
		case toAzureCmd:
			OSTmrStart(&tmr2,&err);
			task->CMD[AzureTransCount].Encode(data_temp,data_length,&task->wifi_hflpb100);
			AzureState.state = toAzureResponse;
			break;
		case toAzureResponse:
			response_state = task->CMD[AzureTransCount].Decode(data_temp,*data_length,&task->wifi_hflpb100);
			if(response_state == RECV_CHECKOK)
			{
				OSTmrStop(&tmr2,OS_OPT_TMR_NONE,0,&err); 
				AzureState.state = toAzureIdle;
				WiFi_State.PostTimes = NULL;
				
				task->CMD[AzureTransCount].Decode(data_temp,0,&task->wifi_hflpb100);
				if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0001)
				{
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFFE;
					//task->wifi_hflpb100.wifi_state.Wifi_devicestate |= 0x0002;
				}
				else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0008)
				{
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFF7;
					//task->wifi_hflpb100.wifi_state.Wifi_devicestate |= 0x0010;
				}
				else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0004)
				{
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= /*0xFFFB*/NULL;
					//task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFBFF;
					WiFi_State.Send_OK = 1;
				}
				else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0010)
				{
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= /*0xFFEF*/NULL;
					//task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFBFF;
					
					WiFi_State.Send_OK = 1;
				}
				
				task->wifi_TaskType = TASK_IDLE;
				task->wifi_CmdCount = 0;	
				AzureTransCount = 0;
			}
			else if(response_state == RECV_CHECKERROR)
			{
				AzureState.state = toAzureIdle;
	
				task->CMD[AzureTransCount].Decode(data_temp,0,&task->wifi_hflpb100);               
				
				if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0001)
				{
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFFE;
					//task->wifi_hflpb100.wifi_state.Wifi_devicestate |= 0x0002;
				}
				if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0008)
				{
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFF7;
					//task->wifi_hflpb100.wifi_state.Wifi_devicestate |= 0x0010;
				}
				else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0004)
				{
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFFB;
				}
				else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0010)
				{
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFEF;
				}
				
				task->wifi_TaskType = TASK_IDLE;
				task->wifi_CmdCount = 0;	
				AzureTransCount = 0;
			}
			else if(response_state == Rec_Data_Error)
			{
				if(!task->Response_TimeOut_POST)
				{
					task->Response_TimeOut_POST = 1;
					task->wifi_TaskType = 0;
					WIFIRunModeLoop = 5; // hard reset wifi module
					AzureState.state = toAzureIdle;
					printf("[Rec_Data_Error]tmr2 timeout,hard reset wifi!\r\n");
					return NULL;
				}
				else
				{
					return Http_Req_Error;
				}
			}
			else if(response_state == Http_Req_Error)
			{
				AzureState.state = toAzureIdle;
				WiFi_State.PostTimes++;
				
				if(WiFi_State.PostTimes > 3)
				{
					WiFi_State.PostTimes = 0;
					WIFIRunModeLoop = 5; // hard reset wifi module					
					printf("[Http_Req_Error]tmr2 timeout,hard reset wifi!\r\n");
					return NULL;
				}
				else	
				{
					return Http_Req_Error;
				}
			}
			else
			{
				if(!task->Response_TimeOut_POST)
				{
					AzureState.state = toAzureIdle;

					printf("[Response_TimeOut]tmr2 timeout,hard reset wifi!\r\n");
	
					task->CMD[AzureTransCount].Decode(data_temp,0,&task->wifi_hflpb100);             
					
					if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0001)
					{
						task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFFE;
						//task->wifi_hflpb100.wifi_state.Wifi_devicestate |= 0x0002;
					}
					if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0008)
					{
						task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFF7;
						//task->wifi_hflpb100.wifi_state.Wifi_devicestate |= 0x0010;
					}
					else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0004)
					{
						task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFFB;
					}
					else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0010)
					{
						task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFEF;
					}
				
					task->wifi_TaskType = TASK_IDLE;
					task->wifi_CmdCount = 0;	
					AzureTransCount = 0;
				}
			}
			break;
		case toAzureDelete:
			break;						
		case toAzureDeleteResponse:
			break;		
		default:
			break;
	}
	
	return AzureState.state;
}


void cjson_content_clip(STRING_ *StringIn)
{
	unsigned int offset = 0;

	for(offset = 0;offset < (strlen(StringIn->s) + 1); offset++)
	{
		if(memcmp(StringIn->s + offset, "\n\t", 2) == 0)
		{
			strncpy(StringIn->s + offset,StringIn->s + offset + 2,strlen(StringIn->s));
		}
		
		if(memcmp(StringIn->s + offset, "\n", 1) == 0)
		{
			strncpy(StringIn->s + offset,StringIn->s + offset + 1,strlen(StringIn->s));
		}
		
		if(memcmp(StringIn->s + offset, "\t", 1) == 0)
		{
			strncpy(StringIn->s + offset,StringIn->s + offset + 1,strlen(StringIn->s));
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////
extern BUFFER_HANDLE_ Base64_Decoder(const char* source);
extern STRING_HANDLE_ Base64_Encoder(BUFFER_HANDLE_ input);
Login_Body_Req POST_Login = { NULL,NULL,NULL };

static int hmac_sha256_base64(const char *devicekey,const char *sig_string,char *base64_output)
{
	  char hash_out[128] = { 0 };

	  //BUFFER *input_decode  = NULL;
	  //BUFFER *input_encode  = (BUFFER *)malloc(128);
	  //STRING *result = NULL;
	  static BUFFER_ BUFFER_F;
	  static STRING_ STRING_F;
	  
	  BUFFER_HANDLE_ input_decode = &BUFFER_F;
	  BUFFER_HANDLE_ input_encode = &BUFFER_F;
	  STRING_HANDLE_ result = &STRING_F;
	  
	  //input_decode->buffer = "aldesiotsuiterecette.azure-devices.net%2Fdevices%2FF0FE6B89C82C_AIR\n1521600707";
	  //input_encode->buffer = "aldesiotsuiterecette.azure-devices.net%2Fdevices%2FF0FE6B89C82C_AIR\n1521600707";
	  //result->s = "aldesiotsuiterecette.azure-devices.net%2Fdevices%2FF0FE6B89C82C_AIR\n1521600707";
	  
	  memset(input_decode, NULL, sizeof(input_decode));
	  memset(input_encode, NULL, sizeof(input_encode));
	  memset(result, NULL, sizeof(result));

	  input_decode = Base64_Decoder(devicekey); // base64 decode for ClientKey

	  hmac(SHA256, sig_string, strlen(sig_string), input_decode->buffer, input_decode->size, hash_out); // hmac-sha256

	  //input_encode->buffer = &hash_out[0];
	  input_encode->size = strlen(hash_out);
	  memcpy(input_encode->buffer,hash_out,input_encode->size);

	  result = Base64_Encoder(input_encode); 	// base64 encode

	  memcpy(base64_output, result->s, strlen(result->s));

	  if(strlen(base64_output) != 44)
	  {
		printf("\r\n%s\r\n",base64_output);
		memcpy(base64_output, "A6ojVp24OspcCoghf+pPE0O6gKL1DgyvG5mI+PY5c3g=", strlen("A6ojVp24OspcCoghf+pPE0O6gKL1DgyvG5mI+PY5c3g="));
	  }

	  //free(input_encode);
	  //free(input_decode);
	  //free(result);
	  //input_decode = NULL;
	  //input_encode = NULL;
	  //result = NULL; 

	  return NULL;
}

#if 0
static int  POST_With_Body_Send(unsigned char *sendbuf, unsigned int *sendbuf_len, WIFI_State *wifi_State)
{
	unsigned int offset = 0;
	unsigned int timestamp = 0;
	unsigned int i_sig = 0;
	unsigned int sig_offset = 0;
	unsigned char tim_temp[12] = { 0 };
	unsigned char sign_base64[45] = { 0 };
	unsigned char sign_hash[64] = { 0 };
	unsigned char sas_raw_string[] = "aldesiotsuiterecette.azure-devices.net%2Fdevices%2FF0FE6B89C82C_AIR\n1521600647";
	char devicekey[] = "XMfsmaI3ouqbMj8D4xwUPg=="; // 注册时返回的密钥，可选 primaryKey 和 secondaryKey
	unsigned char PostReqHeader[]= "POST /devices/F0FE6B89C82C_AIR/messages/events?api-version=2016-02-03 HTTP/1.1\r\n";
	const unsigned char PostReqHost[]= "Host: aldesiotsuiterecette.azure-devices.net\r\n";
	unsigned char PostReqAuthorization[200]= {"Authorization: SharedAccessSignature sr=aldesiotsuiterecette.azure-devices.net%2Fdevices%2FF0FE6B89C82C_AIR&sig=A6ojVp24OspcCoghf+pPE0O6gKL1DgyvG5mI+PY5c3g=&se=1521600647\r\n"};
	const unsigned char PostReqContentType[]= "Content-Type: application/json\r\n";
	const unsigned char PostReqUserAgent[]= "User-Agent: Aldes-Modem/1.0\r\n";
	const unsigned char PostReqConnection[]= "Connection: keep-alive\r\n";
	unsigned char PostReqContentLength[]= "Content-Length: 460\r\n";
	const unsigned char PostReqBlankRow[]= "\r\n";
	unsigned char PostReqBody[]= {"{\"productid\":\"F0FE6B89C81C_AIR\",\"modemid\":\"F0FE6B89C81C\",\"outside_tpt\":\"7\",\"reject_tpt\":\"1\",\"tii_est\":\"5\",\"extf_spd\":\"7\",\"extf_tsn\":\"6\",\"vi_spd\":\"3\",\"vv_tsn\":\"3\",\"ff_cpt\":\"5\",\"ffe_flw\":\"3\",\"extf_flw\":\"3\",\"dep_ind\":\"1\",\"ext_tpt\":\"3\",\"cve_csn\":\"7\",\"vi_csn\":\"1\",\"tte_csn\":\"4\",\"dtb_ind\":\"2\",\"echange_pwr\":\"1\",\"set_spd\":\"1\",\"exch_eng\":\"5\",\"co2_vmc\":\"4\",\"Temp_Capt\":\"5\",\"Hygr_Capt\":\"9\",\"Co2_Capt\":\"1\",\"cov1_Capt\":\"1\",\"cov2_Capt\":\"3\",\"pm2_5_Capt\":\"3\",\"pm10_Capt\":\"6\"}"};

	if(!wifi_State->expiry_time)
	{
		offset = GetSubStrPos(sas_raw_string,"\n");
		timestamp = atoi(&sas_raw_string[0] + offset + 1);
		sprintf(&tim_temp[0],"%d",(timestamp + 60)); 
	}
	else
	{
		sprintf(&tim_temp[0],"%d",wifi_State->expiry_time); 
	}

	// TODO 更新PostReqContentLength长度

	// TODO 更新PostReqBody MAC

	// SAS更新timestample和MAC
	memcpy(sas_raw_string + strlen(&sas_raw_string[0]) - 10, tim_temp,strlen(&tim_temp[0]));
	memcpy(sas_raw_string + 51, wifi_State->wifi_state.Wifi_mac,strlen(wifi_State->wifi_state.Wifi_mac));

	// 更新devicekey
	WIFITaskPend ();
	memcpy(devicekey,POST_Login.primaryKey,strlen(POST_Login.primaryKey));
	WIFITaskPost();

	hmac_sha256_base64(devicekey, sas_raw_string, sign_base64);

	//printf("\r\n%s\r\n",sign_base64);
	
	for(sig_offset = 0; sig_offset < 44; sig_offset++)
	{
		if(sign_base64[sig_offset] == '+')
		{
			sign_hash[i_sig + sig_offset] = '%';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = '2';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = 'b';
			
		}
		else if(sign_base64[sig_offset] == '/')
		{
			sign_hash[i_sig + sig_offset] = '%';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = '2';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = 'f';				
		}
		else if(sign_base64[sig_offset] == '=')
		{
			sign_hash[i_sig + sig_offset] = '%';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = '3';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = 'd';				
		}
		else
			sign_hash[sig_offset + i_sig] = sign_base64[sig_offset];
	}

	// PostReqAuthorization添加secret key和timestample
	offset = GetSubStrPos(PostReqAuthorization,"&sig=");
	memcpy(PostReqAuthorization + offset + 5, sign_hash,strlen(sign_hash));
	memcpy(PostReqAuthorization + offset + 5 + strlen(sign_hash), "&se=",4);
	memcpy(PostReqAuthorization + offset + 5 + strlen(sign_hash) + 4, tim_temp,10);
	memcpy(PostReqAuthorization + offset + 5 + strlen(sign_hash) + 14, "\r\n",2);
	// PostReqAuthorization更新MAC
	offset = GetSubStrPos(PostReqAuthorization,"devices%2F");
	memcpy(PostReqAuthorization + offset + 10, wifi_State->wifi_state.Wifi_mac,strlen(wifi_State->wifi_state.Wifi_mac));
	// PostReqHeader更新MAC
	memcpy(PostReqHeader + 14, wifi_State->wifi_state.Wifi_mac,strlen(wifi_State->wifi_state.Wifi_mac));

	//printf("\r\n--------------%s\r\n",wifi_State->wifi_state.Wifi_mac);
	
	strncat(sendbuf, &PostReqHeader[0], strlen(&PostReqHeader[0]));
	strncat(sendbuf, &PostReqHost[0], strlen(&PostReqHost[0]));
	strncat(sendbuf, &PostReqAuthorization[0], strlen(&PostReqAuthorization[0]));
	strncat(sendbuf, PostReqContentType, strlen(PostReqContentType));
	strncat(sendbuf, PostReqUserAgent, strlen(PostReqUserAgent));
	strncat(sendbuf, PostReqConnection, strlen(PostReqConnection));
	strncat(sendbuf, PostReqContentLength, strlen(PostReqContentLength));
	strncat(sendbuf, PostReqBlankRow, strlen(PostReqBlankRow));
	strncat(sendbuf, PostReqBody, strlen(PostReqBody));
	strncat(sendbuf, PostReqBlankRow, strlen(PostReqBlankRow));

	wifi_State->wifi_state.Wifi_devicestate |= 0x0010;      

	return *sendbuf_len = strlen(sendbuf);
}
#else
static int  POST_With_Body_Send(unsigned char *sendbuf, unsigned int *sendbuf_len, WIFI_State *wifi_State)
{
	unsigned int offset = 0;
	unsigned int timestamp = 0;
	unsigned int i_sig = 0;
	unsigned int sig_offset = 0;
	unsigned char tim_temp[12] = { 0 };
	unsigned char sign_base64[45] = { 0 };
	unsigned char sign_hash[64] = { 0 };
	unsigned char sas_raw_string[] = "aldesiotsuiterecette.azure-devices.net%2Fdevices%2FF0FE6B89C82C_OQAI\n1521600647";
	char devicekey[] = "XMfsmaI3ouqbMj8D4xwUPg=="; // 注册时返回的密钥，可选 primaryKey 和 secondaryKey
	unsigned char PostReqHeader[]= "POST /devices/F0FE6B89C82C_OQAI/messages/events?api-version=2016-02-03 HTTP/1.1\r\n";
	const unsigned char PostReqHost[]= "Host: aldesiotsuiterecette.azure-devices.net\r\n";
	unsigned char PostReqAuthorization[200]= {"Authorization: SharedAccessSignature sr=aldesiotsuiterecette.azure-devices.net%2Fdevices%2FF0FE6B89C82C_OQAI&sig=A6ojVp24OspcCoghf+pPE0O6gKL1DgyvG5mI+PY5c3g=&se=1521600647\r\n"};
	const unsigned char PostReqContentType[]= "Content-Type: application/json\r\n";
	const unsigned char PostReqUserAgent[]= "User-Agent: Aldes-Modem/1.0\r\n";
	const unsigned char PostReqConnection[]= "Connection: keep-alive\r\n";
	unsigned char PostReqContentLength[]= "Content-Length: 461\r\n";
	const unsigned char PostReqBlankRow[]= "\r\n";
	unsigned char PostReqBody[]= {"{\"productid\":\"F0FE6B89C81C_OQAI\",\"modemid\":\"F0FE6B89C81C\",\"outside_tpt\":\"7\",\"reject_tpt\":\"1\",\"tii_est\":\"5\",\"extf_spd\":\"7\",\"extf_tsn\":\"6\",\"vi_spd\":\"3\",\"vv_tsn\":\"3\",\"ff_cpt\":\"5\",\"ffe_flw\":\"3\",\"extf_flw\":\"3\",\"dep_ind\":\"1\",\"ext_tpt\":\"3\",\"cve_csn\":\"7\",\"vi_csn\":\"1\",\"tte_csn\":\"4\",\"dtb_ind\":\"2\",\"echange_pwr\":\"1\",\"set_spd\":\"1\",\"exch_eng\":\"5\",\"co2_vmc\":\"4\",\"Temp_Capt\":\"5\",\"Hygr_Capt\":\"9\",\"Co2_Capt\":\"1\",\"cov1_Capt\":\"1\",\"cov2_Capt\":\"3\",\"pm2_5_Capt\":\"3\",\"pm10_Capt\":\"6\"}"};

	if(!wifi_State->expiry_time)
	{
		offset = GetSubStrPos(sas_raw_string,"\n");
		timestamp = atoi(&sas_raw_string[0] + offset + 1);
		sprintf(&tim_temp[0],"%d",(timestamp + 60)); 
	}
	else
	{
		sprintf(&tim_temp[0],"%d",wifi_State->expiry_time); 
	}

	// TODO 更新PostReqContentLength长度

	// TODO 更新PostReqBody MAC

	// SAS更新timestample和MAC
	memcpy(sas_raw_string + strlen(&sas_raw_string[0]) - 10, tim_temp,strlen(&tim_temp[0]));
	memcpy(sas_raw_string + 51, wifi_State->wifi_state.Wifi_mac,strlen(wifi_State->wifi_state.Wifi_mac));

	// 更新devicekey
	WIFITaskPend ();
	memcpy(devicekey,POST_Login.primaryKey,strlen(POST_Login.primaryKey));
	WIFITaskPost();

	hmac_sha256_base64(devicekey, sas_raw_string, sign_base64);

	//printf("\r\n%s\r\n",sign_base64);
	
	for(sig_offset = 0; sig_offset < 44; sig_offset++)
	{
		if(sign_base64[sig_offset] == '+')
		{
			sign_hash[i_sig + sig_offset] = '%';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = '2';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = 'b';
			
		}
		else if(sign_base64[sig_offset] == '/')
		{
			sign_hash[i_sig + sig_offset] = '%';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = '2';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = 'f';				
		}
		else if(sign_base64[sig_offset] == '=')
		{
			sign_hash[i_sig + sig_offset] = '%';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = '3';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = 'd';				
		}
		else
			sign_hash[sig_offset + i_sig] = sign_base64[sig_offset];
	}

	// PostReqAuthorization添加secret key和timestample
	offset = GetSubStrPos(PostReqAuthorization,"&sig=");
	memcpy(PostReqAuthorization + offset + 5, sign_hash,strlen(sign_hash));
	memcpy(PostReqAuthorization + offset + 5 + strlen(sign_hash), "&se=",4);
	memcpy(PostReqAuthorization + offset + 5 + strlen(sign_hash) + 4, tim_temp,10);
	memcpy(PostReqAuthorization + offset + 5 + strlen(sign_hash) + 14, "\r\n",2);
	// PostReqAuthorization更新MAC
	offset = GetSubStrPos(PostReqAuthorization,"devices%2F");
	memcpy(PostReqAuthorization + offset + 10, wifi_State->wifi_state.Wifi_mac,strlen(wifi_State->wifi_state.Wifi_mac));
	// PostReqHeader更新MAC
	memcpy(PostReqHeader + 14, wifi_State->wifi_state.Wifi_mac,strlen(wifi_State->wifi_state.Wifi_mac));

	//printf("\r\n--------------%s\r\n",wifi_State->wifi_state.Wifi_mac);
	
	strncat(sendbuf, &PostReqHeader[0], strlen(&PostReqHeader[0]));
	strncat(sendbuf, &PostReqHost[0], strlen(&PostReqHost[0]));
	strncat(sendbuf, &PostReqAuthorization[0], strlen(&PostReqAuthorization[0]));
	strncat(sendbuf, PostReqContentType, strlen(PostReqContentType));
	strncat(sendbuf, PostReqUserAgent, strlen(PostReqUserAgent));
	strncat(sendbuf, PostReqConnection, strlen(PostReqConnection));
	strncat(sendbuf, PostReqContentLength, strlen(PostReqContentLength));
	strncat(sendbuf, PostReqBlankRow, strlen(PostReqBlankRow));
	strncat(sendbuf, PostReqBody, strlen(PostReqBody));
	strncat(sendbuf, PostReqBlankRow, strlen(PostReqBlankRow));

	wifi_State->wifi_state.Wifi_devicestate |= 0x0010;      

	return *sendbuf_len = strlen(sendbuf);
}


#endif
void Timstample_Parse(unsigned char *data_temp, int offset, WIFI_State *wifi_State)
{
	unsigned char time_temp[6];
	
	memset(time_temp,0,6);
	memcpy(time_temp,(data_temp  + offset + 11), 2);
	wifi_State->gmt_time.tm_mday  = atoi(time_temp);

	memset(time_temp,0,6);
	memcpy(time_temp,(data_temp  + offset + 14),3);
	wifi_State->gmt_time.tm_mon  = atoi(time_temp);

	if(memcmp(time_temp,"Jan",3) == 0)
		wifi_State->gmt_time.tm_mon = 1;
	else if(memcmp(time_temp,"Feb",3) == 0)
		wifi_State->gmt_time.tm_mon = 2;
	else if(memcmp(time_temp,"Mar",3) == 0)
		wifi_State->gmt_time.tm_mon = 3;
	else if(memcmp(time_temp,"Apr",3) == 0)
		wifi_State->gmt_time.tm_mon = 4;
	else if(memcmp(time_temp,"May",3) == 0)
		wifi_State->gmt_time.tm_mon = 5;
	else if(memcmp(time_temp,"Jun",3) == 0)
		wifi_State->gmt_time.tm_mon = 6;
	else if(memcmp(time_temp,"Jul",3) == 0)
		wifi_State->gmt_time.tm_mon = 7;
	else if(memcmp(time_temp,"Aug",3) == 0)
		wifi_State->gmt_time.tm_mon = 8;
	else if(memcmp(time_temp,"Sep",3) == 0)
		wifi_State->gmt_time.tm_mon = 9;
	else if(memcmp(time_temp,"Oct",3) == 0)
		wifi_State->gmt_time.tm_mon = 10;
	else if(memcmp(time_temp,"Nov",3) == 0)
		wifi_State->gmt_time.tm_mon = 11;
	else if(memcmp(time_temp,"Dec",3) == 0)
		wifi_State->gmt_time.tm_mon = 12;								
	
	memset(time_temp,0,6);
	memcpy(time_temp,(data_temp  + offset + 18),4);
	wifi_State->gmt_time.tm_year  = atoi(time_temp);
	memset(time_temp,0,6);
	memcpy(time_temp,(data_temp  + offset + 23),2);
	wifi_State->gmt_time.tm_hour  = atoi(time_temp);
	memset(time_temp,0,6);
	memcpy(time_temp,(data_temp  + offset + 26),2);
	wifi_State->gmt_time.tm_min  = atoi(time_temp);
	memset(time_temp,0,6);
	memcpy(time_temp,(data_temp  + offset + 29),2);
	wifi_State->gmt_time.tm_sec = atoi(time_temp);
	
	wifi_State->expiry_time = RTC_Set(wifi_State->gmt_time.tm_year,wifi_State->gmt_time.tm_mon,wifi_State->gmt_time.tm_mday,wifi_State->gmt_time.tm_hour,wifi_State->gmt_time.tm_min,wifi_State->gmt_time.tm_sec);
}

unsigned char POST_With_Body_Parse(unsigned char *data_temp, unsigned int data_length, WIFI_State *wifi_State)
{
	int offset = 0;
	unsigned char resp_buff[64] = { 0 };

	if((data_length <= 1)&&(data_temp[0] == 0))
		return Rec_Data_Error;

	if(data_length > 3)
	{
		//printf("\r\n%s\r\n",data_temp);
		
		offset = GetSubStrPos(data_temp,"Date:");

		memcpy(resp_buff,data_temp,sizeof(resp_buff));
		
		if((GetSubStrPos(resp_buff,"401")) >=0)
		{
			Timstample_Parse(data_temp,offset,wifi_State);
			tty.clr();
			return Http_Req_Error;
		}
		else if((GetSubStrPos(resp_buff,"400")) >=0)
		{
			Timstample_Parse(data_temp,offset,wifi_State);
			tty.clr();
			return Http_Req_Error;
		}
		else if((GetSubStrPos(resp_buff,"404")) >=0)
		{
			Timstample_Parse(data_temp,offset,wifi_State);
			tty.clr();
			return Http_Req_Error;
		}
		else if((GetSubStrPos(resp_buff,"403")) >=0)
		{
			tty.clr();
			return Http_Req_Error;
		}
		else if((GetSubStrPos(resp_buff,"500")) >=0)
		{
			Timstample_Parse(data_temp,offset,wifi_State);
			tty.clr();
			return Http_Req_Error;
		}
		else if((GetSubStrPos(resp_buff,"200")) >=0 || (GetSubStrPos(resp_buff,"204")) >=0)
		{
			Timstample_Parse(data_temp,offset,wifi_State);
			Uart_CommState = 0;
			tty.clr();
			return RECV_CHECKOK;
		}
		else
		{
			tty.clr();
			return Http_Req_Error;
		}
	}

	return Http_Req_Error;
}

#if 0
static int  POST_Login_Send(unsigned char *sendbuf, unsigned int *sendbuf_len, WIFI_State *wifi_State)
{
	int offset = 0;
	char DeviceId[32] = { 0 };
	unsigned char sign_base64[45] = { 0 };
	
	char post_login_body[64] = {"{\"DeviceId\":\"F0FE6B89C82C_AIR\",\"DeviceType\":\"AIR\"}"};	
	const unsigned char ClientKey[] = {"ngPjOnSx9VsNMTQF1z1g64CkduX/qBcNnKPGaWqR8naO8t1cHnngqHElLYWBwaOPNxwLvYeJdGnDUfWFI6lRXQ=="};
	unsigned char ProvisionningKey[] = {"\020F0FE6BBBA1C6_AIR\01720180316T113320\x25\xcd\xc6\xa5\x99\x62\x4a\xfc\xfd\x05\x73\x81\x1c\xa5\xeb\x7b\004POST\031/api/v1/devices/AddDevice{\"DeviceId\":\"F0FE6BBBA1C6_AIR\",\"DeviceType\":\"AIR\"}"};
	const unsigned char Nonce[] = {"Jc3GpZliSvz9BXOBHKXrew=="};
	unsigned char PostReqHeader[]= "POST /api/v1/devices/AddDevice HTTP/1.1\r\n";
	const unsigned char PostReqHost[]= "Host: aldesiotsuiterecette-aldesprovisionning.azurewebsites.net\r\n";
	unsigned char PostReqAuthorization[200]= {"Authorization: ApiAuth-v1-Hmac256 F0FE6BBBA1C6_AIR 20180316T113320 Jc3GpZliSvz9BXOBHKXrew== OQasnQYdjdE99J1tBG+9p0K5wz2BMDtYoKXlZeIy3Cs=\r\n"};
	const unsigned char PostReqContentType[]= "Content-Type: application/json\r\n";
	const unsigned char PostReqUserAgent[]= "User-Agent: Aldes-Modem/1.0\r\n";
	const unsigned char PostReqConnection[]= "Connection: keep-alive\r\n";
	unsigned char PostReqContentLength[]= "Content-Length: 50\r\n";
	const unsigned char PostReqBlankRow[]= "\r\n";

	// ProvisionningKey更新MAC
	memcpy(ProvisionningKey + 1, wifi_State->wifi_state.Wifi_mac,strlen(wifi_State->wifi_state.Wifi_mac));
	offset = GetSubStrPos(ProvisionningKey,"DeviceId");
	memcpy(ProvisionningKey + offset + 11, wifi_State->wifi_state.Wifi_mac,strlen(wifi_State->wifi_state.Wifi_mac));
	// 更新PostReqAuthorization中的MAC
	memcpy(PostReqAuthorization + 34, wifi_State->wifi_state.Wifi_mac,strlen(wifi_State->wifi_state.Wifi_mac));

	// TODO timestample / nonce decode /
	
	hmac_sha256_base64(ClientKey, ProvisionningKey, sign_base64);
	memcpy(PostReqAuthorization + strlen(PostReqAuthorization) - 46, sign_base64,44);
	// 更新DeviceId
	memcpy(wifi_State->wifi_state.Wifi_deviceid, ProvisionningKey + 1,16); 
	//AzureState.state = toAzureCmd;
	memset(post_login_body, NULL, strlen(post_login_body));
	create_post_login_objects(wifi_State->wifi_state.Wifi_deviceid,"AIR",post_login_body);
	AzureState.state = toAzureResponse;
	
	// 删除JSON中的\n、\t
	for(offset = 0;offset < (strlen(post_login_body) + 1); offset++)
	{
		if(memcmp(post_login_body + offset, "\n\t", 2) == 0)
		{
			strncpy(post_login_body + offset,post_login_body + offset + 2,strlen(post_login_body));
		}
		
		if(memcmp(post_login_body + offset, "\n", 1) == 0)
		{
			strncpy(post_login_body + offset,post_login_body + offset + 1,strlen(post_login_body));
		}
		
		if(memcmp(post_login_body + offset, "\t", 1) == 0)
		{
			strncpy(post_login_body + offset,post_login_body + offset + 1,strlen(post_login_body));
		}
	}

	strncat(sendbuf, &PostReqHeader[0], strlen(&PostReqHeader[0]));
	strncat(sendbuf, &PostReqHost[0], strlen(&PostReqHost[0]));
	strncat(sendbuf, &PostReqAuthorization[0], strlen(&PostReqAuthorization[0]));
	strncat(sendbuf, PostReqContentType, strlen(PostReqContentType));
	strncat(sendbuf, PostReqUserAgent, strlen(PostReqUserAgent));
	strncat(sendbuf, PostReqConnection, strlen(PostReqConnection));
	strncat(sendbuf, PostReqContentLength, strlen(PostReqContentLength));
	strncat(sendbuf, PostReqBlankRow, strlen(PostReqBlankRow));
	strncat(sendbuf, post_login_body, strlen(post_login_body));
	strncat(sendbuf, PostReqBlankRow, strlen(PostReqBlankRow));  

	wifi_State->wifi_state.Wifi_devicestate |= 0x0004;   

	return *sendbuf_len = strlen(sendbuf);
}

#else

static int  POST_Login_Send(unsigned char *sendbuf, unsigned int *sendbuf_len, WIFI_State *wifi_State)
{
	static STRING_ StrConv;
	int offset = 0;
	char DeviceId[32] = { 0 };
	unsigned char sign_base64[45] = { 0 };
	char post_login_body[64] = {"{\"DeviceId\":\"F0FE6BBBA1C6_OQAI\",\"DeviceType\":\"OQAI\"}"};
	const unsigned char ClientKey[] = {"ngPjOnSx9VsNMTQF1z1g64CkduX/qBcNnKPGaWqR8naO8t1cHnngqHElLYWBwaOPNxwLvYeJdGnDUfWFI6lRXQ=="};
	unsigned char ProvisionningKey[] = {"\021F0FE6B89C82C_OQAI\01720180316T113321\x25\xcd\xc6\xa5\x99\x62\x4a\xfc\xfd\x05\x73\x81\x1c\xa5\xeb\x7b\004POST\031/api/v1/devices/AddDevice{\"DeviceId\":\"F0FE6B89C82C_OQAI\",\"DeviceType\":\"OQAI\"}"};
	const unsigned char Nonce[] = {"Jc3GpZliSvz9BXOBHKXrew=="};
	unsigned char PostReqHeader[]= "POST /api/v1/devices/AddDevice HTTP/1.1\r\n";
	const unsigned char PostReqHost[]= "Host: aldesiotsuiterecette-aldesprovisionning.azurewebsites.net\r\n";
	unsigned char PostReqAuthorization[200]= {"Authorization: ApiAuth-v1-Hmac256 F0FE6BBBA1C6_OQAI 20180316T113321 Jc3GpZliSvz9BXOBHKXrew== OQasnQYdjdE99J1tBG+9p0K5wz2BMDtYoKXlZeIy3Cs=\r\n"};
	const unsigned char PostReqContentType[]= "Content-Type: application/json\r\n";
	const unsigned char PostReqUserAgent[]= "User-Agent: Aldes-Modem/1.0\r\n";
	const unsigned char PostReqConnection[]= "Connection: keep-alive\r\n";
	unsigned char PostReqContentLength[]= "Content-Length: 52\r\n";
	const unsigned char PostReqBlankRow[]= "\r\n";

	// ProvisionningKey更新MAC
	memcpy(ProvisionningKey + 1, wifi_State->wifi_state.Wifi_mac,strlen(wifi_State->wifi_state.Wifi_mac));
	offset = GetSubStrPos(ProvisionningKey,"DeviceId");
	memcpy(ProvisionningKey + offset + 11, wifi_State->wifi_state.Wifi_mac,strlen(wifi_State->wifi_state.Wifi_mac));
	// 更新PostReqAuthorization中的MAC
	memcpy(PostReqAuthorization + 34, wifi_State->wifi_state.Wifi_mac,strlen(wifi_State->wifi_state.Wifi_mac));

	// TODO timestample / nonce decode /
	// sha256 to base 64
	hmac_sha256_base64(ClientKey, ProvisionningKey, sign_base64);
	
	memcpy(PostReqAuthorization + strlen(PostReqAuthorization) - 46, sign_base64,44);
	// 更新DeviceId
	//memcpy(DeviceId, ProvisionningKey + 1,17); 
	// 更新DeviceId
	memcpy(wifi_State->wifi_state.Wifi_deviceid, ProvisionningKey + 1,17);
	//AzureState.state = toAzureCmd;
	memset(post_login_body, NULL, strlen(post_login_body));
	create_post_login_objects(wifi_State->wifi_state.Wifi_deviceid,"OQAI",post_login_body);
	AzureState.state = toAzureResponse;
	
	// 删除JSON中的\n、\t
	#if 0
	for(offset = 0;offset < (strlen(post_login_body) + 1); offset++)
	{
		if(memcmp(post_login_body + offset, "\n\t", 2) == 0)
		{
			strncpy(post_login_body + offset,post_login_body + offset + 2,strlen(post_login_body));
		}
		
		if(memcmp(post_login_body + offset, "\n", 1) == 0)
		{
			strncpy(post_login_body + offset,post_login_body + offset + 1,strlen(post_login_body));
		}
		
		if(memcmp(post_login_body + offset, "\t", 1) == 0)
		{
			strncpy(post_login_body + offset,post_login_body + offset + 1,strlen(post_login_body));
		}
	}
	#else
	
	memcpy(StrConv.s,post_login_body,strlen(post_login_body));
	cjson_content_clip(&StrConv);
	
	#endif

	strncat(sendbuf, &PostReqHeader[0], strlen(&PostReqHeader[0]));
	strncat(sendbuf, &PostReqHost[0], strlen(&PostReqHost[0]));
	strncat(sendbuf, &PostReqAuthorization[0], strlen(&PostReqAuthorization[0]));
	strncat(sendbuf, PostReqContentType, strlen(PostReqContentType));
	strncat(sendbuf, PostReqUserAgent, strlen(PostReqUserAgent));
	strncat(sendbuf, PostReqConnection, strlen(PostReqConnection));
	strncat(sendbuf, PostReqContentLength, strlen(PostReqContentLength));
	strncat(sendbuf, PostReqBlankRow, strlen(PostReqBlankRow));
	strncat(sendbuf, StrConv.s, strlen(StrConv.s));
	strncat(sendbuf, PostReqBlankRow, strlen(PostReqBlankRow));  

	wifi_State->wifi_state.Wifi_devicestate |= 0x0004;   

	return *sendbuf_len = strlen(sendbuf);
}

#endif

unsigned char POST_Login_Parse(unsigned char *data_temp, unsigned int data_length, WIFI_State *wifi_State)
{
	OS_ERR  err;
	unsigned int offset = 0;
	unsigned char resp_buff[64] = { 0 };

	if((data_length <= 1)&&(data_temp[0] == 0))
	{
		WiFi_State.Login_OK = 0;
		return Rec_Data_Error;
	}
	
	if(data_length > 3)
	{
		//printf("\r\n%s\r\n",data_temp);
		
		offset = GetSubStrPos(data_temp,"Date:");

		memcpy(resp_buff,data_temp,sizeof(resp_buff));

		if((GetSubStrPos(resp_buff,"401")) >=0)
		{
			Bluetooth_State.wifi_connect_status = ble_wifi_connect_error_401;
			Timstample_Parse(data_temp,offset,wifi_State);
			tty.clr();
			WiFi_State.Login_OK = 0;
			return Http_Req_Error;
		}
		else if((GetSubStrPos(resp_buff,"400")) >=0)
		{
			Timstample_Parse(data_temp,offset,wifi_State);
			tty.clr();
			WiFi_State.Login_OK = 0;
			return Http_Req_Error;
		}
		else if((GetSubStrPos(resp_buff,"404")) >=0)
		{
			Timstample_Parse(data_temp,offset,wifi_State);
			tty.clr();
			WiFi_State.Login_OK = 0;
			return Http_Req_Error;
		}
		else if((GetSubStrPos(resp_buff,"403")) >=0)
		{
			Timstample_Parse(data_temp,offset,wifi_State);
			tty.clr();
			WiFi_State.Login_OK = 0;
			return Http_Req_Error;
		}
		else if((GetSubStrPos(resp_buff,"500")) >=0)
		{
			Bluetooth_State.wifi_connect_status = ble_wifi_connect_error_500;
			Timstample_Parse(data_temp,offset,wifi_State);
			tty.clr();
			WiFi_State.Login_OK = 0;
			return Http_Req_Error;
		}
		else if(GetSubStrPos(resp_buff,"200") >=0 || GetSubStrPos(resp_buff,"201") >=0)
		{
			WiFi_State.Login_OK = 1;
			Bluetooth_State.wifi_connect_status = ble_wifi_connect_success;
			
			Timstample_Parse(data_temp,offset,wifi_State);
			
			offset = GetSubStrPos(data_temp,"primaryKey");
			WIFITaskPend();
			memcpy(POST_Login.primaryKey,data_temp + offset + 13,24);
			//cJSON_to_str(data_temp + offset + 7,"primaryKey",POST_Login.primaryKey);
			//Uart_CommState = 0;
			WIFITaskPost();
	
			tty.clr();
			return RECV_CHECKOK;
		}
		else
		{
			tty.clr();
			WiFi_State.Login_OK = 0;
			return Http_Req_Error;
		}
	}

	return Http_Req_Error;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void Wifi_GetSetParam(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_WifiState;

	Get_wifistate[0].Encode = Send_Cmd_reset;
	Get_wifistate[0].Decode  = Check_RESET;
	Get_wifistate[1].Encode = Send_Cmd_name;
	Get_wifistate[1].Decode = Check_name;
	Get_wifistate[2].Encode = Send_Cmd_Secret;
	Get_wifistate[2].Decode = Check_Secret;
	Get_wifistate[3].Encode = Send_Cmd_sta;
	Get_wifistate[3].Decode = Check_sta;
	Get_wifistate[4].Encode = Send_Cmd_REBOOT;
	Get_wifistate[4].Decode = Check_REBOOT;	
	
	task->wifi_CmdCount = 5;
	task->CMD = Get_wifistate;
}

void Wifi_GetState(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_WifiState;

	Get_wifistate[0].Encode = Send_Cmd_Mac;
	Get_wifistate[0].Decode = Get_Mac;
	Get_wifistate[1].Encode = Send_Cmd_Connect;
	Get_wifistate[1].Decode = Get_Connect;
	Get_wifistate[2].Encode = Send_Cmd_State;
	Get_wifistate[2].Decode = Get_State;
	
	task->wifi_CmdCount = 3;
	task->CMD = Get_wifistate;
}

void Wifi_ConfigSmartlink(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_WifiState;

	Get_wifistate[0].Encode = Send_Cmd_Config;
	Get_wifistate[0].Decode  = Check_Config;
	
	task->wifi_CmdCount = 1;
	task->CMD = Get_wifistate;
}

void Azure_PostData(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_AzurePost;

	Get_wifistate[0].Encode = Send_Post;
	Get_wifistate[0].Decode = Check_PostResponse;
	
	task->wifi_CmdCount = 1;
	task->CMD = Get_wifistate;
}

void Azure_DeleteData(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_AzureDelete;

	Get_wifistate[0].Encode = Send_Delete;
	Get_wifistate[0].Decode  = Check_DeleteResponse;
	
	task->wifi_CmdCount = 1;
	task->CMD = Get_wifistate;
}

void Azure_POST_With_Body(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_AzurePOST_With_Body;

	Get_wifistate[0].Encode = POST_With_Body_Send;
	Get_wifistate[0].Decode  = POST_With_Body_Parse;
	
	task->wifi_CmdCount = 1;
	task->CMD = Get_wifistate;
}

void Azure_POST_Login(WIFI_TASK * task)
{
	task->wifi_TaskType = Task_POST_Login;

	Get_wifistate[0].Encode = POST_Login_Send;
	Get_wifistate[0].Decode  = POST_Login_Parse;
	
	task->wifi_CmdCount = 1;
	task->CMD = Get_wifistate;
}

//zyn
void Wifi_testState(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_WifiState;
	
	Get_wifistate[0].Encode = Send_Cmd_name;
	Get_wifistate[0].Decode = Check_name;
	Get_wifistate[1].Encode = Send_Cmd_Secret;
	Get_wifistate[1].Decode = Check_Secret;
	Get_wifistate[2].Encode = Send_Cmd_sta;
	Get_wifistate[2].Decode = Check_sta;
	Get_wifistate[3].Encode = Send_Cmd_REBOOT;
	Get_wifistate[3].Decode = Check_REBOOT;	
	
	task->wifi_CmdCount = 4;
	task->CMD = Get_wifistate;
}

void Wifi_RESETState(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_WifiState;

	Get_wifistate[0].Encode = Send_Cmd_reset;
	Get_wifistate[0].Decode  = Check_RESET;
	Get_wifistate[1].Encode = Send_Cmd_name;
	Get_wifistate[1].Decode = Check_name;
	Get_wifistate[2].Encode = Send_Cmd_Secret;
	Get_wifistate[2].Decode = Check_Secret;
	Get_wifistate[3].Encode = Send_Cmd_sta;
	Get_wifistate[3].Decode = Check_sta;
	Get_wifistate[4].Encode = Send_Cmd_REBOOT;
	Get_wifistate[4].Decode = Check_REBOOT;
	
	task->wifi_CmdCount = 5;
	task->CMD = Get_wifistate;
}

/** @defgroup WIFI_toAzureState
  * @{
  */
typedef enum
{
	isUartIdle       = 0x00,                       
	isTransmit	     = 0x01,                        
	isReceive	     = 0x02,                       
	isCycIdle        = 0x03,                       
	isTimeOut        = 0x04,                        
}Uart_State;

//  
// 
//
void Uart_TxRxTask(WIFI_TASK * task)
{
	OS_ERR  err;
	static unsigned char task_state = 0;
	static int iReadLen = 0;

	switch(task->CMD_COM_STATE_CTR)
	{
		case isUartIdle:
			task->wifi_TaskType = NULL;
			switch(task->wifi_taskqueue)
			{
				case Task_01:
					task->wifi_taskqueue &= ~Task_01;
					break;
				case Task_GetState:
					Wifi_GetState(task);
					task->wifi_taskqueue &= ~Task_GetState;
					break;
				case Task_Post:
					if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0400)
					{
						Azure_PostData(task);
						task->wifi_taskqueue &= ~Task_Post;
					}
					break;
				case Task_Delete:
					if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0400)
					{
						Azure_DeleteData(task);
						task->wifi_taskqueue &= ~Task_Delete;
					}
					break;
				case Task_POST_With_Body:
					if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0400)
					{
						Azure_POST_With_Body(task);
						task->wifi_taskqueue &= ~Task_POST_With_Body;
					}
					break;
				case Task_POST_Login:
					if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0400)
					{
						Azure_POST_Login(task);
						task->wifi_taskqueue &= ~Task_POST_Login;
					}
					break;
				case Task_Wifitest:
					Wifi_testState(task);
					task->wifi_taskqueue &= ~Task_Wifitest;
					break;
				case Task_WifiConfig:
					Wifi_ConfigSmartlink(task);
					task->wifi_taskqueue &= ~Task_WifiConfig;
					break;
				case Task_InitWifi:
					Wifi_GetSetParam(task);
					task->wifi_taskqueue &= ~Task_InitWifi;
					break;
				case Task_WifiRESET:
				  	Wifi_RESETState(task);
					task->wifi_taskqueue &= ~Task_WifiRESET;
					break;
				default:
					break;
			}
			
			if(task->wifi_TaskType)
			{
				task->CMD_COM_STATE_CTR = isTransmit;
			}
			break;
		case isTransmit:
			Rx_Length = 0;
			Tx_Length = 0;
			memset(Rx_Buffer, 0, 1537);   
			memset(Tx_Buffer, 0, 1024);
			task->CMD_COM_STATE_CTR = isReceive;
			
			if(task->wifi_TaskType > TYPE_Azure)
			{
				task_state = Device_toAzureTask(Tx_Buffer, &Tx_Length, task);
			}
			else
			{
				task_state = Wifi_ATCmdTask(Tx_Buffer, &Tx_Length, task);
			}
			
			tty.write(Tx_Buffer,Tx_Length);
			
			break;
		case isReceive:
			iReadLen = tty.read(Rx_Buffer + Rx_Length,1024);
			if(iReadLen > 0)
			{
				Rx_Length += iReadLen;
			}
			Rx_Buffer[Rx_Length] = '\0';
			
			if(task->wifi_TaskType > TYPE_Azure)
			{
				task_state = Device_toAzureTask(Rx_Buffer, &Rx_Length, task);
			}
			else
			{
				task_state = Wifi_ATCmdTask(Rx_Buffer, &Rx_Length, task);
			}
			
			if(task_state % 2)
			{
				task->CMD_COM_STATE_CTR = isTransmit;
			}
			else if(task_state == 0)
			{
				task->CMD_COM_STATE_CTR = isUartIdle;
			}
			else if(task_state == Rec_Data_Error)
			{
				Bluetooth_State.wifi_connect_status = ble_wifi_connect_error_598;
				
				OSTimeDly(100, OS_OPT_TIME_DLY, &err);
				task->CMD_COM_STATE_CTR = isTransmit;
			}
			else if(task_state == Http_Req_Error)
			{
				OSTimeDly(100, OS_OPT_TIME_DLY, &err);
				task->CMD_COM_STATE_CTR = isTransmit;
			}
			else
				task->CMD_COM_STATE_CTR = isUartIdle;
			break;
		case isCycIdle:
			task->CMD_COM_STATE_CTR = isUartIdle;
			break;
		default:
			break;
	}
}



