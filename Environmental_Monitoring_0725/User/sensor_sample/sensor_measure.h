
#ifndef __SENSOR_MEASURE_H
#define	__SENSOR_MEASURE_H

typedef enum
{
	VOC 	= 0,		
	CO2 	= 1,		
	PM2005 	= 2,
	H2O,
	TEMP,
	IP_NUM_SET,
	SEN_VALUE,
	NUM_ACK,
}DATA_CAL_TYPE;

/* sensor calibration datas */
typedef struct
{
	int voc_cali_data;			
	int co2_cali_data;			
	int h20_cali_data;	
	int pm2005_cali_data;
	int temp_cail_data;	
	unsigned char ip;
	unsigned char num;
}SENSOR_DATA;

typedef union
{
     char cali_ch[4];
     float cali_f;
}UNION_CALI_DATA;

extern UNION_CALI_DATA union_cali_data;
extern SENSOR_DATA sensor_cali_data;

#endif /* __SENSOR_MEASURE_H */
