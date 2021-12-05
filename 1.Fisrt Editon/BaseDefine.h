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
#include <signal.h>   //包含signal()函数
#include <semaphore.h>
#include <errno.h>
#include <linux/watchdog.h>
#include <linux/sockios.h>
#include <linux/mii.h>
/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/

/*EADS Versionasd History*/
/*1.0.0   20200521*/
#define EADS_VERSION 100 //tcms version
#define EADS_VERSION_PTU 102 /*1.0.1   20210620*/
/*1.0.0   20210419 keep with the trdp version same for 17bian*/
#define TOTAL_VERSION 100 /*1.0.1   20210227*/
/*1.0.0   20210419 keep with the trdp version same for 17bian*/
#define LINUX_VERSION 440
#define BOOT_VERSION 100
#define DTB_VERSION 100
#define RAMDISK_VERSION 100
#define FPGA_VERSION 100
#define BRAM_BLOCK_LENGTH 256
#define BRAM_BLOCK_NUM 128
/*the channel number of once judgea */
//570 channel
#define OUTIO570_CHAN 1
#define COMMAND570_CHAN 3
#define RD_570FEDERR_NUM 10
#define UPSURE_BLOCKNUM 0xDDDD
#define UPSTAR_BLOCKNUM 0xEEEE
#define UPSUCC_BLOCKNUM 0xFFFF
//20200326
 #define CHAN_BUFFER_NUM 5  //maybe is CHAN_BUFFER_NUM = 5;casue threshold change
 #define REAL_BUFFER_NUM 500 //the   
/*Board Date LengthID*/
#define ADU_BRD_RD_LENGTH_U32  29 ///
#define CTU_BRD_RD_LENGTH_U32  29 // include top,but not include the crc

#define COACH1 1
#define COACH2 2
#define COACH3 3
#define COACH4 4
#define COACH16 16
#define COACHZP 5

#define ADU_BOARD_ID 0
#define CTU_BOARD_ID 1
#define CAN_CHAN 8

/*the number of channel  have voltage and current*/
#define VOL_CHAN_NUM 40  //ADU插件通道40采集点,Bram数据转换过程中使用48个通道，需要丢弃[41]-[48]
#define CURR_CHAN_NUM 14 ///CTU插件通道14采集点,Bram数据转换过程中使用16个通道，需要丢弃[15]-[16]
#define ALL_CHAN_NUM 80  /*for File save, !!!! note  not include the oprte time */
#define peri_WriteSize 16

#define  CODE_CREAT  2
#define  CODE_EXIST  1
#define  CODE_OK  0
#define  CODE_ERR -1
#define  CODE_WARN  -2

#define VOL_STATUE_NUM  5
#define CURR_STATUE_NUM  2
#define FPGA_TIME_BYTE  20

/*on train use*/
#define COIL_CHAN_T066 30 //real  channel 31
#define CONT_CHAN_123A 31 //real  channel 32

#define COIL_CHAN_T96  7 //real   channel 8
#define CONT_CHAN_106K 25 //real   channel 26

#define COIL_CHAN_T085 14 //real   channel 15
#define CONT_CHAN_T883 16 //real   channel 17 

#define COIL_CHAN_175H1 36 //real   channel 37
#define CONT_CHAN_103B  37 //real   channel 38

#define COIL_CHAN_175H3 38 //real   channel 39
#define CONT_CHAN_103D  39 //real   channel 40

/*20201202, for relay test*/
#define CONT_CHAN_0 0 //for 3RH2140test 
#define CONT_CHAN_1 1 //for 3RH2140test 
#define CONT_CHAN_2 2 //for 3RH2140test
#define CONT_CHAN_3 3 //for 3RH2140test 
#define CONT_CHAN_4 4 //for 3RH2140test
#define CONT_CHAN_5 5 //for 3RH2140test 
#define CONT_CHAN_6 6 //for 3RH2140test
#define CONT_CHAN_7 7 //for 3RH2140test 
#define CONT_CHAN_8 8 //for 3RH2140test
#define CONT_CHAN_9 9 //for 3RH2140test 
#define CONT_CHAN_10 10 //for 3RH2140test

#define COIL_CHAN_12 12
#define COIL_CHAN_13 13
#define COIL_CHAN_14 14

#define CONT_CHAN_20 20 //for 3RH2140test 
#define CONT_CHAN_21 21 //for 3RH2140test
#define CONT_CHAN_22 22
#define CONT_CHAN_23 23
#define CONT_CHAN_24 24
#define CONT_CHAN_25 25
#define CONT_CHAN_26 26
#define CONT_CHAN_27 27
#define CONT_CHAN_28 28
#define CONT_CHAN_29 29
#define CONT_CHAN_30 30
#define CONT_CHAN_31 31

#define COIL_CHAN_32 32
#define COIL_CHAN_33 33
#define COIL_CHAN_34 34 //real   channel 35
#define COIL_CHAN_35 35 //real   channel 36
#define COIL_CHAN_36 36 //real   channel 37
#define COIL_CHAN_37 37 //real   channel 38
#define COIL_CHAN_38 38 //real   channel 39
#define COIL_CHAN_39 39 //real   channel 40

#define CAL_TIME_CHANL 20 //for relay test

//20200326
/*ADC data Trans Type*/
#define NORMTRANTYPE 0
#define CALIBTRANTYPE 1

#define DIGT_JUDGE_NUM  100 /*3s,loop is 30ms*/
#define CH_DIGT_JUDGE_NUM  500 /*500ms,loop is 50ms(base the DIGIT_PRO_TIME),for channel over curr and over voltage,*/

#define ADCVOLT_110V 1100
#define ADCVOLT_0V 0

#define POWDOW_FILT 10 /*filte the powdown io sign*/

#define FILE_CREAT_FLAG 2 /*mean fltreal file oprtreal file have creat*/
#define FILE_FLT_FLAG 1 /*mean fltreal file oprtreal file have creat*/

#define FILETRY_NUM 3 
#define TRDPTRY_NUM 5

#define LOG_FILE_TYPE 1 /*for log file delete*/
#define RECORD_FILE_TYPE 2 /*for drive record file */

#define PHY_COPPER_CONTRL_REG 0
#define PHY_COPPER_STATUS_REG 1
#define PHY_COPPER_CONTRL_REG_OK 0x2100
/**********************************************************************
*
*Global Struct Define Section
*
*********************************************************************/
typedef struct
{
	uint32_t BLVDSTOP_U32;/*0xC21104*/
	uint32_t BLVDSReser_U32[2];/*0x00*/
    uint32_t BLVDSData_U32[61];

}BRAM_PACKET_DATA;/*256Byte*/

typedef struct
{
	uint32_t BLVDSTOP_U32;/*570:0x11C2,MAX10,0x1144*/
	uint32_t BLVDSReser_U32[2];/*0x00*/
}BRAM_PACKET_TOP;/*16Byte*/

typedef struct 
{

	uint32_t EventNum_U32;
	uint16_t FrameLength_U16;

}CHAN_TOP; 

typedef struct 
{
	int16_t CH_Voltage_I16;
}VOL_CHAN_VALUE;

typedef struct 
{
	int16_t CH_Current_I16;
}CURR_CHAN_VALUE;

typedef struct
{
	VOL_CHAN_VALUE VolChan_ST[VOL_CHAN_NUM];  
	CURR_CHAN_VALUE CurrChan_ST[CURR_CHAN_NUM]; 
}CHAN_DATA;


typedef struct
{	
	int16_t VolChanCalib_I16[VOL_CHAN_NUM];
}CHAN_CALIB_DATA;

typedef struct 
{
	
	uint16_t HighCurr;

}CURR_CHAN_THREHOLD_VALUE;

typedef struct 
{
	uint16_t LowVol_H; //Vol uint is 100mv
	uint16_t HighVol_L;
	uint16_t HighVol_H;
	uint16_t SlugVol;// the huicha judge
	CURR_CHAN_THREHOLD_VALUE CurrChan_ThreValue_ST[CURR_CHAN_NUM];
	uint16_t SlugCurr;
}CHAN_THREHOLD_VALUE;

typedef struct 
{
    uint8_t ChanFltNum_U8; /*have happen fault channel num for falut real file save*/
    uint8_t ChanOprtNum_U8;/*have happen operate channel num for oprate real file save*/
    uint8_t FltRealRecFlag_U8;/*the flag for falt file save*/
	uint8_t OprtRealRecFlag_U8;/*the flag for opreat file save*/
	uint8_t CurrFltFlag_U8; /*for enter the CurrWarnFlag judge*/
    uint8_t CurrWarnFlag_U8[CURR_CHAN_NUM]; /*the flag for trdp */
    uint8_t VolWarnFlag_U8[VOL_CHAN_NUM]; /*the flag for trdp */
    uint8_t VolChanStat_U8[VOL_CHAN_NUM];
    uint8_t OprtDirction_U8[VOL_CHAN_NUM];/*for trdp send*/	
	uint16_t RelayLastLife_U16[VOL_CHAN_NUM];/*the opreate time for number*/
    uint16_t OprtUpTime_U16[VOL_CHAN_NUM];/*the opreate time for number*/
    uint16_t OprtDownTime_U16[VOL_CHAN_NUM];/*the opreate time for number*/
	uint16_t JumpTime_U16[VOL_CHAN_NUM];/*the jump time for number*/
	uint32_t Channel_OperaNum_U32[VOL_CHAN_NUM];	 /*the operature num of channel */
    
}CHAN_STATUS_INFO;

typedef enum _DEBUG_TYPE
{
    RELEASE_DEBUG   	= 0,
	TIME_DEBUG 			= 1,
	BRAM_DEBUG 			= 2,    			//双口RAM数据打印
	LINECAlib_DEBUG 	= 3, 				//110V电压校准数据打印
	DATPRO_DEBUG 		= 4,    			//过压、过流时电压、电流数值打印
	ALRM_DEBUG 			= 5,
	DEVC_DEBUG 			= 6,
	XML_DEBUG 			= 7,
	FILE_DEBUG 			= 8,
	GPIO_DEBUG 			= 9,
	POWTIME_DEBUG 		= 10,
	TCP_DEBUG 			= 11,
	ROMTUP_DEBUG 		= 12,
	TRDP_DEBUG 			= 13,
	SEM_DEBUG 			= 14,
	DOG_DEBUG 			= 15,
	ADC_VDATE			= 16,   			//ADC电压转换数值打印   
	ADC_IDATE 			= 17,   			//ADC电流转换数值打印
	RTU_DEBUG 			= 18,				//Modbus_RTU电流转换数值打印
	CAN_RD_DEBUG 		= 19,				//从Bram接收CAN数据打印
	CAN_WR_DEBUG    	= 20,				//接收CAN数据写入Bram打印
	uart_DEBUG 			= 21,				//Uart收发数据打印
	uart_TO_TRDP_DEBUG 	= 22,				//Uart与TRDP传输数据打印
	peri_File_DEBUG 	= 23,				//外设文件数据调试打印
	FileSave_DEBUG 		= 24,				//外设文件存储调试打印
	socket_Debug 		= 25				//调试CSR_Drive与A9的Socket通讯	
}DEBUG_TYPE_ENUM;

typedef enum _COMMU_MCU
{

	TMS570_MCU = 1,
	MAX10_MCU= 2,

}COMMU_MCU_ENUM;

typedef enum _BRAM_RETN_STATUS
{

	RETURN_ELSE_ERROR = -3,
	RETURN_CRC_ERROR = -2,
	RETURN_ERROR = -1,
	RETURN_OK= 0,
 	RETURN_BUSY= 1,

}BRAM_RETN_ENUM;

typedef enum _CMD_STATUS
{

	CMD_ERROR = 1,
	CMD_OK = 2,
	CMD_BUSY = 3,
	CMD_CRC_ERROR = 4,
	CMD_SUCCESS = 5,

}CMD_STATUS_ENUM;

typedef struct 
{
	uint16_t Total_Ver_U16;
	uint16_t Linux_Ver_U16;
	uint16_t SySCfgFile_Ver_U16;
	uint16_t SftCfgFile_Ver_U16;
	uint16_t RrdCfgFile_Ver_U16;
	uint16_t SysPrmFile_Ver_U16;
	uint16_t EADS_RunVer_U16;
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
	FILE *EventLifeFile_fd;
	FILE *OprtNumFile_fd;
	FILE *OprtRealFile_fd;
	FILE *FltRealFile_fd;
	FILE *CalibFile_fd;
	FILE *CANFile_fd;
	FILE *UARTFile_fd;
}FILE_FD;

typedef struct
{
	uint32_t EventSaveNum_U32;  /*the save number of event data */
	uint32_t OprtRealSaveNum_U32;/*the save number of event LR data */
	uint32_t FltRealSaveNum_U32; //the save number of 75 Chans
}FILE_SAVE_INFO;


typedef struct
{
    pthread_t ReadDataThread;
	pthread_t RealWaveThread;
	pthread_t ModbusThread;
	pthread_t FileSaveThread;
	pthread_t TRDPThread;
	pthread_t LedThread;
	pthread_t DirTarThread;
	pthread_t RTUThread;
	pthread_t CANThread;
	pthread_t UartThread;
	pthread_t TMS570_Bram_Thread;	
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
    uint8_t BLVDSErr:1; 
    uint8_t VolWarmErr:1;
	uint8_t CurrWarmErr:1;
	uint8_t TemptureErr:1;
	uint8_t PowErr:1; 
    uint8_t VechFuctErr:1;
    uint8_t TRDPErr:1; 
	uint8_t EADSErr:1;  /*file creat err*/
}EADS_ERROR_INFO;


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

typedef struct 
{
	BYTE_BIT LG_ZongPeiDrive; /*zong pei  Drive circuit*/
	BYTE_BIT LG_DoorContrl;/*1/2/4/7/8 che,Door control*/
	BYTE_BIT LG_MotorFan; /*2/4/5/7 che Motor Fan and ConverColFan*/
	BYTE_BIT LG_TranformFan;/*3/6che LG_Tranform Cool Fan*/
	BYTE_BIT LG_Pan;/*3/6che*/
	BYTE_BIT LG_VCB;/*3/6che*/
	BYTE_BIT LG_Reser[2];
}CHAN_LG_INFO;


typedef struct 
{
	BYTE_BIT VolWarnFlag_8CH[VOL_STATUE_NUM]; /*the flag for trdp */
	BYTE_BIT VolChanStat_8CH[VOL_STATUE_NUM]; /*2/4/5/7 che Motor Fan and ConverColFan*/
	BYTE_BIT CurrWarnFlag_8CH[CURR_STATUE_NUM]; /*the flag for trdp */
}CHAN_DIGITAL_INFO;


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

typedef struct
{
	uint8_t WakeUpTime;
	uint8_t volumeGain;
    uint8_t Life_voice_U8;	
	BYTE_BIT UartDate_U8[5];//TRDP协议与RS485协议存在转换关系，所以定义BYTE_BIT类型	   
}UART_VOICE_TPYE;

typedef struct
{	   
	uint8_t Life_fingerprint_U8;//指纹模块生命信号	
	uint8_t CanDate_U8[4];//指纹模块数据	
}CAN_FingerPrint_TPYE;

typedef enum __devic__type
{
	CAN_Device = 1,
	Uart_Device = 2,
}peripheralDevice;//枚举外设设备类型

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

/**********************************************************************
*
*Multi-Include-Prevent End Section
*
*********************************************************************/
#endif 
