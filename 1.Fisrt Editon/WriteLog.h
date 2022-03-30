/**********************************************************************
*File name 		:   WriteLog.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2020/4/21
*Description    :   System timer set.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/4/21  Create
*
*
*
*********************************************************************/


/**********************************************************************
*
*Multi-Include-Prevent Sart Section
*
*********************************************************************/
#ifndef _WRITELOG_H
#define _WRITELOG_H

/**********************************************************************
*
*Include File Section
*
*********************************************************************/
#include "Timer.h"
#include "DircMng.h"
/**********************************************************************
*Global Macro Define Section
*********************************************************************/
// 函数宏定义
#define WRITELOGFILE(level, msg)  WriteLogFile(__FILE__, __FUNCTION__, __LINE__, level, msg)
// 日志级别定义
#define LOG_FATAL_1         0     // 严重错误
#define LOG_ERROR_1         1     // 一般错误
#define LOG_WARN_1          2     // 警告
#define LOG_INFO_1          3     // 一般信息
#define LOG_TRACE_1         4     // 跟踪信息
#define LOG_DEBUG_1         5     // 调试信息
#define LOG_ALL_1           6     // 所有信息
#define LOG_CFG             "/tffs0/LogConFig.ini"
#define LOG_FILE_HEAD       "Log"   
#define LOG_PATH_LENG       100
#define LOG_INFO_LENG       200
/***********************************************************************
Global Struct Define Section
*********************************************************************/

/***********************************************************************
Global Prototype Declare Section*
*********************************************************************/
int8_t LogFileCreatePowOn();
int32_t VersionSave(VERSION * Ver_p);
int8_t LogClose(void);
int8_t WriteLogFile(char *FileName, const char * Function, uint32_t CodeLine, uint32_t inLogLevel, char * Content);
/***********************************************************************
Multi-Include-Prevent End Section
*********************************************************************/
#endif 