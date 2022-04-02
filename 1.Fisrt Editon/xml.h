/**********************************************************************
*File name      :   ModifConfig.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/02/25
*Description    :   Read and modify content value of config.ini file
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/02/25  Create
*
*
*********************************************************************/

/**********************************************************************
*
*Multi-Include-Prevent Sart Section
*
*********************************************************************/
#ifndef _XML_H
#define _XML_H

/**********************************************************************
*
*Debug switch  section
*
*********************************************************************/

/**********************************************************************
*
*Include File Section
*
*********************************************************************/
#include "BaseDefine.h"
#include "WriteLog.h"
#include "mxml.h"
/***********************************************************************
*Global Macro Define Section*
*********************************************************************/
#define	EVENT_FILE 1
#define	FAST_FILE  3
#define	REAL_FILE  4
#define	CAN_FILE   5
#define	UART_FILE  6
#define RELAYCFG_NUM 2
#define ZONE 50 
#define RCD_CFG_ZOGPEI_COACH  	"/tffs0/Device_Record_CFG_ZongPei.xml" 
#define RCD_CFG_COACH 			"/tffs0/Device_Record_CFG.xml"
#define RELAY_PARA_CONFIG 		"/tffs0/Relay_Para_Config.xml"
#define ECU_CONFIG 				"/tffs0/ECU_CFG.xml"
#define XML_EVENT_REC_INTEVAL_MIN 100
#define XML_EVENT_REC_TOTAL_MIN 10000
#define XML_REAL_REC_TOTAL_MIN 1000
#define XML_LIFE_NUM_MIN 10000
#define XML_REAL_BEFOR_NUM 400
/***********************************************************************
*Global Struct Define Section*
*********************************************************************/

typedef struct 
{
	uint8_t RecTypeIdx;
	uint8_t RecPath[30];
	uint8_t RecFileHead[30];
	uint8_t RecTotalChn_U8;
	uint8_t RecChnNum_U8;
	uint16_t RecMaxFileNum_U16;
	uint16_t RecChnBitMask1;
	uint16_t RecChnBitMask2;
	uint16_t RecChnBitMask3;
	uint16_t RecChnBitMask4;
	uint16_t RecChnBitMask5;
	uint16_t RecChnBitMask6;
    uint16_t RecInterval;
    uint16_t RecBeforeNum;
    uint16_t RecAfterNum;
    uint16_t RecToTalNum;

}RECORD_TYPE_CFG;

typedef struct 
{
	RECORD_TYPE_CFG  Rec_Event_ST;
	RECORD_TYPE_CFG  Rec_Fault_ST;
	RECORD_TYPE_CFG  Rec_Fast_ST;
	RECORD_TYPE_CFG  Rec_Real_ST;	
}RECORD_XML;

/***********************************************************************
*Global Variable Declare Section*
*********************************************************************/


/***********************************************************************
*Global Prototype Declare Section*
*********************************************************************/
int8_t SysXmlParInit(char *xmlFilePath,RECORD_XML *RecordXml_p,VERSION *Version_p);
int8_t RelayCfgXml(char * xmlFilePath,RECORD_XML *RecordXml_p,RELAY_LIFE_CFG *LifeCfg);
int8_t XmlParJudge(RECORD_XML *RecordXml_p);
/***********************************************************************
*Multi-Include-Prevent End Section*
*********************************************************************/
#endif 





