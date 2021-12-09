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
#define CAN0_TYPE 0
#define CAN1_TYPE 1
#define CAN0_READ_FRAME_NUM  13
#define CAN0_WRITE_FRAME_NUM 4
#define CAN1_READ_FRAME_NUM  10
#define CAN1_WRITE_FRAME_NUM 3
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
int8_t 	BoardDataRead(BRAM_ADDRS *BramAddrs_p,uint32_t *ReadData);
int8_t 	BLVDSDataReadThreadFunc(uint8_t ReadNum_U8,uint8_t EADSType,EADS_ERROR_INFO * EADSErrInfop);
int8_t 	SetTimeFromRTC(BRAM_ADDRS *RDSpcBlk_p,BRAM_ADDRS *WRSpcBlk_p);
int8_t 	BramCloseMap(void);
int8_t 	Bram_Mapping_Init(EADS_ERROR_INFO * EADSErrInfop,uint8_t EADSType);
void 	CAN_FrameInit(struct can_frame *candata_RD,struct can_frame *candata_WR,uint8_t can_devtype);
void 	CAN_WriteData_Pro(struct can_frame *candata_wr,TMS570_BRAM_DATA *bramdata_rd,uint8_t can_devtype);
void 	CAN_ReadData_Pro(struct can_frame *candata_rd,TMS570_BRAM_DATA *bramdata_wr,uint8_t can_devtype);
void 	TMS570_Bram_TopPackDataSetFun(void);
int8_t 	TMS570_Bram_Read_Func(TMS570_BRAM_DATA *bram_data);
int8_t 	TMS570_Bram_write_Func(TMS570_BRAM_DATA bram_data[]);
/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 
