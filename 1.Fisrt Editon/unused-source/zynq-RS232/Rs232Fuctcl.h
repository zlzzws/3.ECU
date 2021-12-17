/**********************************************************************
*File name 		:   RS232Functcl.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2021/9/9
*Description    :   Include uartport control function
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2021/9/9  Create
*********************************************************************/

/**********************************************************************
*
*Multi-Include-Prevent Sart Section
*
*********************************************************************/
#ifndef _RS232FUCTCL_H
#define _RS232FUCTCL_H

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
#include <termios.h>
#include "BaseDefine.h"
#include "Crc.h"
#include "WriteLog.h"

/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/
   
#define FALSE 0
#define TRUE 1
/**********************************************************************
*
*Global Struct Define Section
*
*********************************************************************/

/**********************************************************************
*
*Global Prototype Declare Section
*
*********************************************************************/
int     UART_Open(char* port);
void    UART_Close(int fd);
int     UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
int     UART_Recv(int fd, uint8_t *recv_buf,int date_len);
int     UART_Send(int fd, uint8_t *send_buf,int date_len);

/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 
