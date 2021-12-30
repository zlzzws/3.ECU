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
#ifndef _DIRCMNG_H
#define _DIRCMNG_H

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
#include "WriteLog.h"
#include <sys/vfs.h>  
#include <dirent.h>
/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/
#define  REC_DIR_TYPE 1
#define  REC_FILE_TYPE 2
/**********************************************************************
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

unsigned long GetFileSize(char *Path);
int8_t GetMemSize(uint8_t *Path,uint32_t *TotalSize_MB,uint32_t *FreeSize_MB);
int8_t MultiDircCreate(uint8_t *DirPath);
int8_t DeleteEarliestDir(uint8_t *FilePath_p);
int8_t DeleteEarliestFile(uint8_t *FilePath_p,uint8_t FileType);
int8_t TarDir(uint8_t *FilePath_p,uint8_t * NowFilePah);
int8_t FileDirJudge(uint8_t *FilePath_p);
/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 
