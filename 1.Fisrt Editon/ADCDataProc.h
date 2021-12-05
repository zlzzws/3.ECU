/**********************************************************************
*File name 		:   ADCDataProc.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/5/16
*Description    :   Include process the ADC data of ADE7913
                    1.Trans the ADE7913 binary data to Voltage and Current
                    2.Process the Voltage using filting
                    3.Judge the Relay operate Number
                    4.Judge the L/R time
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/05/16  Create
*********************************************************************/

/**********************************************************************
*
*Multi-Include-Prevent Sart Section
*
*********************************************************************/
#ifndef _ADCDATAPROC_H
#define _ADCDATAPROC_H

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
#include  <math.h> 
//#include "WriteLog.h"
/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/
#define CURR_BINTAN_FACTOR  1.5036 /*8bit = 1.5036uV */
#define VOL_BINTAN_FACTOR  305.17578 /*24.0478 8bit = 24.047uV */
#define CURR_HALL_BINTAN_FACTOR  305.17578   /*24.414 /*8bit = 24.414uV */

//20190821 is 10
//20190823 change to 100
#define CURR_REALTAN_FACTOR  10 /*VI = 0.1 * I */
#define VOL_REALTAN_FACTOR  44.75/*551/*Vp = V /551 mV */

#define LRTIME_FACTOR  1/* max LRtime incre every 100us */


#define ADCCURR_110V 33

#define ADCVOLI_INC 50

#define ADCVOLT_ERR_110V 0

#define ADCVOLT_LOW 500
#define ADCVOLT_HIGH 1100

#define CURRENT_OFFSET 0/* max LRtime incre every 100us */
/**********************************************************************
*
*Global Struct Define Section
*
*********************************************************************/
typedef struct 
{
	uint16_t Volatge_U16;
	uint16_t Current_U16;

}ADC_VI_VALUE;

typedef struct 
{
	uint16_t Volatge_U16;

}ADC_V_VALUE;
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
int8_t ADC7913IDataTranInt(uint32_t *BramInBuf,CHAN_DATA *OutBuf,uint8_t TranLeng,uint8_t TranType);
int8_t ADC7913VDataTranInt(uint32_t BramInBuf[],CHAN_DATA *OutBuf,CHAN_CALIB_DATA * ChanLowCalib_ST_p,CHAN_CALIB_DATA * ChanHighCalib_ST_p,uint16_t TranLeng,uint8_t TranType);
int8_t MAXLifeRead(uint32_t BramInBuf,CHAN_DATA *OutBuf,uint16_t TranLeng);
/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 
