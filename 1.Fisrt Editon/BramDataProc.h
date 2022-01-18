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

/***********************************************************************
*Multi-Include-Prevent Sart Section*
*********************************************************************/
#ifndef _BRAMDATAPROC_H
#define _BRAMDATAPROC_H
/***********************************************************************
*Debug switch  section*
*********************************************************************/

/***********************************************************************
*Include File Section*
*********************************************************************/
#include "BaseDefine.h"
#include "Timer.h"
#include "BramDataInter.h"

/***********************************************************************
*Global Macro Define Section*
*********************************************************************/
#define BRAMERR_NUM 10 	/*read 10 time ,the all data is 0*/
#define MAPERR_NUM 3 	/*map 3 time ,if failed */
#define CAN0_TYPE 0
#define CAN1_TYPE 1
#define CAN0_READ_FRAME_NUM  20
#define CAN0_WRITE_FRAME_NUM 4
#define CAN1_READ_FRAME_NUM  11
#define CAN1_WRITE_FRAME_NUM 3
#define MVB_READ_FRAME_NUM  2
#define MVB_WRITE_FRAME_NUM 6
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

/***********************************************************************
*Global Variable Declare Section*
*********************************************************************/


/***********************************************************************
*Global Prototype Declare Section*
*********************************************************************/
int8_t 	Bram_Mapping_Init(EADS_ERROR_INFO * EADSErrInfop);
int8_t 	BramCloseMap(void);
int8_t 	BoardDataRead(BRAM_ADDRS *BramAddrs_p,uint32_t *ReadData);
int8_t  BLVDSDataReadFunc(TMS570_BRAM_DATA *bram_rd_data,EADS_ERROR_INFO *EADSErrInfop);

void 	CAN_FrameInit(struct can_filter *candata_RD_filter,struct can_frame *candata_WR,uint8_t can_devtype);
int8_t 	CAN_Write_Option(int8_t socket_fd,struct can_frame *can_frame_data,uint8_t frames_num,uint8_t dev_type);
int8_t 	CAN_Read_Option(int8_t socket_fd,struct can_frame *can_frame_data,uint8_t frames_num,uint8_t dev_type);
void 	CAN_WriteData_Pro(struct can_frame *candata_wr,TMS570_BRAM_DATA *bramdata_rd,uint8_t can_devtype);
void 	CAN_ReadData_Pro(struct can_frame *candata_rd,TMS570_BRAM_DATA *bramdata_wr,uint8_t can_devtype);
void 	TMS570_Bram_TopPack_Set(BRAM_CMD_PACKET *cmd_packet_wr,BRAM_CMD_PACKET *cmd_packet_rd,uint8_t can_devtype);
int8_t 	TMS570_Bram_Read_Func(BRAM_CMD_PACKET *cmd_packet_rd,TMS570_BRAM_DATA *bram_data,uint8_t frame_num,TRANS_TYPE_ENUM tans_type);
int8_t 	TMS570_Bram_Write_Func(BRAM_CMD_PACKET *cmd_packet_wr,TMS570_BRAM_DATA *bram_data,uint8_t frame_num,TRANS_TYPE_ENUM tans_type);
void  	MVB_Bram_Init(BRAM_CMD_PACKET *cmd_packet_rd,BRAM_CMD_PACKET *cmd_packet_wr,BRAM_CMD_PACKET *mvb_packet_rd,BRAM_CMD_PACKET *mvb_packet_wr);
int8_t 	MVB_Bram_Write_Func(BRAM_CMD_PACKET *mvb_packet_wr,TMS570_BRAM_DATA *bram_data_mvb_wr);
int8_t  MVB_Bram_Read_Func(BRAM_CMD_PACKET *mvb_packet_rd,TMS570_BRAM_DATA *bram_data_mvb_rd);
int8_t 	MVB_RD_Data_Proc(TMS570_BRAM_DATA *bram_data_mvb_rd,TMS570_BRAM_DATA *bram_data_tms570_wr);
int8_t 	MVB_WR_Data_Proc(TMS570_BRAM_DATA *bram_data_mvb_wr,TMS570_BRAM_DATA *bram_data_tms570_rd);
/***********************************************************************
*Multi-Include-Prevent End Section*
*********************************************************************/
#endif 
