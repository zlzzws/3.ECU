/**********************************************************************
*File name 		:   Timer.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/1/29
*Description    :   System timer set.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/5/16  Create
*
*
*
*********************************************************************/


/**********************************************************************
*
*Multi-Include-Prevent Sart Section
*
*********************************************************************/
#ifndef _TIMER_H
#define _TIMER_H

/**********************************************************************
*
*Include File Section
*
*********************************************************************/
#include "BaseDefine.h"
#include "WriteLog.h"
#include <linux/rtc.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/
#define SYSTIME_LENGTH 16
#define MAX_BYTES            3 
#define DEFAULT_I2C_BUS      "/dev/i2c-1"

/**********************************************************************
*
*Global Struct Define Section
*
*********************************************************************/

typedef struct 
{

    uint8_t Year_U8[2];
	uint8_t Month_U8;
	uint8_t Day_U8;
	uint8_t Hour_U8; 
	uint8_t Minute_U8;
	uint8_t Second_U8; /*8 byte*/
	uint8_t Resevr_U8;

}SYSTEM_TIME;


/**********************************************************************
*
*Global Prototype Declare Section
*
*********************************************************************/
int8_t set_timer(uint32_t second,uint32_t usecond);
int8_t RTCTesT(void);
int8_t SetSystemTime(SYSTEM_TIME *DatStr);
int8_t SetRTCTime(SYSTEM_TIME *DatStr);
int threadDelay(const long lTimeSec, const long lTimeUSec);
void GetTime(uint8_t *pszTimeStr);
void GetCompileTime(void);
/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 