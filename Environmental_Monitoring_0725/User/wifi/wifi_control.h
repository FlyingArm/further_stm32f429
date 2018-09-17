#ifndef  WIFI_CONTROL_H
#define  WIFI_CONTROL_H
#include "HFLPB100.h"

typedef struct{
	
	unsigned char  tm_sec;   /* seconds after the minute, 0 to 60
									 (0 - 60 allows for the occasional leap second) */
	unsigned char  tm_min;   /* minutes after the hour, 0 to 59 */
	unsigned char  tm_hour;  /* hours since midnight, 0 to 23 */
	unsigned char  tm_mday;  /* day of the month, 1 to 31 */
	unsigned char  tm_mon;   /* months since January, 0 to 11 */
	unsigned short tm_year;  /* years since 1900 */
	unsigned short tm_week;  /* week*/
}GET_TIME;

unsigned char Wifi_mode_Select(WIFI_State *wifi_State);
unsigned char Wifi_mode_Analysis(void);
unsigned char Wifi_holidaymode(GET_TIME *HolidayDate,int flashADDr);
//unsigned char Wifi_holidaymodetask(GET_TIME *HolidayStartDate);
unsigned char Wifi_holidaymodetask(GET_TIME *HolidayStartDate,WIFI_State *wifi_State);			
#endif
