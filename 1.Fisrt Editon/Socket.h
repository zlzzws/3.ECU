/**********************************************************************
*File name 		:   Socket.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/5/3
*Description    :   Include Socket Communicate
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/5/3  Create
*
*********************************************************************/

/**********************************************************************
*
*Multi-Include-Prevent Sart Section
*
*********************************************************************/
#ifndef _SOCKET_H
#define _SOCKET_H

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
#include "Crc.h"
#include <sys/socket.h>
#include <netinet/in.h>

/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/
#define SOCKET_LENGTH  2048
#define UDP_LENGTH 701
#define TCP_PORT 5002
/**********************************************************************
*
*Global Struct Define Section
*
*********************************************************************/
typedef struct 
{
    uint8_t BufferTop[2];
    uint8_t MonitSourc[2];
    uint8_t ChanBit[6];
}PTU_REAL_WAVE_SET;


typedef struct
{
    uint8_t RealChanNum_U8;
	uint8_t RealWaveChanName_U8[48];
}REAL_SET_INFO;
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

int8_t RealTimeWaveExtr(uint8_t *BufferIn,uint8_t *RecorChanNum,uint8_t *ChanLenth);
int8_t RealWaveData(uint8_t BufferIn[],uint8_t RecorChanNum[],CHAN_DATA *ChanData_ST_p,uint8_t ChanLenth);
/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 
