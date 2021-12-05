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
#ifndef _MODIFCONFIG_H
#define _MODIFCONFIG_H

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
#include "xml.h"
/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/
#define FIFO_FILE "/tmp/FIFO"
#define CHAN_OPER_NUM_FILE "yaffs/REC_FASTFLT/ChanOperaNum.dat" //"/home/feng/Desktop/xilinx_code/yaffs/REC_FASTFLT/ChanOperaNum.dat" ///
#define CHAN_CALIB_110V_FILE "/tffs0/ChanCalib110VValue.dat"
#define CHAN_CALIB_0V_FILE "/tffs0/ChanCalib0VValue.dat"
#define LOGI_PRMTABLE_FILE "/tffs0/UsrSYSPrmTable.dat" ///home/feng/Desktop/xilinx_code

//for ubuntu test
// #define FIFO_FILE "/home/feng/Desktop/CodeGit/EADS/myfifo"
// //  //for ubuntu linux test
// #define CHAN_OPER_NUM_FILE "/home/feng/Desktop/xilinx_code/yaffs/REC_FASTFLT/ChanOperaNum.dat" //"/home/feng/Desktop/xilinx_code/yaffs/REC_FASTFLT/ChanOperaNum.dat" ///
// #define CHAN_CALIB_110V_FILE "/home/feng/Desktop/xilinx_code/tffs0/ChanCalib110VValue.dat"
// #define CHAN_CALIB_0V_FILE "/home/feng/Desktop/xilinx_code/tffs0/ChanCalib0VValue.dat"
// #define LOGI_PRMTABLE_FILE "/home/feng/Desktop/xilinx_code/tffs0/UsrSYSPrmTable.dat" ///home/feng/Desktop/xilinx_code

#define PARMTABLE_LENGTH 412
#define OPRTNUM_LENGTH  4
/********************************************************************
*
*Global Struct Define Section
*
*********************************************************************/


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

int8_t GetPTUChanThresholdValue(uint8_t *ConfPath_p,CHAN_THREHOLD_VALUE *OutputValue_p,VERSION *Version_p);
int8_t GetChanCalibData(uint8_t *Path,CHAN_CALIB_DATA *ChanCalibDat_ST_p);
int8_t GetChanOperateNum(uint32_t *Channel_OperNum_U32);
int8_t ChanDataPrint(CHAN_DATA *ChanData_ST_p,uint8_t EADSType);
/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 
