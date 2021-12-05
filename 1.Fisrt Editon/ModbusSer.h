/**********************************************************************
*File name 		:   modbus.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/08/27
*Description    :   Include the mapping address and number of modbus server.
*Version        :   REV1.0.0       
*Author:        :   deng
*
*History:
*REV1.0.0     deng    2018/8/27  Create
*
*
*
*********************************************************************/

/**********************************************************************
*
*Multi-Include-Prevent Sart Section
*
*********************************************************************/
#ifndef MODBUSSER_H
#define MODBUSSER_H

/**********************************************************************
*
*Include File Section
*
*********************************************************************/ 
#include "BaseDefine.h"
#include "include/modbus.h"
//#include "include/modbus-tcp.h"

/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/
/*mapping address,nubmer */
#define UT_BITS_ADDRESS_TCP            0x7d0	/*coil address  -R/W  2000    */
#define UT_INPUT_BITS_ADDRESS_TCP      0x00	    /*descrete input address      */
#define UT_REGISTERS_ADDRESS_TCP       0x1f40   /*holding register  -R/W  8000*/
#define UT_INPUT_REGISTERS_ADDRESS_TCP 0x3e8	 /*input register 1000         */

#define UT_BITS_NB_TCP            0x280       /*640   */
#define UT_INPUT_BITS_NB_TCP     0x3c1       /*960   */
#define UT_REGISTERS_NB_TCP       0x140       /*320   */
#define UT_INPUT_REGISTERS_NB_TCP 0x3c1       /*960   */  

/**********************************************************************
*
*Global Structure Define Section
*
******************************************************************/

/**********************************************************************
*
*Global Variable Declare Section
*
***********************************************************************/

/**********************************************************************
*
*Global Prototype Declare Section
*
*********************************************************************/
void VersionSet(modbus_mapping_t *mb_mapping,VERSION *Versi_p);
void LightTest(modbus_mapping_t *mb_mapping);
int8_t VersionInit(VERSION *Versi_p);


/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif /*ARM_MODBUS_SERVER_H*/
