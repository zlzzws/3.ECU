/******************************************************************************

 *File Name    : trdp_idu.c

 *Copyright    : Zhuzhou CSR Times Electric Co.,Ltd. All Rights Reserved.

 *Create Date  : 2016/5/11

 *Description  : app routines for TRDP

 *REV1.0.0     : lisy   2016/5/11  File Create 


******************************************************************************/

/******************************************************************************
 *    Debug switch Section
 ******************************************************************************/


/******************************************************************************
 *    Include File Section
 ******************************************************************************/
#include "trdp_idu.h"
/******************************************************************************
 *    Global Variable Define Section
 ******************************************************************************/
extern DEBUG_TYPE_ENUM g_DebugType_EU;
extern TRAIN_INFO g_TrainInfo_ST;
extern FILE_FD  g_FileFd_ST;/*the save file fd*/
extern RECORD_XML g_Rec_XML_ST;
extern CHAN_STATUS_INFO  g_ChanStatuInfo_ST;
extern EADS_ERROR_INFO g_EADSErrInfo_ST;

/******************************************************************************
 *    Local Macro Define Section
 ******************************************************************************/

/******************************************************************************
 *    Local Structure Define Section
 ******************************************************************************/

/******************************************************************************
 *    Local Prototype Declare Section
 ******************************************************************************/

/******************************************************************************
 *    Static Variable Define Section
 ******************************************************************************/

/******************************************************************************
 *    Function Define Section
 ******************************************************************************/

/******************************************************************************
 
 *Name        : trdp_led
 *Function    : led function, if has no Led twinkle requirement, just implement a function with nothing,
	            DTECS products need to implement
 *para        : 
 *return      : 
 *Created by Lisy 2016/7/20

 ******************************************************************************/
void trdp_led(unsigned char value, unsigned char operation)
{
	return;
}

/******************************************************************************
 
 *Name        : trdp_faultRecord
 *Function    : diagnosis fault record function, if has no this function, just implement 
	            a function with nothing, DTECS products need to implement
 *para        : 
 *return      : 
 *Created by Lisy 2016/7/20

 ******************************************************************************/
void trdp_faultRecord(int faultCode, unsigned short int faultLevel, char *logBuf, int logLength)
{
	return;
}
/******************************************************************************
 
 *Name        : trdp_ecnIpAddr1Get
 *Function    : ECN IP address get function, if return 0, TRDP will scan the interface
	            and set the ECN IP address which fit the standard of IEC61375-3-4 with 10.0.X.Y 
 *para        : 
 *return      : 
 *Created by Lisy 2016/7/20
 ******************************************************************************/
unsigned int trdp_ecnIpAddr1Get(void)
{
	return 0;
}

/******************************************************************************
 
 *Name        : trdp_ecnIpAddr1Get
 *Function    : ECN IP address get function, if return 0, TRDP will scan the interface
	            and set the ECN IP address which fit the standard of IEC61375-3-4 with 10.0.X.Y 
 *para        : 
 *return      : 
 *Created by Lisy 2016/7/20

 ******************************************************************************/
unsigned int trdp_ecnIpAddr2Get(void)
{
	return 0;
}

/******************************************************************************
 
 *Name        : trdp_etbIpAddrGet
 *Function    : ETB IP address get function, if return 0, TRDP will scan the interface
	            and set the ETB IP address which fit the standard of IEC61375-2-5 with 10.128.X.Y
	            only ETBN need to implement, the other devices just return 0 will be OK
 *para        : 
 *return      : 
 *Created by Lisy 2016/7/20

 ******************************************************************************/
unsigned int trdp_etbIpAddrGet(void)
{
	return 0;
}



/******************************************************************************
 
 *Name        : trdp_start
 *Function    : start the TRDP main program
 *para        : 
 *return      : 
 *Created by Lisy 2016/7/20
 *Modified by Lisy 2016/8/1 add diagnosis and IP address function registering steps

 ******************************************************************************/
int trdp_start(void)
{		
	/* Step 1: set the configuration file path */
	trdp_setCfgFile("/tffs0/TRDPCFG.xml");
	//trdp_setPrintEnable(1); //print debug info
	
	/* Step 2: register the led function, if has no Led, just implement a function with nothing,
	 * only DTECS products need to implement */
	trdp_ledRegister(trdp_led);
	
	/* Step 3: register the diagnosis fault record function, if has no this function, just implement 
	 * a function with nothing, only DTECS products need to implement */
	trdp_diagRegister(trdp_faultRecord);
	
	/* Step 4: register the ECN IP address get function, if return 0, TRDP will scan the interface
	 * and set the ECN IP address which fit the standard of IEC61375-3-4 with 10.0.X.Y */
	trdp_ecnIp1FunRegister(trdp_ecnIpAddr1Get);
	trdp_ecnIp2FunRegister(trdp_ecnIpAddr2Get);
		
	/* Step 5: register the ETB IP address get function, if return 0, TRDP will scan the interface
	 * and set the ETB IP address which fit the standard of IEC61375-2-5 with 10.128.X.Y
	 * only ETBN need to implement */
	trdp_etbIpFunRegister(trdp_etbIpAddrGet);
		
	/* Finally: start TRDP main routine, this step must be the final step */
	if(0 == trdpMain())   /* 0 means trdp start OK */
	{
		/* start OK, do what you want */
		return 0;
	}
	else                  /* !=0 means trdp start Failed */
	{
		/* start failed, do what you want(like give a error print) */
		return 0xFFFFFFFF;
	}
	
	/* if any questions, mail to:lisy5@teg.cn please */ 
}


/******************************************************************************
 
 *Name        : trdp_stop
 *Function    : stop the TRDP main program
 *para        : 
 *return      : 
 *Created by Lisy 2016/8/1

 ******************************************************************************/
void trdp_stop(void)
{
	trdpEnd();
}

/**********************************************************************
*Name			:   pdSendDataSet(uint Dec, unsigned char *Bcd, int length)
*Function       :   set trdp pdSendData
*Para     		:   uint8_t *data,
 					const CHAN_DATA * ChanDatap, 
 					const CHAN_STATUS_INFO * ChanInfop,
 					const EADS_ERROR_INFO  EADSErrInfoST,
	                const CHAN_LG_INFO LgInfoST,
	                const CHAN_DIGITAL_INFO ChanDigitalInfo_ST
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
#if 0

void pdSendDataSet(uint8_t *data,const CHAN_DATA * ChanDatap,const CHAN_STATUS_INFO * ChanInfop,const EADS_ERROR_INFO  EADSErrInfoST,
	               const CHAN_LG_INFO LgInfoST,const CHAN_DIGITAL_INFO ChanDigitalInfo_ST)
{
	uint32_t i;
	uint16_t Temp = 0;
	static uint8_t s_VersionBCD[2] = {0};
	uint8_t OptimeChNum = 0;
	EADS_TRDP_DATA * Trdp_Send_data_p;
	Trdp_Send_data_p = (EADS_TRDP_DATA *)data;
	static uint16_t s_life = 0;
	//litter endpoint  change to big endpoint
	Trdp_Send_data_p -> Life_U16 = Litte2BigEnd(s_life); //Byte 0
    if(0 == s_life)
    {
        DectoBCD(EADS_VERSION,s_VersionBCD,2);
    }
	
    Trdp_Send_data_p -> VersionH_U8 =  s_VersionBCD[1];//EADS_VERSION 100
    Trdp_Send_data_p -> VersionL_U8 =  s_VersionBCD[0];
	for(i = 0;i < VOL_CHAN_NUM;i++)
	{
    	/*to clear the negative */
        if(ChanDatap -> VolChan_ST[i].CH_Voltage_I16 < 0)
        {
            Trdp_Send_data_p -> VolChan_U8[i] = 0;
        }
        else
        {
            Trdp_Send_data_p -> VolChan_U8[i] = (uint8_t)(ChanDatap -> VolChan_ST[i].CH_Voltage_I16 / 10);// the send vol value is  uint V         
        }

	}
 	for(i = 0;i < CURR_CHAN_NUM;i++) //just 14 channel
    {
		if(ChanDatap -> CurrChan_ST[i].CH_Current_I16 < 0)
        {
            Trdp_Send_data_p -> CurrChan_U16[i] = 0;
        }
        else
        {
            Trdp_Send_data_p -> CurrChan_U16[i] = Litte2BigEnd(ChanDatap -> CurrChan_ST[i].CH_Current_I16);
        }

	}
    memcpy(Trdp_Send_data_p -> OvrVolStat_8CH,ChanDigitalInfo_ST.VolWarnFlag_8CH,VOL_STATUE_NUM);
    memcpy(Trdp_Send_data_p -> OvrCurrStat_8CH,ChanDigitalInfo_ST.CurrWarnFlag_8CH,CURR_STATUE_NUM);

    for(i = 0; i< VOL_CHAN_NUM;i++)
    {
    	Trdp_Send_data_p -> ChOpertNum_U16[i] = (uint16_t)ChanInfop -> Channel_OperaNum_U32[i];
    }
    OptimeChNum = VOL_CHAN_NUM >> 1;
    Trdp_Send_data_p -> ChOpertTime_U8[0] = ChanInfop -> OprtUpTime_U16[CONT_CHAN_T883];
    Trdp_Send_data_p -> ChOpertTime_U8[1] = ChanInfop -> OprtUpTime_U16[CONT_CHAN_106K];
    Trdp_Send_data_p -> ChOpertTime_U8[2] = ChanInfop -> OprtUpTime_U16[CONT_CHAN_123A];
    Trdp_Send_data_p -> ChOpertTime_U8[3] = ChanInfop -> OprtUpTime_U16[CONT_CHAN_103B];
    Trdp_Send_data_p -> ChOpertTime_U8[4] = ChanInfop -> OprtUpTime_U16[CONT_CHAN_103D];

    Trdp_Send_data_p -> ChOpertTime_U8[5] = ChanInfop -> OprtDownTime_U16[CONT_CHAN_T883];
    Trdp_Send_data_p -> ChOpertTime_U8[6] = ChanInfop -> OprtDownTime_U16[CONT_CHAN_106K];
    Trdp_Send_data_p -> ChOpertTime_U8[7] = ChanInfop -> OprtDownTime_U16[CONT_CHAN_123A];
    Trdp_Send_data_p -> ChOpertTime_U8[8] = ChanInfop -> OprtDownTime_U16[CONT_CHAN_103B];
    Trdp_Send_data_p -> ChOpertTime_U8[9] = ChanInfop -> OprtDownTime_U16[CONT_CHAN_103D];

    memcpy(&Trdp_Send_data_p -> DevFaultInfo_ST,&EADSErrInfoST,1);

    if(TRDP_DEBUG == g_DebugType_EU)
	{
		
		printf("Trdp_Life_U16 %x \n",Trdp_Send_data_p -> Life_U16);
        printf("Trdp_Version %x%x \n",Trdp_Send_data_p -> VersionH_U8,Trdp_Send_data_p -> VersionL_U8);
		for(i = 0;i < 40; i++)
        {
    	    printf("Voltage_U8 %d value %d \n",i,Trdp_Send_data_p -> VolChan_U8[i]);
        }
        for(i = 0;i < 14; i++)
        {
    	    printf("Current_U16 %d value %d \n",i,Trdp_Send_data_p -> CurrChan_U16[i]);
        }
        for(i = 0;i < 5; i++)
        {
    	    printf("ChOvrVol_U8 %d value %x \n",i,Trdp_Send_data_p ->  OvrVolStat_8CH[i]);
        }
        for(i = 0;i < 2; i++)
        {
    	    printf("ChOvrCurr_U8 %d value %x \n",i,Trdp_Send_data_p ->  OvrCurrStat_8CH[i]);
        }
        for(i = 0;i < 40; i++)
        {
    	    printf("ChOpertNum_U16 %d value %d \n",i,Trdp_Send_data_p -> ChOpertNum_U16[i]);
        }
        for(i = 0;i < 20; i++)
        {
    	    printf("ChOpertTime_U8 %d value %x \n",i,Trdp_Send_data_p -> ChOpertTime_U8[i]);
        }
        for(i = 0;i < 8; i++)
        {
    	    printf("LGFaultInfo_U8 %d value %x \n",i,Trdp_Send_data_p -> LGfaultInfoBit_ST[i]);
        }
 	}
    //memcpy is litter endpoint ,origna 0x0006,after 0x0600
	//memcpy(data,&Rtu_xinte_p-> Current_A_U16,RtuByteLength); 
 	s_life++; /*500ms ++*/
}
#endif
/**********************************************************************
*Name			:   TrdpSendDataSetSendDataSet(uint Dec, unsigned char *Bcd, int length)
*Function       :   set trdp pdSendData 设置Trdp发送数据
*Para     		:   uint8_t *data,（outbuf）
 					uint8_t *Candata （inbuf）
 					uint8_t *Uartdata （inbuf）
 					const EADS_ERROR_INFO  EADSErrInfoST,
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
#if 0
void TrdpSendDataSet(uint8_t *data,CAN_FingerPrint_TPYE *CanData,UART_VOICE_TPYE *UartData, BYTE_BIT ADUErrInfo, BYTE_BIT Voice_Finger_ErrInfo)
{
	uint32_t i;
	uint16_t Temp = 0;
	static uint8_t s_VersionBCD[2] = {0};
	uint8_t OptimeChNum = 0;
	ADU_TRDP_DATA *Trdp_Send_data_p;
	Trdp_Send_data_p = (ADU_TRDP_DATA *)data;
	static uint16_t s_life = 0;
	//litter endpoint change to big endpoint 大小端转换
	Trdp_Send_data_p ->  Life_ADU_U16 = Litte2BigEnd(s_life);
    Trdp_Send_data_p -> Life_voice_U8 =UartData ->Life_voice_U8;
    Trdp_Send_data_p -> Life_fingerprint_U8 =CanData ->Life_fingerprint_U8;
    if(0 == s_life)
    {
        DectoBCD(EADS_VERSION,s_VersionBCD,2);//十进制转化为二进制
    }
    //EADS_VERSION 100	
    Trdp_Send_data_p -> VersionH_U8 = s_VersionBCD[1];
    Trdp_Send_data_p -> VersionL_U8 = s_VersionBCD[0];

    memcpy(Trdp_Send_data_p -> UartDate_U8,UartData -> UartDate_U8,5);//语音模块指令数据
    memcpy(Trdp_Send_data_p -> CanDate_U8,CanData -> CanDate_U8,4);//指纹模块指令数据
    Trdp_Send_data_p -> ADU_Fault = ADUErrInfo;
    Trdp_Send_data_p -> Voice_Fingerprintf_Fault = Voice_Finger_ErrInfo;

    if(TRDP_DEBUG == g_DebugType_EU)
	{
		printf("A9_Send_TrdpDate_TO_LCU:\n");
		printf("ADU_Life: %x\n",Trdp_Send_data_p -> Life_ADU_U16);
        printf("voice_Device_Life: %x\n",Trdp_Send_data_p -> Life_voice_U8);
        printf("Fingerprint_Life: %x\n",Trdp_Send_data_p -> Life_fingerprint_U8);
        printf("ADU_Version: %x %x\n",Trdp_Send_data_p -> VersionH_U8,Trdp_Send_data_p -> VersionL_U8);
        for(i = 0;i < 5; i++)
        {
    	    printf("Trdp_Send_UartDate_U8[%d]:%x\n",i,Trdp_Send_data_p -> UartDate_U8[i]);
        }
        for(i = 0;i < 4; i++)
        {
    	    printf("Trdp_Send_CanDate_U8[%d]:%x\n",i,Trdp_Send_data_p -> CanDate_U8[i]);
        }
        printf("Trdp_Send_data_ADU_Fault%x\n",Trdp_Send_data_p -> ADU_Fault);
        printf("Trdp_Send_data_Voice_Fingerprintf_Fault%x\n",Trdp_Send_data_p -> Voice_Fingerprintf_Fault);
 	}     
 	s_life++; /*100ms ++*/
}
#endif
/**********************************************************************
*Name			:   void pdSend(uint32_t comId,uint8_t *PdData,uint32_t size)
*Function       :   set trdp pdSend function
*Para     		:   uint32_t comId, 
					uint8_t *PdData,
					uint32_t size
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
void pdSend(uint32_t comId,uint8_t *PdData,uint32_t size)
{

	uint32_t i;
    uint8_t ret;
    if(TRDP_DEBUG == g_DebugType_EU)
	{
			printf("TRDP SendByte\n");
			for (i = 0; i < 20; i++)
			{				
				printf("[%02x]",PdData[i]);
				if ((i + 1) % 20 == 0)
				{
					printf("\n");
				}
			}
			printf("\n");
 	}
	ret = PDSetData(comId, PdData,size);
    if(ret != 0) 
    {
        printf("PDSetDate failed!\n");
    }
}
/**********************************************************************
*Name			:   pdRecLife(CCU_TRDP_DATA * Trdp_data_p)
*Function       :   judge the trdp life
*Para     		:   CCU_TRDP_DATA * Trdp_data_p : ccu data

*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/

int8_t  pdRecLife(LCU_TRDP_DATA * Trdp_data_p)
{
	static uint16_t s_Life_Last = 0;
	static uint16_t s_LifeLostNum = 0;
	uint16_t CCU_life = 0;
	char loginfo[LOG_INFO_LENG] = {0};
	CCU_life = BigU8ToLitteU16(Trdp_data_p -> LifeH_U8 ,Trdp_data_p-> LifeL_U8); 

    if(CCU_life == s_Life_Last)
    {
    	s_LifeLostNum++;
    	if((s_LifeLostNum >= CCU_LIFE_JUDGE_NUM) && (0 == g_EADSErrInfo_ST.TRDPErr))
    	{
    		
            g_EADSErrInfo_ST.TRDPErr = 1;
            printf("TRDP Rec Life Lost\n");
    		snprintf(loginfo, sizeof(loginfo)-1,"TRDP Rec Life Lost");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
    		s_LifeLostNum = 0;
            return CODE_ERR;
    	}
    }
    else if(s_LifeLostNum > 0)
    {
    	s_LifeLostNum = 0;
    }
    else if(1 == g_EADSErrInfo_ST.TRDPErr)
    {
        g_EADSErrInfo_ST.TRDPErr = 0;
        printf("TRDP Rec Life Recover\n");
        snprintf(loginfo, sizeof(loginfo)-1,"TRDP Rec Life Recover");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    s_Life_Last = CCU_life;
    return CODE_OK;

}

/**********************************************************************
*Name			:   void pdTimePowOff(CCU_TRDP_DATA * Trdp_data_p)
*Function       :   judge the time set and pow off info of ccu data,
*Para     		:   CCU_TRDP_DATA * Trdp_data_p, 

*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
int8_t  pdTimePowOff(CCU_TRDP_DATA * Trdp_data_p)
{
	static uint16_t s_PowNum = 0;
    static uint8_t s_SetTimeNum = 0;
    static uint8_t s_SetTimeflag = 0;
    static uint8_t s_PowOffflag = 0;
    SYSTEM_TIME SysTime_ST = {0};
    uint16_t Year_U16 = 0;
    char LogInfo[LOG_INFO_LENG] = {0};
    uint8_t fd = 0;
	int8_t err = 0;
    int8_t TimeErrFlag = 0;
	//static uint16_t s_TimeUpdateNUm = 0;
	char loginfo[LOG_INFO_LENG] = {0};
	Trdp_data_p -> CCUST_U8.BatOFF;
	Trdp_data_p -> VechlST_U8.TimeValid;
    if(1 == Trdp_data_p -> CCUST_U8.BatOFF)
    {
    	s_PowNum++;
    	if((s_PowNum >= CCU_POW_JUDGE_NUM)&&(0 == s_PowOffflag))/*30 loop*/
    	{  		
            printf("TRDP Power Down happen ,sync file\n");
            if(NULL != g_FileFd_ST.FltRealFile_fd)
            {

                fflush(g_FileFd_ST.FltRealFile_fd);
                fd = fileno(g_FileFd_ST.FltRealFile_fd);
                fsync(fd);
                printf("finish FltRealFile sync \n");
            }
            if(NULL != g_FileFd_ST.OprtRealFile_fd)
            {
                fflush(g_FileFd_ST.OprtRealFile_fd);
                fd = fileno(g_FileFd_ST.OprtRealFile_fd);
                fsync(fd);
                printf("finish OprtRealFile sync \n");
            }
            if(NULL != g_FileFd_ST.EventFile_fd)
            {
                fflush(g_FileFd_ST.EventFile_fd);
                fd = fileno(g_FileFd_ST.EventFile_fd);
                fsync(fd);
                printf("finish evetfile sync\n");
            }
            OperNumFileSave(&g_FileFd_ST,&g_ChanStatuInfo_ST,&g_Rec_XML_ST,&g_TrainInfo_ST);
            snprintf(LogInfo, sizeof(LogInfo)-1, "TRDP Power Down,sync File");
            WRITELOGFILE(LOG_INFO_1,LogInfo);
            LogFileSync();
            printf("end TRDP Power Down\n");
    		s_PowNum = 0;
            s_PowOffflag = 1;
    	}
    }
    else if(s_PowNum > 0)
    {
    
    	s_PowNum = 0;
    }
    /*set trdp time*/
    if((1 == Trdp_data_p -> VechlST_U8.TimeValid) && (0 == s_SetTimeflag))
    {
        s_SetTimeNum++;
        if(s_SetTimeNum  >= CCU_TIME_JUDGE_NUM)/*30 loop*/
        {

            printf("TRDP Set Time\n");           
            if((Trdp_data_p -> Year_U8 > 99) || (0== Trdp_data_p -> Month_U8) || (Trdp_data_p -> Month_U8 > 12)||(0 == Trdp_data_p -> Day_U8 ) || (Trdp_data_p -> Day_U8 >31))
            {
                printf("Time Error ,Year %u Month %u Day %u \n",Trdp_data_p -> Year_U8,Trdp_data_p -> Month_U8,Trdp_data_p -> Day_U8);
                TimeErrFlag = 1;
            }
            else if((Trdp_data_p -> Hour_U8 > 23) || (Trdp_data_p -> Minute_U8 > 59) ||(Trdp_data_p -> Second_U8 > 59))
            {
                printf("Time Error ,Hour %u Minute %u Second %u \n",Trdp_data_p -> Hour_U8,Trdp_data_p -> Minute_U8,Trdp_data_p -> Second_U8);
                TimeErrFlag = 1;
            }
            else
            {
                Year_U16 = Trdp_data_p -> Year_U8 + 2000;
                SysTime_ST.Year_U8[0] =  (uint8_t)((Year_U16 & 0xFF00) >> 8); 
                SysTime_ST.Year_U8[1] = (uint8_t)(Year_U16 & 0xFF);
                SysTime_ST.Month_U8 = Trdp_data_p -> Month_U8;
                SysTime_ST.Day_U8 = Trdp_data_p -> Day_U8;
                SysTime_ST.Hour_U8 = Trdp_data_p -> Hour_U8;
                SysTime_ST.Minute_U8 = Trdp_data_p -> Minute_U8;
                SysTime_ST.Second_U8 = Trdp_data_p -> Second_U8;
                err = SetSystemTime(&SysTime_ST);
                err = SetRTCTime(&SysTime_ST);
                if(CODE_OK == err)
                {
                    s_SetTimeflag = 1;  
                }

            }               
            if(1 == TimeErrFlag)
            {
                snprintf(loginfo, sizeof(loginfo)-1,"TCMS time error %d %d %d %d:%d:%d",Trdp_data_p -> Year_U8,Trdp_data_p -> Month_U8,Trdp_data_p -> Day_U8,Trdp_data_p -> Hour_U8,Trdp_data_p -> Minute_U8,Trdp_data_p -> Second_U8);
                WRITELOGFILE(LOG_ERROR_1,loginfo);
            }
            //system("hwclock -s");
            s_SetTimeNum = 0;
        }      
    }
    else if(s_SetTimeNum > 0)
    {
    
        s_SetTimeNum = 0;
    }
    return err;
}
/**********************************************************************
*Name			:   ZongPeiLGPro 
*Function       :   judge the ZongPei Logic
*Para     		:   CCU_TRDP_DATA * trdp_data_p,
					CHAN_STATUS_INFO * ChanInfop,
					CHAN_LG_INFO * ChanLgInfop)
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
int8_t ZongPeiLGPro(CCU_TRDP_DATA * trdp_data_p,CHAN_STATUS_INFO * ChanInfop,CHAN_LG_INFO * ChanLgInfop)
{
    uint8_t temjudge = 0;
    static uint8_t s_ZongBit0Num = 0;
    static uint8_t s_ZongBit1Num = 0;
    static uint8_t s_ZongBit2Num = 0;
    static uint8_t s_ZongBit3Num = 0;
   	static uint8_t s_ZongBit4Num = 0;
   	static uint8_t s_ZongBit1RaiseFlag = 0;
   	static uint8_t s_ZongBit1RaiseFlagTime = 0;
    static uint8_t s_CarContrl_N_last = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    if(1 == ChanInfop -> VolChanStat_U8[36])
    {
    	/*both is 1,is OK*/
        /*can try use last == now  to filt.*/
    	if((1 == ChanInfop -> VolChanStat_U8[0])&&(1 == ChanInfop -> VolChanStat_U8[6]))
    	{
    		if(1 == ChanLgInfop -> LG_ZongPeiDrive.Bit0)
    		{
    			
                ChanLgInfop -> LG_ZongPeiDrive.Bit0 = 0;
    			printf("LG_ZongPeiDrive.Bit0 err clear\n");
		    	snprintf(loginfo, sizeof(loginfo)-1, "LG_ZongPeiDrive.Bit0 err clear");
	            WRITELOGFILE(LOG_WARN_1,loginfo);
    		}
            s_ZongBit0Num = 0;
    		
    	}
    	else
    	{ 		
    		s_ZongBit0Num++;
    		//printf("s_ZongBit0Num %d \n",s_ZongBit0Num );
    		if(s_ZongBit0Num >= DIGT_JUDGE_NUM)
	    	{
	    		    		
	    		if(0 == ChanLgInfop -> LG_ZongPeiDrive.Bit0)
	    		{
	    			ChanLgInfop -> LG_ZongPeiDrive.Bit0 = 1;
		    		printf("LG_ZongPeiDrive.Bit0 err happen\n");
		    		snprintf(loginfo, sizeof(loginfo)-1, "LG_ZongPeiDrive.Bit0 err happen");
	                WRITELOGFILE(LOG_WARN_1,loginfo);
	    		}
                s_ZongBit0Num = 0;
	    	}
    	}
    	
    }
    if((1 == trdp_data_p -> VECH1_EADS_Info.Handle_info1.CarContrl_N) && (0 == s_CarContrl_N_last))
    {
    	s_ZongBit1RaiseFlag = 1;
    }
    else if((0 == trdp_data_p -> VECH1_EADS_Info.Handle_info1.CarContrl_N) && (1 == s_CarContrl_N_last))
    {
        s_ZongBit1RaiseFlag = 0;
    }  

    if((1 == ChanInfop -> VolChanStat_U8[0]) && (1 == s_ZongBit1RaiseFlag))
    {
    	/*both is 1,is OK*/
    	if(1 == ChanInfop -> VolChanStat_U8[1])
    	{
    		if(1 == ChanLgInfop -> LG_ZongPeiDrive.Bit1)
    		{    
    			
                ChanLgInfop -> LG_ZongPeiDrive.Bit1 = 0;
    			printf("LG_ZongPeiDrive.Bit1 err clear\n");
	    		snprintf(loginfo, sizeof(loginfo)-1, "LG_ZongPeiDrive.Bit1 err clear");
                WRITELOGFILE(LOG_WARN_1,loginfo); 
    		}
            s_ZongBit1Num = 0;

    	}
    	else
    	{
    		s_ZongBit1Num++;
    		if(s_ZongBit1Num >= DIGT_JUDGE_NUM)
	    	{
	    		if(0 == ChanLgInfop -> LG_ZongPeiDrive.Bit1)
	    		{	
	    			ChanLgInfop -> LG_ZongPeiDrive.Bit1 = 1;
		    		printf("LG_ZongPeiDrive.Bit1 err happen\n");
		    		snprintf(loginfo, sizeof(loginfo)-1, "LG_ZongPeiDrive.Bit1 err happen");
	                WRITELOGFILE(LOG_WARN_1,loginfo); 
	    		}
	    		s_ZongBit1Num = 0;
	    	}  		
    	}
    	//s_ZongBit1RaiseFlagTime++;
    }

    // if(s_ZongBit1RaiseFlagTime > DIGT_JUDGE_NUM) /**/
    // {
    // 	s_ZongBit1RaiseFlag = 0;
    //     s_ZongBit1RaiseFlagTime = 0;
    // }
    s_CarContrl_N_last = trdp_data_p -> VECH1_EADS_Info.Handle_info1.CarContrl_N;/*is pulse and,recongize the raise edge*/

    if(1 == ChanInfop -> VolChanStat_U8[1])
    {
    	/*both is 1,is OK*/
    	if(1 == ChanInfop -> VolChanStat_U8[2])
    	{  		
    		if(1== ChanLgInfop -> LG_ZongPeiDrive.Bit2)
    		{
				ChanLgInfop -> LG_ZongPeiDrive.Bit2 = 0;
				printf("LG_ZongPeiDrive.Bit2 err clear\n");
		    	snprintf(loginfo, sizeof(loginfo)-1, "LG_ZongPeiDrive.Bit2 err clear");
	            WRITELOGFILE(LOG_WARN_1,loginfo);	
    		}
            s_ZongBit2Num = 0;
    	}
    	else
    	{
    		s_ZongBit2Num++;
    		if(s_ZongBit2Num >= DIGT_JUDGE_NUM)
	    	{
	    		
	    		if(0 == ChanLgInfop -> LG_ZongPeiDrive.Bit2)
	    		{
	    			ChanLgInfop -> LG_ZongPeiDrive.Bit2 = 1;	
		    		printf("LG_ZongPeiDrive.Bit2 err happen\n");
		    		snprintf(loginfo, sizeof(loginfo)-1, "LG_ZongPeiDrive.Bit2 err happen");
	                WRITELOGFILE(LOG_WARN_1,loginfo);	
	    		}
	    		s_ZongBit2Num  = 0; 
	    	}    		
    	}    	
    }

    if(trdp_data_p -> VECH1_EADS_Info.Handle_info1.CarContrl_K1 || trdp_data_p -> VECH1_EADS_Info.Handle_info1.CarContrl_K2 
    	|| trdp_data_p -> VECH1_EADS_Info.Handle_info1.CarContrl_C || trdp_data_p -> VECH1_EADS_Info.Handle_info1.CarContrl_K3
    	|| trdp_data_p -> VECH1_EADS_Info.Handle_info1.CarContrl_K4)
    {
        temjudge = 1;	
    }
    if((1 == ChanInfop -> VolChanStat_U8[2]) && (1 == temjudge))
    {
    	/*both is 1,is OK*/
    	if(1 == ChanInfop -> VolChanStat_U8[3])
    	{
    		if(1 == ChanLgInfop -> LG_ZongPeiDrive.Bit3)
    		{
        		ChanLgInfop -> LG_ZongPeiDrive.Bit3 = 0;
	    		printf("LG_ZongPeiDrive.Bit3 err clear\n");
	    		snprintf(loginfo, sizeof(loginfo)-1, "LG_ZongPeiDrive.Bit3 err clear");
                WRITELOGFILE(LOG_WARN_1,loginfo);			
    		}
            s_ZongBit3Num =0;
    	}
    	else
    	{
    		s_ZongBit3Num++;
    		if(s_ZongBit3Num >= DIGT_JUDGE_NUM)
	    	{	    		
	    		if(0 == ChanLgInfop -> LG_ZongPeiDrive.Bit3)
	    		{
	    			ChanLgInfop -> LG_ZongPeiDrive.Bit3 = 1;		    		
		    		printf("LG_ZongPeiDrive.Bit3 err happen\n");
		    		snprintf(loginfo, sizeof(loginfo)-1, "LG_ZongPeiDrive.Bit3 err happen");
	                WRITELOGFILE(LOG_WARN_1,loginfo);  	
	    		}
				s_ZongBit3Num = 0;
	    	}		
    	}
    }

    if(1 == ChanInfop -> VolChanStat_U8[3])
    {
    	/*both is 1,is OK*/
    	if(1 == ChanInfop -> VolChanStat_U8[4])
    	{
    		if(1 == ChanLgInfop -> LG_ZongPeiDrive.Bit4)
    		{
    			ChanLgInfop -> LG_ZongPeiDrive.Bit4 = 0;
				printf("LG_ZongPeiDrive.Bit4 err clear\n");
		    	snprintf(loginfo, sizeof(loginfo)-1, "LG_ZongPeiDrive.Bit4 err clear");
	            WRITELOGFILE(LOG_WARN_1,loginfo);  
    		}
            s_ZongBit4Num = 0;
    	}
    	else
    	{
    		s_ZongBit4Num++;
    		if(s_ZongBit4Num >= DIGT_JUDGE_NUM)
	    	{	    		
	    		if(0 == ChanLgInfop -> LG_ZongPeiDrive.Bit4 )
	    		{
	 	    		ChanLgInfop -> LG_ZongPeiDrive.Bit4 = 1;
		    		printf("LG_ZongPeiDrive.Bit4 err happen\n");
		    		snprintf(loginfo, sizeof(loginfo)-1, "LG_ZongPeiDrive.Bit4 err happen");
	                WRITELOGFILE(LOG_WARN_1,loginfo);  
	    			
	    		}
	    		s_ZongBit4Num = 0;

	    	}
    		
    	}
    	
    }

}
/**********************************************************************
*Name			:   DoorLGPro 
*Function       :   judge the Door Logic
*Para     		:   CCU_TRDP_DATA * trdp_data_p,
					CHAN_STATUS_INFO * ChanInfop,
					CHAN_LG_INFO * ChanLgInfop)
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
#if 0 
int8_t DoorLGPro(CCU_TRDP_DATA * trdp_data_p,CHAN_STATUS_INFO * ChanInfop,CHAN_LG_INFO * ChanLgInfop)
{
    uint8_t temjudge = 0;
    static uint8_t s_DoorBit0Num = 0;
    static uint8_t s_DoorBit1Num = 0;
    static uint8_t s_DoorBit2Num = 0;
    static uint8_t s_DoorBit3Num = 0;
    static uint8_t s_OpenRightDoor_last = 0;
    static uint8_t s_OpenLefttDoor_last = 0;
    static uint8_t s_CloseRightDoor_last = 0;
    static uint8_t s_CloseLeftDoor_last = 0;
    static uint8_t s_OpenRightDoorFlag = 0;
    static uint8_t s_OpenLefttDoorFlag = 0;
    static uint8_t s_CloseRightDoorFlag = 0;
    static uint8_t s_CloseLeftDoorFlag = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    if((1 == trdp_data_p -> VECH1_EADS_Info.Door_TransformFan.OpenRightDoor_T233) &&(0 == s_OpenRightDoor_last))
    { 	
       s_OpenRightDoorFlag = 1;
    }
    else if((0 == trdp_data_p -> VECH1_EADS_Info.Door_TransformFan.OpenRightDoor_T233) &&(1 == s_OpenRightDoor_last))
    {   
       s_OpenRightDoorFlag = 0;
    }
        
    if(1 == s_OpenRightDoorFlag) 
    {
        s_DoorBit0Num++;
    	/*both is 1,is OK*/
    	if(s_DoorBit0Num <= PULSE_NUM) /*3s*/
    	{
    		if(1 == ChanInfop -> VolChanStat_U8[16])
    		{
    		    if(1 == ChanLgInfop -> LG_DoorContrl.Bit0)
    		    {
    		    	ChanLgInfop -> LG_DoorContrl.Bit0 = 0;
    		    	printf("LG_DoorContrl.Bit0 err clear \n");
			    	snprintf(loginfo, sizeof(loginfo)-1, "LG_DoorContrl.Bit0 err clear");
		            WRITELOGFILE(LOG_WARN_1,loginfo); 

    		    }
                s_DoorBit0Num = 0;
    		}
    	}
    	else if(s_DoorBit0Num > PULSE_NUM)
    	{  		
    		if(0 == ChanLgInfop -> LG_DoorContrl.Bit0)
    		{
    			ChanLgInfop -> LG_DoorContrl.Bit0 = 1;
    			printf("LG_DoorContrl.Bit0 err happen \n");
		    	snprintf(loginfo, sizeof(loginfo)-1, "LG_DoorContrl.Bit0 err happen");
	            WRITELOGFILE(LOG_WARN_1,loginfo); 
    		}
    		s_DoorBit0Num = 0;		
    	}
    }
    s_OpenRightDoor_last = trdp_data_p -> VECH1_EADS_Info.Door_TransformFan.OpenRightDoor_T233;

    if((1 == trdp_data_p -> VECH1_EADS_Info.Door_TransformFan.OpenLefttDoor_T234) &&(0 == s_OpenLefttDoor_last))
    { 	
       s_OpenLefttDoorFlag = 1;
    }
    else if((0 == trdp_data_p -> VECH1_EADS_Info.Door_TransformFan.OpenLefttDoor_T234) &&(1 == s_OpenLefttDoor_last))
    {   
       s_OpenLefttDoorFlag = 0;
    }
        
    if(1 == s_OpenLefttDoorFlag) 
    {
        s_DoorBit1Num++;
    	/*both is 1,is OK*/
    	if(s_DoorBit1Num <= PULSE_NUM) /*3s*/
    	{
    		if(1 == ChanInfop -> VolChanStat_U8[6])
    		{
    		    if(1 == ChanLgInfop -> LG_DoorContrl.Bit1 )
    		    {

    		    	ChanLgInfop -> LG_DoorContrl.Bit1 = 0;
    		    	printf("LG_DoorContrl.Bit1 err clear\n");
			    	snprintf(loginfo, sizeof(loginfo)-1, "LG_DoorContrl.Bit1 err clear");
		            WRITELOGFILE(LOG_WARN_1,loginfo); 

    		    }
                s_DoorBit1Num = 0;  
    		}
    	}
    	else if(s_DoorBit1Num > PULSE_NUM)
    	{

    		if(0 == ChanLgInfop -> LG_DoorContrl.Bit1 )
    		{
	       		ChanLgInfop -> LG_DoorContrl.Bit1 = 1;
	    		printf("LG_DoorContrl.Bit1 err happen\n");
		    	snprintf(loginfo, sizeof(loginfo)-1, "LG_DoorContrl.Bit1 err happen");
	            WRITELOGFILE(LOG_WARN_1,loginfo); 
 			
    		}
            s_DoorBit1Num = 0;	
    		
    	}
    }
    s_OpenLefttDoor_last = trdp_data_p -> VECH1_EADS_Info.Door_TransformFan.OpenLefttDoor_T234;

    if((1 == trdp_data_p -> VECH1_EADS_Info.Door_TransformFan.CloseRightDoor_T235) &&(0 == s_CloseRightDoor_last))
    { 	
       s_CloseRightDoorFlag = 1;
    }
    else if((0 == trdp_data_p -> VECH1_EADS_Info.Door_TransformFan.CloseRightDoor_T235) &&(1 == s_CloseRightDoor_last))
    {   
       s_CloseRightDoorFlag = 0;
    }
        
    if(1 == s_CloseRightDoorFlag) 
    {
        s_DoorBit2Num++;
    	/*both is 1,is OK*/
    	if(s_DoorBit2Num <= PULSE_NUM) /*3s*/
    	{
    		if(1 == ChanInfop -> VolChanStat_U8[14])
    		{
    		    if(1 ==  ChanLgInfop -> LG_DoorContrl.Bit2)
    		    {
    		    	ChanLgInfop -> LG_DoorContrl.Bit2 = 0;
    		    	printf("LG_DoorContrl.Bit2 err clear\n");
			    	snprintf(loginfo, sizeof(loginfo)-1, "LG_DoorContrl.Bit2 err clear");
		            WRITELOGFILE(LOG_WARN_1,loginfo);
  
    		    }
                s_DoorBit2Num = 0;  
    		  	
    		}
    	}
    	else if(s_DoorBit2Num > PULSE_NUM)
    	{

    		if(0 == ChanLgInfop -> LG_DoorContrl.Bit2 )
    		{
    			ChanLgInfop -> LG_DoorContrl.Bit2 = 1;
    			printf("LG_DoorContrl.Bit2 err happen\n");
		    	snprintf(loginfo, sizeof(loginfo)-1, "LG_DoorContrl.Bit2 err happen");
	            WRITELOGFILE(LOG_WARN_1,loginfo);  
    		}			
    		s_DoorBit2Num = 0;			
    	}
    }
    s_CloseRightDoor_last = trdp_data_p -> VECH1_EADS_Info.Door_TransformFan.CloseRightDoor_T235;

    if((1 == trdp_data_p -> VECH1_EADS_Info.Door_TransformFan.CloseLeftDoor_T236) &&(0 == s_CloseLeftDoor_last))
    { 	
       s_CloseLeftDoorFlag = 1;
    }
    else if((0 == trdp_data_p -> VECH1_EADS_Info.Door_TransformFan.CloseLeftDoor_T236) &&(1 == s_CloseLeftDoor_last))
    {   
       s_CloseLeftDoorFlag = 0;
    }
        
    if(1 == s_CloseLeftDoorFlag) 
    {
        s_DoorBit3Num++;
    	/*both is 1,is OK*/
    	if(s_DoorBit3Num <= PULSE_NUM) /*3s*/
    	{
    		if(1 == ChanInfop -> VolChanStat_U8[14])
    		{
    		    if(1 == ChanLgInfop -> LG_DoorContrl.Bit3)
    		    {
    		    	ChanLgInfop -> LG_DoorContrl.Bit3 = 0;
    		    	printf("LG_DoorContrl.Bit3 err clear\n");
			    	snprintf(loginfo, sizeof(loginfo)-1, "LG_DoorContrl.Bit3 err clear");
		            WRITELOGFILE(LOG_WARN_1,loginfo); 

    		    }
                s_DoorBit3Num = 0;

    		}
    	}
    	else if(s_DoorBit3Num > PULSE_NUM)
    	{

    		if(0 == ChanLgInfop -> LG_DoorContrl.Bit3)
    		{
    			ChanLgInfop -> LG_DoorContrl.Bit3 = 1;
    			printf("LG_DoorContrl.Bit3 err happen\n");
		    	snprintf(loginfo, sizeof(loginfo)-1, "LG_DoorContrl.Bit3 err happen");
	            WRITELOGFILE(LOG_WARN_1,loginfo); 
    		}
    		s_DoorBit3Num = 0;
  		
    	}
    }
    s_CloseLeftDoor_last = trdp_data_p -> VECH1_EADS_Info.Door_TransformFan.CloseLeftDoor_T236;
}
#endif
/**********************************************************************
*Name			:   MotorFanLGPro 
*Function       :   judge the Motor sFan Logic
*Para     		:   CCU_TRDP_DATA * trdp_data_p,
					CHAN_STATUS_INFO * ChanInfop,
					CHAN_LG_INFO * ChanLgInfop)
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
#if 0
int8_t MotorFanLGPro(CCU_TRDP_DATA * trdp_data_p,CHAN_STATUS_INFO * ChanInfop,CHAN_LG_INFO * ChanLgInfop)
{
    static uint8_t s_MotorBit0Num = 0;
    static uint8_t s_MotorBit1Num = 0;
    static uint8_t s_MotorBit2Num = 0;
    static uint8_t s_MotorBit3Num = 0;
   	static uint8_t s_MotorBit4Num = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    if((1 == ChanInfop -> VolChanStat_U8[20]) && (1 == trdp_data_p -> VECH2_EADS_Info.ConvertFan.MotorColFan1_HighSped))
    {
    	/*both is 1,is OK*/
    	if(1 == ChanInfop -> VolChanStat_U8[9])
    	{

            if(1 == ChanLgInfop -> LG_MotorFan.Bit0)
            {
                ChanLgInfop -> LG_MotorFan.Bit0 = 0;
                printf("LG_MotorFan.Bit0 err clear \n");
                snprintf(loginfo, sizeof(loginfo)-1, "LG_MotorFan.Bit0 err clear");
                WRITELOGFILE(LOG_WARN_1,loginfo);   
            }
            s_MotorBit0Num = 0;
 
    	}
    	else
    	{
    		s_MotorBit0Num++;
    		if(s_MotorBit0Num >= DIGT_JUDGE_NUM)
	    	{
	    		
                if(0 == ChanLgInfop -> LG_MotorFan.Bit0)
                {
                    ChanLgInfop -> LG_MotorFan.Bit0 = 1;
                    printf("LG_MotorFan.Bit0 err happen\n");
                    snprintf(loginfo, sizeof(loginfo)-1, "LG_MotorFan.Bit0 err happen");
                    WRITELOGFILE(LOG_WARN_1,loginfo);                      
                }
                s_MotorBit0Num = 0;	
	    	}   		
    	}
    }

    if((1 == ChanInfop -> VolChanStat_U8[38]) && (1 == trdp_data_p -> VECH2_EADS_Info.ConvertFan.MotorColFan1_LowSped))
    {
    	/*both is 1,is OK*/
    	if(1 == ChanInfop -> VolChanStat_U8[33])
    	{   		
            if(1 == ChanLgInfop -> LG_MotorFan.Bit1)
            {
                ChanLgInfop -> LG_MotorFan.Bit1 = 0;
                printf("LG_MotorFan.Bit1 err clear\n");
                snprintf(loginfo, sizeof(loginfo)-1, "LG_MotorFan.Bit1 err clear");
                WRITELOGFILE(LOG_WARN_1,loginfo);
            }
            s_MotorBit1Num = 0;
    	}
    	else
    	{
    		s_MotorBit1Num++;
    		if(s_MotorBit1Num >= DIGT_JUDGE_NUM)
	    	{
	    		if(0 == ChanLgInfop -> LG_MotorFan.Bit1)
                {
                    ChanLgInfop -> LG_MotorFan.Bit1 = 1;
                    printf("LG_MotorFan.Bit1 err happen\n");
                    snprintf(loginfo, sizeof(loginfo)-1, "LG_MotorFan.Bit1 err happen");
                    WRITELOGFILE(LOG_WARN_1,loginfo);
                }
                s_MotorBit1Num = 0;
	    	}
    		
    	}
    	
    }

    if((1 == ChanInfop -> VolChanStat_U8[37]) && (1 == trdp_data_p -> VECH2_EADS_Info.ConvertFan.MotorColFan2_HighSped))
    {
    	/*both is 1,is OK*/
    	if(1 == ChanInfop -> VolChanStat_U8[5])
    	{
    		if(1 == ChanLgInfop -> LG_MotorFan.Bit2)
            {
                ChanLgInfop -> LG_MotorFan.Bit2 = 0;
                printf("LG_MotorFan.Bit2 err clear\n");
                snprintf(loginfo, sizeof(loginfo)-1, "LG_MotorFan.Bit2 err clear");
                WRITELOGFILE(LOG_WARN_1,loginfo);
            }
            s_MotorBit2Num = 0;
    	}
    	else
    	{
    		s_MotorBit2Num++;
    		if(s_MotorBit2Num >= DIGT_JUDGE_NUM)
	    	{
	    		
	    		if(0 == ChanLgInfop -> LG_MotorFan.Bit2)
                {
                    ChanLgInfop -> LG_MotorFan.Bit2 = 1;
                    printf("LG_MotorFan.Bit2 err happen\n");
                    snprintf(loginfo, sizeof(loginfo)-1, "LG_MotorFan.Bit2 err happen");
                    WRITELOGFILE(LOG_WARN_1,loginfo);
                }
                s_MotorBit2Num = 0;
	    	}
	    		
    	}
    	
    }

    if((1 == ChanInfop -> VolChanStat_U8[36]) && (1 == trdp_data_p -> VECH2_EADS_Info.ConvertFan.MotorColFan2_LowSped))
    {
    	/*both is 1,is OK*/
    	if(1 == ChanInfop -> VolChanStat_U8[23])
    	{
    		if(1 == ChanLgInfop -> LG_MotorFan.Bit3)
            { 
                ChanLgInfop -> LG_MotorFan.Bit3 = 0;
                printf("LG_MotorFan.Bit3 err clear\n");
                snprintf(loginfo, sizeof(loginfo)-1, "LG_MotorFan.Bit3 err clear");
                WRITELOGFILE(LOG_WARN_1,loginfo);

            }
            s_MotorBit3Num = 0;
    	}
    	else
    	{
    		s_MotorBit3Num++;
    		if(s_MotorBit3Num >= DIGT_JUDGE_NUM)
	    	{
	    		if(0 == ChanLgInfop -> LG_MotorFan.Bit3)
                { 
                    ChanLgInfop -> LG_MotorFan.Bit3 = 1;
                    printf("LG_MotorFan.Bit3 err happen\n");
                    snprintf(loginfo, sizeof(loginfo)-1, "LG_MotorFan.Bit3 err happen");
                    WRITELOGFILE(LOG_WARN_1,loginfo);
                }
                s_MotorBit3Num = 0;               
	    	}    		
      	}
    	
    }

}
#endif
/**********************************************************************
*Name			:   TranformFanLGPro 
*Function       :   judge the TranformFan Logic
*Para     		:   CCU_TRDP_DATA * trdp_data_p,
					CHAN_STATUS_INFO * ChanInfop,
					CHAN_LG_INFO * ChanLgInfop)
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
#if 0
int8_t TranformFanLGPro(CCU_TRDP_DATA * trdp_data_p,CHAN_STATUS_INFO * ChanInfop,CHAN_LG_INFO * ChanLgInfop)
{
    
    static uint8_t s_TranformBit0Num = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    if((1 == ChanInfop -> VolChanStat_U8[34]) && (1 == trdp_data_p -> VECH3_EADS_Info.Door_TransformFan.TranformFan1_HighSped))
    {
    	/*both is 1,is OK*/
    	if(1 == ChanInfop -> VolChanStat_U8[9])
    	{
    		if(1 == ChanLgInfop -> LG_TranformFan.Bit0)
            {
                ChanLgInfop -> LG_TranformFan.Bit0 = 0;
                printf("LG_TranformFan.Bit0 err clear\n");
                snprintf(loginfo, sizeof(loginfo)-1, "LG_TranformFan.Bit0 err clear");
                WRITELOGFILE(LOG_WARN_1,loginfo);                   
            }
            s_TranformBit0Num = 0;
    	}
    	else
    	{
    		s_TranformBit0Num++;
    		if(s_TranformBit0Num >= DIGT_JUDGE_NUM)
	    	{
	    		
	    		if(0 == ChanLgInfop -> LG_TranformFan.Bit0)
                {
                    ChanLgInfop -> LG_TranformFan.Bit0 = 1;
                    printf("LG_TranformFan.Bit0 err happen\n");
                    snprintf(loginfo, sizeof(loginfo)-1, "LG_TranformFan.Bit0 err happen");
                    WRITELOGFILE(LOG_WARN_1,loginfo);                   
                }
                s_TranformBit0Num = 0;              
	    	}  		
    	}    	
    }
}
#endif
/**********************************************************************
*Name			:   PanLGPro 
*Function       :   judge the Pan Logic
*Para     		:   CCU_TRDP_DATA * trdp_data_p,
					CHAN_STATUS_INFO * ChanInfop,
					CHAN_LG_INFO * ChanLgInfop)
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
#if 0
int8_t PanLGPro(CCU_TRDP_DATA * trdp_data_p,CHAN_STATUS_INFO * ChanInfop,CHAN_LG_INFO * ChanLgInfop)
{
    
    static uint8_t s_PanBit0Num = 0;
    static uint8_t s_PanBit1Num = 0;
    static uint8_t s_PanBit2Num = 0;
    static uint8_t s_PanBit3Num = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    if(0 == ChanInfop -> VolChanStat_U8[3])
    {
    	if(((1 == trdp_data_p -> VECH3_EADS_Info.Drive_Info.MainContrl) && (1 == trdp_data_p -> VECH3_EADS_Info.Handle_info2.QianPan)) 
         || ((2 == trdp_data_p -> VECH3_EADS_Info.Drive_Info.MainContrl) && (1 == trdp_data_p -> VECH3_EADS_Info.Handle_info2.HouPan)))
    	{
	/*both is 1,is OK*/
	    	if(1 == ChanInfop -> VolChanStat_U8[2])
	    	{
                if(1 == ChanLgInfop -> LG_Pan.Bit0)
                {
                    ChanLgInfop -> LG_Pan.Bit0 = 0;
                    printf("LG_Pan.Bit0 err clear\n");
                    snprintf(loginfo, sizeof(loginfo)-1, "LG_Pan.Bit0 err clear");
                    WRITELOGFILE(LOG_WARN_1,loginfo);

                }
                s_PanBit0Num = 0;	    	
            }
	    	else
	    	{
	    		s_PanBit0Num++;
	    		if(s_PanBit0Num >= DIGT_JUDGE_NUM)
	    		{
	    			if(0 == ChanLgInfop -> LG_Pan.Bit0)
                    {
                        ChanLgInfop -> LG_Pan.Bit0 = 1;
                        printf("LG_Pan.Bit0 err happen\n");
                        snprintf(loginfo, sizeof(loginfo)-1, "LG_Pan.Bit0 err happen");
                        WRITELOGFILE(LOG_WARN_1,loginfo);
 
                    }
                    s_PanBit0Num = 0;     
	    		}
	    	}

	    	if(1 == ChanInfop -> VolChanStat_U8[25])
	    	{

    		    if(1 == ChanLgInfop -> LG_Pan.Bit1)
                {
                    ChanLgInfop -> LG_Pan.Bit1 = 0;
                    printf("LG_Pan.Bit1 err clear\n");
                    snprintf(loginfo, sizeof(loginfo)-1, "LG_Pan.Bit1 err clear");
                    WRITELOGFILE(LOG_WARN_1,loginfo);                        
                }
                s_PanBit1Num = 0;

                if(1 == ChanInfop -> VolChanStat_U8[4])
				{
                    if(1 == ChanLgInfop -> LG_Pan.Bit3)
                    {
                        ChanLgInfop -> LG_Pan.Bit3 = 0;
                        printf("LG_Pan.Bit3 err clear\n");
                        snprintf(loginfo, sizeof(loginfo)-1, "LG_Pan.Bit3 err clear");
                        WRITELOGFILE(LOG_WARN_1,loginfo);                           
                    }
                    s_PanBit3Num = 0;				
                }
				else
				{
					s_PanBit3Num++;
					if(s_PanBit3Num >= DIGT_JUDGE_NUM)
					{
						if(0 == ChanLgInfop -> LG_Pan.Bit3)
                        {
                            ChanLgInfop -> LG_Pan.Bit3 = 1;
                            printf("LG_Pan.Bit3 err happen\n");
                            snprintf(loginfo, sizeof(loginfo)-1, "LG_Pan.Bit3 err happen");
                            WRITELOGFILE(LOG_WARN_1,loginfo);                           
                        }
                        s_PanBit3Num = 0;
					}
					
				}
	    	}
	    	else
	    	{
	    		s_PanBit1Num++;
	    		if(s_PanBit1Num >= DIGT_JUDGE_NUM) /*if the error no clear,will alawy judge error*/
	    		{
	    			if(0 == ChanLgInfop -> LG_Pan.Bit1)
                    {
                        ChanLgInfop -> LG_Pan.Bit1 = 1;
                        printf("LG_Pan.Bit1 err happen\n");
                        snprintf(loginfo, sizeof(loginfo)-1, "LG_Pan.Bit1 err happen");
                        WRITELOGFILE(LOG_WARN_1,loginfo);                        
                    }
                    s_PanBit1Num = 0;

	    		}
	    	}
    	}
    }
    else if(1 == ChanInfop -> VolChanStat_U8[3])
	{
		if(((1 == trdp_data_p -> VECH3_EADS_Info.Drive_Info.MainContrl) && (1 == trdp_data_p -> VECH3_EADS_Info.Handle_info2.QianPan)) 
		 || ((2 == trdp_data_p -> VECH3_EADS_Info.Drive_Info.MainContrl) && (1 == trdp_data_p -> VECH3_EADS_Info.Handle_info2.HouPan)))
		{
		/*both is 1,is OK*/
			if(0 == ChanInfop -> VolChanStat_U8[25])
			{
				if(1 == ChanLgInfop -> LG_Pan.Bit2)
                {
                    ChanLgInfop -> LG_Pan.Bit2 = 0;
                    printf("LG_Pan.Bit2 err clear\n");
                    snprintf(loginfo, sizeof(loginfo)-1, "LG_Pan.Bit2 err clear");
                    WRITELOGFILE(LOG_WARN_1,loginfo);                        
                }
                s_PanBit2Num = 0;
			}
			else
			{
				
                s_PanBit2Num++;
	    		if(s_PanBit2Num >= DIGT_JUDGE_NUM) /*if the error no clear,will alawy judge error*/
	    		{
	    			if(0 == ChanLgInfop -> LG_Pan.Bit2)
                    {
                        ChanLgInfop -> LG_Pan.Bit2 = 1;
                        printf("LG_Pan.Bit2 err happen\n");
                        snprintf(loginfo, sizeof(loginfo)-1, "LG_Pan.Bit2 err happen");
                        WRITELOGFILE(LOG_WARN_1,loginfo);                        
                    }
                    s_PanBit2Num = 0;

	    		}
				
			}	
		}
    }
}
#endif
/**********************************************************************
*Name			:   VCBGPro 
*Function       :   judge the VCB Logic
*Para     		:   CCU_TRDP_DATA * trdp_data_p,
					CHAN_STATUS_INFO * ChanInfop,
					CHAN_LG_INFO * ChanLgInfop)
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
#if 0
int8_t VCBGPro(CCU_TRDP_DATA * trdp_data_p,CHAN_STATUS_INFO * ChanInfop,CHAN_LG_INFO * ChanLgInfop)
{
    
	static uint8_t s_VCBBit0Num = 0;
    static uint8_t s_VCBBit1Num = 0;
    static uint8_t s_VCBBit2Num = 0;
    static uint8_t s_VCBBit3Num = 0;
    char loginfo[LOG_INFO_LENG] = {0};
	if(1 == trdp_data_p -> VECH3_EADS_Info.Drive_Info.VCBClose)
	{
	/*both is 1,is OK*/
		if(1 == ChanInfop -> VolChanStat_U8[19])
		{
			s_VCBBit0Num = 0;
            if(1 == ChanLgInfop -> LG_Pan.Bit0)
            {
                ChanLgInfop -> LG_Pan.Bit0 = 0;
                printf("LG_VCB.Bit0 err clear\n");
                snprintf(loginfo, sizeof(loginfo)-1, "LG_VCB.Bit0 err clear");
                WRITELOGFILE(LOG_WARN_1,loginfo);                       
            }

			if(1 == ChanInfop -> VolChanStat_U8[12])
			{
				s_VCBBit1Num = 0;
                if(1 == ChanLgInfop -> LG_VCB.Bit1)
                {
                    ChanLgInfop -> LG_VCB.Bit1 = 0;
                    printf("LG_VCB.Bit1 err clear\n");
                    snprintf(loginfo, sizeof(loginfo)-1, "LG_VCB.Bit1 err clear");
                    WRITELOGFILE(LOG_WARN_1,loginfo);                        
                }

				if(1 == ChanInfop -> VolChanStat_U8[13])
				{
					if(1 == ChanLgInfop -> LG_VCB.Bit2)
                    {
                        ChanLgInfop -> LG_VCB.Bit2 = 0;
                        printf("LG_VCB.Bit2 err clear\n");
                        snprintf(loginfo, sizeof(loginfo)-1, "LG_VCB.Bit2 err clear");
                        WRITELOGFILE(LOG_WARN_1,loginfo);                            
                    }
                    s_VCBBit2Num = 0;

					if(1 == ChanInfop -> VolChanStat_U8[11])
					{
                        if(1 ==  ChanLgInfop -> LG_VCB.Bit3)
                        {
                            ChanLgInfop -> LG_VCB.Bit3 = 0; 
                            printf("LG_VCB.Bit3 err clear\n");
                            snprintf(loginfo, sizeof(loginfo)-1, "LG_VCB.Bit3 err clear");
                            WRITELOGFILE(LOG_WARN_1,loginfo);   
                        }
                        s_VCBBit3Num = 0; 
					}
					else
					{
						s_VCBBit3Num ++;
						if(s_VCBBit3Num >= DIGT_JUDGE_NUM)
						{
							
                            if(0 ==  ChanLgInfop -> LG_VCB.Bit3)
                            {
                                ChanLgInfop -> LG_VCB.Bit3 = 1; 
                                printf("LG_VCB.Bit3 err happen\n");
                                snprintf(loginfo, sizeof(loginfo)-1, "LG_VCB.Bit3 err happen");
                                WRITELOGFILE(LOG_WARN_1,loginfo);   
                            }
                            s_VCBBit3Num = 0; 
						}
					}
				}
				else
				{
					s_VCBBit2Num ++;
					if(s_VCBBit2Num >= DIGT_JUDGE_NUM)
					{
						if(0 == ChanLgInfop -> LG_VCB.Bit2)
                        {
                            ChanLgInfop -> LG_VCB.Bit2 = 1;
                            printf("LG_VCB.Bit2 err happen\n");
                            snprintf(loginfo, sizeof(loginfo)-1, "LG_VCB.Bit2 err happen");
                            WRITELOGFILE(LOG_WARN_1,loginfo);                            
                        }
                        s_VCBBit2Num = 0;
					}
					
				}
			}
			else
			{
				s_VCBBit1Num++;
				if(s_VCBBit1Num >= DIGT_JUDGE_NUM)
				{
					
                    if(0 == ChanLgInfop -> LG_VCB.Bit1)
                    {
                        ChanLgInfop -> LG_VCB.Bit1 = 1;
                        printf("LG_VCB.Bit1 err happen\n");
                        snprintf(loginfo, sizeof(loginfo)-1, "LG_VCB.Bit1 err happen");
                        WRITELOGFILE(LOG_WARN_1,loginfo);                        
                    }
                    s_VCBBit1Num = 0;				
				}
			}
		}
		else
		{
			s_VCBBit0Num ++;
			if(s_VCBBit0Num >= DIGT_JUDGE_NUM)
			{
				if(0 == ChanLgInfop -> LG_Pan.Bit0)
                {
                    ChanLgInfop -> LG_Pan.Bit0 = 1;
                    printf("LG_VCB.Bit0 err happen\n");
                    snprintf(loginfo, sizeof(loginfo)-1, "LG_VCB.Bit0 err happen");
                    WRITELOGFILE(LOG_WARN_1,loginfo);                       
                }
                s_VCBBit0Num = 0;
			}
		}
	}

}
#endif
/**********************************************************************
*Name			:   pdChLogPro
*Function       :   judge the all Chan Logic
*Para     		:   CCU_TRDP_DATA * trdp_data_p,
					CHAN_STATUS_INFO * ChanInfop,
					CHAN_LG_INFO * ChanLgInfop)
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
#if 0
int8_t pdChLogPro(CCU_TRDP_DATA * trdp_data_p,CHAN_STATUS_INFO * ChanInfop,CHAN_LG_INFO * ChanLgInfop)
{
    
    ZongPeiLGPro(trdp_data_p,ChanInfop,ChanLgInfop);
    DoorLGPro(trdp_data_p,ChanInfop,ChanLgInfop);
    MotorFanLGPro(trdp_data_p,ChanInfop,ChanLgInfop);
    PanLGPro(trdp_data_p,ChanInfop,ChanLgInfop);
    VCBGPro(trdp_data_p,ChanInfop,ChanLgInfop);        
}
#endif
/**********************************************************************
*Name			:   pdRecDataPro
*Function       :   Process the  CCU data,
 					1.include the time set and pow off
 					2.Channel logic Pro
 					3.buffer the EADS Channel info
*Para     		:   uint8_t PdData[],CHAN_STATUS_INFO * ChanInfop,CHAN_LG_INFO * ChanLgInfop,VECH_EADS_INFO * VechEADSInfop
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*
*********************************************************************/
#if 0
int8_t pdRecDataPro(uint8_t PdData[],CHAN_STATUS_INFO * ChanInfop,CHAN_LG_INFO * ChanLgInfop,VECH_EADS_INFO * VechEADSInfop)
{
	static uint16_t s_Life_Last = 0;
	static uint16_t s_LifeLostNum = 0;
	char loginfo[LOG_INFO_LENG] = {0};
	CCU_TRDP_DATA * CCU_trdp_data_p;
    CCU_trdp_data_p = (CCU_TRDP_DATA *)PdData;
    pdRecLife(CCU_trdp_data_p);
    pdTimePowOff(CCU_trdp_data_p);
    pdChLogPro(CCU_trdp_data_p,ChanInfop,ChanLgInfop);
    memcpy(VechEADSInfop,&CCU_trdp_data_p -> VECH3_EADS_Info,4);
    memcpy(&VechEADSInfop -> ConvertFan2Che,&CCU_trdp_data_p -> VECH2_EADS_Info.ConvertFan,1);
    memcpy(&VechEADSInfop -> ConvertFan4Che,&CCU_trdp_data_p -> VECH4_EADS_Info.ConvertFan,4);
    if(TRDP_DEBUG == g_DebugType_EU)
    {
        printf("VechEADSInfop -> Drive_Info %x\n ",VechEADSInfop -> Drive_Info);
        printf("VechEADSInfop -> Handle_info1 %x\n ",VechEADSInfop -> Handle_info1);
        printf("VechEADSInfop -> Handle_info2 %x\n ",VechEADSInfop -> Handle_info2);
        printf("VechEADSInfop -> Door_TransformFan %x\n ",VechEADSInfop -> Door_TransformFan);
        printf("Vech2EADSInfop -> ConvertFan %x\n ",VechEADSInfop -> ConvertFan2Che);
        printf("Vech4EADSInfop -> ConvertFan %x\n ",VechEADSInfop -> ConvertFan4Che); 
		printf("VechEADSInfop -> ParkButtn %x\n ",VechEADSInfop -> ParkButtn);
        printf("VechEADSInfop -> ResetButtn %x\n ",VechEADSInfop -> ResetButtn);  
		printf("VechEADSInfop -> BatterButtn %x\n ",VechEADSInfop -> BatterButtn);

    }

}
#endif
/**********************************************************************
*Name			:   TrdpRecvDataPro
*Function       :   接收TRDP数据，将指纹模块和语音模块的指令放入对应的数组
*Para     		:   uint8_t PdData[],CAN_FingerPrint_TPYE *Can_RecvDate,UART_VOICE_TPYE *Uart_RecvDate
*Return    		:   
*Version        :   REV1.0.0       
*Author:        :   ZLZ
*
*History:
*REV1.0.0     ZLZ    2020/9/10  Create
*
*********************************************************************/
#if 0
int8_t TrdpRecvDataPro(uint8_t PdData[],CAN_FingerPrint_TPYE *Can_RecvDate,UART_VOICE_TPYE *Uart_RecvDate)
{
    uint8_t i,ret;
    static uint8_t RecvTimes=0;
    static uint8_t RecvFlag=0;
    SYSTEM_TIME Recv_Lcu_Time={0};
	LCU_TRDP_DATA *LCU_trdp_data_p =NULL;
    LCU_trdp_data_p = (LCU_TRDP_DATA *)PdData;
    ret = pdRecLife(LCU_trdp_data_p);
    Can_RecvDate -> Life_fingerprint_U8 = LCU_trdp_data_p ->LifeL_U8;//复制LCU生命信号低8为给指纹模块
    memcpy(&Uart_RecvDate -> WakeUpTime,&LCU_trdp_data_p -> WakeUpTime,1);
    memcpy(&Uart_RecvDate -> volumeGain,&LCU_trdp_data_p -> volumeGain,1);
    memcpy(Uart_RecvDate -> UartDate_U8,LCU_trdp_data_p -> UartDate_U8,2);
    memcpy(Can_RecvDate -> CanDate_U8,LCU_trdp_data_p -> CanDate_U8,3);
    Recv_Lcu_Time.Year_U8[0] = 0x07;
    Recv_Lcu_Time.Year_U8[1] = 0xD0+LCU_trdp_data_p -> LCU_TIME[0];
    memcpy(&Recv_Lcu_Time.Month_U8,&LCU_trdp_data_p -> LCU_TIME[1],5);
    if(RecvFlag==1 && ret == CODE_OK)
    {
        SetRTCTime(&Recv_Lcu_Time);
        RecvFlag=2;
    }
    else if(RecvFlag==0)
    {
        RecvTimes++;
        if(RecvTimes>=120)//考虑到生命信号停止100周期才会返回错误，此处滤波120个周期
        {RecvFlag=1;}
    }
    if(TRDP_DEBUG == g_DebugType_EU)
    {   
        printf("ADU_Recv_TrdpDate_From_LCU:\n");
        printf("Recv_fingerprint_Life %x\n ",Can_RecvDate -> Life_fingerprint_U8);
        printf("Recv_WakeUpTime %x\n ",Uart_RecvDate -> WakeUpTime);
        for(i=0;i<2;i++) 
        {
            printf("Recv_trdp_data: UartDate_U8[%d]:%x\n",i,Uart_RecvDate -> UartDate_U8[i]);
        }
        for(i=0;i<3;i++)
        {
            printf("Recv_trdp_data: CanDate_U8[%d]:%x\n",i,Can_RecvDate -> CanDate_U8[i]);
        }
        printf("Recv_Lcu_Time.Year:%d\n",Recv_Lcu_Time.Year_U8[0]<<8+Recv_Lcu_Time.Year_U8[1]);
        printf("Recv_Lcu_Time.Month:%d\n",Recv_Lcu_Time.Month_U8);
        printf("Recv_Lcu_Time.Day:%d\n",Recv_Lcu_Time.Day_U8);
        printf("Recv_Lcu_Time.Hour:%d\n",Recv_Lcu_Time.Hour_U8);
        printf("Recv_Lcu_Time.Minute:%d\n",Recv_Lcu_Time.Minute_U8);
        printf("Recv_Lcu_Time.Second:%d\n",Recv_Lcu_Time.Second_U8);
    }
}
#endif
/*add this the bin +200KB*/
/**********************************************************************
*Name			:   pdRecv(uint32_t comId, uint8_t RecData[],uint32_t size)
*Function       :   Rec the trdp data

*Para     		:   uint32_t comId, uint8_t RecData[],uint32_t size)
*Return    		:   1.Ok,is CODE_OK;
					2. time out or other err ,CODE_ERR
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*********************************************************************/
int8_t pdRecv(uint32_t comId, uint8_t RecData[],uint32_t size)
{
	uint16_t i = 0;
	int8_t ret = 0;
	int8_t err = 0;
	char loginfo[LOG_INFO_LENG] = {0};
    static s_ErrFlag = 0; 
	ret = PDGetData(comId, RecData,size);
    
	if (1 == ret)
	{
		if(TRDP_DEBUG == g_DebugType_EU)
		{
			printf("TRDP RecByte:\n");
			for (i = 0; i < 20; i++)
			{
				printf("[%02x]", RecData[i]);
                if ((i + 1) % 20 == 0)
				{
					printf("\n");
				}
			}		
		}
        if(1 == s_ErrFlag)
        {
            s_ErrFlag = 0;
            g_EADSErrInfo_ST.TRDPErr = 0;
            printf("recv from com %u  Recover\n", comId);
            snprintf(loginfo, sizeof(loginfo)-1,"recv from com %u  Recover!",comId);
            WRITELOGFILE(LOG_ERROR_1,loginfo); 
        }

 	}
	else if (0 == ret)
	{
        if(0 == s_ErrFlag)
        {
            s_ErrFlag = 1;
            g_EADSErrInfo_ST.TRDPErr = 1;
            printf("recv from com %u  time out!\n", comId);
            snprintf(loginfo, sizeof(loginfo)-1,"recv from com %u time out!",comId);
            WRITELOGFILE(LOG_ERROR_1,loginfo); 
        }
        err =  CODE_ERR;
	}
	else if (2 == ret)
	{
		printf("subscribe comId:%u OK!\n", comId);
		err =  CODE_OK;        
	}
	else
	{
		if(0 == s_ErrFlag)
        {
            s_ErrFlag = 1;
            g_EADSErrInfo_ST.TRDPErr = 1;
            printf("recv from com %d err!\n", comId);/*if not connet to CCU,will happen error*/
    		snprintf(loginfo, sizeof(loginfo)-1,"recv from com %u err!", comId);
            WRITELOGFILE(LOG_ERROR_1,loginfo);
        }
       	err =  CODE_ERR;
	}
}
