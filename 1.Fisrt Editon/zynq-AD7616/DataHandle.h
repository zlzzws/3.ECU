/**********************************************************************
*File name 		:   DataHandle.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/1/29
*Description    :    Include the voltage change from 24bit to 16bit decimal value,judge the overvoltage overcurrent, 
*                   judge the channel operation, and save the g_ChanData_ST to FrameSave_ST function
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/02/05  Create
*REV1.0.1     feng    2018/03/04  Change the the channel high status sequence
*
*
*********************************************************************/

/**********************************************************************
*
*Multi-Include-Prevent Sart Section
*
*********************************************************************/
#ifndef _DATAHANDLE_H
#define _DATAHANDLE_H

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
#include "ModifConfig.h"
#include "xml.h"
#include "Bram.h"
#include "ADCDataProc.h"

/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/

/*the voltage judge*/

/*the current judge*/

#define LOWCURR_L 0
#define CUR_NEGA -10
#define VOL_NEGA -20
#define VOL_COR_JUDEG 20 //5V
//when voltage is low,add the fite if the voltage is >20V or <-20V
#define CURR_NEGA -20 //mA
#define CURR_COR_JUDEG 20 //mA
#define OPRT_TIME_MAX 500
#define JUMP_TIME_MAX 100
#define CURR_3I_WARN_TIME 2000 //2s mean 2000 loop 
#define CURR_2I_WARN_TIME 6000 //6s mean 6000 loop 
#define CURR_1I4_WARN_TIME 15000 //15s mean 15000  loop 
#define CURR_1I1_WARN_TIME 3000000 //3000s mean 3000000  loop 
#define DIGIT_PRO_TIME 50 //50ms  to pro the digital status data
#define FILE_REC_TIME 20 //10ms + filt the chan Status 20ms = 30ms  for file record,20ms is according the power off
#define STATS_FILT_TIME 10 //10ms  for filt the chan status
#define OPRT_UP 1
#define OPRT_DOWN 2
#define NO_RELAY 1
#define NC_RELAY 2

/**********************************************************************
*
*Global Struct Define Section
*
*********************************************************************/

typedef struct 
{

	uint8_t ChanOverCurrNum_1I1_U8;
    uint8_t ChanElmtOverCurrNum_1I1_U8;
    uint8_t ChanOverCurrNum_1I4_U8;
    uint8_t ChanElmtOverCurrNum_1I4_U8;
    uint8_t ChanOverCurrNum_2I_U8;
    uint8_t ChanElmtOverCurrNum_2I_U8;
    uint8_t ChanOverCurrNum_3I_U8;
    uint8_t ChanElmtOverCurrNum_3I_U8; 

    //uint8_t ChanElmtUndrCurrNum_U8;
	uint8_t ChanOverCurrFlag_1I1_U8;
	uint8_t ChanOverCurrFlag_1I4_U8;
	uint8_t ChanOverCurrFlag_2I_U8;
	uint8_t ChanOverCurrFlag_3I_U8;

	uint8_t ChanOverCurrChangeFlag_U8;
	
	uint8_t ChanNormCurrSteadNum_U8;
	uint8_t ChanNormCurrFlag_U8; /*for channel status,and filt the current near 0mA*/  	//0 1 status
	uint32_t OverCurrTime_1I1_U32;/*simple,just for ++ to calu the time */
	uint32_t OverCurrTime_1I4_U32;/*simple,just for ++ to calu the time */
	uint32_t OverCurrTime_2I_U32;/*simple,just for ++ to calu the time */
	uint32_t OverCurrTime_3I_U32;/*simple,just for ++ to calu the time */
}CURR_CHAN_JUDGE;


typedef struct 
{

	uint8_t ChanOverVolNum_U8; /*filt 10ms when ChanOverVolFiltFLag_U8 is 1 */
	uint8_t ChanUndrVolSteadNum_U8;
	uint8_t ChanHighVolSteadNum_U8;
	uint8_t ChanLowVolSteadNum_U8;
	uint8_t ChanElmtOverVolNum_U8;
	uint8_t ChanElmtUndrVolNum_U8;
	
	uint8_t ChanOverVolNumFlag_U8; /*for filt  10ms */
	uint8_t ChanUndrVolSteadNumFlag_U8;
	uint8_t ChanHighVolSteadNumFlag_U8;
	uint8_t ChanLowVolSteadNumFlag_U8;
	uint8_t ChanElmtOverVolNumFlag_U8;
	uint8_t ChanElmtUndrVolNumFlag_U8;

	uint8_t ChanOverVolFiltNum_U8; /*every proce num */
	uint8_t ChanUndrVolSteadFiltNum_U8;
	uint8_t ChanHighVolSteadFiltNum_U8;
	uint8_t ChanLowVolSteadFiltNum_U8;
	uint8_t ChanElmtOverVolFiltNum_U8;
	uint8_t ChanElmtUndrVolFiltNum_U8;
	
	uint8_t ChanOverVolFlag_U8; /*the over voltage  10ms filt resulte*/
	uint8_t ChanUndrVolFlag_U8;/*the undr voltage  10ms filt resulte*/
	uint8_t ChanOverVolChangeFlag_U8; /*for over voltage data save,once happen or clear save a file*/
	uint8_t ChanUndrVolChangeFlag_U8; /*for under voltage data save,once happen or clear save a file*/

	uint8_t ChanVolStatus_U8; /*for channel status after filt 10ms*/  	//0 1 status
	uint8_t ChanOperUpStartFlag_U8; /*for channel status,1ms,for judge the relay contact jump */  	//0 1 status
	uint8_t ChanOperUpFlag_U8;/*when operate 10ms, it set 1*/	
	uint8_t ChanOperDownFlag_U8;/*when operate 10ms, it set 1*/	
	uint16_t ChanJumpNum_U16;
	uint16_t ChanStartTime_U16; /*the unit is us,need 32bit*/
	uint16_t ChanEndTime_U16;/*can delete*/
	

}VOL_CHAN_JUDGE;

typedef struct 
{
	uint16_t FPGATime_U16;
	//uint16_t OprtTimeFlag_U16;
	VOL_CHAN_JUDGE VolChanJudge[VOL_CHAN_NUM];
	CURR_CHAN_JUDGE CurrChanJudge[CURR_CHAN_NUM];
}CHAN_JUDGE;



/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/

//extern uint8_t g_file_Flag_U8; /*when a new file is ture,else  is false*/
//extern uint8_t g_file_Name_U8[40];
//extern uint32_t g_frame_EventNum_U32; /*the event data number*/

/**********************************************************************
*
*Global Prototype D1eclare Section
*
*********************************************************************/
int8_t DataProcPowerOn(CHAN_STATUS_INFO * ChanStatInfo_ST_p, CHAN_CALIB_DATA * ChLowCalibDtap,CHAN_CALIB_DATA * ChHighCalibDtap, uint8_t ChanNumTmp,uint8_t EADSType);
int8_t EADSDataProc(uint8_t ProcNum,uint8_t EADSType_U8,EADS_ERROR_INFO * EADSErrInfop);
int8_t EADSDataBuf(uint8_t ProcNum);
int8_t ChanDataFilt(CHAN_DATA * InChdata,CHAN_DATA * OutChData,uint8_t ProcNum,uint8_t EADSType);
int8_t AllChanDataCalib(int16_t VolRefValue,CHAN_CALIB_DATA *ChanCalib_ST_p,CHAN_DATA *ChanAcqr_ST_p,uint8_t ClibType);
int8_t VolChanDataTrans(BLVDS_BRD_DATA * BrdData_ST_p, CHAN_DATA * OutBuf_p,CHAN_CALIB_DATA * ChanLowCalib_ST_p,CHAN_CALIB_DATA * ChanHighCalib_ST_p,uint8_t TranNum,uint8_t TrnsType);
int8_t RelayLifePrect(CHAN_STATUS_INFO *ChanInfop,RELAY_LIFE_CFG *LifeCfg_p);

/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 
