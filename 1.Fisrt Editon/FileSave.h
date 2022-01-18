/**********************************************************************
*File name 		:   FileSave.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/1/29
*Description    :   Include file creat,assign value to FRAME struct,transfer char to ASCII and write file functions.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*
*
*
*********************************************************************/

/**********************************************************************
*
*Multi-Include-Prevent Sart Section
*
*********************************************************************/
#ifndef _FILESAVE_H
#define _FILESAVE_H

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
#include <sys/vfs.h>
#include "Crc.h"
#include "xml.h"
#include "DircMng.h"
/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/
#define EVENT_SAVE_NUM 		60000 	// big than 65535,csr drive cannot export to excel
#define OPERT_SAVE_TIME 	36000 	//* 1800s save fot test*/
#define EVENT_MAX_NUM 		60000
#define FILE_SYNC_NUM 		600 	//for file sync
#define Peri_FILE_SYNC_NUM 	50 
#define POWOFF_SAVE_NUM 	10000
#define DIGITAL_NUM_BYTE 	32  	
#define ANOLOG_NUM_BYTE 	160
#define OPRTFILE_DIR       "/yaffs/REC_REALOPRT/" 
#define OPRTFILE_HEAD       "REC_OPRT_" 
#define BLVDS_EVENT_PATH   "/yaffs/REC_BLVDS_EVTDATA/"
#define BLVDS_EVENT_FileName_head "REC_BLVDS_EVTDATA_"
/***********************************************************************
*Global Struct Define Section*
*********************************************************************/
typedef struct
{
	uint16_t RecEnDianType_U16;
	uint16_t RecHeadSize_U16;
	uint16_t RecHeadVer_U16;
	uint16_t RecordType_U16;
	uint16_t ProjectNo_U16;
	uint16_t DCUType_U16;
	uint16_t VehicleNo_U16;
	uint16_t DCUNO_U16;
	uint16_t RecordTotalChnNum_U16;
	uint16_t RecordChnNum_U16;
	uint16_t RecordInterval_U16;
	uint16_t ChnMask1_U16;
	uint16_t ChnMask2_U16;
	uint16_t ChnMask3_U16;
	uint16_t ChnMask4_U16;
	uint16_t ChnMask5_U16;
	uint16_t ChnMask6_U16;
	uint16_t RecordBefore_U16;
	uint16_t RecordAfter_U16;
	uint16_t RecordTotal_U16;
	uint8_t RecordTime_MT;
	uint8_t RecordTime_YY;
	uint8_t RecordTime_HH;
	uint8_t RecordTime_DD;
	uint8_t RecordTime_SS;
	uint8_t RecordTime_MN;
	uint16_t EnvData_U16[16];
	uint16_t FaultCode_U16[16];
	uint16_t HavePulse_U16;
	uint16_t RecordTotalChnNumPulse_U16;
	uint16_t RecordChnNumPulse_U16;
	uint16_t RecordIntervalPulse_U16;
	uint16_t PulseChnMask1_U16;
	uint16_t PulseChnMask2_U16;
	uint16_t PulseChnMask3_U16;
	uint16_t PulseChnMask4_U16;
	uint16_t PulseChnMask5_U16;
	uint16_t PulseChnMask6_U16;
}DRIVE_FILE_TOP;

typedef struct
{
  	uint8_t DriveDigital_U8[DIGITAL_NUM_BYTE]; 	//max 16 word  	Digital
  	uint16_t DriveAnalog_U16[ALL_CHAN_NUM]; 	//max 80 word   Analog
}DRIVE_FILE_DATA;

typedef struct
{
   uint8_t RecordTime_MT;
   uint8_t RecordTime_YY;
   uint8_t RecordTime_HH;
   uint8_t RecordTime_DD;
   uint8_t RecordTime_SS;
   uint8_t RecordTime_MN;
   uint8_t EnvDigital[8];
   uint8_t EnvAnalog[24];
   uint8_t FaultCode[32];
}DRIVE_LOG_DATA;

/***********************************************************************
*Global Variable Declare Section*
*********************************************************************/

/***********************************************************************
*Global Prototype Declare Section*
*********************************************************************/
int8_t FileSpaceProc(RECORD_XML * RrdXml_p);
int8_t ECU_Record_Data_Pro_Fun(DRIVE_FILE_DATA *Drive_ST_p,TMS570_BRAM_DATA *bram_data_rd,TMS570_BRAM_DATA *bram_data_wr,EADS_ERROR_INFO EADSErrInfoST);
int8_t MAX10_DataProc(TMS570_BRAM_DATA *bram__data,DRIVE_FILE_DATA *Drive_ST_p);
int8_t ECU_EventDataSave(FILE_FD *file_p,DRIVE_FILE_DATA *Drive_ST_p);
int8_t MAX10_EventDataSave(FILE_FD *file_p,DRIVE_FILE_DATA *Drive_ST_p);				 
int8_t EventFileCreateByNum(FILE_FD * file_p,RECORD_XML * RrdXml_p,TRAIN_INFO * TranInfo_p,EADS_ERROR_INFO  *EADSErrInfo_ST);				 
int8_t DirFileTar(RECORD_XML * RrdXml_p);
/***********************************************************************
*Multi-Include-Prevent End Section*
*********************************************************************/
#endif 
