/**********************************************************************
*File name 		:   DataHandle.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :    2020/2/21
*Description    :   1.judge every chan data range
                    2.channel over current and over voltage
                    3.channel data trans to  digital judge
                    4.Oprt time and oprt Num calibrate
                    5.Buffer the Chan Data
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/2/21  optmize for ADU  and CTU data proc
*
*********************************************************************/

/**********************************************************************
*
*Debug switch  section
*
*********************************************************************/

#if 0

/**********************************************************************
*
*Include file  section
*
*********************************************************************/
#include "DataHandle.h"
 
/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/
//2020026

//20200326
//extern BLVDS_BRD_DATA g_BrdRdData_ST;
extern BLVDS_BRD_DATA g_BrdRdBufData_ST;
extern CHAN_DATA  g_ChanData_ST[CHAN_BUFFER_NUM];
extern CHAN_DATA  g_ChanRealBuf_ST[REAL_BUFFER_NUM];
extern CHAN_DATA  g_ChanData_ProcBuf_ST[CHAN_BUFFER_NUM];  /*304 byte*/
extern CHAN_DATA  g_ChanSendBuf_ST[CHAN_BUFFER_NUM];  /*304 byte*/
//extern CHAN_DIGITAL_INFO g_ChanDigitalInfo_ST;
extern CHAN_STATUS_INFO  g_ChanStatuInfo_ST;
extern RELAY_LIFE_CFG   g_ChanRelayCfg_ST;
extern CHAN_CALIB_DATA  g_ChanCalib110VData_ST;
extern CHAN_CALIB_DATA  g_ChanCalib0VData_ST;
extern PTHREAD_LOCK g_PthreadLock_ST;
extern RECORD_XML g_Rec_XML_ST;
extern RECORD_XML g_LifeRec_XML_ST;
extern VERSION  g_Version_ST;
extern TRAIN_INFO g_TrainInfo_ST;

// extern TRDP_DATA g_Trdp_SendData_ST;

extern DEBUG_TYPE_ENUM g_DebugType_EU;
extern uint32_t g_RealBufNum_U32;
extern uint16_t g_ChanFiltDebug;
extern uint16_t g_ChanFiltNum_U16;
/**********************************************************************
*
*Local Macro Define Section
*
*********************************************************************/

/**********************************************************************
*
*Local Struct Define Section
*
*********************************************************************/

/**********************************************************************
*
*Local Prototype Declare Section
*
*********************************************************************/

/**********************************************************************
*
*Static Variable Define Section
*
*********************************************************************/
static CHAN_JUDGE s_ChanJudge_ST = {0};
static CHAN_THREHOLD_VALUE s_channel_ThreValue_ST = {0}; /*in ModifConfig.h*/

//s_UndrVoltJudge_H is the 77V + 5V
//s_UndrVoltJudge_L is the 33V - 5V
static uint16_t s_OverVoltElemtJudge = 0;
static uint16_t s_UndrVoltElemtJudge_H = 0;
static uint16_t s_UndrVoltElemtJudge_L = 0;
static uint8_t s_FirstRunFlag = 0;

static uint16_t s_OverCurrElemtJudge_1I1[CURR_CHAN_NUM] = {0};
static uint16_t s_OverCurrElemtJudge_1I4[CURR_CHAN_NUM] = {0};
static uint16_t s_OverCurrElemtJudge_2I[CURR_CHAN_NUM] = {0};
static uint16_t s_OverCurrElemtJudge_3I[CURR_CHAN_NUM] = {0};
static uint16_t s_OverCurrJudge_1I1[CURR_CHAN_NUM] = {0}; /* I=1.13In时，60min＜t*/
static uint16_t s_OverCurrJudge_1I4[CURR_CHAN_NUM] = {0};	/*I=1.45In时，20s＜t＜60min*/
static uint16_t s_OverCurrJudge_2I[CURR_CHAN_NUM] = {0};	/*I=2In时，8s＜t＜6min*/
static uint16_t s_OverCurrJudge_3I[CURR_CHAN_NUM] = {0};	/*I=3In时，3s＜t＜60s*/
/**********************************************************************
*
*Function Define Section
*
*********************************************************************/

/**********************************************************************
*Name			:   int8_t RecChanNameExtr(uint16_t *BufferIn,uint8_t *RecorChanName)
*Function       :   Extra the Chan mask value to get every chan name 
*Para     		:   uint16_t *BufferIn
*					uint8_t *RecorChanName 
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/5/7  Create
* not use
*********************************************************************/
int8_t RecChanNameExtr(uint16_t *BufferIn,uint8_t *RecorChanName)
{
    uint8_t RecTotalNum = 0;
    uint16_t ChanBitTem = 0;
    uint8_t i = 0,j = 0;
	
    for(i = 0;i < 6;i++)//6 word
    {
        ChanBitTem = *BufferIn;
        for(j = 0;j < 16;j++)//1 byte mean 8 channel
        {           
            //C code is start from channel 0,but Drive is start from channel 1
            if((ChanBitTem >> j) & 1)
            {                  	
            	*RecorChanName = j + i * 16;
            	if(XML_DEBUG == g_DebugType_EU)
            	{
					printf("RecorChanName %u\n",*RecorChanName);
            	} 
            	RecorChanName++;
            } 
        }
        BufferIn++;        	
  	}
}

/**********************************************************************
*Name			:   int8_t SysThreValuInit(void)

*Function       :  Init the  OverVoltage ,UnderVoltage,OverCurrJudge,UndrCurrJudge Judge,s_OverCurrJudge_1I1,s_OverCurrJudge_1I4
                    s_OverCurrJudge_2I,s_OverCurrJudge_3I
*Para     		:   
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2020/2/27  Create
*********************************************************************/
int8_t SysThreValuInit(void)
{
    uint16_t  j = 0;
    float CurrTem = 0;
    /*the ThreValue  judge,Calculate the OverVoltJudge ,s_OverCurrJudge with Slug */
    s_OverVoltElemtJudge = s_channel_ThreValue_ST.HighVol_H - s_channel_ThreValue_ST.SlugVol;/*1375 +50 */
    s_UndrVoltElemtJudge_H = s_channel_ThreValue_ST.HighVol_L + s_channel_ThreValue_ST.SlugVol; /*770 +50 */
    s_UndrVoltElemtJudge_L = s_channel_ThreValue_ST.LowVol_H - s_channel_ThreValue_ST.SlugVol;/*330  -50 */
	/* I=1.13In时，60min＜t*/
	/*I=1.45In时，20s＜t＜60min*/
	/*I=2In时，8s＜t＜6min*/
	/*I=3In时，3s＜t＜60s*/
    for(j = 0;j < CURR_CHAN_NUM;j++)
    {
        
        
        CurrTem = (float)s_channel_ThreValue_ST.CurrChan_ThreValue_ST[j].HighCurr * 1.13;
        s_OverCurrJudge_1I1[j] = (uint16_t) CurrTem;
        s_OverCurrElemtJudge_1I1[j] =  s_OverCurrJudge_1I1[j] - s_channel_ThreValue_ST.SlugCurr;

        CurrTem = (float)s_channel_ThreValue_ST.CurrChan_ThreValue_ST[j].HighCurr * 1.45;
        s_OverCurrJudge_1I4[j] = (uint16_t) CurrTem;
        s_OverCurrElemtJudge_1I4[j] =  s_OverCurrJudge_1I4[j] - s_channel_ThreValue_ST.SlugCurr;

        CurrTem = s_channel_ThreValue_ST.CurrChan_ThreValue_ST[j].HighCurr * 2;
        s_OverCurrJudge_2I[j] = (uint16_t) CurrTem;
        s_OverCurrElemtJudge_2I[j] =  s_OverCurrJudge_2I[j] - s_channel_ThreValue_ST.SlugCurr;

        CurrTem = s_channel_ThreValue_ST.CurrChan_ThreValue_ST[j].HighCurr * 3;
        s_OverCurrJudge_3I[j] = (uint16_t) CurrTem; 
        s_OverCurrElemtJudge_3I[j] =  s_OverCurrJudge_3I[j] - s_channel_ThreValue_ST.SlugCurr;

    }

}
/**********************************************************************
*Name			:   uint8_t DataProcPowerOn(uint32_t *Channel_OperaNum_U32,CHAN_THREHOLD_VALUE *ChanThreValue_ST_p,uint8_t *ChanRecordStatus)
*Function       :   when power on,read the OperateNum ,UsrSYSPrmTable.dat.Device_Record_CFG.xml.ChanCalib110VValue.dat file,and init the judge value
*Para     		:   CHAN_STATUS_INFO * ChanStatInfo_ST_p, 
                    CHAN_CALIB_DATA * ChCalibDtap, 
                    uint8_t ChanNumTmp,uint8_t EADSType
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2020/02/27  Create
*********************************************************************/
int8_t DataProcPowerOn(CHAN_STATUS_INFO * ChanStatInfo_ST_p, CHAN_CALIB_DATA * ChLowCalibDtap,CHAN_CALIB_DATA * ChHighCalibDtap, uint8_t ChanNumTmp,uint8_t EADSType)
{
    uint8_t i =0,j=0;
	if(0 == ChanNumTmp)
	{
		printf("The ChanNum is %u\n",ChanNumTmp);
		return CODE_ERR;
	}
    if(EADSType == CTU_BOARD_ID)
    {
		SysXmlParInit(RCD_CFG_ZOGPEI_COACH,&g_Rec_XML_ST,&g_Version_ST);
    }
	else
	{
        SysXmlParInit(RCD_CFG_COACH,&g_Rec_XML_ST,&g_Version_ST);
	}
	/*RelayCfgXml(RELAY_PARA_CONFIG,&g_LifeRec_XML_ST,&g_ChanRelayCfg_ST);
    XmlParJudge(&g_Rec_XML_ST);//记录相关xml文件*/
	/********************************************************************
    XmlParJudge(&g_LifeRec_XML_ST); //寿命分析相关xml文件
    GetChanOperateNum(ChanStatInfo_ST_p -> Channel_OperaNum_U32);
    GetPTUChanThresholdValue(LOGI_PRMTABLE_FILE,&s_channel_ThreValue_ST,&g_Version_ST);
    SysThreValuInit();//must after GetPTUChanThresholdValue    
    GetChanCalibData(CHAN_CALIB_0V_FILE,ChLowCalibDtap);
    GetChanCalibData(CHAN_CALIB_110V_FILE,ChHighCalibDtap);
    if(DEVC_DEBUG == g_DebugType_EU)
    {
	    printf("SysPrmFile_Ver %u\n",g_Version_ST.SysPrmFile_Ver_U16);
		printf("LowVOl_H %u\n",s_channel_ThreValue_ST.LowVol_H);
		printf("HigVOl_H %u\n",s_channel_ThreValue_ST.HighVol_H);
        printf("HigVOl_L %u\n",s_channel_ThreValue_ST.HighVol_L);		   
		printf("SLugVOl %u\n",s_channel_ThreValue_ST.SlugVol);
		printf("OverVoltElemtJudge %u\n",s_OverVoltElemtJudge);
		printf("chan %u SLugCurr %u\n",i,s_channel_ThreValue_ST.SlugCurr);
	    for(i = 0;i < CURR_CHAN_NUM;i++)
		{  

		    printf("chan %u HighCurr %d \n",i,s_channel_ThreValue_ST.CurrChan_ThreValue_ST[i].HighCurr);
		    printf("chan %u s_OverCurrElemtJudge_1I1 %u\n",i, s_OverCurrElemtJudge_1I1[i]);
		    printf("chan %u s_OverCurrElemtJudge_1I4 %u\n",i, s_OverCurrElemtJudge_1I4[i]);
		    printf("chan %u s_OverCurrElemtJudge_2I %u\n",i, s_OverCurrElemtJudge_2I[i]);
		    printf("chan %u s_OverCurrElemtJudge_3I %u\n",i, s_OverCurrElemtJudge_3I[i]);
			printf("chan %u s_OverCurrJudge_1I1 %u\n",i,s_OverCurrJudge_1I1[i]);
			printf("chan %u s_OverCurrJudge_1I4 %u\n",i,s_OverCurrJudge_1I4[i]);
			printf("chan %u s_OverCurrJudge_2I %u\n",i,s_OverCurrJudge_2I[i]);
			printf("chan %u s_OverCurrJudge_3I %u\n",i,s_OverCurrJudge_3I[i]);

        }  
	}
	***************************************************************************/
    return CODE_OK;
}


/**********************************************************************
*Name			:   int8_t VolChanDataTrans
*Function       :   trans TranNum times of Voltage BLVDS data
*Para     		:   BLVDS_BRD_DATA * BrdData_ST_p,bram Data
                     CHAN_DATA * OutBuf_p ,Chan data
                     CHAN_CALIB_DATA * ChanCalib_ST_p, the calib data
                     uint8_t TranNum,uint8_t TrnsType) ,how much trans times
*                   uint8_t TrnsType: 0 is Tran with Calib Value;
*                                  1 is Tran without Calib Value for Calib Data Save
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/02/20  Create
*********************************************************************/
int8_t VolChanDataTrans(BLVDS_BRD_DATA * BrdData_ST_p, CHAN_DATA *OutBuf_p,CHAN_CALIB_DATA * ChanLowCalib_ST_p,CHAN_CALIB_DATA * ChanHighCalib_ST_p,uint8_t ProcNum,uint8_t TrnsType)
{    
    uint8_t ChanNum;
    ChanNum = VOL_CHAN_NUM;
    uint8_t i = 0,j= 0;
	//g_procnum = 4
    for(i = 0; i< ProcNum;i++)
    {
	    if(ADC_VDATE == g_DebugType_EU)
	    {	
			printf("Voltage transform:\n",i);
            printf("Proc %u times VolChan\n",i);
            for(j = 0 ;j< ChanNum;j++)
            {           	
            	printf("Byte [%u]  Value:%08x\n",j,BrdData_ST_p -> Board0_Data_U32[i][j]);
            }	    	
	    }
	    //when have a config file,every time trans one channel  
	    ADC7913VDataTranInt(&BrdData_ST_p -> Board0_Data_U32[i][0],OutBuf_p,ChanLowCalib_ST_p,ChanHighCalib_ST_p,ChanNum,TrnsType); 
		OutBuf_p++;
    }
    s_ChanJudge_ST.FPGATime_U16 = BrdData_ST_p -> Board0_Data_U32[0][FPGA_TIME_BYTE];
    return CODE_OK;
}
/**********************************************************************
*Name           :   int8_t CurrChanDataTrans
*Function       :   trans TranNum times of Current BLVDS data
*Para           :   BLVDS_BRD_DATA * BrdData_ST_p,bram Data
                     CHAN_DATA * OutBuf_p ,Chan data
                     uint8_t TranNum,uint8_t TrnsType) ,how much trans times
*                   uint8_t TrnsType (not use): 0 is Tran with Calib Value;
*                                  1 is Tran without Calib Value for Calib Data Save
*Return         :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/02/20  Create
*********************************************************************/
int8_t CurrChanDataTrans(BLVDS_BRD_DATA * BrdData_ST_p, CHAN_DATA *OutBuf_p,uint8_t ProcNum,uint8_t TrnsType)
{    
    uint8_t ChanNum =0;
    ChanNum = CURR_CHAN_NUM;
    uint8_t i=0,j=0;
    for(i = 0; i< ProcNum; i++)
    {
	    if(ADC_IDATE == g_DebugType_EU)
	    {
            printf("Current transform:\n",i);
			printf("Proc %u times Trans CurrChan\n",i);
            for(j = 0 ;j<ChanNum;j++)
            {            	
            	printf("Byte[%u]  Value:%08x\n",j,BrdData_ST_p -> Board0_Data_U32[i][j]);
            }	    	
	    }
	    //根据与飞鸿的协议，电流数据从数据区的Byte_U32[24]开始
	    ADC7913IDataTranInt(&BrdData_ST_p -> Board0_Data_U32[i][24],OutBuf_p,ChanNum,TrnsType);
        //地址从g_ChanData_ST[0]++
		OutBuf_p++;
    }    
    return CODE_OK;
}

/**********************************************************************
*Name           :   int8_t AllChanDataCalib
*Function       :   Compare the 1 frame acquire  V chan to the RefValue
*Para           :   int16_t VolRefValue, 
*                   int16_t CurrRefValue,
*                   CHAN_DATA *ChanCalib_ST_p,
*                   CHAN_DATA *ChanAcqr_ST_p 
*Return         :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/07/20  Create

*********************************************************************/
int8_t AllChanDataCalib(int16_t VolRefValue,CHAN_CALIB_DATA *ChanCalib_ST_p,CHAN_DATA *ChanAcqr_ST_p,uint8_t ClibType)
{
    
    uint8_t i =0;
    // if(ClibType == 0)//for 0 Low Voltage
    // {
    //     for(i = 0;i < VOL_CHAN_NUM;i++)
    //     {
    //         if(LINECAlib_DEBUG == g_DebugType_EU)
    //         {
    //             printf("VOL chan %d,Vol %d,\n",i,ChanAcqr_ST_p ->  VolChan_ST[i].CH_Voltage_I16);
    //         }
    //         ChanCalib_ST_p ->  VolChan_ST[i].CH_Voltage_I16 = ChanAcqr_ST_p -> ChanWithVI_ST[i].CH_Voltage_I16 - VolRefValue;           
    //     }

    // }
    // else if(ClibType == 1)//for > 5V high voltage
    // {
    for(i = 0;i < VOL_CHAN_NUM;i++)
    {
        if( ADCVOLT_110V == VolRefValue)
        {
         //when channel have high Voltage to save cali
            if(ChanAcqr_ST_p -> VolChan_ST[i].CH_Voltage_I16 >= 800) /*>80V*/
            {
                ChanCalib_ST_p -> VolChanCalib_I16[i] = ChanAcqr_ST_p -> VolChan_ST[i].CH_Voltage_I16 - VolRefValue;

            }
            else //for off channel
            {
                ChanCalib_ST_p -> VolChanCalib_I16[i] = 0;
            }
        }
        else if(ADCVOLT_0V == VolRefValue)
        {
            //when channel have high Voltage to save cali
            if(ChanAcqr_ST_p -> VolChan_ST[i].CH_Voltage_I16 <= 100) /*<10V*/
            {
                ChanCalib_ST_p -> VolChanCalib_I16[i] = ChanAcqr_ST_p -> VolChan_ST[i].CH_Voltage_I16 - VolRefValue;

            }
            else //for off channel
            {
                ChanCalib_ST_p -> VolChanCalib_I16[i] = 0;
            }
        }
        if(LINECAlib_DEBUG == g_DebugType_EU)
        {
            printf("VI chan %d,Vol %d sorce Vol %d\n",i,ChanCalib_ST_p -> VolChanCalib_I16[i],ChanAcqr_ST_p -> VolChan_ST[i].CH_Voltage_I16);
        }
    }
    //}      
}

/**********************************************************************
*Name			:   int8_t ChanDataJudge
*Function       :   Judge the channels data
*Para     		:   CHAN_DATA *TemChanData_ST_p, 
                    uint8_t EADSType:CTU_BOARD_ID is include CTU and ADU,other is just include ADU

*
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/2/29  Create
*********************************************************************/
int8_t ChanDataJudge(CHAN_DATA *TemChanData_ST_p, uint8_t EADSType)
{
    uint8_t i = 0,j =0;
	char loginfo[LOG_INFO_LENG] = {0};
	
    /*start of for(i = 0;i <  VOL_CHAN_NUM;i++) */
 	for(i = 0;i < VOL_CHAN_NUM;i++) 
	{
		
		/*elemt over voltage judge*/
		//when overvol, the voltage must lower than slugVol to disable warm
	   if((TemChanData_ST_p -> VolChan_ST[i].CH_Voltage_I16 <= s_OverVoltElemtJudge)&&(s_ChanJudge_ST.VolChanJudge[i].ChanOverVolFlag_U8 == 1))//1425
		{
            s_ChanJudge_ST.VolChanJudge[i].ChanElmtOverVolFiltNum_U8++;
            //printf("Vol i,%u, ChanElmtOverVolNum\n",i);
		}
        //when Under vol, the voltage must bigger than slugVol to disable under warm
        else if((TemChanData_ST_p -> VolChan_ST[i].CH_Voltage_I16 >= s_UndrVoltElemtJudge_H)&&(s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolFlag_U8 == 1)) //600
        {
            s_ChanJudge_ST.VolChanJudge[i].ChanElmtUndrVolFiltNum_U8++;
            //printf("VI i,%d, ChanElmtOverVolNum\n",i);
        }
        //when Under vol, the voltage must smaller than slugVol to disable under warm
        else if((TemChanData_ST_p -> VolChan_ST[i].CH_Voltage_I16 <= s_UndrVoltElemtJudge_L)&&(s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolFlag_U8 == 1))//270
        {
            s_ChanJudge_ST.VolChanJudge[i].ChanElmtUndrVolFiltNum_U8++;
            //printf("VI i,%d, ChanElmtOverVolNum\n",i);
        }
		
		/*overvol judge*/
		if((TemChanData_ST_p -> VolChan_ST[i].CH_Voltage_I16) >= s_channel_ThreValue_ST.HighVol_H) //1375
		{
			
			s_ChanJudge_ST.VolChanJudge[i].ChanOverVolFiltNum_U8++;
			if(DATPRO_DEBUG == g_DebugType_EU)
			{
				printf("VChan:%u,CH_Voltage_I16:%d\n",i, TemChanData_ST_p -> VolChan_ST[i].CH_Voltage_I16);
			    printf("OverVol:%u\n",s_channel_ThreValue_ST.HighVol_H);
			}

		}
		/*highvol_l judge*/
		else if((TemChanData_ST_p -> VolChan_ST[i].CH_Voltage_I16) >= s_channel_ThreValue_ST.HighVol_L)//450
		{
            s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadFiltNum_U8++;
            /*20210315 cal the jump time.when the voltage tran from 0 to 1，
			20210407  add the ChanHighVolSteadFiltNum_U8 >1,for once >0 because of disturb
			 && (s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadFiltNum_U8 > 1)*/
			if ((!s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8) && (!s_ChanJudge_ST.VolChanJudge[i].ChanOperUpStartFlag_U8))
			{
				s_ChanJudge_ST.VolChanJudge[i].ChanOperUpStartFlag_U8 = 1; //mean start the oprt 
				s_ChanJudge_ST.VolChanJudge[i].ChanStartTime_U16 = s_ChanJudge_ST.FPGATime_U16;
//                printf("Chan %u Start Up Opertn",i);                   
//                snprintf(loginfo, sizeof(loginfo)-1, "Chan %u Start Up Opert",i);
//                WRITELOGFILE(LOG_INFO_1,loginfo); 
			}
		}
        /*lowvol_h judge*/
        else if( TemChanData_ST_p -> VolChan_ST[i].CH_Voltage_I16 > s_channel_ThreValue_ST.LowVol_H) //330
        {
            s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadFiltNum_U8++;

        }
		/*lowvol_l judge*/
		else if((TemChanData_ST_p -> VolChan_ST[i].CH_Voltage_I16) <= s_channel_ThreValue_ST.LowVol_H)//330
		{
			s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadFiltNum_U8++;
		}
		/*voltage under 0V ,not run*/
		else 
		{

			s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadFiltNum_U8++;
		}
		if(DATPRO_DEBUG == g_DebugType_EU)
		{
			
			printf("VChan:%u,CH_Voltage_I16:%d\n",i, TemChanData_ST_p -> VolChan_ST[i].CH_Voltage_I16);
		}

	}/*end of for(i = 0;i < VOL_CHAN_NUM;i++) */		
    
    /***must judge every channel for overcurrent and overvoltage 
    cannot  use break */
    if(EADSType == CTU_BOARD_ID)
    {
		for(i = 0;i < CURR_CHAN_NUM ;i++) 
		{
		
			/*current judge*/
			//when overcurrent, the current must lower than slugVol to disable warm
			if((TemChanData_ST_p -> CurrChan_ST[i].CH_Current_I16 <= s_OverCurrElemtJudge_3I[i])&&(s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_3I_U8 == 1))
			{
			  	s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_3I_U8++;

			}
			if((TemChanData_ST_p -> CurrChan_ST[i].CH_Current_I16 <= s_OverCurrElemtJudge_2I[i])&&(s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_2I_U8 == 1))
			{
			 	s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_2I_U8++;

			}
			if((TemChanData_ST_p -> CurrChan_ST[i].CH_Current_I16 <= s_OverCurrElemtJudge_1I4[i])&&(s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_1I4_U8 == 1))
			{
			  	s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_1I4_U8++;

			}
			if((TemChanData_ST_p -> CurrChan_ST[i].CH_Current_I16 <= s_OverCurrElemtJudge_1I1[i])&&(s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_1I1_U8 == 1))
			{
			    s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_1I1_U8++;

			}
			/*overcurr judge*/
			if(TemChanData_ST_p -> CurrChan_ST[i].CH_Current_I16 >= s_OverCurrJudge_3I[i])
			{
				/*when over 3I,also over 2I,1I4,1I1*/
				s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_3I_U8++;
				s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_2I_U8++;
				s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I4_U8++;
				s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I1_U8++;

			}
			else if(TemChanData_ST_p -> CurrChan_ST[i].CH_Current_I16 >= s_OverCurrJudge_2I[i])
			{

				/*when over 2I,also over,1I4,1I1*/
				s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_2I_U8++;
				s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I4_U8++;
				s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I1_U8++;

			}
			else if(TemChanData_ST_p -> CurrChan_ST[i].CH_Current_I16 >= s_OverCurrJudge_1I4[i])
			{

				/*when over 1I4,also ,1I1*/
				s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I4_U8++;
				s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I1_U8++;

			}
			else if(TemChanData_ST_p -> CurrChan_ST[i].CH_Current_I16 >= s_OverCurrJudge_1I1[i]) 		
			{

				/*when over 3I,also over 2I,1I4,1I1*/
				s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I1_U8++;

			}
			/* normal curr_l judge*/
			else if(TemChanData_ST_p -> CurrChan_ST[i].CH_Current_I16 < s_channel_ThreValue_ST.CurrChan_ThreValue_ST[i].HighCurr) //0ml
			{
			  	s_ChanJudge_ST.CurrChanJudge[i].ChanNormCurrSteadNum_U8++;
			}
			else 
			{
			    
			}
			if(DATPRO_DEBUG == g_DebugType_EU)
			{
				printf("judge Curr Chan:%u CH_Current_I16:%d\n",i, TemChanData_ST_p -> CurrChan_ST[i].CH_Current_I16);
				printf("ChanOverCurrNum_3I_U8:%u\n",s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_3I_U8);
				printf("ChanOverCurrNum_2I_U8:%u\n",s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_2I_U8);
				printf("ChanOverCurrNum_1I4_U8:%u\n",s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I4_U8);
				printf("ChanOverCurrNum_1I1_U8:%u\n",s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I1_U8);
			}

		}/*end of for(i = 0;i <  VOL_CHAN_NUM;i++) */	
    
    }
    return CODE_OK;    
}
/**********************************************************************
*Name			:   uint8_t AllChanDataJudge
*Function       :   Judge ChanNumTmp Frame channels data 
*Para     		:   CHAN_DATA *ChanData_ST_p,
                    uint8_t ChanNumTmp,
                    uint8_t EADSType
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2020/02/20  Create
*********************************************************************/
int8_t AllChanDataJudge(CHAN_DATA *ChanData_ST_p,uint8_t ProcNum,uint8_t EADSType)
{
    
    uint8_t i = 0;
    for(i = 0; i < ProcNum; i++)
    {
    	ChanDataJudge(ChanData_ST_p,EADSType);
        ChanData_ST_p++;
    }
    return CODE_OK;

}

/**********************************************************************
*Name			:   VolChanDataProce(uint8_t ChanNumTmp)
*Function       :   Judge the  Voltage range
*Para     		:   uint8_t ChanNumTmp 
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/02/22  Create
*REV1.0.1     feng	  2020/11/19  the function is filt 1ms.for recognize the contact jump
*********************************************************************/
int8_t VolChanDataProce(uint8_t ChanNumTmp)
{

	uint8_t i;
	int8_t err = 0;
	char loginfo[LOG_INFO_LENG] = {0};
	/*one by one judge the Voltage channel*/
    for(i = 0; i < VOL_CHAN_NUM; i++)
    {
    	  /*Voltage Process*/
	    	/*Elimate the OverVoltage warm*/
	    	if (ChanNumTmp == s_ChanJudge_ST.VolChanJudge[i].ChanElmtOverVolFiltNum_U8)
	    	{
	 	
                s_ChanJudge_ST.VolChanJudge[i].ChanElmtOverVolNumFlag_U8 = 1;
	    	}
            /*Elimate the UndrVoltage warm*/           
            else if (ChanNumTmp == s_ChanJudge_ST.VolChanJudge[i].ChanElmtUndrVolFiltNum_U8)
            {

                s_ChanJudge_ST.VolChanJudge[i].ChanElmtUndrVolNumFlag_U8 = 1;
            }
			else
			{

			}
			
	    	/*Judge Whether is overvol*/
	    	if (ChanNumTmp == s_ChanJudge_ST.VolChanJudge[i].ChanOverVolFiltNum_U8)
	    	{
			
	    		s_ChanJudge_ST.VolChanJudge[i].ChanOverVolNumFlag_U8 = 1;

//				}
	    	}/*end of ChanOverVolFiltNum_U8>0 */
             /*Judge Whether is Undrvol*/
            else if (ChanNumTmp == s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadFiltNum_U8)
            {
                 s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadNumFlag_U8 = 1;
            }
	    	/*when vol from 0 to 1,the s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8 =1, else keep 0*/
	        else if (ChanNumTmp == s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadFiltNum_U8)
	        {
	        	s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadNumFlag_U8 = 1;
	    		
	        }
	        /*Judge Whether is Low vol Status*/
	        /*when vol from 1 to 0,the s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8 =0, else keep 1*/
	        else if (ChanNumTmp == s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadFiltNum_U8)
	        {
				s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadNumFlag_U8 = 1;
	        }
			else
			{
                
			}
        /*end of voltage process*/
        /*clear the All Number*/
	    if(DATPRO_DEBUG == g_DebugType_EU)
	    {
	        printf("Vol CHAN:%u ChanHighVolSteadFiltNum_U8 %u\n",i,s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadFiltNum_U8);
	        printf("Vol CHAN:%u ChanLowVolSteadFiltNum_U8 %u\n",i,s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadFiltNum_U8);
	    }
		s_ChanJudge_ST.VolChanJudge[i].ChanElmtOverVolFiltNum_U8 = 0;
		s_ChanJudge_ST.VolChanJudge[i].ChanElmtUndrVolFiltNum_U8 = 0;
		s_ChanJudge_ST.VolChanJudge[i].ChanOverVolFiltNum_U8 = 0;
		s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadFiltNum_U8 = 0;
        s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadFiltNum_U8 = 0;
   		s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadFiltNum_U8 = 0;

    }/*end of for(i = 0; i < CHAN_BUFFER_NUM; i++)*/
    return CODE_OK;
}

/**********************************************************************
*Name			:   VolChanDataProce(uint8_t ChanNumTmp)
*Function       :   Judge the  Voltage range
*Para     		:   uint8_t ChanNumTmp ,the filt for chan status
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/02/22  Create
*REV1.0.1     feng	  2020/11/19  the chan status is filter by 10ms
*********************************************************************/
int8_t VolChanDataProceFilt(uint8_t ChanNumTmp)
{

	uint8_t i;
	int8_t err = 0;
	uint8_t NumTemp = 0;
	char loginfo[LOG_INFO_LENG] = {0};
	/*one by one judge the Voltage channel*/
    for(i = 0; i < VOL_CHAN_NUM; i++)
    {
    	/*Voltage Process*/
	    	/*Elimate the OverVoltage warm*/
	    	if(1 == s_ChanJudge_ST.VolChanJudge[i].ChanElmtOverVolNumFlag_U8)
	    	{

				s_ChanJudge_ST.VolChanJudge[i].ChanElmtOverVolNum_U8++;
     			//printf("VI i,%u ChanElmtOverVolNum,%u\n",i,s_ChanJudge_ST.VolChanJudge[i].ChanElmtOverVolNum_U8);
	    		if (s_ChanJudge_ST.VolChanJudge[i].ChanElmtOverVolNum_U8 >= ChanNumTmp)
	    		{
	    			//lower than the Slug Voltage
                    if(1 == s_ChanJudge_ST.VolChanJudge[i].ChanOverVolFlag_U8)
                    {
                        s_ChanJudge_ST.VolChanJudge[i].ChanOverVolFlag_U8 = 0;
                        s_ChanJudge_ST.VolChanJudge[i].ChanOverVolChangeFlag_U8 = 1;/*when falt change to create file*/
                        printf("Chan %u,OverVol clear\n",i);
                        snprintf(loginfo, sizeof(loginfo)-1, "Chan %u,OverVol clear",i);
                        WRITELOGFILE(LOG_WARN_1,loginfo);                      
                    }
					s_ChanJudge_ST.VolChanJudge[i].ChanElmtOverVolNum_U8 = 0;
	    		}
				s_ChanJudge_ST.VolChanJudge[i].ChanElmtOverVolNumFlag_U8  = 0;
				s_ChanJudge_ST.VolChanJudge[i].ChanElmtUndrVolNum_U8 = 0;
	    	}
            /*Elimate the UndrVoltage warm*/           
            else if(1 == s_ChanJudge_ST.VolChanJudge[i].ChanElmtUndrVolNumFlag_U8)
            {

				s_ChanJudge_ST.VolChanJudge[i].ChanElmtUndrVolNum_U8++;
			//printf("VI i,%d ChanElmtOverVolNum,%d\n",i,s_ChanJudge_ST.VolChanJudge[i].ChanElmtOverVolNum_U8);
                if (s_ChanJudge_ST.VolChanJudge[i].ChanElmtUndrVolNum_U8 >= ChanNumTmp)
                {
                    //lower than the Slug Voltage
                    if(1 == s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolFlag_U8)
                    {
                        s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolFlag_U8 = 0;
                        s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolChangeFlag_U8 = 1;/*when falt change to create file*/
                        printf("Chan %u,UndrVol clear\n",i);
                        snprintf(loginfo, sizeof(loginfo)-1, "Chan %u,UndrVol clear",i);
                        WRITELOGFILE(LOG_WARN_1,loginfo);                      
                    }
					s_ChanJudge_ST.VolChanJudge[i].ChanElmtUndrVolNum_U8 = 0;
                } 
				s_ChanJudge_ST.VolChanJudge[i].ChanElmtUndrVolNumFlag_U8 = 0;
				s_ChanJudge_ST.VolChanJudge[i].ChanElmtOverVolNum_U8 = 0;
            }
			else
			{
				s_ChanJudge_ST.VolChanJudge[i].ChanElmtOverVolNum_U8 = 0;
				s_ChanJudge_ST.VolChanJudge[i].ChanElmtUndrVolNum_U8 = 0;
			}
			
	    	/*Judge Whether is overvol*/
	    	if(1 == s_ChanJudge_ST.VolChanJudge[i].ChanOverVolNumFlag_U8)
	    	{

				s_ChanJudge_ST.VolChanJudge[i].ChanOverVolNum_U8++;
			    if (s_ChanJudge_ST.VolChanJudge[i].ChanOverVolNum_U8 >= ChanNumTmp)
	    		{	    			
	    			if(0 == s_ChanJudge_ST.VolChanJudge[i].ChanOverVolFlag_U8)
                    {
                        s_ChanJudge_ST.VolChanJudge[i].ChanOverVolFlag_U8 = 1; 
                        s_ChanJudge_ST.VolChanJudge[i].ChanOverVolChangeFlag_U8 = 1;    /*clear at ChanDataAssigValue */            
                        printf("Chan %u,OverVol Happen\n",i);                   
                        snprintf(loginfo, sizeof(loginfo)-1, "Chan %u,OverVol Happen",i);
                        WRITELOGFILE(LOG_WARN_1,loginfo); 
                    }
	    			//for low voltage to over volatege, operate num ++
	    			if (0 == s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8)
	    			{
	    				s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8 = 1;
	    				s_ChanJudge_ST.VolChanJudge[i].ChanOperUpFlag_U8 = 1; /*clear at ChanDataAssigValue */
                        s_ChanJudge_ST.VolChanJudge[i].ChanEndTime_U16 = s_ChanJudge_ST.FPGATime_U16;
                      //  printf("Chan %u Up Operate\n",i);                   
                        snprintf(loginfo, sizeof(loginfo)-1, "Chan %u Up Operate",i);
                        WRITELOGFILE(LOG_INFO_1,loginfo); 
                        
	    			}

	    			//to avod when overvoltage to high voltage,the operate ++
	    			s_ChanJudge_ST.VolChanJudge[i].ChanOverVolNum_U8 = ChanNumTmp;
	    		}
				s_ChanJudge_ST.VolChanJudge[i].ChanOverVolNumFlag_U8 = 0;
				s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadNum_U8 = 0;
				s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadNum_U8 = 0;
		   		s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadNum_U8 = 0;
	    	}/*end of ChanOverVolNum_U8>0 */
             /*Judge Whether is Undrvol*/
            else if(1 == s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadNumFlag_U8)
            {

				s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadNum_U8++;
				if (s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadNum_U8 >= ChanNumTmp)
                {
                    
                    if(0 == s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolFlag_U8)
                    {
                        s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolFlag_U8 = 1; 
                        s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolChangeFlag_U8 = 1;    /*clear at ChanDataAssigValue */            
                        printf("Chan %u,UndrVol Happen\n",i);                   
                        snprintf(loginfo, sizeof(loginfo)-1, "Chan %u,UndrVol Happen",i);
                        WRITELOGFILE(LOG_WARN_1,loginfo); 
                    }
					s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadNum_U8 = ChanNumTmp;
                }
				 s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadNumFlag_U8 = 0;
				 s_ChanJudge_ST.VolChanJudge[i].ChanOverVolNum_U8 = 0;
				 s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadNum_U8 = 0;
				 s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadNum_U8 = 0;

            }/*end of ChanOverVolNum_U8>0 */
	    	/*when vol from 0 to 1,the s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8 =1, else keep 0*/
	        else if (1 == s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadNumFlag_U8)
	        {

                NumTemp = s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadNum_U8 | s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadNum_U8;
                if((NumTemp > 0) && (NumTemp < ChanNumTmp) && (s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8))
                {
//                     //printf("Chan %u Down Jump Operate\n",i);                   
                     snprintf(loginfo, sizeof(loginfo)-1, "Chan %u Down Jump Operate",i);
                     WRITELOGFILE(LOG_INFO_1,loginfo); 		
                }
				s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadNum_U8++;
	
				if (s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadNum_U8 >= ChanNumTmp)
	    		{
	    			/*the voltage tran from 0 to 1*/
	    			if (0 == s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8)
	    			{
	    				s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8 = 1;
	    				s_ChanJudge_ST.VolChanJudge[i].ChanOperUpFlag_U8 = 1; /*clear at ChanDataAssigValue */
                        s_ChanJudge_ST.VolChanJudge[i].ChanEndTime_U16 = s_ChanJudge_ST.FPGATime_U16;
//                     //   printf("Chan %u Up Operate\n",i);
                      //  snprintf(loginfo, sizeof(loginfo)-1, "Chan %u Up Operate",i);
                      //  WRITELOGFILE(LOG_INFO_1,loginfo); 
                        
	    			}
					s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadNum_U8 = ChanNumTmp;
	    		}
				s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadNumFlag_U8 = 0;
				s_ChanJudge_ST.VolChanJudge[i].ChanOverVolNum_U8 = 0;
				s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadNum_U8 = 0;
		   		s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadNum_U8 = 0;
	        }
	        /*Judge Whether is Low vol Status*/
	        /*when vol from 1 to 0,the s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8 =0, else keep 1*/
	        else if(1 == s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadNumFlag_U8)
	        {
                /*chagne to high and <10ms */
			    NumTemp = s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadNum_U8 | s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadNum_U8 | 
			    s_ChanJudge_ST.VolChanJudge[i].ChanOverVolNum_U8;
                if((NumTemp > 0) && (NumTemp < ChanNumTmp) && (!s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8))
                {
//                     printf("Chan %u Up Jump Operate\n",i);                   
                     snprintf(loginfo, sizeof(loginfo)-1, "Chan %u Up Jump Operate",i);
                     WRITELOGFILE(LOG_INFO_1,loginfo); 		
                }
				s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadNum_U8++;	
				if (s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadNum_U8 >= ChanNumTmp)
	    		{
	    			/*the voltage tran from 1 to 0*/
	    			if (1 == s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8)
	    			{
	    				
	    				s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8 = 0; 
                        s_ChanJudge_ST.VolChanJudge[i].ChanOperDownFlag_U8 = 1; /*clear at ChanDataAssigValue */
                        s_ChanJudge_ST.VolChanJudge[i].ChanEndTime_U16 = s_ChanJudge_ST.FPGATime_U16; //ChanEndTime_U16
//                        printf("Chan %u Down Operate\n",i);                   
                     //   snprintf(loginfo, sizeof(loginfo)-1, "Chan %u Down Operate",i);
                      //  WRITELOGFILE(LOG_INFO_1,loginfo); 			    
	   			    }
					s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadNum_U8 = ChanNumTmp;
	    		}
				s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadNumFlag_U8 = 0;
				s_ChanJudge_ST.VolChanJudge[i].ChanOverVolNum_U8 = 0;
				s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadNum_U8 = 0;
				s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadNum_U8 = 0;
	        }
			else
			{
				s_ChanJudge_ST.VolChanJudge[i].ChanOverVolNum_U8 = 0;
				s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolSteadNum_U8 = 0;
				s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadNum_U8 = 0;
		   		s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadNum_U8 = 0;	                
			}
        /*end of voltage process*/
        /*clear the All Number*/
	    if(DATPRO_DEBUG == g_DebugType_EU)
	    {
	        printf("Vol CHAN:%u ChanHighVolSteadNum_U8 %u\n",i,s_ChanJudge_ST.VolChanJudge[i].ChanHighVolSteadNum_U8);
	        printf("Vol CHAN:%u ChanLowVolSteadNum_U8 %u\n",i,s_ChanJudge_ST.VolChanJudge[i].ChanLowVolSteadNum_U8);
	        printf("Vol CHAN:%u ChanVolStatus_U8 %u\n",i,s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8);
	        printf("Vol CHAN:%u ChanOverVolFlag_U8 %u\n",i,s_ChanJudge_ST.VolChanJudge[i].ChanOverVolFlag_U8);
	        printf("Vol chan %u,status %u\n",i,s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8);
	    }
    }/*end of for(i = 0; i < CHAN_BUFFER_NUM; i++)*/
    return CODE_OK;
}


/**********************************************************************
*Name           :   CurrChanDataProce(uint8_t ChanNumTmp)
*Function       :   Judge the  Current data  range
*Para           :   uint8_t ChanNumTmp 
*Return         :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/02/22  Create
*********************************************************************/
int8_t CurrChanDataProce(uint8_t ChanNumTmp)//ChanNunTmp==4
{
    uint8_t i;
	int8_t err = 0;
	char loginfo[LOG_INFO_LENG] = {0};
    for(i = 0; i < CURR_CHAN_NUM; i++)
	    {	
	        /*Start of Current Process*/
	        /*Elimate the OverCurrent warm*/
	    	if(s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_3I_U8 > 0)//判断3倍过流故障消失
	    	{
	    		if (ChanNumTmp == s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_3I_U8)
	    		{
	    			
	    			if(1 == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_3I_U8)
                    {
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_3I_U8 = 0;
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrChangeFlag_U8 = 1;

                        printf("Chan %u,Over 3I clear \n",i);                    

                        memset(loginfo,0, sizeof(loginfo));
                        snprintf(loginfo, sizeof(loginfo)-1, "Chan %u,Over 3I clear",i);
                        WRITELOGFILE(LOG_WARN_1,loginfo);    
                    }
                    
	    		}
	    		s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_3I_U8 = 0;
	    		
	    	}
	    	if(s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_2I_U8 > 0)//判断2倍过流故障消失
	    	{
	    		if (ChanNumTmp == s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_2I_U8)
	    		{
	    			
	    			if(1 == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_2I_U8)
                    {
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_2I_U8 = 0;
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrChangeFlag_U8 = 1;

                        printf("Chan %u,Over 2I clear\n",i);                      
                            
                        memset(loginfo,0, sizeof(loginfo));
                        snprintf(loginfo, sizeof(loginfo)-1, "Chan %u,Over 2I clear",i);
                        WRITELOGFILE(LOG_WARN_1,loginfo);
                    }

	    		}
	    		s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_2I_U8 = 0;

	    	}
	    	if(s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_1I4_U8 > 0)//判断1.4倍过流故障消失
	    	{
	    		if (ChanNumTmp == s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_1I4_U8)
	    		{
	    			
	    			if(1 == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_1I4_U8)
                    {
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_1I4_U8 = 0;
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrChangeFlag_U8 = 1;

                        printf("Chan %u,Over 1I4 clear\n",i);                      
                            
                        memset(loginfo,0, sizeof(loginfo));
                        snprintf(loginfo, sizeof(loginfo)-1, "Chan %u,Over 1I4 clear",i);
                        WRITELOGFILE(LOG_WARN_1,loginfo);
                    }
	    		}
	    		s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_1I4_U8 = 0;
	    		
	    	}
	    	if(s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_1I1_U8 > 0)//判断1.1倍过流故障消失
	    	{
	    		if (ChanNumTmp == s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_1I1_U8)
	    		{
	    			
	    			if(1 == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_1I1_U8)
                    {
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_1I1_U8 = 0;
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrChangeFlag_U8 = 1;

                        printf("Chan %u,Over 1I1 clear\n",i);                      
                            
                        memset(loginfo,0, sizeof(loginfo));
                        snprintf(loginfo, sizeof(loginfo)-1, "Chan %u,Over 1I1 clear",i);
                        WRITELOGFILE(LOG_WARN_1,loginfo);
                    }
	    		}
	    		s_ChanJudge_ST.CurrChanJudge[i].ChanElmtOverCurrNum_1I1_U8 = 0;
	    		
	    	}

	        /*Judge Whether is overcurrent*/
	    	if(s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_3I_U8 > 0)//判断3倍过流故障发生
	    	{
	    		if (ChanNumTmp == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_3I_U8)
	    		{
	    			
	    			if(0 == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_3I_U8)
                    {
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_3I_U8 = 1;
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrChangeFlag_U8 = 1;

                        printf("Chan %u,Over 3I Happen\n",i);              
                        memset(loginfo,0, sizeof(loginfo));
                        snprintf(loginfo, sizeof(loginfo)-1, "Chan %u,Over 3I Happen",i);
                        WRITELOGFILE(LOG_WARN_1,loginfo);
                    }	    			
	    		}
	    		s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_3I_U8 = 0;
	    		
	    	}/*end of ChanOverCurrNum_3I_U8>0 */
	    	if(s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_2I_U8 > 0)//判断2倍过流故障发生
	    	{
	    		if (ChanNumTmp == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_2I_U8)
	    		{
	    			
	    			if(0 == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_2I_U8)
                    {

                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_2I_U8 = 1;
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrChangeFlag_U8 = 1;
                                         
                        printf("Chan %u,Over 2I Happen\n",i);
                                             
                        memset(loginfo,0, sizeof(loginfo));
                        snprintf(loginfo, sizeof(loginfo)-1, "Chan %u,Over 2I Happen",i);
                        WRITELOGFILE(LOG_WARN_1,loginfo);
                    }
	    			
	    		}
	    		s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_2I_U8 = 0;
	    		
	    	}/*end of ChanOverCurrNum_3I_U8>0 */
	    	if(s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I4_U8 > 0)//判断1.4倍过流故障发生
	    	{
	    		if (ChanNumTmp == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I4_U8)
	    		{
	    			
	    			if(0 == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_1I4_U8)
                    {
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_1I4_U8 = 1;
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrChangeFlag_U8 = 1;
                        
                        printf("Chan %u,Over 1I4 Happen\n",i);                                               
                        memset(loginfo,0, sizeof(loginfo));
                        snprintf(loginfo, sizeof(loginfo)-1, "Chan %u,Over 1I4 Happen",i);
                        WRITELOGFILE(LOG_WARN_1,loginfo);                      
                    }                   
	    			
	    		}
	    		s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I4_U8 = 0;
	    		
	    	}/*end of ChanOverCurrNum_3I_U8>0 */
	    	if(s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I1_U8 > 0)//判断1.4倍过流故障发生
	    	{
	    		if (ChanNumTmp == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I1_U8)
	    		{
	    			
	    			if(0 == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_1I1_U8)
                    {
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_1I1_U8 = 1;
                        s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrChangeFlag_U8 = 1;

                        printf("Chan %u,Over 1I1 Happen\n",i); 
                                    
                        memset(loginfo,0, sizeof(loginfo));
                        snprintf(loginfo, sizeof(loginfo)-1, "Chan %u,Over 1I1 Happen",i);
                        WRITELOGFILE(LOG_WARN_1,loginfo);
                    }
                   			
	    		}
	    		s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrNum_1I1_U8 = 0;
	    		
	    	}/*end of ChanOverCurrNum_1I1_U8>0 */
        	
        	if(s_ChanJudge_ST.CurrChanJudge[i].ChanNormCurrSteadNum_U8 > 0)
	    	{
	    		if (ChanNumTmp == s_ChanJudge_ST.CurrChanJudge[i].ChanNormCurrSteadNum_U8)
	    		{
	    			
	    			s_ChanJudge_ST.CurrChanJudge[i].ChanNormCurrFlag_U8 = 1;
	    			
	    		}
	    	}/*end of ChanOverCurrNum_1I1_U8>0 */
	    	/*Judge Whether is high current Status*/
	        /*end of current process*/
	        if(DATPRO_DEBUG == g_DebugType_EU)
		    {
		        printf("Curr CHAN:%u ChanNormCurrSteadNum_U8 %u\n",i,s_ChanJudge_ST.CurrChanJudge[i].ChanNormCurrSteadNum_U8);
		        printf("Curr CHAN:%u ChanNormCurrFlag_U8 %u\n",i,s_ChanJudge_ST.CurrChanJudge[i].ChanNormCurrFlag_U8);


		    }
	        /*need finish. should clear the All Number*/
	   		s_ChanJudge_ST.CurrChanJudge[i].ChanNormCurrSteadNum_U8 = 0;
	    }/*end of for(i = 0; i < VOL_CHAN_NUM; i++*/
}
/**********************************************************************
*Name			:   int8_t ChanDataProce
*Function       :   Proce the  Voltage ,Current level
*Para     		:   uint8_t ChanNumTmp 
*
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/02/22  Create
*********************************************************************/
int8_t ChanDataProce(uint8_t ChanNumTmp,uint8_t EADSType)
{

	int8_t err = 0;
	char loginfo[LOG_INFO_LENG] = {0};
	if(0 == ChanNumTmp)
	{
		printf("the ChanNum is %u \n",ChanNumTmp);
        snprintf(loginfo, sizeof(loginfo)-1, "the ChanNum is %u",ChanNumTmp);
        WRITELOGFILE(LOG_WARN_1,loginfo);
		err = CODE_WARN;
	}
	
	err = VolChanDataProce(ChanNumTmp);
	err = VolChanDataProceFilt(STATS_FILT_TIME); //10ms
	if(EADSType == CTU_BOARD_ID)
	{		
		err = CurrChanDataProce(ChanNumTmp);		
	}
    /*one by one judge every channel*/
	 return err;
	
}

/**********************************************************************
*Name			:   int8_t ChanDataAssigValue
*Function       :   Assign the Operate Number,Vol , Current channel Status,and FltFlag or OprtFlag for file save 
*Para     		:   CHAN_STATUS_INFO * ChanStatInfo_ST_p, 
                    uint8_t ChanNumTmp,
                    uint8_t EADSType
*
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/02/22  Create
*
*********************************************************************/
int8_t ChanDataAssigValue(CHAN_STATUS_INFO * ChanStatInfo_ST_p, uint8_t ChanNumTmp,uint8_t EADSType)
{
 	uint8_t i,j;
 	char loginfo[LOG_INFO_LENG] = {0};
    uint8_t TemFltFlag_U8 = 0;
    uint8_t TemOprtFlag_U8 = 0;
    static uint16_t s_VolFileFltTime[VOL_CHAN_NUM] = {0};
    static uint16_t s_VolFileFltFlag[VOL_CHAN_NUM] = {0};

    if(EADSType == CTU_BOARD_ID)
    {
        /*AssigValue for channel have voltage and current*/
        for(i = 0;i < CURR_CHAN_NUM;i++)
        {      
            TemFltFlag_U8 |= s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrChangeFlag_U8;
            if(1 == TemFltFlag_U8)
            {
                ChanStatInfo_ST_p ->  CurrFltFlag_U8 = 1; /*the flag is clear in funtion OverCurrWarnCal*/
            }
             s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrChangeFlag_U8 = 0;//*must clear for next judge,when happen or clear,is set*//

        }
    }
    for(i = 0;i < VOL_CHAN_NUM;i++)
 	{	    
        /*30ms creat file*/
		if((1 == s_ChanJudge_ST.VolChanJudge[i].ChanOverVolChangeFlag_U8) &&( 0 == s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolChangeFlag_U8))
        {
			s_VolFileFltTime[i]++;
			
		}
		else if((0 == s_ChanJudge_ST.VolChanJudge[i].ChanOverVolChangeFlag_U8) &&(1 == s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolChangeFlag_U8))
        {
			s_VolFileFltTime[i]++;
			
		}
		else
		{
		    s_VolFileFltTime[i] = 0;
			s_VolFileFltFlag[i] = 0;
			s_ChanJudge_ST.VolChanJudge[i].ChanOverVolChangeFlag_U8 = 0;			
		    s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolChangeFlag_U8 = 0;
		}
		if(s_VolFileFltTime[i] >= FILE_REC_TIME)
		{
			s_VolFileFltFlag[i] = 1;
			s_ChanJudge_ST.VolChanJudge[i].ChanOverVolChangeFlag_U8 = 0;			
		    s_ChanJudge_ST.VolChanJudge[i].ChanUndrVolChangeFlag_U8 = 0;			
		}

		TemFltFlag_U8 |= s_VolFileFltFlag[i];
        TemOprtFlag_U8 |= s_ChanJudge_ST.VolChanJudge[i].ChanOperUpFlag_U8; //10ms
        TemOprtFlag_U8 |= s_ChanJudge_ST.VolChanJudge[i].ChanOperDownFlag_U8;

        if(s_ChanJudge_ST.VolChanJudge[i].ChanOperUpFlag_U8) 
        {
            if(s_FirstRunFlag)//run after 50ms
            {
				ChanStatInfo_ST_p -> Channel_OperaNum_U32[i]++; /*can ++ with in serverl second by and time */
			}
            s_ChanJudge_ST.VolChanJudge[i].ChanOperUpFlag_U8 = 0; /*must clear for next judge,otherwise will entr oprttime cal*/
        }
        else if(1 == s_ChanJudge_ST.VolChanJudge[i].ChanOperDownFlag_U8)
        {
            s_ChanJudge_ST.VolChanJudge[i].ChanOperDownFlag_U8 = 0;//*must clear for next judge,otherwise will entr oprttime cal*/
        }
		
        ChanStatInfo_ST_p -> VolWarnFlag_U8[i] = s_ChanJudge_ST.VolChanJudge[i].ChanOverVolFlag_U8; /*for Digital data pro*/
        ChanStatInfo_ST_p -> VolChanStat_U8[i] = s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8;/*for Digital data pro,and file save*/
	}

    /*for fault recored*/
	if((1 == TemFltFlag_U8)&&(0 == ChanStatInfo_ST_p -> FltRealRecFlag_U8)) 
    {	    	
    	ChanStatInfo_ST_p -> FltRealRecFlag_U8 = 1; /**/
        printf("FltRealRecFlag_U8 set \n");
    	snprintf(loginfo, sizeof(loginfo)-1, "FltRealRecFlag set");
    	WRITELOGFILE(LOG_WARN_1,loginfo);
    }
    /*when file creat,set to 2*/
    /***********************
    more than one channel happed over voltage not at the same,will creat not one file 
    such as: 001ms channel 1 happen over,002 channel 2 happen over
    channel 1 creat a file ,FltRealRecFlag_U8 clear.
    when channel 2 happen over,FltRealRecFlag_U8 set,and wait channel 1 finish save.then create channel 2 file
    NOTE: have optimize by when file creat and close,set FltRealRecFlag_U8 to 2
    ****************************/
    else if((0 == TemFltFlag_U8)&&(2 == ChanStatInfo_ST_p -> FltRealRecFlag_U8))/*when file creat and close,set to 2,to avoid above repeate creat file*/                    
    {
        ChanStatInfo_ST_p -> FltRealRecFlag_U8 = 0;
        printf("FltRealRecFlag_U8 clear \n");
        snprintf(loginfo, sizeof(loginfo)-1, "FltRealRecFlag clear");
        WRITELOGFILE(LOG_WARN_1,loginfo);
    }

    if((1 == TemOprtFlag_U8)&&(0 == ChanStatInfo_ST_p -> OprtRealRecFlag_U8))  
    {
        ChanStatInfo_ST_p -> OprtRealRecFlag_U8 = 1; /**/              
        printf("OprtRealRecFlag set\n"); 
        snprintf(loginfo, sizeof(loginfo)-1, "OprtRealRecFlag set");
        WRITELOGFILE(LOG_INFO_1,loginfo);
    }
    else if((0 == TemOprtFlag_U8)&&(2 == ChanStatInfo_ST_p -> OprtRealRecFlag_U8))/*when file creat and close,set to 2*/  /*when file creat and close,set to 2*/  
    {
        ChanStatInfo_ST_p -> OprtRealRecFlag_U8 = 0; /**/                        
        printf("OprtRealRecFlag clear\n");  
        snprintf(loginfo, sizeof(loginfo)-1, "OprtRealRecFlag clear");
        WRITELOGFILE(LOG_INFO_1,loginfo);
    }

//    printf("ChanDataAssigValue\n");  
//    snprintf(loginfo, sizeof(loginfo)-1, "ChanDataAssigValue");
//    WRITELOGFILE(LOG_INFO_1,loginfo);
	return CODE_OK;
}
/**********************************************************************
*Name			:   int8_t OverCurrWarnCal
*Function       :   calibrate the over current. for 1I1 1I3 2I 3I
*Para     		:   CHAN_STATUS_INFO * ChanStatInfo_ST_p
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/02/22  Create
*********************************************************************/
int8_t OverCurrWarnCal(CHAN_STATUS_INFO * ChanStatInfo_ST_p)
{
    uint8_t i = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    static uint8_t s_CurrFltHapped = 0;
    if(1 == ChanStatInfo_ST_p -> CurrFltFlag_U8) /*have happend over current*/
    {
    	ChanStatInfo_ST_p -> CurrFltFlag_U8 = 0;
    	s_CurrFltHapped = 1;
    	for(i = 0;i < CURR_CHAN_NUM;i++)
	    {
	    	/*calculate the over current time*/
	    	if(1 == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_3I_U8) /*alway over curr 3I*/
		    {
		        s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_3I_U32++;
		        if(ALRM_DEBUG == g_DebugType_EU)
		        {
		        	printf("Chan %d,OverCurrTime_3I %u\n", i,s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_3I_U32 );
	
		        }
		    }
		    else/*if  other channel  happen over current,this channel num clear*/
		    {
				s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_3I_U32 = 0;

		    }

		    if(1 == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_2I_U8)
		    {
		        s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_2I_U32++;
		        if(ALRM_DEBUG == g_DebugType_EU)
		        {
		       		printf("Chan %d,OverCurrTime_2I %u\n", i,s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_2I_U32 );

		        }
		       
		    }
		    else
		    {
		    	s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_2I_U32 = 0;
		    			  
		    }

		    if(1 == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_1I4_U8)
		    {
		        s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_1I4_U32++;
		        if(ALRM_DEBUG == g_DebugType_EU)
		        {
		   		    printf("Chan %d,OverCurrTime_1I4 %u\n", i,s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_1I4_U32 );

		        }
		    }
		    else
		    {
		    	s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_1I4_U32 = 0;
		    } 

		    if(1 == s_ChanJudge_ST.CurrChanJudge[i].ChanOverCurrFlag_1I1_U8)
		    {
		        s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_1I1_U32++;
		        if(ALRM_DEBUG == g_DebugType_EU)
		        {
		        	printf("Chan %d,OverCurrTime_1I1 %u\n", i,s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_1I1_U32 );

		        }
		    }
	        else
	        {
	 			s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_1I1_U32 = 0;
	        }
            /*set the trdp overcurrt flag*/
		    if(s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_3I_U32 > CURR_3I_WARN_TIME)
		    {
		    	if(0 == ChanStatInfo_ST_p -> CurrWarnFlag_U8[i])
                {
                    ChanStatInfo_ST_p -> CurrWarnFlag_U8[i] = 1;
                    printf("Chan %d,set trdp 3I warn\n", i);
                    memset(loginfo,0, sizeof(loginfo));
                    snprintf(loginfo, sizeof(loginfo)-1, "Chan %d,set trdp 3I warn",i);
                    WRITELOGFILE(LOG_WARN_1,loginfo); 
                }

		    }
		    else if(s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_2I_U32 > CURR_2I_WARN_TIME)
		    {
		    	if(0 == ChanStatInfo_ST_p -> CurrWarnFlag_U8[i])
                {
                    ChanStatInfo_ST_p -> CurrWarnFlag_U8[i] = 1;
                    printf("Chan %d,set trdp 2I warn\n", i);
                    memset(loginfo,0, sizeof(loginfo));
                    snprintf(loginfo, sizeof(loginfo)-1, "Chan %d,set trdp 2I warn",i);
                    WRITELOGFILE(LOG_WARN_1,loginfo);    
                }

		    }
		    else if(s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_1I4_U32 > CURR_1I4_WARN_TIME)
		    {
		    	if(0 == ChanStatInfo_ST_p -> CurrWarnFlag_U8[i])
                {
                    ChanStatInfo_ST_p -> CurrWarnFlag_U8[i] = 1;
                    printf("Chan %d,set trdp 1I4 warn\n", i);
                    memset(loginfo,0, sizeof(loginfo));
                    snprintf(loginfo, sizeof(loginfo)-1, "Chan %d,set trdp 1I4 warn",i);
                    WRITELOGFILE(LOG_WARN_1,loginfo);  
                }

		    }
		    else if(s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_1I1_U32 > CURR_1I1_WARN_TIME)
		    {
		    	if(0 ==  ChanStatInfo_ST_p -> CurrWarnFlag_U8[i])
                {
                    ChanStatInfo_ST_p -> CurrWarnFlag_U8[i] = 1;
                    printf("Chan %d,set trdp 1I1 warn\n", i);
                    memset(loginfo,0, sizeof(loginfo));
                    snprintf(loginfo, sizeof(loginfo)-1, "Chan %d,set trdp 1I1 warn",i);
                    WRITELOGFILE(LOG_WARN_1,loginfo);

                }
 		    }
		    else
		    {	    	
                /*if  other channel  happen over current,this channel flag clear*/
                if(1 == ChanStatInfo_ST_p -> CurrWarnFlag_U8[i])
                {
                    ChanStatInfo_ST_p -> CurrWarnFlag_U8[i] = 0;
                    printf("Chan %d,clear trdp  overcurr flag\n", i);
                    memset(loginfo,0, sizeof(loginfo));
                    snprintf(loginfo, sizeof(loginfo)-1, "Chan %d,clear trdp overcurr",i);
                    WRITELOGFILE(LOG_WARN_1,loginfo);
                }
		    }
		   
	    }

    }
    else if(1 == s_CurrFltHapped) /*to clear to avoid the same channel repeat happen over current,and not clear*/
    {
    	s_CurrFltHapped = 0;
    	for(i =0;i < CURR_CHAN_NUM;i++)
    	{
    		s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_3I_U32 = 0;
    		s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_2I_U32 = 0;
    		s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_1I4_U32 = 0;
    		s_ChanJudge_ST.CurrChanJudge[i].OverCurrTime_1I1_U32 = 0;
            if(1 == ChanStatInfo_ST_p -> CurrWarnFlag_U8[i])
            {
                ChanStatInfo_ST_p -> CurrWarnFlag_U8[i] = 0;
                printf("Chan %d,clear trdp currwarn\n", i);
                memset(loginfo,0, sizeof(loginfo));
                snprintf(loginfo, sizeof(loginfo)-1, "Chan %d,clear trdp currwarn",i);
                WRITELOGFILE(LOG_WARN_1,loginfo);
            }
    	}
    } 

}
/**********************************************************************
*Name			:   int8_t ChanOpertTimeCal
*Function       :   Calibrate the Chan Oprerate time,one channel gather the coil voltage,and the other channel gather the constant vol
*Para     		:   CHAN_STATUS_INFO * ChanStatInfo_ST_p,
                    const uint8_t InCoilChan,
                    const uint8_t InContChan,
                    const uint8_t Oprttype:  OPRT_UP is Up time 
                                             OPRT_DOWN is Down time
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/02/22  Create
*********************************************************************/
int8_t ContChanJumpTimeCal(CHAN_STATUS_INFO * ChanStatInfo_ST_p,const uint8_t ChanNum)
{
	// if(s_ChanJudge_ST.VolChanJudge[InContChan].ChanEndTime_U16 > 0)
 //    {    
    	uint16_t TimeTem = 0;
        if(s_ChanJudge_ST.VolChanJudge[ChanNum].ChanEndTime_U16 >= s_ChanJudge_ST.VolChanJudge[ChanNum].ChanStartTime_U16)
    	{
    		TimeTem = (s_ChanJudge_ST.VolChanJudge[ChanNum].ChanEndTime_U16 - s_ChanJudge_ST.VolChanJudge[ChanNum].ChanStartTime_U16);
    	}
    	else if(s_ChanJudge_ST.VolChanJudge[ChanNum].ChanEndTime_U16 < s_ChanJudge_ST.VolChanJudge[ChanNum].ChanStartTime_U16)
    	{
    		TimeTem = (65535 - s_ChanJudge_ST.VolChanJudge[ChanNum].ChanStartTime_U16 + s_ChanJudge_ST.VolChanJudge[ChanNum].ChanEndTime_U16);

    	}

		if(TimeTem >= STATS_FILT_TIME)
		{
			ChanStatInfo_ST_p -> JumpTime_U16[ChanNum] = TimeTem - STATS_FILT_TIME;//for the filt time	  

		}
		else
		{
			ChanStatInfo_ST_p -> JumpTime_U16[ChanNum] = 0;//for the filt time	  
		}
    	//printf("%u chan OprtTime %u \n",InContChan ,TimeTem);
    //}
    return CODE_OK;
}

/**********************************************************************
*Name			:   int8_t ChanOpertTimeCal
*Function       :   Calibrate the Chan Oprerate time,one channel gather the coil voltage,and the other channel gather the constant vol
*Para     		:   CHAN_STATUS_INFO * ChanStatInfo_ST_p,
                    const uint8_t InCoilChan,
                    const uint8_t InContChan,
                    const uint8_t Oprttype:  OPRT_UP is Up time 
                                             OPRT_DOWN is Down time
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/02/22  Create
*********************************************************************/
int8_t ContChanOpertTimeCal(CHAN_STATUS_INFO * ChanStatInfo_ST_p,const uint8_t InCoilChan,const uint8_t InContChan,const uint8_t Oprttype)
{
	// if(s_ChanJudge_ST.VolChanJudge[InContChan].ChanEndTime_U16 > 0)
 //    {    
    	uint16_t TimeTem = 0;
        if(s_ChanJudge_ST.VolChanJudge[InContChan].ChanEndTime_U16 >= s_ChanJudge_ST.VolChanJudge[InCoilChan].ChanEndTime_U16)
    	{
    		TimeTem = (s_ChanJudge_ST.VolChanJudge[InContChan].ChanEndTime_U16 - s_ChanJudge_ST.VolChanJudge[InCoilChan].ChanEndTime_U16);
    	}
    	else if(s_ChanJudge_ST.VolChanJudge[InContChan].ChanEndTime_U16 < s_ChanJudge_ST.VolChanJudge[InCoilChan].ChanEndTime_U16)
    	{
    		TimeTem = (65535 - s_ChanJudge_ST.VolChanJudge[InCoilChan].ChanEndTime_U16 + s_ChanJudge_ST.VolChanJudge[InContChan].ChanEndTime_U16);

    	}
        if(OPRT_UP == Oprttype)
        {
            ChanStatInfo_ST_p -> OprtUpTime_U16[InContChan] = TimeTem;
        }
        else if(OPRT_DOWN == Oprttype)
        {
            ChanStatInfo_ST_p -> OprtDownTime_U16[InContChan]  = TimeTem;
        }
    	//printf("%u chan OprtTime %u \n",InContChan ,TimeTem);
    //}
    return CODE_OK;
}
/**********************************************************************
*Name           :   int8_t ChanOpertTimeCal
*Function       :   Calibrate the Chan Oprerate time,one channel gather the coil voltage,and the other channel gather the constant vol
                    when the constant chan is not oprate ,the time limit to 2000
*Para           :   CHAN_STATUS_INFO * ChanStatInfo_ST_p,
                    const uint8_t InCoilChan,
                    const uint8_t InContChan,
                    const uint8_t Oprttype:  OPRT_UP is Up time 
                                             OPRT_DOWN is Down time
*Return         :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/7/10  Create
*REV1.0.1     feng    2020/8/13  And  No  and NC relay 
*********************************************************************/
int8_t ChanOpertJumpTimeCal(CHAN_STATUS_INFO * ChanStatInfo_ST_p)
{
    char loginfo[LOG_INFO_LENG] = {0};
	uint8_t i = 0;
    static uint16_t s_OprtJumpTimeNum[VOL_CHAN_NUM] = {0};
    
//    if(NO_RELAY == RelayType)
//    {
//        if(1 == s_ChanJudge_ST.VolChanJudge[ChanNum].ChanOperUpFlag_U8)
//        {
//            s_OprtJumpFlag[InNum] = OPRT_UP;
//        } 
//       
//    }
//    else if(NC_RELAY == RelayType)
//    {
//		if (1 == s_ChanJudge_ST.VolChanJudge[ChanNum].ChanOperDownFlag_U8)
//        {
//            s_OprtJumpFlag[InNum] = OPRT_UP;
//        }  
//    }


//    if(OPRT_UP == s_OprtJumpFlag[InNum])
//    {
	 for(i = 0;i<VOL_CHAN_NUM;i++)
	 {
		 if(s_ChanJudge_ST.VolChanJudge[i].ChanOperUpStartFlag_U8)
         {

			  s_OprtJumpTimeNum[i]++;
			  //ChanStatInfo_ST_p -> OprtDirction_U8[InContChan] = OPRT_UP;
			  if(1 == s_ChanJudge_ST.VolChanJudge[i].ChanOperUpFlag_U8)
			  { 	   
				  s_OprtJumpTimeNum[i] = 0;
				  s_ChanJudge_ST.VolChanJudge[i].ChanOperUpStartFlag_U8 = 0; //mean finsh once  oprt 
				  ContChanJumpTimeCal(ChanStatInfo_ST_p,i);
//				  printf("%u chan UpOprtJumpTime %u \n",i ,ChanStatInfo_ST_p -> JumpTime_U16[i]);
//				  snprintf(loginfo, sizeof(loginfo)-1, "%u chan UpOprtJumpTime %u",i ,ChanStatInfo_ST_p -> JumpTime_U16[i]);
//				  WRITELOGFILE(LOG_INFO_1,loginfo);
//			  
			  }
			  else if(s_OprtJumpTimeNum[i] > JUMP_TIME_MAX) //when task is inter,the loop can larger than 2s,and can calculte such as 3456ms,note disturb
			  {
				  s_OprtJumpTimeNum[i] = 0;	  
				  s_ChanJudge_ST.VolChanJudge[i].ChanOperUpStartFlag_U8 = 0; //mean not  oprt 
				  ChanStatInfo_ST_p -> JumpTime_U16[i] = 0;//when set JUMP_TIME_MAX,will cause life danger
				  printf("%u chan UpOprtJumpTime %u \n",i ,ChanStatInfo_ST_p -> JumpTime_U16[i]);
				  snprintf(loginfo, sizeof(loginfo)-1, "%u chan UpOprtJumpTime %u",i ,ChanStatInfo_ST_p -> JumpTime_U16[i]);
				  WRITELOGFILE(LOG_ERROR_1,loginfo);
			  }

		 }
		 
	 }
	 	

   // }
   
}

/**********************************************************************
*Name           :   int8_t ChanOpertTimeCal
*Function       :   Calibrate the Chan Oprerate time,one channel gather the coil voltage,and the other channel gather the constant vol
                    when the constant chan is not oprate ,the time limit to 2000
*Para           :   CHAN_STATUS_INFO * ChanStatInfo_ST_p,
                    const uint8_t InCoilChan,
                    const uint8_t InContChan,
                    const uint8_t Oprttype:  OPRT_UP is Up time 
                                             OPRT_DOWN is Down time
*Return         :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/7/10  Create
*REV1.0.1     feng    2020/8/13  And  No  and NC relay 
*********************************************************************/
int8_t ChanOpertTimeCal(CHAN_STATUS_INFO * ChanStatInfo_ST_p,const uint8_t InCoilChan,const uint8_t InContChan,const uint8_t InNum,uint8_t RelayType)
{
    char loginfo[LOG_INFO_LENG] = {0};
    static uint8_t s_OprtFlag[CAL_TIME_CHANL] = {0};
    static uint16_t s_OprtTimeNum[CAL_TIME_CHANL] = {0};
    if(NO_RELAY == RelayType)
    {
        if(1 == s_ChanJudge_ST.VolChanJudge[InCoilChan].ChanOperUpFlag_U8)
        {
            s_OprtFlag[InNum] = OPRT_UP;
        } 
        else if (1 == s_ChanJudge_ST.VolChanJudge[InCoilChan].ChanOperDownFlag_U8)
        {
            s_OprtFlag[InNum] = OPRT_DOWN;
        }  
    }
    else if((NC_RELAY == RelayType) && s_FirstRunFlag)//20210408 after run 50ms to calculate the down time 
    {
        if(1 == s_ChanJudge_ST.VolChanJudge[InCoilChan].ChanOperUpFlag_U8) 
        {
            s_OprtFlag[InNum] = OPRT_DOWN;
        } 
        else if (1 == s_ChanJudge_ST.VolChanJudge[InCoilChan].ChanOperDownFlag_U8)
        {
            s_OprtFlag[InNum] = OPRT_UP;
        }  
    }

    if(OPRT_UP == s_OprtFlag[InNum])
    {
        s_OprtTimeNum[InNum]++;
        ChanStatInfo_ST_p -> OprtDirction_U8[InContChan] = OPRT_UP;
        if(1 == s_ChanJudge_ST.VolChanJudge[InContChan].ChanOperUpFlag_U8)
        {        
            s_OprtTimeNum[InNum] = 0;
            s_OprtFlag[InNum] = 0;
            ContChanOpertTimeCal(ChanStatInfo_ST_p,InCoilChan,InContChan,OPRT_UP);
           // printf("%u chan UpOprtTime %u \n",InContChan ,ChanStatInfo_ST_p -> OprtUpTime_U16[InContChan]);
            snprintf(loginfo, sizeof(loginfo)-1, "%u chan UpOprtTime %u",InContChan ,ChanStatInfo_ST_p -> OprtUpTime_U16[InContChan]);
            WRITELOGFILE(LOG_INFO_1,loginfo);

        }
        else if(s_OprtTimeNum[InNum] > OPRT_TIME_MAX) //when task is inter,the loop can larger than 2s,and can calculte such as 3456ms
        {
            s_OprtTimeNum[InNum] = 0;
            s_OprtFlag[InNum] = 0;
            ChanStatInfo_ST_p -> OprtUpTime_U16[InContChan] = OPRT_TIME_MAX;
            printf("%u chan UpOprtTime %u \n",InContChan ,ChanStatInfo_ST_p -> OprtUpTime_U16[InContChan]);
            snprintf(loginfo, sizeof(loginfo)-1, "%u chan UpOprtTime %u",InContChan ,ChanStatInfo_ST_p -> OprtUpTime_U16[InContChan]);
            WRITELOGFILE(LOG_ERROR_1,loginfo);
        }
    }
    else if(OPRT_DOWN == s_OprtFlag[InNum]) 
    {
        s_OprtTimeNum[InNum]++;
        ChanStatInfo_ST_p -> OprtDirction_U8[InContChan] = OPRT_DOWN;
        if(1 == s_ChanJudge_ST.VolChanJudge[InContChan].ChanOperDownFlag_U8)
        {
            s_OprtTimeNum[InNum] = 0;
            s_OprtFlag[InNum] = 0;
            ContChanOpertTimeCal(ChanStatInfo_ST_p,InCoilChan,InContChan,OPRT_DOWN);
            //printf("%u chan DownOprtTime %u \n",InContChan ,ChanStatInfo_ST_p -> OprtDownTime_U16[InContChan]);
            snprintf(loginfo, sizeof(loginfo)-1, "%u chan DownOprtTime %u",InContChan ,ChanStatInfo_ST_p -> OprtDownTime_U16[InContChan]);
            WRITELOGFILE(LOG_INFO_1,loginfo);

        }
        else if(s_OprtTimeNum[InNum] > OPRT_TIME_MAX)
        {
            s_OprtTimeNum[InNum] = 0;
            s_OprtFlag[InNum] = 0;
            ChanStatInfo_ST_p -> OprtDownTime_U16[InContChan] = OPRT_TIME_MAX;
            printf("%u chan DownOprtTime %u \n",InContChan ,ChanStatInfo_ST_p -> OprtDownTime_U16[InContChan]);
            snprintf(loginfo, sizeof(loginfo)-1, "%u chan DownOprtTime %u",InContChan ,ChanStatInfo_ST_p -> OprtDownTime_U16[InContChan]);
            WRITELOGFILE(LOG_ERROR_1,loginfo);
        } 
    } 
}

/**********************************************************************
*Name			:   OpertTimeAssigValue
*Function       :   Assig the all gather channel Oprate Time
*Para     		:   CHAN_STATUS_INFO * ChanStatInfo_ST_p
*
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/02/22  Create
*********************************************************************/
int8_t OpertTimeAssigValue(CHAN_STATUS_INFO * ChanStatInfo_ST_p,TRAIN_INFO TrainInfo_p)
{
    
	ChanOpertJumpTimeCal(ChanStatInfo_ST_p);

	if(COACH1 == TrainInfo_p.CoachNum_U8) 
    {
		/*1/8 coach ,Coil Chan:31 is T066,Chan:32 is  123A,relay:MK2KP0*/
		ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_T066,CONT_CHAN_123A,0,NO_RELAY);

	}
    else if(COACH3 == TrainInfo_p.CoachNum_U8) 
    {
		    
	    /*3/6 coach ,Coil Chan:8 is T96,Chan:26 is  106K,relay:RZDR-E02D2C*/
	    /*notice  the relay is NC*/
	    ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_T96,CONT_CHAN_106K,1,NO_RELAY);
	    /*3/6 coach ,Coil Chan:15 is T085,Chan:17 is T883,relay:RB4P-G22D2C*/
	    ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_T085,CONT_CHAN_T883,2,NC_RELAY);
		
		/*3/6 coach ,Coil Chan:37 is 175H11,Chan:38 is 103B,relay:BD-N20*/
		ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_175H1,CONT_CHAN_103B,3,NC_RELAY);
	   /*3/6 coach ,Coil Chan:39 is 175H31,Chan:40 is 103D,relay:BD-N65RN*/
	    ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_175H3,CONT_CHAN_103D,4,NC_RELAY);

	}
	 else if(COACH2 == TrainInfo_p.CoachNum_U8) //20201112 add for relay test
     {
//          /*for 3RH2131 Test*/
//
//         ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_32,CONT_CHAN_20,1,NO_RELAY);
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_32,CONT_CHAN_21,2,NO_RELAY);
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_32,CONT_CHAN_6,12,NO_RELAY);
//		 
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_33,CONT_CHAN_22,3,NO_RELAY);
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_33,CONT_CHAN_23,4,NC_RELAY);
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_33,CONT_CHAN_7,13,NO_RELAY);	
//		 
//         ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_34,CONT_CHAN_24,5,NO_RELAY);
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_34,CONT_CHAN_25,6,NC_RELAY);
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_34,CONT_CHAN_8,14,NC_RELAY);
//		 
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_35,CONT_CHAN_26,7,NO_RELAY);
//		 
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_36,CONT_CHAN_27,8,NO_RELAY);
//
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_37,CONT_CHAN_28,9,NO_RELAY);
//		 
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_38,CONT_CHAN_29,10,NO_RELAY);
//
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_39,CONT_CHAN_30,11,NO_RELAY);
//
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_12,CONT_CHAN_0,0,NO_RELAY);
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_12,CONT_CHAN_1,15,NO_RELAY);
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_13,CONT_CHAN_2,16,NO_RELAY);
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_13,CONT_CHAN_3,17,NC_RELAY);
//		 ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_14,CONT_CHAN_4,18,NO_RELAY);
		// ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_35,CONT_CHAN_31,12,NO_RELAY);
     }
    else if(COACHZP == TrainInfo_p.CoachNum_U8)
    {
         /*zongpei coach ,Coil Chan:37 is 53,Chan:38 is 72A,relay:B400-115EG*/
        ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_175H1,CONT_CHAN_103B,3,NO_RELAY);
        /*zongpei  coach ,Coil Chan:39 is 158B,Chan:40 is 158A,relay:H3CR-H8L*/
        ChanOpertTimeCal(ChanStatInfo_ST_p,COIL_CHAN_175H3,CONT_CHAN_103D,4,NO_RELAY);

    }
   
}
/**********************************************************************
*Name			:   int8_t ChanDataLowLevelCorrect_1Coach
*Function       :   if the ChanVolStatus_U8 is low,the voltage is {-10V ~ 10V},clear the Voltage value
*Para     		:   uint8_t ChanNumTmp
*
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/02/22  Create
*********************************************************************/
int8_t ChanDataLowLevelCorrect(CHAN_DATA InChanData[],uint8_t ChanNumTmp,uint8_t EADSType) 
{
	uint8_t i,j;
 	/*AssigValue for channel have voltage and current*/
	for(i = 0;i < VOL_CHAN_NUM;i++)
 	{
	   
	   if(0 == s_ChanJudge_ST.VolChanJudge[i].ChanVolStatus_U8)
	    {
	        //+-5V
	        for(j = 0;j < ChanNumTmp;j++)
	        {
                if((InChanData[j].VolChan_ST[i].CH_Voltage_I16 <= VOL_COR_JUDEG)&&
                    (InChanData[j].VolChan_ST[i].CH_Voltage_I16 >= VOL_NEGA)) //  10V >~ >-10V 
	        	{
	        		InChanData[j].VolChan_ST[i].CH_Voltage_I16 = 0;
	        	}
	        	
	        }
	    }
 	}/* end of for(i = 0;i < VOL_CHAN_NUM;i++)*/
	// if(CTU_BOARD_ID == EADSType)
	// {
	// 	for(i = 0;i < CURR_CHAN_NUM;i++)
 // 		{
	// 		if(1 == s_ChanJudge_ST.CurrChanJudge[i].ChanNormCurrFlag_U8)
	// 	    {
	// 	        //+-5V
	// 	        for(j = 0;j < ChanNumTmp;j++)
	// 	        {
	//                 if((InChanData[j].CurrChan_ST[i].CH_Current_I16 <= CURR_COR_JUDEG)&&
	//                     (InChanData[j].CurrChan_ST[i].CH_Current_I16 >= CURR_NEGA)) //  5mA >~ >-10mA
	// 	        	{
	// 	        		InChanData[j].CurrChan_ST[i].CH_Current_I16 = 0;
	// 	        	}
		        	
	// 	        }
	// 	    }
	// 	}

	// }	
}

/**********************************************************************
*Name			:   int8_t ChanSendDataBuf
*Function       :   copy the chandata to TCP sendbuf,to avoid proc and send the same data source  
*Para     		:   uint8_t ChanNumTmp  
*
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/07/09  Create
*
*********************************************************************/
int8_t ChanSendDataBuf(CHAN_DATA * InChanDatap,CHAN_DATA * SendBufChanDatap,int8_t ChanNumTmp)
{
	uint8_t i = 0;
	char loginfo[LOG_INFO_LENG] = {0};
	if(0 == ChanNumTmp)
	{
		printf("ChanNum %u \n",ChanNumTmp);
        snprintf(loginfo, sizeof(loginfo)-1, "ChanNum %u",ChanNumTmp);
        WRITELOGFILE(LOG_WARN_1,loginfo);
		return CODE_WARN;
	}
	//should and lock
	for(i = 0;i < ChanNumTmp; i++)
	{
		memcpy(SendBufChanDatap,InChanDatap,sizeof(CHAN_DATA));	
		InChanDatap++;
		SendBufChanDatap++;	
	}
	return CODE_OK;

}
/**********************************************************************
*Name			:   ChanRealDataBuf(uint8_t ChanNumTmp)
*Function       :   Buffer the chandata to ChanRealDataBuf,
                    to avoid memcpy cost too much time,Optimize by using pointer for FIFO move address,
                    the test result is ok.
*Para     		:   uint8_t ChanNumTmp
*
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/5/16  Create
*
*********************************************************************/
int8_t ChanRealDataBuf(CHAN_DATA InChanDatap[],CHAN_DATA RelBufChanDatap[],uint8_t ProcNum,uint16_t RecBeforNum)
{
	uint8_t i = 0;
	uint32_t memSize_U32 = 0;
	char loginfo[LOG_INFO_LENG] = {0};
	if(RecBeforNum > REAL_BUFFER_NUM)
	{
		printf("RecBeforNum %u > REAL_BUFFER_NUM %u \n", RecBeforNum,REAL_BUFFER_NUM);
        snprintf(loginfo, sizeof(loginfo)-1, "RecBeforNum %u > REAL_BUFFER_NUM %u",RecBeforNum,REAL_BUFFER_NUM);
        WRITELOGFILE(LOG_ERROR_1,loginfo);
		return CODE_ERR;
	}

	if(0 == ProcNum)
	{
		printf("ChanNum %u \n",ProcNum);
        snprintf(loginfo, sizeof(loginfo)-1, "ChanNum %u",ProcNum);
        WRITELOGFILE(LOG_WARN_1,loginfo);
		return CODE_WARN;
	}
	//增加到400，清零
	if(g_RealBufNum_U32 >= RecBeforNum) 
	{
		g_RealBufNum_U32 -= RecBeforNum;
		   
	}
	else
	{		
		
	}
	memSize_U32 = sizeof(CHAN_DATA) * ProcNum ;
	memcpy(&RelBufChanDatap[g_RealBufNum_U32],&InChanDatap[0],memSize_U32);	
	g_RealBufNum_U32  += ProcNum; /*at here for realfile save*/
	return CODE_OK;

}
/**********************************************************************
*Name			:   int8_t ChanDataFilt
*Function       :   Filt the data,filt ChanNumTmp frames to average value,  for the eventfile save,data send,
*					save to  g_ChanData_ProcBuf_ST[BufNum],then save to Event File
**Para     		:   uint8_t BufNum  :g_ChanData_ProcBuf_ST[BufNum]
*					uint8_t ChanNumTmp : frames number
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/02/22  Create
*********************************************************************/
int8_t ChanDataFilt(CHAN_DATA *InChdata,CHAN_DATA *OutChData,uint8_t ProcNum,uint8_t EADSType)
{
	uint8_t i,j;
	uint32_t TemVol = 0,TemCur = 0;
	uint16_t ChanBit = 0;
	char loginfo[LOG_INFO_LENG] = {0};
	if(ProcNum > 0)
	{
		ChanBit = (ProcNum >> 1);
	}
	else 
	{
        printf("ChanNum is %u,cannot divide \n",ProcNum);
        snprintf(loginfo, sizeof(loginfo)-1, "ChanNum is %u,cannot divide",ProcNum);
        WRITELOGFILE(LOG_ERROR_1,loginfo);
		return CODE_ERR;
	}
 	/*对四帧数据求平均值*/
	for(i = 0;i <VOL_CHAN_NUM;i++)
 	{
	    TemVol = 0; 
		for(j = 0;j < ProcNum;j++)
		{
			TemVol +=  InChdata -> VolChan_ST[i].CH_Voltage_I16;
			InChdata++;
        }
        OutChData -> VolChan_ST[i].CH_Voltage_I16 = TemVol >> ChanBit;        
		InChdata -= ProcNum;
 	}
	if(EADSType ==  CTU_BOARD_ID)
	{
        for(i = 0;i <CURR_CHAN_NUM;i++)
	 	{
	        TemCur = 0;
		    for(j = 0;j < ProcNum;j++)
	        {
	        	TemCur += InChdata -> CurrChan_ST[i].CH_Current_I16;
	        	InChdata++;
	        }
	        OutChData -> CurrChan_ST[i].CH_Current_I16 = TemCur >> ChanBit;	         
		    InChdata -= ProcNum;

	 	}
	}
    return CODE_OK;
}
/**********************************************************************
*Name           :   void DigitalStatusSet(CHAN_DIGITAL_INFO * ChanDigitalInfop,const CHAN_STATUS_INFO * ChanInfop)
*Function       :   Set the Channel analog data to digital statu for save and logic judge 
*Para           :   CHAN_DIGITAL_INFO * ChanDigitalInfop,
                    const CHAN_STATUS_INFO * ChanInfop,
                    uint8_t EADSType_U8
*
*Return         :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/04/22  Creat
*REV1.0.1     feng    2020/05/18  add filt,when ChanInfop -> VolWarnFlag_U8 is 1 for 500ms,to tell the TCMS, when over current should tell as soon as possible
                                  add EADSType judge
*********************************************************************/
void DigitalStatusSet(CHAN_DIGITAL_INFO * ChanDigitalInfop,const CHAN_STATUS_INFO * ChanInfop,uint8_t EADSType_U8,EADS_ERROR_INFO * EADSErrInfop)
{
    
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t warTemp =0 ;
    char loginfo[LOG_INFO_LENG] = {0};
    static uint16_t s_VolWarFiltNum[VOL_CHAN_NUM] = {0};
	static uint16_t s_VolWarClearFiltNum[VOL_CHAN_NUM] = {0};
    static uint16_t s_CurrWarFiltNum[CURR_CHAN_NUM] = {0};
    static uint16_t s_VolWarFlagTemp[VOL_CHAN_NUM] = {0};
    static uint16_t s_CurrWarFlagTemp[CURR_CHAN_NUM] = {0};

    /*vol*/

    for(i = 0;i < VOL_CHAN_NUM;i++)
    {    
        if(1 == ChanInfop -> VolWarnFlag_U8[i])
        {
            s_VolWarFiltNum[i]++;
            if(s_VolWarFiltNum[i] >= CH_DIGT_JUDGE_NUM) /*> 500ms set*/
            {
                if(0 == s_VolWarFlagTemp[i])
                {
                    s_VolWarFlagTemp[i] = 1;
                    printf("Ch %d VolWarnBit set\n",i); 
                    snprintf(loginfo, sizeof(loginfo)-1, "Ch %d VolWarnBit set",i);
                    WRITELOGFILE(LOG_WARN_1,loginfo);
                }
                s_VolWarFiltNum[i] = 0;
            }
			s_VolWarClearFiltNum[i] = 0;
        }
        else
        {
            s_VolWarClearFiltNum[i]++;
			if(s_VolWarClearFiltNum[i] >= CH_DIGT_JUDGE_NUM) /*> 500ms set*/
			{
				if(1 == s_VolWarFlagTemp[i])
				{
					s_VolWarFlagTemp[i] = 0; 
					printf("Ch %d VolWarnBit clear\n",i); 
					snprintf(loginfo, sizeof(loginfo)-1, "Ch %d VolWarnBit clear",i);
					WRITELOGFILE(LOG_WARN_1,loginfo);
				}
				s_VolWarClearFiltNum[i] = 0;

			}
			s_VolWarFiltNum[i] = 0;
        }
        warTemp |= s_VolWarFlagTemp[i];

    }

    for(i = 0;i < VOL_STATUE_NUM;i++)
    {
       
        j = i << 3;
        ChanDigitalInfop -> VolWarnFlag_8CH[i].Bit0 = s_VolWarFlagTemp[j];
        ChanDigitalInfop -> VolWarnFlag_8CH[i].Bit1 = s_VolWarFlagTemp[j + 1];
        ChanDigitalInfop -> VolWarnFlag_8CH[i].Bit2 = s_VolWarFlagTemp[j + 2];
        ChanDigitalInfop -> VolWarnFlag_8CH[i].Bit3 = s_VolWarFlagTemp[j + 3];
        ChanDigitalInfop -> VolWarnFlag_8CH[i].Bit4 = s_VolWarFlagTemp[j + 4];
        ChanDigitalInfop -> VolWarnFlag_8CH[i].Bit5 = s_VolWarFlagTemp[j + 5];
        ChanDigitalInfop -> VolWarnFlag_8CH[i].Bit6 = s_VolWarFlagTemp[j + 6];
        ChanDigitalInfop -> VolWarnFlag_8CH[i].Bit7 = s_VolWarFlagTemp[j + 7];
        // printf("VolChanStat_U8 %d,%d\n",j,ChanInfop -> VolChanStat_U8[j]);
        // printf("VolChanStat_8CH %d,%d\n",j,ChanDigitalInfop -> VolChanStat_8CH[i].Bit0);
        ChanDigitalInfop -> VolChanStat_8CH[i].Bit0 = ChanInfop -> VolChanStat_U8[j];
        ChanDigitalInfop -> VolChanStat_8CH[i].Bit1 = ChanInfop -> VolChanStat_U8[j + 1];
        ChanDigitalInfop -> VolChanStat_8CH[i].Bit2 = ChanInfop -> VolChanStat_U8[j + 2];
        ChanDigitalInfop -> VolChanStat_8CH[i].Bit3 = ChanInfop -> VolChanStat_U8[j + 3];
        ChanDigitalInfop -> VolChanStat_8CH[i].Bit4 = ChanInfop -> VolChanStat_U8[j + 4];
        ChanDigitalInfop -> VolChanStat_8CH[i].Bit5 = ChanInfop -> VolChanStat_U8[j + 5];
        ChanDigitalInfop -> VolChanStat_8CH[i].Bit6 = ChanInfop -> VolChanStat_U8[j + 6];
        ChanDigitalInfop -> VolChanStat_8CH[i].Bit7 = ChanInfop -> VolChanStat_U8[j + 7];
    }
    if(warTemp)
    {
        EADSErrInfop -> VolWarmErr = 1;
    }
    else 
    {
        EADSErrInfop -> VolWarmErr = 0;
    }
    
    warTemp = 0;
    if(CTU_BOARD_ID == EADSType_U8)
    {
         for(i = 0;i < CURR_CHAN_NUM;i++)
        {
           
            if(1 == ChanInfop -> CurrWarnFlag_U8[i])
            {
                s_CurrWarFiltNum[i]++;
                if(s_CurrWarFiltNum[i] >= CH_DIGT_JUDGE_NUM) /*> 500ms set*/
                {
                    if(0 == s_CurrWarFlagTemp[i])
                    {
                        s_CurrWarFlagTemp[i] = 1;
                        printf("Ch %d CurrWarnBit set\n",i); 
                        snprintf(loginfo, sizeof(loginfo)-1, "Ch %d CurrWarnBit set\n",i);
                        WRITELOGFILE(LOG_WARN_1,loginfo);
                    }
                    s_CurrWarFiltNum[i] = 0;

                }
            }
            else
            {
                s_CurrWarFiltNum[i] = 0;
                if(1 == s_CurrWarFlagTemp[i]) 
                {
                    s_CurrWarFlagTemp[i] = 0; 
                    printf("Ch %d CurrWarnBit clear\n",i); 
                    snprintf(loginfo, sizeof(loginfo)-1, "Ch %d CurrWarnBit clear\n",i);
                    WRITELOGFILE(LOG_WARN_1,loginfo);
                }
            }
            warTemp |= s_CurrWarFlagTemp[i];
        
        }
        if(warTemp)
        {
            EADSErrInfop -> CurrWarmErr = 1;
        }
        else 
        {
            EADSErrInfop -> CurrWarmErr = 0;
        }

        ChanDigitalInfop -> CurrWarnFlag_8CH[0].Bit0 = ChanInfop -> CurrWarnFlag_U8[0];
        ChanDigitalInfop -> CurrWarnFlag_8CH[0].Bit1 = ChanInfop -> CurrWarnFlag_U8[1];
        ChanDigitalInfop -> CurrWarnFlag_8CH[0].Bit2 = ChanInfop -> CurrWarnFlag_U8[2];
        ChanDigitalInfop -> CurrWarnFlag_8CH[0].Bit3 = ChanInfop -> CurrWarnFlag_U8[3];
        ChanDigitalInfop -> CurrWarnFlag_8CH[0].Bit4 = ChanInfop -> CurrWarnFlag_U8[4];
        ChanDigitalInfop -> CurrWarnFlag_8CH[0].Bit5 = ChanInfop -> CurrWarnFlag_U8[5];
        ChanDigitalInfop -> CurrWarnFlag_8CH[0].Bit6 = ChanInfop -> CurrWarnFlag_U8[6];
        ChanDigitalInfop -> CurrWarnFlag_8CH[0].Bit7 = ChanInfop -> CurrWarnFlag_U8[7];

        /*the  over Curr*/
        ChanDigitalInfop -> CurrWarnFlag_8CH[1].Bit0 = ChanInfop -> CurrWarnFlag_U8[8];
        ChanDigitalInfop -> CurrWarnFlag_8CH[1].Bit1 = ChanInfop -> CurrWarnFlag_U8[9];
        ChanDigitalInfop -> CurrWarnFlag_8CH[1].Bit2 = ChanInfop -> CurrWarnFlag_U8[10];
    }
}

/**********************************************************************
*Name			:   int8_t JudgeTimeIntgal(uint8_t ChanNumTmpP)
*Function       :   Judge Time Intgal
*Para     		:   uint8_t ChanNumTmpP : the frame number
*                   uint8_t EADSType_U8
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2021/03/11  Create
*REV1.0.1     feng    2021/05/06  Add judge MAX==0
*********************************************************************/
float JudgeTimeIntgal(uint16_t Min, uint16_t Max,uint16_t ValueIn)
{
    float Tmp = 0.0;
	char loginfo[LOG_INFO_LENG] = {0};
	static s_ParMcaxErrFlag = 0;
    if((Max <= Min) && (!s_ParMcaxErrFlag))
   	{  
		printf("Error the Max < Min \n");
        snprintf(loginfo, sizeof(loginfo)-1, "Error the Max < Min,max :%d,min: %d  ",Max,Min);
        WRITELOGFILE(LOG_ERROR_1,loginfo);
		s_ParMcaxErrFlag = 1;
		return CODE_ERR;
	}
    if(ValueIn <= Min)
    {
		Tmp =  0.0;
	}
	else if(ValueIn <= Max)
	{
     	Tmp =(float) (ValueIn - Min) / (Max - Min);
	}
	else
	{
	    Tmp =  1.0;
	}
	return Tmp;

}
/**********************************************************************
*Name			:   int8_t RelayLifePrect(uint8_t ChanNumTmpP)
*Function       :   Predict the Relay Life
*Para     		:   uint8_t ChanNumTmpP : the frame number
*                   uint8_t EADSType_U8
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2021/03/11  Create

*********************************************************************/
int8_t RelayLifePrect(CHAN_STATUS_INFO * ChanInfop,RELAY_LIFE_CFG *LifeCfg_p)
{
     uint8_t i = 0;
	 uint8_t Index = 0;
	char loginfo[LOG_INFO_LENG] = {0};
	static uint8_t s_LifeLogFlag[VOL_CHAN_NUM] = {0};

	 float TimeTmpRslt = 0.0;
	 float TmpRslt = 0.0;
	 float LifeNumRslt = 0.0;
	 float UpTimeRslt = 0.0;
	 float DownTimeRslt = 0.0;
	 float JumpRslt = 0.0;
	 Index = 0; //change the Index for different type rela, Relay_Para_Config.xml include two Type 
	 if(!LifeCfg_p ->  RelayChanCfg_ST[Index].RelayOprtLife_U32)
	 {
		 LifeCfg_p ->  RelayChanCfg_ST[Index].RelayOprtLife_U32 = XML_LIFE_NUM_MIN;

	 }
     for(i = 0;i< VOL_CHAN_NUM;i++)
     {
		 LifeNumRslt = 	(float) ChanInfop -> Channel_OperaNum_U32[i] / LifeCfg_p ->  RelayChanCfg_ST[Index].RelayOprtLife_U32;
		 UpTimeRslt = JudgeTimeIntgal(LifeCfg_p -> RelayChanCfg_ST[Index].UpOprtTimeMin_U16, LifeCfg_p -> RelayChanCfg_ST[Index].UpOprtTimeMax_U16,ChanInfop -> OprtUpTime_U16[i]);
		 DownTimeRslt = JudgeTimeIntgal(LifeCfg_p -> RelayChanCfg_ST[Index].DownOprtTimeMin_U16, LifeCfg_p -> RelayChanCfg_ST[Index].DownOprtTimeMax_U16,ChanInfop -> OprtDownTime_U16[i]);
		 JumpRslt = JudgeTimeIntgal(LifeCfg_p -> RelayChanCfg_ST[Index].JumpTimeMin_U16, LifeCfg_p -> RelayChanCfg_ST[Index].JumpTimeMax_U16,ChanInfop -> JumpTime_U16[i]);
         TimeTmpRslt = LifeCfg_p -> UpOprtTimeFactor * UpTimeRslt;
		 TimeTmpRslt += LifeCfg_p -> DownOprtTimeFactor * DownTimeRslt;
		 TimeTmpRslt += LifeCfg_p -> JumpTimeFactor * JumpRslt;
		 TimeTmpRslt += LifeCfg_p -> RelayNumFactor * LifeNumRslt;
		 TmpRslt = 1 - TimeTmpRslt;
		 if(TmpRslt < 0)
		 {
			TmpRslt = 0;
		 }
		 ChanInfop -> RelayLastLife_U16[i] = (uint16_t)(TmpRslt * 10000);

		 if((TmpRslt < 0.1) && ( !s_LifeLogFlag[i]))
		 {	
			 printf("Chan %d Life is danger\n",i);
			 snprintf(loginfo, sizeof(loginfo)-1, "Chan %d Life is danger",i);
			 WRITELOGFILE(LOG_WARN_1,loginfo);
			 s_LifeLogFlag[i] = 1;

		 }
		 if(DATPRO_DEBUG == g_DebugType_EU)
		 {
			 printf("Chan Life %d\n",i);
			 printf("LifeNumRslt %f\n",LifeNumRslt);
			 printf("UpTimeRslt %f\n",UpTimeRslt);
			 printf("DownTimeRslt %f\n",DownTimeRslt);
			 printf("JumpRslt %f\n",JumpRslt);
			 printf("TmpRslt %f\n",TmpRslt);	 
			 printf("ChanInfop -> RelayLastLife_U16 %d\n", ChanInfop -> RelayLastLife_U16[i]);	 

		 }

	 }

}
/**********************************************************************
*Name			:   int8_t EADSDataProc(uint8_t ChanNumTmpP)
*Function       :   Proce the EADS data 
*Para     		:   uint8_t ChanNumTmpP : the frame number
*                   uint8_t EADSType_U8
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/04/22  Create
*REV1.0.1     feng    2020/5/18  shield the ChanDataLowLevelCorrect()
                                 move DigitalStatusSet() from EADSdatBuf to here
*REV 1.0.2     feng   2021/4/8   after run 50ms,then calculate the operate time and jump time 
*********************************************************************/
int8_t EADSDataProc(uint8_t ProcNum,uint8_t EADSType_U8,EADS_ERROR_INFO * EADSErrInfop)
{
 
	uint8_t i = 0;
	static uint8_t s_FirstRunTime = 0;

	if(0 == ProcNum)
	{
		printf("The Proce ChanNum is %u\n",ProcNum);
		return CODE_ERR;
	}

	//电压、电流数据转化
	pthread_rwlock_wrlock(&g_PthreadLock_ST.ChanDatalock);
	pthread_rwlock_rdlock(&g_PthreadLock_ST.BramDatalock); 	
	VolChanDataTrans(&g_BrdRdBufData_ST,&g_ChanData_ST[0],&g_ChanCalib0VData_ST,&g_ChanCalib110VData_ST,ProcNum,NORMTRANTYPE);
	if(EADSType_U8 == CTU_BOARD_ID)
	{		
		CurrChanDataTrans(&g_BrdRdBufData_ST,&g_ChanData_ST[0],ProcNum,NORMTRANTYPE);		
	} 
	pthread_rwlock_unlock(&g_PthreadLock_ST.BramDatalock);	
	pthread_rwlock_unlock(&g_PthreadLock_ST.ChanDatalock);    
	//通道数据判断
	pthread_rwlock_rdlock(&g_PthreadLock_ST.ChanDatalock);
	AllChanDataJudge(&g_ChanData_ST[0],ProcNum,EADSType_U8);
	pthread_rwlock_unlock(&g_PthreadLock_ST.ChanDatalock);
	//如果第一次运行s_FirstRunFlag==0，则50次循环后，将s_FirstRunFlag置1
	if(!s_FirstRunFlag)
	{
		s_FirstRunTime++;
		if(s_FirstRunTime >= DIGIT_PRO_TIME)
		{
		   s_FirstRunFlag = 1;
		}
	}
	//判断过压、过流  
    ChanDataProce(ProcNum,EADSType_U8);	 
    pthread_rwlock_wrlock(&g_PthreadLock_ST.ChanInfolock);
	/*must before ChanDataAssigValue,because the ChanOperUpFlag_U8 not clear*/
	//确定动作时间
	OpertTimeAssigValue(&g_ChanStatuInfo_ST,g_TrainInfo_ST);
	//确定动作次数、电压电流通道状态及故障标志位、动作标志位
	ChanDataAssigValue(&g_ChanStatuInfo_ST,ProcNum,EADSType_U8);
	/*ChanDataAssigValue will operate ++*/
	//继电器寿命分析
	RelayLifePrect(&g_ChanStatuInfo_ST,&g_ChanRelayCfg_ST);
	if(CTU_BOARD_ID == EADSType_U8)
	{
		OverCurrWarnCal(&g_ChanStatuInfo_ST);//过流判断
	}
	pthread_rwlock_unlock(&g_PthreadLock_ST.ChanInfolock);
	//将模拟量转为数字量，来判断车辆逻辑故障
	//DigitalStatusSet(&g_ChanDigitalInfo_ST,&g_ChanStatuInfo_ST,EADSType_U8,EADSErrInfop);	
}

/**********************************************************************
*Name			:   int8_t EADSDataBuf(uint8_t ChanNumTmp)
*Function       :   Buf the Send and RealSave data

*Para     		:   uint8_t ChanNumTmpP 
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/02/22  Create
*REV1.0.1     feng    2020/03/22  
                      1.add rwlock, shield the ChanSendDataBuf
*********************************************************************/
int8_t EADSDataBuf(uint8_t ProcNum)
{
	/*avoid not send,and the data is update*/
	if(0 == ProcNum)
	{
		printf("The Buf ChanNum is %u\n",ProcNum);
		return CODE_ERR;
	}	
	pthread_rwlock_wrlock(&g_PthreadLock_ST.RealDatalock);
	ChanRealDataBuf(g_ChanData_ST,g_ChanRealBuf_ST,ProcNum,g_Rec_XML_ST.Rec_Real_ST.RecBeforeNum);//change for  1k  buffer      	
	pthread_rwlock_unlock(&g_PthreadLock_ST.RealDatalock);

}
#endif
