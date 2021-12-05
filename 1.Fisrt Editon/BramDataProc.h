/**********************************************************************
*File name 		:   BramDataProc.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/5/16
*Description    :   Include the Write data to 570,Read data from 570,
*                   Write data to MAX10,Read data from MAX10
*                   Read 570 RTC information and set system time

*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/05/16  Create
*
*********************************************************************/

/**********************************************************************
*
*Multi-Include-Prevent Sart Section
*
*********************************************************************/
#ifndef _BRAMDATAPROC_H
#define _BRAMDATAPROC_H

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
#include "Timer.h"
#include "BramDataInter.h"

/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/
#define BRAMERR_NUM 10 /*read 10 time ,the all data is 0*/
#define MAPERR_NUM 3 /*map 3 time ,if failed */


/**********************************************************************
*
*Global Struct Define Section
*
*********************************************************************/
typedef struct 
{	
	uint16_t protocol_version;
	uint8_t PacktLength_U32;
	uint8_t PacktCMD_U8;
	uint8_t ChanNum_U8;
	uint16_t BlockNum_U16;
	COMMU_MCU_ENUM TargeMCU;
}BRAM_CMD_PACKET;



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
int8_t BoardDataRead(BRAM_ADDRS *BramAddrs_p,uint32_t *ReadData,uint8_t ChanNum);
int8_t BLVDSDataReadThreadFunc(uint8_t ReadNum_U8,uint8_t EADSType,EADS_ERROR_INFO * EADSErrInfop);
int8_t CANDataReadThreadFunc(CAN_FingerPrint_TPYE *CANSendDate) ;
int8_t CANDataWriteThreadFunc(CAN_FingerPrint_TPYE *CANRecvDate,BRAM_CMD_PACKET *CmdPact_p);
int8_t SetTimeFromRTC(BRAM_ADDRS *RDSpcBlk_p,BRAM_ADDRS *WRSpcBlk_p);
int8_t BramCloseMap(void);
int8_t Bram_Mapping_Init(EADS_ERROR_INFO * EADSErrInfop,uint8_t EADSType);
void TMS570_Bram_TopPackDataSetFun(void);


/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 
