/**********************************************************************
*File name      :   GPIOControl.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/8/1
*Description    :   Include file read and write the GPIO of PS
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/8/1 Create
*********************************************************************/

/**********************************************************************
*
*Multi-Include-Prevent Sart Section
*
*********************************************************************/
#ifndef _GPIOCONTROL_H
#define _GPIOCONTROL_H

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
/***********************************************************************
*Global Macro Define Section*
*********************************************************************/

#define    LEDLIFE_BRIGHTNESS   "/sys/class/leds/applife/brightness"
#define    LEDLIFE_TRIGGER      "/sys/class/leds/applife/trigger"
#define    PERIOD_COEFF     500000 //500ms

#define    LEDERR_BRIGHTNESS    "/sys/class/leds/lederr/brightness"
#define    LEDERR_TRIGGER       "/sys/class/leds/lederr/trigger"
#define    LED_On 0
#define    LED_Off 1

#define GPIO_POWDOW  "948" //GPIo 42
#define GPIO_POWDOW_DIREC  "/sys/class/gpio/gpio948/direction"
#define GPIO_POWDOW_VALUE  "/sys/class/gpio/gpio948/value"

#define GPIO_RS485EN  "956" //GPIo 50
#define GPIO_RS485EN_DIREC  "/sys/class/gpio/gpio956/direction"
#define GPIO_RS485EN_VALUE  "/sys/class/gpio/gpio956/value"

/***********************************************************************
*Global Struct Define Section*
*********************************************************************/


/***********************************************************************
*Global Variable Declare Section*
*********************************************************************/


/***********************************************************************
*Global Prototype Declare Section*
*********************************************************************/
int8_t GPIO_PowDownIoCreat(void);
int8_t GPIO_PowDowOpen(void);
int8_t GPIO_PowDowRead(uint8_t *value);
int8_t GPIO_PowDowWrite(uint8_t value);
int8_t GPIO_SysFlWrLow(void);

int8_t GPIO_RS485Open(void);
int8_t GPIO_RS485EnOpen(void);
int8_t GPIO_RS484EnWrite(uint8_t value);
/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 
