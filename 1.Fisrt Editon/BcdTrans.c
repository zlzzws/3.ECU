/**********************************************************************
*File name 		:   BcdTrans.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2020/3/26
*Description    :   BCD trans to Dec, or Dec tans to BCD
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*
*
*********************************************************************/

/**********************************************************************
*
*Include file  section
*
*********************************************************************/
#include "BcdTrans.h"

/**********************************************************************
*
*Global Table Define Section
*
*********************************************************************/

/**********************************************************************
*Name			:   int8_t DectoBCD(int Dec, unsigned char *Bcd, int length)
*Function       :   DectoBCD trans
*Para     		:   int Dec,
*					unsigned char *Bcd
*                   int length
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
* little end
*********************************************************************/
int8_t DectoBCD(uint16_t Dec,uint8_t Bcd[],uint8_t length)
{	
	uint8_t i ;
	uint16_t temp;
	uint8_t BCDTem = 0;
	for (i = 0;i < length;i++)
	{
		temp = Dec % 100;
		BCDTem = ((temp/10) << 4) + ((temp %10) & 0x0F);
		Bcd[i] = BCDTem;
		Dec /= 100;
	}
	return 0;
}
/**********************************************************************
*Name			:   int8_t Litte2BigEnd(uint Dec, unsigned char *Bcd, int length)
*Function       :   change the  U16 date frome Litte end to BigEnd
*Para     		:   int Dec,
*					unsigned char *Bcd
*                   int length
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
uint16_t Litte2BigEnd(uint16_t Indata)
{
	uint16_t result = 0;
	result = (((Indata & 0xFF) << 8 )|((Indata >> 8) & 0xFF));
	return  result;
}


/**********************************************************************
*Name			:   uint16_t BigU8ToLitteU16(uint8_t IndataH,uint8_t IndataL)
*Function       :   change the U8 High date and low data to Little nd
*Para     		:   int Dec,
*					unsigned char *Bcd
*                   int length
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
uint16_t BigU8ToLitteU16(uint8_t IndataH,uint8_t IndataL)
{
	uint16_t result = 0;
	result = (IndataH << 8 )|IndataL;
	return  result;
}



