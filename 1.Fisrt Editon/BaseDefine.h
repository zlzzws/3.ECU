/**********************************************************************
*File name 		:   BaseDefine.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/3/29
*Description    :   Include base define and library
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/03/29  Create
*
*
*********************************************************************/

/**********************************************************************
*
*Multi-Include-Prevent Sart Section
*
*********************************************************************/
#ifndef _BASEDEFINE_H
#define _BASEDEFINE_H

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
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <memory.h>
#include <net/if.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <errno.h>
#include <linux/watchdog.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/
/*Function Select*/
//#define REDUNDANCY_FUNCION //软件冗余功能
//#define UDP_FUNCTION
//#define ETH_FRAME_COUNT	//PHY-读取以太网收发包帧数
#define CAN0_FUNCTION
#define CAN1_FUNCTION
#define MVB_FUNCTION
#define BLVDS_READ_FUNCITON
/*EADS Versionasd History*/
#define EADS_VERSION 				100 
#define EADS_VERSION_PTU 			100
#define TOTAL_VERSION 				100
#define LINUX_VERSION 				440
#define BOOT_VERSION 				100
#define DTB_VERSION 				100
#define RAMDISK_VERSION 			100
#define FPGA_VERSION 				100
#define BRAM_BLOCK_LENGTH 			256
#define BRAM_BLOCK_NUM 				128
#define ECU_VERSION_PTU 			100  

#define BLVDS_MAX10_CHAN 			1
#define ALL_CHAN_NUM 				80  	/*文件存储时，CSR_DRIVER模拟量通道数量80*/
#define MAX_RealWave_Num 			40
#define CURR_CHAN_NUM 				14
#define VOL_CHAN_NUM  				40
/*CODE_STATUS*/
#define CODE_CREAT  				2
#define CODE_EXIST  				1
#define CODE_OK  					0
#define CODE_ERR 					-1
#define CODE_WARN  					-2

#define VOL_STATUE_NUM  			5
#define CURR_STATUE_NUM  			2
#define FPGA_TIME_BYTE  			20

#define POWDOW_FILT 				10 		/*掉电信号滤波次数*/
#define FILETRY_NUM 				3 		/*文件创建失败尝试次数*/

#define LOG_FILE_TYPE 				1 		/*for log file delete*/
#define RECORD_FILE_TYPE 			2 		/*for drive record file */

#define PHY_COPPER_CONTRL_REG 		0
#define PHY_COPPER_STATUS_REG 		1
#define PHY_COPPER_CONTRL_REG_OK 	0x2100

#define FIFO_FILE 					"/tmp/FIFO"

/***********************************************************************
*Global Struct Define Section*
*********************************************************************/

typedef struct
{
	uint32_t BLVDSTOP_U32;					/*0xC21104*/
	uint32_t BLVDSReser_U32[2];				/*0x00*/
    uint32_t BLVDSData_U32[61];

}BRAM_PACKET_DATA;							/*256Byte*/

typedef struct
{
	uint32_t BLVDSTOP_U32;					/*570:0x11C2,MAX10,0x1144*/
	uint32_t BLVDSReser_U32[2];				/*0x00*/
}BRAM_PACKET_TOP;							/*12Byte*/

typedef enum _DEBUG_TYPE
{ 
	BRAM_RD_DEBUG   	= 1,
	BRAM_WR_DEBUG 		= 2,    			
	TIME_DEBUG 			= 3,
	LINECAlib_DEBUG     = 4,	    			
	ALRM_DEBUG 			= 5,
	DEVC_DEBUG 			= 6,
	XML_DEBUG 			= 7,
	FILE_DEBUG 			= 8,
	GPIO_DEBUG 			= 9,
	POWTIME_DEBUG 		= 10,
	TCP_DEBUG 			= 11,
	UDP_DEBUG			= 12,
	LCU_MVB_DEBUG		= 13,	
	SEM_DEBUG 			= 14,
	DOG_DEBUG 			= 15,
	ETH_FRAMES_DEBUG	= 16,   			
	RTU_DEBUG 			= 18,				
	TMS570_BRAM_RD_DEBUG= 19,				
	TMS570_BRAM_WR_DEBUG= 20,				
    CAN_RD_DEBUG        = 21,
    CAN_WR_DEBUG        = 22,
	MVB_WR_DEBUG		= 23,
	MVB_RD_DEBUG		= 24,
	FileSave_DEBUG 		= 25,				
	socket_Debug 		= 26,
	BLVDS_RD_DEBUG		= 27,
	BLVDS_WR_DEBUG		= 28,				
}DEBUG_TYPE_ENUM;

typedef enum _COMMU_MCU
{
	TMS570_MCU = 1,
	MAX10_MCU= 2,

}COMMU_MCU_ENUM;

typedef struct 
{
	uint16_t Total_Ver_U16;
	uint16_t Linux_Ver_U16;
	uint16_t SySCfgFile_Ver_U16;
	uint16_t SftCfgFile_Ver_U16;
	uint16_t RrdCfgFile_Ver_U16;
	uint16_t SysPrmFile_Ver_U16;
	uint16_t ECU_RunVer_U16;
	uint16_t Boot_Ver_U16;
	uint16_t FPGA_Ver_U16;
	uint16_t Urmdik_Ver_U16;
	uint16_t DevicTree_Ver_U16;
	uint16_t TMS570_RTSVer_U16;
	uint16_t TMS570_IECVer_U16;
	uint16_t MAX1_Ver_U16;
}VERSION;

typedef struct
{
   uint16_t TrainNum_U16;
   uint8_t  CoachNum_U8;
}TRAIN_INFO;

typedef struct
{
	FILE *EventFile_fd;
	FILE *EventBLVDS_fd;
}FILE_FD;


typedef struct
{
    pthread_t ReadDataThread;
	pthread_t RealWaveThread;
	pthread_t ModbusThread;
	pthread_t FileSaveThread;
	pthread_t TRDPThread;
	pthread_t LedThread;
	pthread_t DirTarThread;	
	pthread_t CAN0Thread;
    pthread_t CAN1Thread;
	pthread_t MVBThread;
	pthread_t UdpThread;
	pthread_t RedundancyThread;
	pthread_t BlvdsThread;	
}PTHREAD_INFO;

typedef struct
{
    pthread_rwlock_t  BramDatalock;
	pthread_rwlock_t  ChanDatalock;
	pthread_rwlock_t  RealDatalock;
	pthread_rwlock_t  ChanInfolock;
}PTHREAD_LOCK;

typedef struct
{
    uint8_t Bit0:1;
    uint8_t Bit1:1;
    uint8_t Bit2:1;
	uint8_t Bit3:1;
	uint8_t Bit4:1;
	uint8_t Bit5:1;
	uint8_t Bit6:1;
	uint8_t Bit7:1; 
      
}BYTE_BIT;

typedef struct _ecu_app_error_
{
	uint16_t bram_blvds_rd_err:1;
	uint16_t bram_blvds_wr_err:1;
	uint16_t bram_emif_rd_err:1;
	uint16_t bram_emif_wr_err:1;
	uint16_t bram_mvb_rd_err:1;
	uint16_t bram_mvb_wr_err:1;
	uint16_t filesave_EVENT_err:1;
	uint16_t filesave_BLVDS_err:1;
	uint16_t bram_init_err:1;
	uint16_t tcp_err:1;
	uint16_t modbus_err:1;
	uint16_t udp_err:1;
	uint16_t phy_link_err:1;
	uint16_t power_err:1;
	uint16_t max10_life_err:1;
	uint16_t tms570_life_err:1;
}app_err;

typedef struct _ecu_communication_error_
{
	uint8_t mvb_err:1;
	uint8_t bms_can_err:1;
	uint8_t dcdc_can_err:1;
	uint8_t fcu_a_can_err:1;
	uint8_t fcu_b_can_err:1;
	uint8_t inverter_1_can_err:1;
	uint8_t inverter_2_can_err:1;
	uint8_t extension_can_err:1;
}communication_err;

typedef struct _device_err_level_
{
	uint8_t level1:1;
	uint8_t level2:1;
	uint8_t level3:1;
	uint8_t reserve:5;
}err_level;

typedef struct _ecu_error_info_
{	
	app_err ecu_app_err;
	communication_err ecu_commu_err;
	err_level fcu_err_level;	
	err_level bms_err_level;
	err_level dc_dc_err_level;
	err_level h2_store_err_level;
	err_level heat_dissipation_err_level;
}ECU_ERROR_INFO;

typedef struct 
{
	uint16_t UpOprtTimeMin_U16;
	uint16_t UpOprtTimeMax_U16;
	uint16_t DownOprtTimeMin_U16;
	uint16_t DownOprtTimeMax_U16;
	uint16_t JumpTimeMin_U16;
	uint16_t JumpTimeMax_U16;
    uint32_t RelayOprtLife_U32;
}RELAY_LIFE_PARA_CFG;

typedef struct
{
	RELAY_LIFE_PARA_CFG RelayChanCfg_ST[VOL_CHAN_NUM];
	float RelayNumFactor;	
	float UpOprtTimeFactor;
	float DownOprtTimeFactor;
	float JumpTimeFactor;
}RELAY_LIFE_CFG;

typedef struct
{
	uint32_t MIN_RESER_SPACE;
	uint32_t EVENT_RESER_SPACE;	
	uint32_t FLT_RESER_SPACE;
	uint32_t OPRT_RESER_SPACE;
	uint16_t LOGFILE_NUM;
	uint32_t CAN_RESER_SPACE;
    uint32_t UART_RESER_SPACE;
}SPACE_JUDGE_VALUE;

typedef enum devic_type
{
	CAN_Device = 1,
	Uart_Device = 2,
}peripheralDevice;//枚举外设设备类型


typedef struct tms570_bram_data
{
    //数据区不包含CRC32的4个字节+帧头12个字节
	uint32_t buffer[60];	
}TMS570_BRAM_DATA;

typedef enum _trans_type
{
	CAN0_BRAM = 0,
	CAN1_BRAM = 1,
	MVB_BRAM  = 2
}TRANS_TYPE_ENUM;

/***********************************************************************
*Global Variable Declare Section*
*********************************************************************/


/***********************************************************************
*Global Prototype Declare Section*
*********************************************************************/

/***********************************************************************
*Multi-Include-Prevent End Section*
*********************************************************************/
#endif 
