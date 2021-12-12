/**********************************************************************
*File name 		:   BramDataInter.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/1/29
*Description    :   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/03/29  Create
*
*********************************************************************/

/**********************************************************************
*
*Multi-Include-Prevent Sart Section
*
*********************************************************************/
#ifndef _BRAMDATAINTER_H
#define _BRAMDATAINTER_H

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
#include "Bram.h"
#include "Crc.h"
/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/

#define RD_CRCERR_NUM 5  /*repeate 4 read,total number 5 rd */
#define WR_CRCERR_NUM 5  /*repeate 4 write,total number 5 write */

/**********************************************************************
*
*Global Struct Define Section
*********************************************************************/
typedef struct 
{
	uint8_t 	*MapBlckAddr_p;
	uint8_t 	*MapBlckFlgAddr_p;
	uint8_t  	DataU32Length; //include the top
	uint8_t  	ChanNum_U8;
	uint32_t 	BramBlckAddr;
	uint32_t 	BramBlckFlgAddr;	
}BRAM_ADDRS;


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
int8_t BramReadWithChek(BRAM_ADDRS *BramAddrs_p,uint32_t *Outbuf);
int8_t BramWriteWithChek(BRAM_ADDRS *BramAddrs_p,uint32_t Inbuf[],BRAM_PACKET_TOP TopPackST);
/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 
