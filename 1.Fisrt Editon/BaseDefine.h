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
/**********************************************************************
*
*Global Macro Define Section
*
*********************************************************************/

/*EADS Versionasd History*/
#define EADS_VERSION 100 
#define EADS_VERSION_PTU 100
#define TOTAL_VERSION 100
#define LINUX_VERSION 440
#define BOOT_VERSION 100
#define DTB_VERSION 100
#define RAMDISK_VERSION 100
#define FPGA_VERSION 100
#define BRAM_BLOCK_LENGTH 256
#define BRAM_BLOCK_NUM 128
#define ECU_VERSION_PTU 100
/*the channel number of once judgea */
//570 channel
#define OUTIO570_CHAN 1
#define COMMAND570_CHAN 3
#define RD_570FEDERR_NUM 10
#define UPSURE_BLOCKNUM 0xDDDD
#define UPSTAR_BLOCKNUM 0xEEEE
#define UPSUCC_BLOCKNUM 0xFFFF
#define CHAN_BUFFER_NUM 5
#define REAL_BUFFER_NUM 500   
/*Board Date LengthID*/
#define ADU_BRD_RD_LENGTH_U32  29 
#define CTU_BRD_RD_LENGTH_U32  29
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
#define VOL_CHAN_NUM 40  
#define CURR_CHAN_NUM 14 
#define ALL_CHAN_NUM 80  /*for File save, !!!! note / not include the oprte time */
#define peri_WriteSize 16

#define  CODE_CREAT  2
#define  CODE_EXIST  1
#define  CODE_OK  0
#define  CODE_ERR -1
#define  CODE_WARN  -2

#define VOL_STATUE_NUM  5
#define CURR_STATUE_NUM  2
#define FPGA_TIME_BYTE  20

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
    uint8_t BusErr:1;
    uint8_t VolChErr:1;
    uint8_t CurrChErr:1;
    uint8_t TemptureErr:1;
    uint8_t PowerErr:1;
    uint8_t Bit6:1;
    uint8_t Bit7:1;
    uint8_t DeviceErr:1;
}FAULT_INFO;


typedef struct
{
    uint8_t BatOFF:1; //3s pulse
    uint8_t WarnRSet:1;
    uint8_t ATOMmode:1;
    uint8_t Bit4:1;
    uint8_t Bit5:1;
    uint8_t Bit6:1;
    uint8_t Bit7:1;
    uint8_t Bit8:1;

}CCU_ST;

typedef struct
{
    uint8_t TimeValid:1; //1 mean time valid
    uint8_t BianzuValid:1;
    uint8_t Bit3:1;
    uint8_t Bit4:1;
    uint8_t Bit5:1;
    uint8_t Bit6:1;
    uint8_t Bit7:1;
    uint8_t Bit8:1;
}VECH_ST;

typedef struct
{
    uint8_t ReConect:2; //
    uint8_t MainContrl:2;
    uint8_t OverPhase:1;
    uint8_t DriveEnable1_T903A:1;
    uint8_t DriveEnable2_T903B:1;
    uint8_t VCBClose:1;
}DRIVE_INFO;
typedef struct
{
    uint8_t CarContrl_K1:1;
    uint8_t CarContrl_K2:1;
    uint8_t CarContrl_C:1;
    uint8_t CarContrl_K3:1;
    uint8_t CarContrl_K4:1;
    uint8_t CarContrl_N:1;
    uint8_t CarContrl_B1:1;
    uint8_t CarContrl_B2:1;
}HANDLE_INFO1;

typedef struct
{
    uint8_t CarContrl_B3:1;
    uint8_t CarContrl_B4:1;
    uint8_t CarContrl_B5:1;
    uint8_t CarContrl_B6:1;
    uint8_t CarContrl_B7:1;
    uint8_t CarContrl_EB:1;
    uint8_t QianPan:1;
    uint8_t HouPan:1;
}HANDLE_INFO2;
typedef struct
{
    uint8_t OpenRightDoor_T233:1;
    uint8_t OpenLefttDoor_T234:1;
    uint8_t CloseRightDoor_T235:1;
    uint8_t CloseLeftDoor_T236:1;
    uint8_t TranformFan1_HighSped:1;  /*just for 3/6che */
    uint8_t TranformFan1_LowSped:1;  /*just for 3/6che */
    uint8_t TranformFan2_HighSped:1; /*just for 3/6che */
    uint8_t TranformFan2_LowSped:1;  /*just for 3/6che */
}DOOR_FAN;

typedef struct
{
    uint8_t MotorColFan1_HighSped:1;  /*2/7che ,or 4/5che*/
    uint8_t MotorColFan1_LowSped:1;   /*2/7che ,or 4/5che*/
    uint8_t MotorColFan2_HighSped:1; /*2/7che ,or 4/5che*/
    uint8_t MotorColFan2_LowSped:1; /*2/7che ,or 4/5che*/
    uint8_t ConverColFan3_HighSped:1;  /*2/7che ,or 4/5che*/
    uint8_t ConverColFan3_LowSped:1; /*2/7che ,or 4/5che*/
    uint8_t res:2;

}CONVERT_FAN;

typedef struct
{
    uint8_t PanUpKnob_T221A:1;
    uint8_t ParkBrakPresButtn_T229A:1;
    uint8_t CleanBrakeButtn_T226A:1;
    uint8_t KeepBrakeButtn_T228A:1;
    uint8_t ParkBrakeFreedButtn_T230A:1;
    uint8_t FreedRightDoorButtn_T231:1;
    uint8_t FreedLeftDoorButtn_T232:1;
    uint8_t ManuVCBButtn_T260A:1;
}PARK_BUTTON;

typedef struct
{
    uint8_t ProtGndContrlValid_T272:1;
    uint8_t EmergDriveMode_T321A:1;
    uint8_t ATPPow_T291:1;
    uint8_t RestButtn_T240A:1;
    uint8_t EmergRestButtn_T247A:1;
    uint8_t Res1:1;
    uint8_t ScaleParkButtn_T255A:1;
    uint8_t PanDownKnob_T222A:1;
}RESET_BUTTON;

typedef struct
{
    uint8_t NoneAlert_T258A:1;
    uint8_t BPRSUnRescue_T276:1;
    uint8_t BPRSRescue_T275:1;
    uint8_t ForcZeroSpeed_T280:1;
    uint8_t VechWireContrl_T293:1;
    uint8_t BatterButtn_T256:1;
    uint8_t EmergPowOffButtn_T223:1;
    uint8_t EmergBrakButtn_T252A:1;
}BATTER_BUTTON;

typedef struct
{
    DRIVE_INFO Drive_Info;
    HANDLE_INFO1 Handle_info1;
    HANDLE_INFO2 Handle_info2;
    DOOR_FAN Door_TransformFan;
    CONVERT_FAN  ConvertFan2Che;
    CONVERT_FAN  ConvertFan4Che;
    PARK_BUTTON  ParkButtn;
    RESET_BUTTON  ResetButtn;
    BATTER_BUTTON  BatterButtn;
}VECH_EADS_INFO;
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
	SEM_DEBUG 			= 14,
	DOG_DEBUG 			= 15,   			
	RTU_DEBUG 			= 18,				
	TMS570_BRAM_RD_DEBUG= 19,				
	TMS570_BRAM_WR_DEBUG= 20,				
    CAN_RD_DEBUG        = 21,
    CAN_WR_DEBUG        = 22,
	MVB_WR_DEBUG		= 23,
	MVB_RD_DEBUG		= 24,
	FileSave_DEBUG 		= 25,				
	socket_Debug 		= 26				
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
	pthread_t CAN0Thread;
    pthread_t CAN1Thread;
	pthread_t MVBThread;		
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

typedef enum devic_type
{
	CAN_Device = 1,
	Uart_Device = 2,
}peripheralDevice;//枚举外设设备类型


typedef struct tms570_bram_data
{
    uint32_t buffer[60];//数据区不包含CRC32的4个字节
	uint8_t length;
}TMS570_BRAM_DATA;

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
