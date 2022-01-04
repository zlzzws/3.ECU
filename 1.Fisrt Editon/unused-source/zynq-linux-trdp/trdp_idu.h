/******************************************************************************

 *File Name    : trdp_idu.h

 *Copyright    : Zhuzhou CSR Times Electric Co.,Ltd. All Rights Reserved.

 *Create Date  : 2016/5/11

 *Description  : 

 *REV1.0.0     : lisy   2016/5/11  File Create 


******************************************************************************/

/******************************************************************************
 *    Multi-Include-Prevent Start Section
 ******************************************************************************/
#ifndef TRDP_IDU_H
#define TRDP_IDU_H

/******************************************************************************
 *    Debug switch Section
 ******************************************************************************/


/******************************************************************************
 *    Include File Section
 ******************************************************************************/
#include "BaseDefine.h"
#include "BcdTrans.h"
#include "Timer.h"
#include "WriteLog.h"
#include "xml.h"
/******************************************************************************
 *    Global Macro Define Section
 ******************************************************************************/
#define TRDP_REC_DATA_SIZE  300  //EADS:1400  ADU to LCU:300
#define TRDP_SEND_DATA_SIZE 300  //EADS:400   ADU to LCU:300
//EADS	
#define EADS_VECH1_COM_ID 3510 	/*1/8/9/17che */
#define EADS_VECH2_COM_ID 3520 	/*2/7/10/15che */
#define EADS_VECH3_COM_ID 3530 	/*3/6/11/14che */
#define EADS_VECH4_COM_ID 3540 	/*4/5/12/13che */
#define EADS_VECH16_COM_ID 3550 /*16che */
#define EADS_ZP_COM_ID 3511  	/*1/8che zongpei 1/17che zongpei */
#define CCU_COM_ID 1200 		/*CCU comID*/
//ADU 插件
#define LCU_COM_ID 1130		/*LCU comID*/
#define ADU_COM_ID 1131 		/*ADU comID*/

#define CCU_LIFE_JUDGE_NUM 100 	/*30ms once mean 3s*/
#define CCU_POW_JUDGE_NUM 30 	/*mean 0.9s*/
#define CCU_TIME_JUDGE_NUM 30 	/*mean  0.9s*/

#define TRDP_CURR_CHAN_NUM 14 	/*CURR_CHAN_NUM is 11*/
#define PULSE_NUM  60 			/*50ms, 3s*/
/******************************************************************************
 *    Global Structure Define Section
 ******************************************************************************/
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
    DRIVE_INFO Drive_Info; 
    HANDLE_INFO1 Handle_info1;
	HANDLE_INFO2 Handle_info2;
	DOOR_FAN Door_TransformFan;
	CONVERT_FAN  ConvertFan;
	PARK_BUTTON  ParkButtn;
	RESET_BUTTON  ResetButtn;
	BATTER_BUTTON  BatterButtn;
}CCU_EADS_INFO;

typedef struct 
{
	uint8_t LifeH_U8;
	uint8_t LifeL_U8;
    CCU_ST CCUST_U8;
    VECH_ST VechlST_U8;
    uint8_t Year_U8;
    uint8_t Month_U8;
    uint8_t Day_U8;
    uint8_t Hour_U8;
    uint8_t Minute_U8;
    uint8_t Second_U8;
    uint8_t BianzuH_U8;
    uint8_t BianzuL_U8;
    uint8_t Resrv1[6];
    uint16_t VechlSpeed_U16;
    uint8_t VechlMode_U8;
    uint8_t VechOutTem_U8;
    uint8_t Resrv2[10]; //32Byte
    uint8_t Resrv3[1096]; // 31+1096= 1127
    CCU_EADS_INFO VECH1_EADS_Info;//byte 1128
    CCU_EADS_INFO VECH2_EADS_Info;
    CCU_EADS_INFO VECH3_EADS_Info;
    CCU_EADS_INFO VECH4_EADS_Info;
    uint8_t Resrv4[170]; //byte 1160
    CCU_EADS_INFO VECH16_EADS_Info; //byte 1330
	uint8_t Resrv5[62]; //byte 1138
}CCU_TRDP_DATA;
//length of 1400Byte


typedef struct 
{
	uint16_t Life_U16; //system is litter endpoint ,origna 0x00 06,U8 is  0x06 0x00
	uint8_t VersionH_U8;//BCD version,large endpoint
	uint8_t VersionL_U8;
    uint8_t VolChan_U8[VOL_CHAN_NUM];//40
	uint8_t Resevr1_U8[40];
	uint16_t CurrChan_U16[TRDP_CURR_CHAN_NUM];//14
	uint8_t Resevr2_U8[20];
	BYTE_BIT OvrVolStat_8CH[VOL_STATUE_NUM];			
	uint8_t Resevr3_U8[5];
	BYTE_BIT OvrCurrStat_8CH[CURR_STATUE_NUM];
	uint8_t Resevr4_U8[2];
	uint16_t ChOpertNum_U16[VOL_CHAN_NUM];
	uint8_t  ChOpertTime_U8[20];//BCD,include 40 channel,need change
	FAULT_INFO DevFaultInfo_ST;
	BYTE_BIT  LGfaultInfoBit_ST[8]; //Byte 254
    uint8_t Resrv5_U8[146];
}EADS_TRDP_DATA;
//length of 100Byte
typedef struct 
{
	uint8_t LifeH_U8;
	uint8_t LifeL_U8; //system is litter endpoint ,orignal 0x00 06,U8 is  0x06 0x00	
    uint8_t Resevr1_U8;
	uint8_t WakeUpTime;//设置语音模块唤醒时间
	uint8_t volumeGain;//设置音量增益
	uint8_t Resevr2_U8[1];
	uint8_t CanDate_U8[3];//发送给指纹模块数据，CAN协议
	uint8_t UartDate_U8[2];//发送给语音模块数据，Uart协议
    uint8_t Resrv5_U8[9];
    uint8_t LCU_TIME[6];//LCU同步时间
    uint8_t Resrv6_U8[274];
}LCU_TRDP_DATA;//LCU发送给ADU数据 300字节
typedef struct 
{
	uint16_t Life_ADU_U16; //system is litter endpoint ,orignal 0x00 06,U8 is  0x06 0x00	
    uint8_t Life_fingerprint_U8;//指纹模块生命信号
	uint8_t Life_voice_U8;//语音模块生命信号
	uint8_t VersionH_U8;//BCD version,large endpoint
	uint8_t VersionL_U8;//ADU插件软件版本	
	uint8_t Resevr1_U8;	
	uint8_t UartDate_U8[5];//发送给语音模块数据，Uart协议
	uint8_t CanDate_U8[3];//发送给指纹模块数据，CAN协议
	BYTE_BIT ADU_Fault;//显示ADU插件故障
	BYTE_BIT Voice_Fingerprintf_Fault;//显示指纹、语音故障
    uint8_t Resrv5_U8[282];
}ADU_TRDP_DATA;//ADU发送给LCU数据 字节

/******************************************************************************
 *    Global Variable Declare Section
 ******************************************************************************/

/******************************************************************************
 *    Global Prototype Declare Section
 ******************************************************************************/
/******************************************************************************
 
 *Name        : trdp_ledRunRegister
 *Function    : if the module has LED, should register a function
 *para        : 
 *return      : 0 if OK,!=0 if error  
 *Created by Lisy 2016/5/11

 ******************************************************************************/
extern void trdp_ledRegister(void (*inetLed)(unsigned char value, unsigned char operation));

/******************************************************************************
 
 *Name        : trdp_setCfgFile
 *Function    : set the configuration file path for TRDP
 *para        : 
 *return      : 0 if OK,!=0 if error  
 *Created by Lisy 2016/5/11

 ******************************************************************************/
extern void trdp_setCfgFile(char *pFilePath);

/******************************************************************************
 
 *Name        : trdp_diagRegister
 *Function    : fault record for TRDP, should register a function
 *para        : 
 *return      : 
 *Created by Lisy 2016/7/20

 ******************************************************************************/
extern void trdp_diagRegister(void(*faultRecord)(int faultCode, unsigned short int faultLevel, 
		char *logBuf, int logLength));

/******************************************************************************
 
 *Name        : trdp_ecnIp1FunRegister
 *Function    : register a function for get ECN IP address
 *para        : 
 *return      : 
 *Created by Lisy 2016/7/20

 ******************************************************************************/
extern void trdp_ecnIp1FunRegister(unsigned int (*ecnIpAddr1Get)(void));

/******************************************************************************
 
 *Name        : trdp_ecnIp2FunRegister
 *Function    : register a function for get ECN IP address
 *para        : 
 *return      : 
 *Created by Lisy 2016/7/20

 ******************************************************************************/
extern void trdp_ecnIp2FunRegister(unsigned int (*ecnIpAddr2Get)(void));

/******************************************************************************
 
 *Name        : trdp_etbIpAddrGet
 *Function    : register a function for get ETB IP address
 *para        : 
 *return      : 
 *Created by Lisy 2016/7/20

 ******************************************************************************/
extern void trdp_etbIpFunRegister(unsigned int (*etbIpAddrGet)(void));

/******************************************************************************
 
 *Name        : PDpublish
 *Function    : publish a comid port for start send(the second parameter SchedGrp can set 0)
 *para        : 
 *return      : 0 if OK,!=0 if error  
 *Created by Lisy 2016/5/11

 ******************************************************************************/
extern int PDpublish(unsigned int ComId, unsigned int SchedGrp);

/****************************************************************************
 
 *Name        : PDunpublish
 *Function    : unpublish a comid port for stop send
 *para        : 
 *return      : 0 if OK,!=0 if error  
 *Created by Lisy 2016/5/11

 ****************************************************************************/
extern int PDunpublish(unsigned int ComId);

/******************************************************************************
 
 *Name        : PDsubscribe
 *Function    : subsrcibe a comid port for start receive, (the second parameter SchedGrp can set 0)
 *para        : 
 *return      : 0 if OK,!=0 if error  
 *Created by Lisy 2016/5/11

 ******************************************************************************/
extern int PDsubscribe(unsigned int ComId, unsigned int SchedGrp);

/******************************************************************************
 
 *Name        : PDunsubscribe
 *Function    : unsubscribe a comid port for stop receive
 *para        : 
 *return      : 0 if OK,!=0 if error  
 *Created by Lisy 2016/5/11

 ******************************************************************************/
extern int PDunsubscribe(unsigned int ComId);

/******************************************************************************
 
 *Name        : PDSetData
 *Function    : change the sent buffer for the comId port 
 *para        : 
 *return      : 0 if OK,!=0 if error  
 *Created by Lisy 2016/5/11
 *feng            2021/6/3   fisrt frame is 2,then is 0,whetner eth is ok
 ******************************************************************************/
extern int PDSetData(unsigned int ComId, unsigned char* Data,unsigned int Size);

/******************************************************************************
 
 *Name        : PDGetData
 *Function    : get the data from receive buffer 
 *para        : 
 *return      : 0:time out;1:get data OK;2:subscribe OK;otherwise:ERROR 
 *Created by Lisy 2016/5/11

 ******************************************************************************/
extern int PDGetData(unsigned int ComId, unsigned char* Data, unsigned int Size);


/******************************************************************************
 
 *Name        : trdpEnd
 *Function    : end the TRDP task
 *para        : 
 *return      : 0 if OK,!=0 if error  
 *Created by Lisy 2016/5/11

 ******************************************************************************/
extern void trdpEnd(void);


/******************************************************************************
 
 *Name        : trdp_stop
 *Function    : stop TRDP, no parameter and no return value 
 *para        : 
 *return      : 
 *Created by Lisy 2016/5/11

 ******************************************************************************/
extern void trdp_stop(void);


/******************************************************************************
 
 *Name        : trdpMain
 *Function    : main entry for TRDP start
 *para        : 
 *return      : 0 if OK,!=0 if error  
 *Created by Lisy 2016/5/11

 ******************************************************************************/
extern int trdpMain(void);


/******************************************************************************
 
 *Name        : trdp_appStart
 *Function    : trdp start, used for Application or modules
 *para        : 
 *return      : 0 if OK,!=0 if error  
 *Created by Lisy 2016/5/17

 ******************************************************************************/
extern int trdp_start(void);

extern int8_t OperNumFileSave(FILE_FD * File_p, CHAN_STATUS_INFO * ChanStaInfo_p,RECORD_XML * RrdXml_p,TRAIN_INFO * inTranInfo_p);

void 	pdSend(uint32_t comId,uint8_t *PdData,uint32_t size);
int8_t	pdRecv(uint32_t comId, uint8_t *RecData,uint32_t size);
int8_t 	pdRecDataPro(uint8_t PdData[],CHAN_STATUS_INFO * ChanInfop,CHAN_LG_INFO * ChanLgInfop,VECH_EADS_INFO * VechEADSInfop);
void 	pdSendDataSet(uint8_t *data,const CHAN_DATA * ChanDatap,const CHAN_STATUS_INFO * ChanInfop,const EADS_ERROR_INFO  EADSErrInfoST,
	               const CHAN_LG_INFO LgInfoST,const CHAN_DIGITAL_INFO ChanDigitalInfo_ST);
//void 	TrdpSendDataSet(uint8_t *data,CAN_FingerPrint_TPYE *CanData,UART_VOICE_TPYE *UartData, BYTE_BIT ADUErrInfo, BYTE_BIT Voice_Finger_ErrInfo);
//int8_t 	TrdpRecvDataPro(uint8_t PdData[],CAN_FingerPrint_TPYE *Can_RecvDate,UART_VOICE_TPYE *Uart_RecvDate);
/******************************************************************************
 *    Multi-Include-Prevent End Section
 ******************************************************************************/  
#endif

