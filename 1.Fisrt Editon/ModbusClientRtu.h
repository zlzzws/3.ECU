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
#ifndef MODBUSCLIENTRTU_H
#define MODBUSCLIENTRTU_H

/**********************************************************************
*
*Include File Section
*
*********************************************************************/ 
#include "BaseDefine.h"
#include "include/modbus.h"
#include <stdarg.h>
#include <limits.h>
//#include <config.h>
#include <errno.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

//#include "include/modbus-tcp.h"


#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# ifndef _MSC_VER
# include <stdint.h>
# else
# include "stdint.h"
# endif
#endif


#define MSG_LENGTH_UNDEFINED -1

#define SERVER_ID         17
#define XINTE_ID         3
#define RTU_DMU_ID         1
#define RTU_HONGWAI_ID     16
#define INVALID_SERVER_ID 18


#define  PIXEL_NUM 0x40 //RTU once read PIXEL NUM
#define  PIXEL_READ_NUM 12 //RTU once read PIXEL NUM
// #define UT_BITS_ADDRESS            
// #define UT_INPUT_BITS_ADDRESS      0x00	    
// #define UT_REGISTERS_ADDRESS       0x1f40   
// #define UT_INPUT_REGISTERS_ADDRESS 0x3e8	 /*input register 1000         */
#define UT_BITS_ADDRESS 0x130 /*coil address  -R/W  2000    Code 01 */
#define UT_BITS_NB 0x25

#define UT_INPUT_BITS_ADDRESS 0x1C4 /*descrete input address      */
#define UT_INPUT_BITS_NB  0x16


/*Hongwai */
#define UT_REGISTERS_ADDRESS_NUM1  0x00 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_NUM2  0x40 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_NUM3  0x80 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_NUM4  0xC0 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_NUM5  0x100 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_NUM6  0x140 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_NUM7  0x180 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_NUM8  0x1C0 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_NUM9  0x200 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_NUM10  0x240 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_NUM11  0x280 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_NUM12  0x2C0 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_NUM13  0x300 /*holding register  -R/W  0000  Code 03*/

#define UT_REGISTERS_NB_HONGWAI  0x40  /*holding register  number (by wor9)*/
#define UT_REGISTERS_NB_MAX  0x50

/*DMU Address*/
#define UT_REGISTERS_ADDRESS_DMU  0x10 /*holding register  -R/W  0000  Code 03*/

#define UT_REGISTERS_NB_DMU  12  /*holding register  number (by wor9)*/
//#define UT_REGISTERS_NB_MAX  0x20

/*Smart Sensor*/
#define UT_REGISTERS_ADDRESS_SENSOR1  0 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR2  1 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR3  2 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR4  3 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR5  4 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR6  5 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR7  6 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR8  7 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR9  8 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR10  9 /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR11  0x0A /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR12  0x0B /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR13  0x0C /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR14  0x0D /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR15  0x0E /*holding register  -R/W  0000  Code 03*/
#define UT_REGISTERS_ADDRESS_SENSOR16  0x0F /*holding register  -R/W  0000  Code 03*/

#define UT_REGISTERS_NB_SENSOR  9  /*holding register  number (by wor9)*/
//#define UT_REGISTERS_NB_MAX  0x20

/*Xinte Address*/
#define UT_REGISTERS_ADDRESS  5 /*holding register  -R/W  0000  Code 03*/

#define UT_REGISTERS_NB  5  /*holding register  number (by wor9)*/
//#define UT_REGISTERS_NB_MAX  0x20

/* Raise a manual exception when this address is used for the first byte */
#define UT_REGISTERS_ADDRESS_SPECIAL 0x170
/* The response of the server will contains an invalid TID or slave */
#define UT_REGISTERS_ADDRESS_INVALID_TID_OR_SLAVE 0x171
/* The server will wait for 1 second before replying to test timeout */
#define UT_REGISTERS_ADDRESS_SLEEP_500_MS 0x172
/* The server will wait for 5 ms before sending each byte */
#define UT_REGISTERS_ADDRESS_BYTE_SLEEP_5_MS 0x173

/* If the following value is used, a bad response is sent.
   It's better to test with a lower value than
   UT_REGISTERS_NB_POINTS to try to raise a segfault. */
#define  UT_REGISTERS_NB_SPECIAL 0x2

#define  UT_INPUT_REGISTERS_ADDRESS  0x108 /*input register 1000         */
#define  UT_INPUT_REGISTERS_NB  0x1

#define UT_REAL  123456.00

#define UT_IREAL_ABCD 0x0020F147
#define UT_IREAL_DCBA 0x47F12000
#define UT_IREAL_BADC 0x200047F1
#define UT_IREAL_CDAB 0xF1470020

/* const uint32_t UT_IREAL_ABCD = 0x47F12000);
const uint32_t UT_IREAL_DCBA = 0x0020F147;
const uint32_t UT_IREAL_BADC = 0xF1470020;
const uint32_t UT_IREAL_CDAB = 0x200047F1;*/

#define EXCEPTION_RC  2
#define MAX_MESSAGE_LENGTH 260


/* 3 steps are used to parse the query */
typedef enum {
    _STEP_FUNCTION,
    _STEP_META,
    _STEP_DATA
} _step_t;
/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/


/**********************************************************************
*
*Global Structure Define Section
*
******************************************************************/

typedef struct 
{

	uint8_t Year_U8;
	uint8_t Month_U8;
	uint8_t Day_U8;
	uint8_t Hour_U8;
	uint8_t Minute_U8;
	uint8_t Second_U8;
	uint16_t AC_Current_U16[6];


}RTU_SENSOR;

typedef struct 
{
	uint16_t TemArr_U16[PIXEL_NUM * PIXEL_READ_NUM];
	uint16_t TemEnvir_U16;
}RTU_HONGWAI;


typedef enum _SENSOR_NUM
{

	Sensor_1 = 0,
	Sensor_2 = 1,
	Sensor_3 = 2,
	Sensor_4, 
	Sensor_5, 
	Sensor_6, 
    Sensor_7, 
	Sensor_8, 
	Sensor_9, 
	Sensor_10,
	Sensor_11,
	Sensor_12, 
	Sensor_13, 
	Sensor_14, 
	Sensor_15, 
	Sensor_16 = 15,
	Sensor_DMU= 16,  

}SENSOR_NUM_ENUM;


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
int RtuInit(int baudrate);
int RtuReadRegister(uint16_t *tab_rp_registers);
int RtuReadRegister_DMU(uint16_t *tab_rp_registers,SENSOR_NUM_ENUM SenSorNum);
int RtuReadRegister_HongWai(uint16_t *tab_rp_registers,SENSOR_NUM_ENUM SenSorNum);
/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif /*ARM_MODBUS_SERVER_H*/
