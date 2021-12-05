/**********************************************************************
*File name 		:   Crc.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/1/29
*Description    :   crc32 calculate
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
#ifndef _CRC_H
#define _CRC_H


/**********************************************************************
*
*Include File Section
*
*********************************************************************/
#include <stdint.h>

/**********************************************************************
*
*Global Prototype Declare Section
*
*********************************************************************/
uint32_t Crc32CalU32Bit(uint32_t *InBuff, uint32_t Length );
uint8_t calc_checksum(uint8_t *date, uint32_t date_len);
uint8_t calc_Xor(uint8_t *date, uint32_t data_len);

/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 

