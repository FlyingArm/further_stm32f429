#include "stm32f10x.h"
#include "Flash_W25Q64.h" 
#include "HFLPB100.h"
#include "wifi_control.h"

extern u8 const table_week[12]; //月修正数据表	  
//平年的月份日期表
extern const u8 mon_table[12];
//unsigned char weekplaning[]="00V01V02V03V04V05V06V10W11W12W13W14W15W16W\
//20X21X22X23X24X25X26X30Y31Y32Y33Y34Y35Y36Y40Z41Z42Z43Z44Z45Z46Z\
//50V51V52V53V54V55V56V60W61W62W63W64W65W66W70X71X72X73X74X75X76X\
//80Y81Y82Y83Y84Y85Y86Y90Z91Z92Z93Z94Z95Z96ZA0VA1WA2VA3VA4VA5VA6V\
//B0WB1WB2WB3WB4WB5WB6WC0XC1XC2XC3XC4XC5XC6XD0YD1YD2YD3YD4YD5YD6Y\
//E0ZE1ZE2ZE3ZE4ZE5ZE6ZF0VF1VF2VF3VF4VF5VF6VG0WG1WG2WG3WG4WG5WG6W\
//H0XH1XH2XH3XH4XH5XH6XI0YI1YI2YI3YI4YI5YI6YJ0ZJ1ZJ2ZJ3ZJ4ZJ5ZJ6Z\
//K0XK1XK2XK3XK4XK5WK6VL0WL1WL2WL3WL4WL5XL6XM0VM1VM2VM3VM4VM5WM6V\
//N0YN1YN2YN3YN4YN5YN6Y";

unsigned char Wifi_mode_Select(WIFI_State *wifi_State)
{
	int i;
	unsigned char Wifi_Fan_speed;
	unsigned char mode_Control_thing[169];


	switch(wifi_State->mode_Control.mode)
	{
		case	1://度假
			//Wifi_Fan_speed = 1;
		  SPI_Flash_Write(wifi_State->mode_Control.start_date,4268,14);
		  SPI_Flash_Write(wifi_State->mode_Control.end_date,4284,14); 
		
		break;
    case	2:			
			 for (i = 0; i < 169; i++)
				{
			   // mode_Control_thing[i]= weekplaning[i*3+2];
					mode_Control_thing[i]= wifi_State->mode_Control.timer_order[i*3+2];//wifi_State->mode_Control.modeControl[i*3+2];
				}
			 SPI_Flash_Write(mode_Control_thing,4097,169);
		break;
		case 3://日常
			Wifi_Fan_speed = 2;	
		break;
    case 4://访客
			Wifi_Fan_speed = 4;	
		break;
    case 5://烹饪
			Wifi_Fan_speed = 3;
    break;		
		case 6://定时
			Wifi_Fan_speed = 1;
		break;
		default:
			Wifi_Fan_speed = 1;break;	
	}
	return Wifi_Fan_speed;
}

GET_TIME Get_week;
//度假模式时间
unsigned char Wifi_holidaymode(GET_TIME *HolidayDate,int flashADDr)//flashADDr= 4268  ,4284
{

	int i;
	unsigned char holidaytime[14];
	int data=0;
	SPI_Flash_Read(holidaytime,flashADDr,14);
	if(holidaytime[0] == 0xff)
	{return 0;}
	for (i = 0; i < 4; i++)
	{
		data = data*10 + (holidaytime[i] - '0');
	}
	HolidayDate->tm_year = data;
	data = 0;
	for (i = 4; i < 6; i++)
	{
		data = data*10 + (holidaytime[i] - '0');
	}
  HolidayDate->tm_mon = data;
	data = 0;
	for (i = 6; i < 8; i++)
	{
		data = data*10 + (holidaytime[i] - '0');
	}
  HolidayDate->tm_mday= data;
	data = 0;	
	for (i = 8; i < 10; i++)
	{
		data = data*10 + (holidaytime[i] - '0');
	}
  HolidayDate->tm_hour= data;
	data = 0;
	for (i = 10; i < 12; i++)
	{
		data = data*10 + (holidaytime[i] - '0');
	}
  HolidayDate->tm_min= data;
	data = 0;
	for (i = 12; i < 14; i++)
	{
		data = data*10 + (holidaytime[i] - '0');
	}
  HolidayDate->tm_sec= data;
	data = 0;	
}
//度假模式的时间判断
unsigned char Wifi_holidaymodetask(GET_TIME *HolidayStartDate,WIFI_State *wifi_State)										
{
	u32 starttime=0;
	u32 currenttime=0;	
	if(HolidayStartDate->tm_year == 0)
	{return 0;}
	starttime = RTC_Set(HolidayStartDate->tm_year,HolidayStartDate->tm_mon,HolidayStartDate->tm_mday,HolidayStartDate->tm_hour,HolidayStartDate->tm_min,HolidayStartDate->tm_sec);
	currenttime = wifi_State->expiry_time;
	if(currenttime > starttime)
	{
	   return 1;
	}
	else
		return 0;
//	if(HolidayStartDate->tm_year != Get_week.tm_year)
//	{
//		return 0;
//	}
// if(HolidayStartDate->tm_mon != Get_week.tm_mon)
//  {
//		return 0;	
//	}
// if(HolidayStartDate->tm_mday != Get_week.tm_mday)
//	{
//		return 0;				
//	}
// if(HolidayStartDate->tm_hour != Get_week.tm_hour)
//	{
//		return 0;		 
//	}
// if(HolidayStartDate->tm_min <= Get_week.tm_min)
//	{
//    return 1;
//	}	
//	if(HolidayStartDate->tm_sec >=Get_week.tm_sec)
//	{
//		return 1;					
//	}					
}										
/*
根据时间查模式
*/
unsigned char Wifi_mode_Analysis(void)
{
	char mode;
	int time;
	unsigned char Wifi_Fan_speed;
	unsigned char mode_Control_thing[169];
	time =Get_week.tm_week +(Get_week.tm_hour)*7;
	SPI_Flash_Read(mode_Control_thing,4097,169);
  	mode = mode_Control_thing[time];
	switch (mode)
	{
		case 'V':
			Wifi_Fan_speed = 2;
			break;
		case 'W':
			Wifi_Fan_speed = 1;
			break;
		case 'X':
			Wifi_Fan_speed = 4;
			break;
		case 'Y':
			Wifi_Fan_speed = 3;
			break;
		case 'Z':
		  Wifi_Fan_speed = 0;
			break;
		default:	Wifi_Fan_speed = 1;break;
	}
return Wifi_Fan_speed;
}
//得到当前的时间
//返回值:0,成功;其他:错误代码.
u8 RTC_Get_Week(u16 year,u8 month,u8 day);

u8 RTC_GetHour(WIFI_State *wifi_State)
{
	static u16 daycnt=0;
	u32 timecount=0; 
	u32 temp=0;
	u16 temp1=0;	  
		 
  timecount = wifi_State->expiry_time;
 	temp=timecount/86400;   //得到天数(秒钟数对应的)
	if(daycnt!=temp)//超过一天了
	{	  
		daycnt=temp;
		temp1=1970;	//从1970年开始
		while(temp>=365)
		{				 
			if(Is_Leap_Year(temp1))//是闰年
			{
				if(temp>=366)temp-=366;//闰年的秒钟数
				else {temp1++;break;}  
			}
			else temp-=365;	  //平年 
			temp1++;  
		}   
		Get_week.tm_year=temp1;//得到年份
		temp1=0;
		while(temp>=28)//超过了一个月
		{
			if(Is_Leap_Year(Get_week.tm_year)&&temp1==1)//当年是不是闰年/2月份
			{
				if(temp>=29)temp-=29;//闰年的秒钟数
				else break; 
			}
			else 
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];//平年
				else break;
			}
			temp1++;  
		}
		Get_week.tm_mon=temp1+1;	//得到月份
		Get_week.tm_mday=temp+1;  	//得到日期 
	}
	temp=timecount%86400;     		//得到秒钟数   	   
	Get_week.tm_hour=temp/3600;     	//小时
	Get_week.tm_min=(temp%3600)/60; 	//分钟	
	Get_week.tm_sec=(temp%3600)%60; 	//秒钟
	Get_week.tm_week=RTC_Get_Week(Get_week.tm_year,Get_week.tm_mon,Get_week.tm_mday);//获取星期   		
	return 0;
}	 
//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日 
//返回值：星期号0,1,2,3,4,5,6																						 
u8 RTC_Get_Week(u16 year,u8 month,u8 day)
{	
	u16 temp2;
	u8 yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// 如果为21世纪,年份数加100  
	if (yearH>19)yearL+=100;
	// 所过闰年数只算1900年之后的  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}
