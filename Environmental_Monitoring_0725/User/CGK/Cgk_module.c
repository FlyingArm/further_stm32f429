#include <includes.h>
#include "bsp_i2c_cgk.h"
#include "Cgk_module.h"


CgkSensorData g_CgkSensorData;

//  CGK_CMD_TYPE
//	ELECMEASURE = 0x01,
//	PMMEASURE   = 0x05,
//	CO2MEASURE  = 0x06,
//  CO2OPEN     = 0x10,
//	CO2CLOSE    = 0x12,
//	PMCLOSE     = 0x15,
uint8_t cgk_sendcmd(uint8_t _cmd) 
{
	uint8_t ucCmdret = 0;
	ucCmdret = CGK_SendCmd(CGKWRITE,_cmd);
	if(ucCmdret == 0) {
		g_CgkSensorData.uiNowCmdStatus = _cmd;
	}
	return ucCmdret;
}

void CgkPMDataCollection()
{
	AutoPM2005_Read(CGKREAD,&g_CgkSensorData.uiPm03Dat,&g_CgkSensorData.uiPm25Dat,&g_CgkSensorData.uiPm10Dat);
	g_CgkSensorData.uiPm25Ug =(unsigned int)( (float)g_CgkSensorData.uiPm03Dat * 0.00015);
	g_CgkSensorData.uiPm10Ug =(unsigned int)( g_CgkSensorData.uiPm25Ug + (float)g_CgkSensorData.uiPm25Dat * 0.08);
}

void CgkCO2DataCollection()
{
	AutoEleCo2_Read(CGKREAD,&g_CgkSensorData.uiCO2Dat);
}


void CgkElecDataCollection()
{
	AutoEleCo2_Read(CGKREAD,&g_CgkSensorData.uiElecDat);
}


