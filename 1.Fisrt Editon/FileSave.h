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
#include "ModifConfig.h"
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
#define FILE_SYNC_NUM 		5000 	//for file sync
#define Peri_FILE_SYNC_NUM 	50 
#define POWOFF_SAVE_NUM 	10000
#define DIGITAL_NUM_BYTE 	32  	//byte //0 digital signal tube = 0 digital signals 
#define OPRTFILE_DIR       "/yaffs/REC_REALOPRT/" 
#define OPRTFILE_HEAD       "REC_OPRT_" 
/**********************************************************************
*
*Global Struct Define Section
*
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

}DRIVE_FILE_TOP;//csr_driver的文件头格式，130个字节

typedef struct
{
  	uint8_t DriveDigital_U8[DIGITAL_NUM_BYTE]; //max 16 word  now 8 Word
  	uint16_t DriveAnalog_U16[ALL_CHAN_NUM]; //max 80 word   ANALOG_NUM_WORD 
}DRIVE_FILE_DATA;//csr_driver的文件头格式，130个字节

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

/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/

/**********************************************************************
*
*Global Prototype Declare Section
*
*********************************************************************/
/*
int8_t FileCreatePowOn(FILE_FD *file_p,RECORD_XML *RrdXml_p,TRAIN_INFO *TranInfo_p,EADS_ERROR_INFO *EADSErrInfo_p);
int8_t WriteFile(FILE *FilePath,uint16_t Length,uint8_t *BufferIn);
int8_t OperNumFileSave(FILE_FD * File_p, CHAN_STATUS_INFO * ChanStaInfo_p,RECORD_XML * RrdXml_p,TRAIN_INFO * inTranInfo_p);
int8_t FltRealFileProc(FILE_FD * File_p, CHAN_STATUS_INFO * InStusInfo_p,RECORD_XML * xml_p,TRAIN_INFO * TranInfo_p,uint8_t ProcNum,uint8_t EADSType);
int8_t ChanCalibFileCreat(FILE_FD *file_p,uint8_t *path,EADS_ERROR_INFO  *EADSErrInfo_ST);
int8_t ChanCalibDataSave(FILE * fd,CHAN_CALIB_DATA *CalibDataSave_p);
int8_t EventDataSaveForRelayTest(FILE_FD * file_p,CHAN_DATA * ChanDatap,uint8_t EADSType,const EADS_ERROR_INFO	EADSErrInfoST,const CHAN_LG_INFO LgInfoST,
			const CHAN_DIGITAL_INFO ChanDigitalInfo_ST,const CHAN_STATUS_INFO  ChanInfop);
int8_t EventRelayLifeSave(FILE_FD * file_p,uint8_t EADSType,const CHAN_STATUS_INFO ChanInfop);
int8_t EventLifeFileCreateByNum(FILE_FD * file_p,RECORD_XML * RrdXml_p,TRAIN_INFO * TranInfo_p,EADS_ERROR_INFO  *EADSErrInfo_ST);
void   FileSaveFunc(peripheralDevice Device_Type_enum,FILE **device_FP,uint8_t *databuf,uint8_t datalength,RECORD_TYPE_CFG *REC_TYPE,\
                 uint32_t Record_Num_U32);*/
int8_t FileSpaceProc(RECORD_XML * RrdXml_p);
int8_t EventDataSave(FILE_FD * file_p,CHAN_DATA * ChanDatap,uint8_t EADSType,const EADS_ERROR_INFO  EADSErrInfoST,const CHAN_LG_INFO LgInfoST,
		const CHAN_DIGITAL_INFO ChanDigitalInfo_ST,const CHAN_STATUS_INFO  ChanInfop);				 
int8_t EventFileCreateByNum(FILE_FD * file_p,RECORD_XML * RrdXml_p,TRAIN_INFO * TranInfo_p,EADS_ERROR_INFO  *EADSErrInfo_ST);				 
int8_t DirFileTar(RECORD_XML * RrdXml_p);
/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 
