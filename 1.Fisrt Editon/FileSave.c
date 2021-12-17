/**********************************************************************
*File name    :     FileSave.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2020/1/29
*Description    :   Include  Event ,RealFlt,RealOprt,OprtNum File creat,save
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/1/29  Create
*********************************************************************/

/**********************************************************************
*
*Debug switch  section
*
*********************************************************************/


/**********************************************************************
*
*Include file  section
*
*********************************************************************/
#include "FileSave.h"

/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/
//extern CHAN_THREHOLD_VALUE g_channel_ThreValue_ST;
//20200328ssdsd
extern DEBUG_TYPE_ENUM g_DebugType_EU;
extern FILE_FD   g_FileFd_ST;
extern RECORD_XML g_Rec_XML_ST;
extern TRAIN_INFO g_TrainInfo_ST;
extern PTHREAD_LOCK g_PthreadLock_ST;
extern SPACE_JUDGE_VALUE  g_SpaceJudge_ST;

extern CHAN_DATA g_ChanData_ST[CHAN_BUFFER_NUM];
extern CHAN_DATA  g_ChanRealBuf_ST[REAL_BUFFER_NUM];
extern CHAN_DATA  g_ChanSendBuf_ST[CHAN_BUFFER_NUM];  /*304 byte*/
//extern CHAN_LG_INFO g_ChanLgInfo_ST;

extern EADS_ERROR_INFO     g_EADSErrInfo_ST;
//extern VECH_EADS_INFO      g_CCU_EADsInfo_ST;

extern uint32_t g_LinuxDebug;
//extern uint32_t g_RealBufNum_U32;

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

/**********************************************************************
*
*Function Define Section
*
*********************************************************************/
#if 0
/**********************************************************************
*Name     	  :   FileWriteWithTry
*Function     :   write with try 调用fwrite函数对文件进行写数据，并进行判断，若失败则进行尝试（3次）
*Para         :  
*
*Return       :   int8_t 0,success;-1 false.
*Version      :   REV1.0.0       
*Author:      :   feng
*History:
*REV1.0.0     feng    2020/8/26  Create
*********************************************************************/
int8_t FileWriteWithTry(const void *Indata,uint16_t WriteSize,uint8_t WriteLength,FILE *fd)
{

    uint8_t TryNum = 0,i;
    int8_t  Err = 0;
    uint8_t FileWrite = 0;
    char loginfo[LOG_INFO_LENG] = {0};

    if(NULL ==  fd)
    {
        perror("File fp  is NULL");
        snprintf(loginfo, sizeof(loginfo)-1, "fp is NULL");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    FileWrite = fwrite(Indata,WriteSize,WriteLength,fd);
    if(FileWrite == WriteLength)
    {
        Err = CODE_OK;
    }
    else
    {
        TryNum++;
    }
    
    while((TryNum > 0)&&(TryNum < FILETRY_NUM))
    {
        FileWrite = fwrite(Indata,WriteSize,WriteLength,fd);  
        if(FileWrite != WriteLength)
        {
            TryNum ++;
            perror("File data Write failde ,try again");
            snprintf(loginfo, sizeof(loginfo)-1, "File data Write failed,try again %m");
            WRITELOGFILE(LOG_WARN_1,loginfo);
        }
        else
        {
            TryNum = 0;/*quit the while*/
        }
    }
    if(TryNum >= FILETRY_NUM)
    {
        perror("File data Write failde");
        snprintf(loginfo, sizeof(loginfo)-1, "File data Write failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        Err  = CODE_ERR; 
    }
    return Err;
}
/**********************************************************************
*Name     :   EventFileTopSet
*Function       :   Set the Event File Top Data According Record.xml 
*Para         :   (TRAIN_INFO * TranInfo_ST_p, RECORD_XML * RecXm_p,DRIVE_FILE_TOP  *DriveTop_ST_p)
*
*Return       :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2020/3/7  Create
*********************************************************************/
int8_t EventFileTopSet(TRAIN_INFO * TranInfo_ST_p, RECORD_XML * RecXm_p,DRIVE_FILE_TOP  *DriveTop_ST_p)
{   
    int i;
    time_t timep_ST;
    struct tm *now_time_p_ST;
    time(&timep_ST); 
    now_time_p_ST = localtime(&timep_ST); /*change to the local time*/

    DriveTop_ST_p ->RecEnDianType_U16 = 0x01; //small End
    DriveTop_ST_p ->RecHeadSize_U16=0x82; // the top size by byte
    DriveTop_ST_p ->RecHeadVer_U16=100; 
    DriveTop_ST_p ->RecordType_U16 = RecXm_p -> Rec_Event_ST.RecTypeIdx; //  TYPE 1=Event 2=Log 3=Fast 4=Real
    DriveTop_ST_p ->ProjectNo_U16 = 0x00;
    DriveTop_ST_p ->DCUType_U16 = 0x00;
    DriveTop_ST_p ->VehicleNo_U16 = TranInfo_ST_p -> CoachNum_U8;
    DriveTop_ST_p ->DCUNO_U16 = 0x00;
    DriveTop_ST_p ->RecordTotalChnNum_U16 = RecXm_p -> Rec_Event_ST.RecTotalChn_U8; //96 number
    DriveTop_ST_p ->RecordChnNum_U16 = RecXm_p -> Rec_Event_ST.RecChnNum_U8 ;      //Event chanal num is 8 +70=78
    DriveTop_ST_p ->RecordInterval_U16= RecXm_p -> Rec_Event_ST.RecInterval;
      
    DriveTop_ST_p ->ChnMask1_U16 = RecXm_p -> Rec_Event_ST.RecChnBitMask1; 
    DriveTop_ST_p ->ChnMask2_U16 = RecXm_p -> Rec_Event_ST.RecChnBitMask2;// Analog signal
    DriveTop_ST_p ->ChnMask3_U16 = RecXm_p -> Rec_Event_ST.RecChnBitMask3;
    DriveTop_ST_p ->ChnMask4_U16 = RecXm_p -> Rec_Event_ST.RecChnBitMask4;
    DriveTop_ST_p ->ChnMask5_U16 = RecXm_p -> Rec_Event_ST.RecChnBitMask5;
    DriveTop_ST_p ->ChnMask6_U16 = RecXm_p -> Rec_Event_ST.RecChnBitMask6;

    DriveTop_ST_p ->RecordBefore_U16= RecXm_p -> Rec_Event_ST.RecBeforeNum;  //fault-before point number
    DriveTop_ST_p ->RecordAfter_U16= RecXm_p ->  Rec_Event_ST.RecAfterNum;    //fault-after point number
    DriveTop_ST_p ->RecordTotal_U16= RecXm_p -> Rec_Event_ST.RecToTalNum;   //wave count number/ log count number /not event count number

     //uint8_T
    DriveTop_ST_p ->RecordTime_MT= (1 + now_time_p_ST->tm_mon);
    DriveTop_ST_p ->RecordTime_YY = (uint8_t)((1900 + now_time_p_ST->tm_year) - 2000);
    DriveTop_ST_p ->RecordTime_HH = (now_time_p_ST->tm_hour);
    DriveTop_ST_p ->RecordTime_DD = (now_time_p_ST->tm_mday);
    DriveTop_ST_p ->RecordTime_SS = (now_time_p_ST->tm_sec);
    DriveTop_ST_p ->RecordTime_MN = (now_time_p_ST->tm_min);
    for(i=0;i<16;i++)
    {
      DriveTop_ST_p ->EnvData_U16[i] = 0x00;
      DriveTop_ST_p ->FaultCode_U16[i] = 0x00;
    }

    DriveTop_ST_p ->HavePulse_U16=0x00;
    DriveTop_ST_p ->RecordTotalChnNumPulse_U16=0x00;
    DriveTop_ST_p ->RecordChnNumPulse_U16=0x00;;
    DriveTop_ST_p ->RecordIntervalPulse_U16=0x00;

    DriveTop_ST_p ->PulseChnMask1_U16=0x00;
    DriveTop_ST_p ->PulseChnMask2_U16=0x00;
    DriveTop_ST_p ->PulseChnMask3_U16=0x00;
    DriveTop_ST_p ->PulseChnMask4_U16=0x00;
    DriveTop_ST_p ->PulseChnMask5_U16=0xEEEE;
    DriveTop_ST_p ->PulseChnMask6_U16=0xEEEE;
    return CODE_OK;
}
/**********************************************************************
*Name           :   peripheralFileTopSet
*Function       :   Set the Event File Top Data According Record.xml 
*Para 1         :   RECORD_TYPE_CFG *REC_TYPE
*Para 2         :   TRAIN_INFO *TranInfo_ST_p
*Para 3         :   DRIVE_FILE_TOP  *DriveTop_ST_p
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2020/3/7  Create
*********************************************************************/
int8_t  peripheralFileTopSet(TRAIN_INFO *TranInfo_ST_p , RECORD_TYPE_CFG *REC_TYPE , DRIVE_FILE_TOP  *DriveTop_ST_p)
{   
    int i;
    time_t timep_ST;
    struct tm *now_time_p_ST;
    time(&timep_ST); 
    now_time_p_ST = localtime(&timep_ST); /*change to the local time*/

    DriveTop_ST_p ->RecEnDianType_U16 = 0x01; //small End
    DriveTop_ST_p ->RecHeadSize_U16=0x82; // the top size by byte
    DriveTop_ST_p ->RecHeadVer_U16=100; 
    DriveTop_ST_p ->RecordType_U16 = 0x1; //由于外设记录周期与EVENT类似，RECTypeIdx强制为EVENTtype
    DriveTop_ST_p ->ProjectNo_U16 = 0x00;
    DriveTop_ST_p ->DCUType_U16 = 0x00;
    DriveTop_ST_p ->VehicleNo_U16 = TranInfo_ST_p -> CoachNum_U8;
    DriveTop_ST_p ->DCUNO_U16 = 0x00;
    DriveTop_ST_p ->RecordTotalChnNum_U16 = REC_TYPE -> RecTotalChn_U8; //16数字+80模拟
    DriveTop_ST_p ->RecordChnNum_U16 = REC_TYPE -> RecChnNum_U8 ;    
    DriveTop_ST_p ->RecordInterval_U16= REC_TYPE ->RecInterval;
      
    DriveTop_ST_p ->ChnMask1_U16 = REC_TYPE ->RecChnBitMask1;// Digtal signal[16位代表16个chan，每个chan包含16个通道]
    DriveTop_ST_p ->ChnMask2_U16 = REC_TYPE ->RecChnBitMask2;// Analog signal
    DriveTop_ST_p ->ChnMask3_U16 = REC_TYPE ->RecChnBitMask3;// Analog signal
    DriveTop_ST_p ->ChnMask4_U16 = REC_TYPE ->RecChnBitMask4;// Analog signal
    DriveTop_ST_p ->ChnMask5_U16 = REC_TYPE ->RecChnBitMask5;// Analog signal
    DriveTop_ST_p ->ChnMask6_U16 = REC_TYPE ->RecChnBitMask6;// Analog signal

    DriveTop_ST_p ->RecordBefore_U16= REC_TYPE ->RecBeforeNum;  //fault-before point number
    DriveTop_ST_p ->RecordAfter_U16 = REC_TYPE ->RecAfterNum;   //fault-after point number
    DriveTop_ST_p ->RecordTotal_U16 = REC_TYPE ->RecToTalNum;   //wave count number/ log count number /not event count number

    //8位
    DriveTop_ST_p ->RecordTime_MT= (1 + now_time_p_ST->tm_mon);
    DriveTop_ST_p ->RecordTime_YY = (uint8_t)((1900 + now_time_p_ST->tm_year) - 2000);
    DriveTop_ST_p ->RecordTime_HH = (now_time_p_ST->tm_hour);
    DriveTop_ST_p ->RecordTime_DD = (now_time_p_ST->tm_mday);
    DriveTop_ST_p ->RecordTime_SS = (now_time_p_ST->tm_sec);
    DriveTop_ST_p ->RecordTime_MN = (now_time_p_ST->tm_min);
    for(i=0;i<16;i++)
    {
      DriveTop_ST_p ->EnvData_U16[i] = 0x00;//环境数据
      DriveTop_ST_p ->FaultCode_U16[i] = 0x00;//故障代码
    }
    //脉冲信号相关
    DriveTop_ST_p ->HavePulse_U16=0x00;
    DriveTop_ST_p ->RecordTotalChnNumPulse_U16=0x00;
    DriveTop_ST_p ->RecordChnNumPulse_U16=0x00;;
    DriveTop_ST_p ->RecordIntervalPulse_U16=0x00;

    DriveTop_ST_p ->PulseChnMask1_U16=0x00;
    DriveTop_ST_p ->PulseChnMask2_U16=0x00;
    DriveTop_ST_p ->PulseChnMask3_U16=0x00;
    DriveTop_ST_p ->PulseChnMask4_U16=0x00;
    DriveTop_ST_p ->PulseChnMask5_U16=0xEEEE;
    DriveTop_ST_p ->PulseChnMask6_U16=0xEEEE;
    return CODE_OK;
}
/**********************************************************************
*Name     :   int8_t EventFileTopSave(void)
*Function       :   Save Event File Top  to event file
*               :   
*Para         :   
*Return       :   uint8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/1/7  Create
*********************************************************************/
int8_t EventFileTopSave(FILE *Fd_p, RECORD_XML * RecXm_p,TRAIN_INFO * TranInfo_ST_p)
{
    int8_t fwerr = 0;
    DRIVE_FILE_TOP DriveEventTop_ST = {0};
    char loginfo[LOG_INFO_LENG] = {0};
    EventFileTopSet(TranInfo_ST_p,RecXm_p,&DriveEventTop_ST);

    fwerr = FileWriteWithTry(&DriveEventTop_ST,sizeof(DRIVE_FILE_TOP),1,Fd_p);
    if(CODE_OK != fwerr)
    {
        printf("fwrite Event Top error");
        snprintf(loginfo, sizeof(loginfo)-1, "fwrite  Event Top error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    fflush(Fd_p);
    return fwerr;
}
/**********************************************************************
*Name           :   int8_t peripheralFileTopSave(void)
*Function       :   Save Event File Top  to event file
*               :   
*Para           :   
*Return         :   uint8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/1/7  Create
*********************************************************************/
int8_t  peripheralFileTopSave(FILE *device_FP,RECORD_TYPE_CFG *REC_TYPE,TRAIN_INFO *TranInfo_ST_p)
{
    int8_t fwerr = 0;
    DRIVE_FILE_TOP DriveEventTop_ST = {0};
    char loginfo[LOG_INFO_LENG] = {0};
    peripheralFileTopSet(TranInfo_ST_p,REC_TYPE,&DriveEventTop_ST);

    fwerr = FileWriteWithTry(&DriveEventTop_ST,sizeof(DRIVE_FILE_TOP),1,device_FP);
    if(CODE_OK != fwerr)
    {
        printf("Write peripheralFile Top error!\n");
        snprintf(loginfo, sizeof(loginfo)-1, "Write peripheralFile Top error!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    fflush(device_FP);
    return fwerr;
}
/**********************************************************************
*Name         :   int8_t EventFileCreateByNum(void)
*Function     :   Creat Event file by Eventsave Num,For PTU analysis
                  if the remain mmc space not enough  will delete the earliest file 
*Para         :   void 
*
*Return       :   int8_t 0,success;else false.
*Version      :   REV1.0.0       
*Author:      :   feng
*
*History:
*REV1.0.0     feng    2020/6/29  Create
*********************************************************************/
int8_t EventLifeFileCreateByNum(FILE_FD * file_p,RECORD_XML * RrdXml_p,TRAIN_INFO * TranInfo_p,EADS_ERROR_INFO  *EADSErrInfo_ST)
{ 
    //FILE *fd;
    uint8_t File_EventName_U8[200] = {0};
    uint8_t File_Directory_U8[200] = {0};
    uint8_t TimeString_U8[40] = {0};
    int8_t err = 0;
    uint8_t TryNum = 0;
    uint32_t TotalSizeMB_U32 = 0,FreeSizeMB_U32 = 0;
    time_t timep_ST;   
    struct tm *now_time_p_ST;
    char loginfo[LOG_INFO_LENG] = {0};
    static uint8_t s_EventPowOnDataString[10] = {0};

    time(&timep_ST);
    now_time_p_ST = localtime(&timep_ST); /*change to the local time*/
    sprintf(TimeString_U8,"%04d%02d%02d%02d%02d%02d",
          (1900 + now_time_p_ST->tm_year), 
          (1 + now_time_p_ST->tm_mon), now_time_p_ST->tm_mday,
          now_time_p_ST->tm_hour, now_time_p_ST->tm_min, now_time_p_ST->tm_sec); 
        // add year month folder
    if(0 == g_LinuxDebug)
    {
        sprintf(File_Directory_U8,"%s",RrdXml_p-> Rec_Event_ST.RecPath); 
   
    }
    //for ubuntu linux test 
    else
    {
        sprintf(File_Directory_U8,"%s","/home/feng/Desktop/xilinx_code/yaffs/REC_EVTLIFE/");//for ubuntu linux test  
   
    }  
    /*a new day*/
    if(strncmp(s_EventPowOnDataString,TimeString_U8,8) < 0)
    {
        //powon not run
        memcpy(s_EventPowOnDataString,TimeString_U8,8);
        err = MultiDircCreate(File_Directory_U8);
        if(CODE_CREAT == err)
        {
            memset(loginfo,0,sizeof(loginfo));
            snprintf(loginfo, sizeof(loginfo)-1, "make Dir %s success",File_Directory_U8);
            WRITELOGFILE(LOG_INFO_1,loginfo); 
        }
		DeleteEarliestFile(File_Directory_U8,LOG_FILE_TYPE);//>100,delete  
    }
    snprintf(File_EventName_U8,sizeof(File_EventName_U8),"%s/%s%02d_%8.8s-%s.dat",File_Directory_U8,
        RrdXml_p->Rec_Event_ST.RecFileHead,TranInfo_p -> CoachNum_U8,TimeString_U8,&TimeString_U8[8]);

    //g_fd_EventName = fopen(File_EventName_U8, "a+"); 
    file_p -> EventLifeFile_fd = fopen(File_EventName_U8, "a+");
    if(NULL ==  file_p ->EventLifeFile_fd)
    {
        TryNum ++;
        perror("creat EventLifefile.dat file failed");
    }
    else
    {
        EADSErrInfo_ST -> EADSErr = 0;
        printf("creat file %s\n",File_EventName_U8);
        snprintf(loginfo, sizeof(loginfo)-1, "creat file %s",File_EventName_U8);
        WRITELOGFILE(LOG_INFO_1,loginfo);        
    } 
    while((TryNum > 0)&&(TryNum < FILETRY_NUM))
    {
        file_p -> EventLifeFile_fd = fopen(File_EventName_U8, "a+"); //every time creat the file and write replace   
        if(NULL ==  file_p -> EventLifeFile_fd )
        {
            TryNum ++;
            perror("creat EventLifefile.dat File failed again");
        }
        else
        {
            TryNum = 0;/*quit the while*/
            EADSErrInfo_ST -> EADSErr = 0;
            printf("creat file %s\n",File_EventName_U8);
            snprintf(loginfo, sizeof(loginfo)-1, "creat  file %s",File_EventName_U8);
            WRITELOGFILE(LOG_INFO_1,loginfo);    
        }

    }
    if(TryNum >= FILETRY_NUM)
    {
        
        EADSErrInfo_ST -> EADSErr = 1;
        snprintf(loginfo, sizeof(loginfo)-1, "creat Eventfile failed %m");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        err  = CODE_ERR;
        return err;
    }
    err = EventFileTopSave(file_p ->EventLifeFile_fd,RrdXml_p,TranInfo_p);
    return err;

}

/**********************************************************************
*Name         :   int8_t EventFileCreateByNum(void)
*Function     :   Creat Event file by Eventsave Num,For PTU analysis
                  if the remain mmc space not enough  will delete the earliest file 
*Para         :   void 
*
*Return       :   int8_t 0,success;else false.
*Version      :   REV1.0.0       
*Author:      :   feng
*
*History:
*REV1.0.0     feng    2020/6/29  Create
*********************************************************************/
int8_t EventFileCreateByNum(FILE_FD * file_p,RECORD_XML * RrdXml_p,TRAIN_INFO * TranInfo_p,EADS_ERROR_INFO  *EADSErrInfo_ST)
{ 
    //FILE *fd;
    uint8_t File_EventName_U8[200] = {0};
    uint8_t File_Directory_U8[200] = {0};
    uint8_t TimeString_U8[40] = {0};
    int8_t err = 0;
    uint8_t TryNum = 0;
    uint32_t TotalSizeMB_U32 = 0,FreeSizeMB_U32 = 0;
    time_t timep_ST;   
    struct tm *now_time_p_ST;
    char loginfo[LOG_INFO_LENG] = {0};
    static uint8_t s_EventPowOnDataString[10] = {0};

    time(&timep_ST);
    now_time_p_ST = localtime(&timep_ST); /*change to the local time*/
    sprintf(TimeString_U8,"%04d%02d%02d%02d%02d%02d",
          (1900 + now_time_p_ST->tm_year), 
          (1 + now_time_p_ST->tm_mon), now_time_p_ST->tm_mday,
          now_time_p_ST->tm_hour, now_time_p_ST->tm_min, now_time_p_ST->tm_sec); 
        // add year month folder
    if(0 == g_LinuxDebug)
    {
        sprintf(File_Directory_U8,"%s%8.8s",RrdXml_p-> Rec_Event_ST.RecPath,TimeString_U8); 
   
    }
    //for ubuntu linux test 
    else
    {
        sprintf(File_Directory_U8,"%s%8.8s","/home/feng/Desktop/xilinx_code/yaffs/REC_EVTDATA/",TimeString_U8);//for ubuntu linux test  
   
    }  
    /*a new day*/
    if(strncmp(s_EventPowOnDataString,TimeString_U8,8) < 0)
    {
        //powon not run
        if(strlen(s_EventPowOnDataString) != 0) 
        {
			FileSpaceProc(&g_Rec_XML_ST);//for new day
			LogFileCreatePowOn();
            printf("after LogFileCreatePowOn\n");

        }
        memcpy(s_EventPowOnDataString,TimeString_U8,8);        

         //should make sure
        if(0 == g_LinuxDebug)
        {
            GetMemSize(RrdXml_p -> Rec_Event_ST.RecPath,&TotalSizeMB_U32,&FreeSizeMB_U32);
            if(FreeSizeMB_U32 < g_SpaceJudge_ST.EVENT_RESER_SPACE)//whether alway delete until the size is meet the size 
            {
               printf("%s free %uMB,Rrq %uMB \n",RrdXml_p -> Rec_Event_ST.RecPath,FreeSizeMB_U32,g_SpaceJudge_ST.EVENT_RESER_SPACE);
               snprintf(loginfo, sizeof(loginfo)-1, "%s free %uMB,Rrq %uMB",RrdXml_p -> Rec_Event_ST.RecPath,FreeSizeMB_U32,g_SpaceJudge_ST.EVENT_RESER_SPACE);
               WRITELOGFILE(LOG_WARN_1,loginfo);
			   err = FileDirJudge(RrdXml_p -> Rec_Event_ST.RecPath);
		       if(REC_FILE_TYPE == err)
		       {
		           DeleteEarliestFile(RrdXml_p -> Rec_Event_ST.RecPath,RECORD_FILE_TYPE);

		       }
		       else if(REC_DIR_TYPE == err)
		       {
		           DeleteEarliestDir(RrdXml_p -> Rec_Event_ST.RecPath);

		       }
     
               // DeleteEarliestDir(RrdXml_p -> Rec_Event_ST.RecPath);
            }
        }
        //for ubuntu linux test 
        else
        {
            //DeleteEarliestDir("/home/feng/Desktop/xilinx_code/yaffs/REC_EVTDATA");
            DeleteEarliestFile("/home/feng/Desktop/xilinx_code/yaffs/REC_EVTDATA/",RECORD_FILE_TYPE);

        }
        err = MultiDircCreate(File_Directory_U8);
        if(CODE_CREAT == err)
        {
            memset(loginfo,0,sizeof(loginfo));
            snprintf(loginfo, sizeof(loginfo)-1, "make Dir %s success",File_Directory_U8);
            WRITELOGFILE(LOG_INFO_1,loginfo); 
        }
    }
    snprintf(File_EventName_U8,sizeof(File_EventName_U8),"%s/%s%02d_%8.8s-%s.dat",File_Directory_U8,
        RrdXml_p->Rec_Event_ST.RecFileHead,TranInfo_p -> CoachNum_U8,TimeString_U8,&TimeString_U8[8]);

    //g_fd_EventName = fopen(File_EventName_U8, "a+"); 
    file_p -> EventFile_fd = fopen(File_EventName_U8, "a+");
    if(NULL ==  file_p ->EventFile_fd)
    {
        TryNum ++;
        perror("creat Eventfile.dat file failed");
    }
    else
    {
        EADSErrInfo_ST -> EADSErr = 0;
        printf("creat file %s\n",File_EventName_U8);
        snprintf(loginfo, sizeof(loginfo)-1, "creat file %s",File_EventName_U8);
        WRITELOGFILE(LOG_INFO_1,loginfo);        
    } 
    while((TryNum > 0)&&(TryNum < FILETRY_NUM))
    {
        file_p -> EventFile_fd = fopen(File_EventName_U8, "a+"); //every time creat the file and write replace   
        if(NULL ==  file_p -> EventFile_fd )
        {
            TryNum ++;
            perror("creat Eventfile.dat File failed again");
        }
        else
        {
            TryNum = 0;/*quit the while*/
            EADSErrInfo_ST -> EADSErr = 0;
            printf("creat file %s\n",File_EventName_U8);
            snprintf(loginfo, sizeof(loginfo)-1, "creat  file %s",File_EventName_U8);
            WRITELOGFILE(LOG_INFO_1,loginfo);    
        }

    }
    if(TryNum >= FILETRY_NUM)
    {
        
        EADSErrInfo_ST -> EADSErr = 1;
        snprintf(loginfo, sizeof(loginfo)-1, "creat Eventfile failed %m");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        err  = CODE_ERR;
        return err;
    }
    err = EventFileTopSave(file_p ->EventFile_fd,RrdXml_p,TranInfo_p);
    return err;

}
/**********************************************************************
*Name         :   int8_t peripheralFileCreate(void)
*Function     :   创建外设数据记录文件，可通过PTU解析
*Para         :   void 
*Return       :   int8_t 0,success;else false.
*Version      :   REV1.0.0       
*Author:      :   feng
*
*History:
*REV1.0.0     feng    2020/6/29  Create
*********************************************************************/
int8_t peripheralFileCreate(peripheralDevice Device_Type_enum,FILE **device_FP,RECORD_TYPE_CFG *Rcdtype,TRAIN_INFO *TranInfo_p,EADS_ERROR_INFO *EADSErrInfo_ST)
{ 
    
    uint8_t File_Name_U8[200] = {0};
    uint8_t File_Directory_U8[200] = {0};
    uint8_t TimeString_U8[40] = {0};    
    uint8_t TryNum = 0,err = 0;
    uint32_t TotalSizeMB_U32 = 0,FreeSizeMB_U32 = 0;
    time_t timep_ST;   
    struct tm *now_time_p_ST;
    char loginfo[LOG_INFO_LENG] = {0};
    static uint8_t s_CANPowOnData[10] = {0};
    static uint8_t s_UartPowOnData[10] = {0};       
    time(&timep_ST);//返回时间（单位s），从1970-00-00-00-00-00至目前的差值
    now_time_p_ST = localtime(&timep_ST); /*赋值给tm结构体*/
    //添加时间信息
    sprintf(TimeString_U8,"%04d%02d%02d%02d%02d%02d",(1900 + now_time_p_ST->tm_year), 
          (1 + now_time_p_ST->tm_mon), now_time_p_ST->tm_mday,now_time_p_ST->tm_hour, 
          now_time_p_ST->tm_min, now_time_p_ST->tm_sec);
    //%m.ns：输出占m列，但只取字符串中左端n个字符。这n个字符输出在m列的右侧，左补空格       
    sprintf(File_Directory_U8,"%s%8.8s",Rcdtype->RecPath,TimeString_U8);
    
    switch (Device_Type_enum)
    {  
        
        case CAN_Device:              
        if(strncmp(s_CANPowOnData,TimeString_U8,8) < 0)//比较日期
        {
            //代表不是刚上电，而是进入到新的一天
            if(strlen(s_CANPowOnData) != 0) 
            {
			    FileSpaceProc(&g_Rec_XML_ST);//for new day
			    LogFileCreatePowOn();//创建日志
                printf("A new day,Creat a new logfile!\n");
            }        
            memcpy(s_CANPowOnData,TimeString_U8,8);//时间信息加到文件名中
            //判断剩余容量，若容量小于规定值，则删除最旧的文件        
            if(0 == g_LinuxDebug)
            {
                GetMemSize(Rcdtype->RecPath,&TotalSizeMB_U32,&FreeSizeMB_U32);     
                if(FreeSizeMB_U32 < g_SpaceJudge_ST.CAN_RESER_SPACE)
                {
                    printf("%s free %uMB,Rrq %uMB \n",Rcdtype->RecPath,FreeSizeMB_U32,g_SpaceJudge_ST.CAN_RESER_SPACE);
                    snprintf(loginfo, sizeof(loginfo)-1, "%s free %uMB,Rrq %uMB",Rcdtype->RecPath,FreeSizeMB_U32,g_SpaceJudge_ST.CAN_RESER_SPACE);
                    WRITELOGFILE(LOG_WARN_1,loginfo);
			        err = FileDirJudge(Rcdtype->RecPath);
		            if(REC_FILE_TYPE == err)
		            {
		                DeleteEarliestFile(Rcdtype->RecPath,RECORD_FILE_TYPE);
		            }
		            else if(REC_DIR_TYPE == err)
		            {
		                 DeleteEarliestDir(Rcdtype->RecPath);
		            }   
                }    
            }      
            err = MultiDircCreate(File_Directory_U8);//目录
            if(CODE_CREAT == err)
            {            
                memset(loginfo,0,sizeof(loginfo));
                snprintf(loginfo, sizeof(loginfo)-1, "make Dir %s success",File_Directory_U8);
                WRITELOGFILE(LOG_INFO_1,loginfo); 
            }
        }
        break;
        case Uart_Device:        
        if(strncmp(s_UartPowOnData,TimeString_U8,8) < 0)//比较日期
        {
            //代表不是刚上电，而是进入到新的一天
            if(strlen(s_UartPowOnData) != 0) 
            {
			    FileSpaceProc(&g_Rec_XML_ST);//for new day
			    LogFileCreatePowOn();//创建日志
                printf("A new day,Creat a new logfile!\n");
            }        
            memcpy(s_UartPowOnData,TimeString_U8,8);//时间信息加到文件名中
            //判断剩余容量，若容量小于规定值，则删除最旧的文件        
            if(0 == g_LinuxDebug)
            {
                GetMemSize(Rcdtype->RecPath,&TotalSizeMB_U32,&FreeSizeMB_U32);     
                if(FreeSizeMB_U32 < g_SpaceJudge_ST.UART_RESER_SPACE)
                {
                    printf("%s free %uMB,Rrq %uMB \n",Rcdtype->RecPath,FreeSizeMB_U32,g_SpaceJudge_ST.UART_RESER_SPACE);
                    snprintf(loginfo, sizeof(loginfo)-1, "%s free %uMB,Rrq %uMB",Rcdtype->RecPath,FreeSizeMB_U32,g_SpaceJudge_ST.UART_RESER_SPACE);
                    WRITELOGFILE(LOG_WARN_1,loginfo);
			        err = FileDirJudge(Rcdtype->RecPath);
		            if(REC_FILE_TYPE == err)
		            {
		                DeleteEarliestFile(Rcdtype->RecPath,RECORD_FILE_TYPE);
		            }
		            else if(REC_DIR_TYPE == err)
		            {
		                 DeleteEarliestDir(Rcdtype->RecPath);
		            }   
                }    
            }             
            err = MultiDircCreate(File_Directory_U8);//目录
            if(CODE_CREAT == err)
            {            
                memset(loginfo,0,sizeof(loginfo));
                snprintf(loginfo, sizeof(loginfo)-1, "make Dir %s success",File_Directory_U8);
                WRITELOGFILE(LOG_INFO_1,loginfo); 
            }
        }
        break;
        default:
        break;
    }
    //文件名
    snprintf(File_Name_U8,sizeof(File_Name_U8),"%s/%s%02d_%8.8s-%s.dat",File_Directory_U8,
        Rcdtype->RecFileHead,TranInfo_p -> CoachNum_U8,TimeString_U8,&TimeString_U8[8]);
    //打开一个用于读取和追加的文件。写操作向文件末尾追加数据。如果文件不存在,则创建文件。
    *device_FP = fopen(File_Name_U8, "a+");
    if(NULL ==  *device_FP)
    {
        TryNum ++;
        printf("creat %s file failed %d times!\n",File_Name_U8,TryNum);
    }
    else
    {
        EADSErrInfo_ST -> EADSErr = 0;//故障清零
        printf("creat file %s success!\n",File_Name_U8);
        snprintf(loginfo, sizeof(loginfo)-1, "creat file %s success!",File_Name_U8);
        WRITELOGFILE(LOG_INFO_1,loginfo);        
    } 
    while((TryNum > 0)&&(TryNum < FILETRY_NUM))
    {
        *device_FP = fopen(File_Name_U8, "a+"); //every time creat the file and write replace   
        if(NULL ==  *device_FP )
        {
            TryNum ++;
            printf("creat %s file failed %d times!\n",File_Name_U8,TryNum);
        }
        else
        {
            TryNum = 0;
            EADSErrInfo_ST -> EADSErr = 0;
            printf("creat file %s success!\n",File_Name_U8);
            snprintf(loginfo, sizeof(loginfo)-1, "creat file %s success!",File_Name_U8);
            WRITELOGFILE(LOG_INFO_1,loginfo);    
        }

    }
    if(TryNum >= FILETRY_NUM)
    {        
        EADSErrInfo_ST -> EADSErr = 1;
        snprintf(loginfo, sizeof(loginfo)-1, "creat %s file finally failed ,return err!",File_Name_U8);
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        err  = CODE_ERR;
        return err;
    }
    err = peripheralFileTopSave(*device_FP,Rcdtype,TranInfo_p);
    return err;

}

/**********************************************************************
*Name       :   int8_t EventDataSet
*Function   :   Set the Event data,include vol digital statue,volwarn,currwarn,
                        EADS err info,Logic info ,ccu info,
                        oprt up and oprt down time
*Para       :   DRIVE_FILE_DATA  *Drive_ST_p   the address of Save DriveEventST
*                   CHAN_DATA *g_ChanData_ST_p   the address of Save ChanData_ST
                EADSType:CTU_BOARD_ID 1
                         ADU_BOARD_ID 0
*Return     :   int8_t 0,success;else false.
*Version    :   REV1.0.0       
*Author:    :   feng

*History:
*REV1.0.0     feng    2020/8/29  Create
*REV1.0.1     feng    2020/5/11  vol and curr Change to mencpy
                                 add Digital save
*********************************************************************/
int8_t EventRelayLifeSet(DRIVE_FILE_DATA * Drive_ST_p,uint8_t EADSType,const CHAN_STATUS_INFO ChanInfop)
{
  
    uint8_t j= 0;
    uint8_t i = 0;    
	for(i = 0;i <VOL_CHAN_NUM;i++)
	{ 
		Drive_ST_p -> DriveAnalog_U16[i] = ChanInfop.RelayLastLife_U16[i];
		Drive_ST_p -> DriveAnalog_U16[i+40] = ChanInfop.JumpTime_U16[i] ;
	}

    if(g_DebugType_EU == FILE_DEBUG)
    {
        for(i = 0;i < VOL_CHAN_NUM;i++)
        {                  
            printf("EVENT_Life DriveAnalog_U16[%u]:%u\n",i,Drive_ST_p -> DriveAnalog_U16[i]);
            printf("EVENT_Life DriveAnalog_U16[%u]:%u\n",i+40,Drive_ST_p -> DriveAnalog_U16[i+40]);
        }
    }
    return CODE_OK;
}

/**********************************************************************
*Name       :   int8_t EventDataSet
*Function   :   Set the Event data,include vol digital statue,volwarn,currwarn,
                        EADS err info,Logic info ,ccu info,
                        oprt up and oprt down time
*Para       :   DRIVE_FILE_DATA  *Drive_ST_p   the address of Save DriveEventST
*                   CHAN_DATA *g_ChanData_ST_p   the address of Save ChanData_ST
                EADSType:CTU_BOARD_ID 1
                         ADU_BOARD_ID 0
*Return     :   int8_t 0,success;else false.
*Version    :   REV1.0.0       
*Author:    :   feng

*History:
*REV1.0.0     feng    2020/8/29  Create
*REV1.0.1     feng    2020/5/11  vol and curr Change to mencpy
                                 add Digital save
*REV1.0.2     feng    2021/4/14  optimize the volwarnFlag save for save  CCU data
                                                              
*********************************************************************/
int8_t EventDataSet(DRIVE_FILE_DATA  *Drive_ST_p,uint8_t EADSType,const EADS_ERROR_INFO  EADSErrInfoST,
                   const CHAN_LG_INFO LgInfoST,const CHAN_DIGITAL_INFO ChanDigitalInfo_ST,const CHAN_STATUS_INFO ChanInfop)
{
  
    uint8_t j= 0;
    uint8_t i = 0;
    	
    memcpy(&Drive_ST_p -> DriveDigital_U8[0],ChanDigitalInfo_ST.VolChanStat_8CH,VOL_STATUE_NUM);
    memcpy(&Drive_ST_p -> DriveDigital_U8[5],ChanDigitalInfo_ST.VolWarnFlag_8CH,VOL_STATUE_NUM);
    memcpy(&Drive_ST_p -> DriveDigital_U8[10],ChanDigitalInfo_ST.CurrWarnFlag_8CH,CURR_STATUE_NUM);
    memcpy(&Drive_ST_p -> DriveDigital_U8[12],&EADSErrInfoST,1);
    memcpy(&Drive_ST_p -> DriveDigital_U8[14],&LgInfoST,8);
    memcpy(&Drive_ST_p -> DriveDigital_U8[22],&g_CCU_EADsInfo_ST,sizeof(VECH_EADS_INFO));
    if(g_DebugType_EU == FILE_DEBUG)
    {
        for(j = 0;j < DIGITAL_NUM_BYTE;j++)
        { 
            printf("Event DriveDigital_U8 %u : %u \n",j,Drive_ST_p -> DriveDigital_U8[j]);   
        }
    }
    /*note DriveAnalog_U16[] number is  51 , !!!! note  not include the oprte time */
    Drive_ST_p -> DriveAnalog_U16[0] = ChanInfop.OprtUpTime_U16[CONT_CHAN_T883] ;
    Drive_ST_p -> DriveAnalog_U16[1] = ChanInfop.OprtUpTime_U16[CONT_CHAN_106K] ;    
    Drive_ST_p -> DriveAnalog_U16[2] = ChanInfop.OprtUpTime_U16[CONT_CHAN_123A] ;
    Drive_ST_p -> DriveAnalog_U16[3] = ChanInfop.OprtUpTime_U16[CONT_CHAN_103B] ;
    Drive_ST_p -> DriveAnalog_U16[4] = ChanInfop.OprtUpTime_U16[CONT_CHAN_103D] ;
    Drive_ST_p -> DriveAnalog_U16[5] = ChanInfop.OprtDownTime_U16[CONT_CHAN_T883] ;
    Drive_ST_p -> DriveAnalog_U16[6] = ChanInfop.OprtDownTime_U16[CONT_CHAN_106K] ;    
    Drive_ST_p -> DriveAnalog_U16[7] = ChanInfop.OprtDownTime_U16[CONT_CHAN_123A] ;
    Drive_ST_p -> DriveAnalog_U16[8] = ChanInfop.OprtDownTime_U16[CONT_CHAN_103B] ;
    Drive_ST_p -> DriveAnalog_U16[9] = ChanInfop.OprtDownTime_U16[CONT_CHAN_103D] ;
    return CODE_OK;
}


/**********************************************************************
*Name	   :   int8_t EventDataSet
*Function   :   Set the Event data,include vol digital statue,volwarn,currwarn,
					   EADS err info,Logic info ,ccu info,
					   oprt up and oprt down time
*Para	   :   DRIVE_FILE_DATA	*Drive_ST_p   the address of Save DriveEventST
*				   CHAN_DATA *g_ChanData_ST_p	the address of Save ChanData_ST
			   EADSType:CTU_BOARD_ID 1
						ADU_BOARD_ID 0
*Return	   :   int8_t 0,success;else false.
*Version    :   REV1.0.0 	  
*Author:    :   feng

*History:
*REV1.0.0	 feng	 2020/8/29	Create
*REV1.0.1	 feng	 2020/5/11	vol and curr Change to mencpy
								add Digital save
*********************************************************************/
int8_t EventDataSetForRelayTest(DRIVE_FILE_DATA	*Drive_ST_p,uint8_t EADSType,const EADS_ERROR_INFO  EADSErrInfoST,
				  const CHAN_LG_INFO LgInfoST,const CHAN_DIGITAL_INFO ChanDigitalInfo_ST,const CHAN_STATUS_INFO ChanInfop)//uint16_t length
{
 
   uint8_t j= 0;
   uint8_t i = 0;
   memcpy(&Drive_ST_p -> DriveDigital_U8[0],ChanDigitalInfo_ST.VolChanStat_8CH,VOL_STATUE_NUM);
   memcpy(&Drive_ST_p -> DriveDigital_U8[5],ChanDigitalInfo_ST.VolWarnFlag_8CH,VOL_STATUE_NUM);
   memcpy(&Drive_ST_p -> DriveDigital_U8[10],ChanDigitalInfo_ST.CurrWarnFlag_8CH,CURR_STATUE_NUM);
   memcpy(&Drive_ST_p -> DriveDigital_U8[12],&EADSErrInfoST,1);
   memcpy(&Drive_ST_p -> DriveDigital_U8[14],&LgInfoST,8);
   memcpy(&Drive_ST_p -> DriveDigital_U8[22],&g_CCU_EADsInfo_ST,sizeof(VECH_EADS_INFO));
   if(g_DebugType_EU == FILE_DEBUG)
   {
	   for(j = 0;j < DIGITAL_NUM_BYTE;j++)
	   { 

		   //Drive_ST_p -> DriveAnalog_U16[j] = ChanData_ST_p -> VolChan_ST[j].CH_Voltage_I16 ; 	
			printf("Event DriveDigital_U8 %u : %u \n",j,Drive_ST_p -> DriveDigital_U8[j]);   
	   }
   }
   /*note DriveAnalog_U16[] number is  51 , !!!! note  not include the oprte time */
   Drive_ST_p -> DriveAnalog_U16[0] = ChanInfop.OprtUpTime_U16[CONT_CHAN_20];
   Drive_ST_p -> DriveAnalog_U16[1] = ChanInfop.OprtUpTime_U16[CONT_CHAN_21];
   Drive_ST_p -> DriveAnalog_U16[2] = ChanInfop.OprtUpTime_U16[CONT_CHAN_22];
   Drive_ST_p -> DriveAnalog_U16[3] = ChanInfop.OprtUpTime_U16[CONT_CHAN_23];
   
   Drive_ST_p -> DriveAnalog_U16[4] = ChanInfop.OprtUpTime_U16[CONT_CHAN_24];
   Drive_ST_p -> DriveAnalog_U16[5] = ChanInfop.OprtUpTime_U16[CONT_CHAN_25];
   Drive_ST_p -> DriveAnalog_U16[6] = ChanInfop.OprtUpTime_U16[CONT_CHAN_26];
   Drive_ST_p -> DriveAnalog_U16[7] = ChanInfop.OprtUpTime_U16[CONT_CHAN_27];

   Drive_ST_p -> DriveAnalog_U16[8] = ChanInfop.OprtUpTime_U16[CONT_CHAN_28];
   Drive_ST_p -> DriveAnalog_U16[9] = ChanInfop.OprtUpTime_U16[CONT_CHAN_29];
   Drive_ST_p -> DriveAnalog_U16[10] = ChanInfop.OprtUpTime_U16[CONT_CHAN_30];
   Drive_ST_p -> DriveAnalog_U16[11] = ChanInfop.OprtUpTime_U16[CONT_CHAN_31];
   
   Drive_ST_p -> DriveAnalog_U16[12] = ChanInfop.OprtUpTime_U16[CONT_CHAN_0];
   Drive_ST_p -> DriveAnalog_U16[13] = ChanInfop.OprtUpTime_U16[CONT_CHAN_1];
   Drive_ST_p -> DriveAnalog_U16[14] = ChanInfop.OprtUpTime_U16[CONT_CHAN_2];
   Drive_ST_p -> DriveAnalog_U16[15] = ChanInfop.OprtUpTime_U16[CONT_CHAN_3];
   
   Drive_ST_p -> DriveAnalog_U16[16] = ChanInfop.OprtUpTime_U16[CONT_CHAN_4];
   Drive_ST_p -> DriveAnalog_U16[17] = ChanInfop.OprtUpTime_U16[CONT_CHAN_6];
   Drive_ST_p -> DriveAnalog_U16[18] = ChanInfop.OprtUpTime_U16[CONT_CHAN_7];
   Drive_ST_p -> DriveAnalog_U16[19] = ChanInfop.OprtUpTime_U16[CONT_CHAN_8];

   Drive_ST_p -> DriveAnalog_U16[20] = ChanInfop.OprtDownTime_U16[CONT_CHAN_20];
   Drive_ST_p -> DriveAnalog_U16[21] = ChanInfop.OprtDownTime_U16[CONT_CHAN_21];	  
   Drive_ST_p -> DriveAnalog_U16[22] = ChanInfop.OprtDownTime_U16[CONT_CHAN_22];
   Drive_ST_p -> DriveAnalog_U16[23] = ChanInfop.OprtDownTime_U16[CONT_CHAN_23];

   Drive_ST_p -> DriveAnalog_U16[24] = ChanInfop.OprtDownTime_U16[CONT_CHAN_24];
   Drive_ST_p -> DriveAnalog_U16[25] = ChanInfop.OprtDownTime_U16[CONT_CHAN_25];
   Drive_ST_p -> DriveAnalog_U16[26] = ChanInfop.OprtDownTime_U16[CONT_CHAN_26];
   Drive_ST_p -> DriveAnalog_U16[27] = ChanInfop.OprtDownTime_U16[CONT_CHAN_27];

   Drive_ST_p -> DriveAnalog_U16[28] = ChanInfop.OprtDownTime_U16[CONT_CHAN_28];
   Drive_ST_p -> DriveAnalog_U16[29] = ChanInfop.OprtDownTime_U16[CONT_CHAN_29];  
   Drive_ST_p -> DriveAnalog_U16[30] = ChanInfop.OprtDownTime_U16[CONT_CHAN_30];
   Drive_ST_p -> DriveAnalog_U16[31] = ChanInfop.OprtDownTime_U16[CONT_CHAN_31];

   Drive_ST_p -> DriveAnalog_U16[32] = ChanInfop.OprtDownTime_U16[CONT_CHAN_0];
   Drive_ST_p -> DriveAnalog_U16[13] = ChanInfop.OprtDownTime_U16[CONT_CHAN_1];
   Drive_ST_p -> DriveAnalog_U16[34] = ChanInfop.OprtDownTime_U16[CONT_CHAN_2];
   Drive_ST_p -> DriveAnalog_U16[35] = ChanInfop.OprtDownTime_U16[CONT_CHAN_3];
   
   Drive_ST_p -> DriveAnalog_U16[36] = ChanInfop.OprtDownTime_U16[CONT_CHAN_4];
   Drive_ST_p -> DriveAnalog_U16[37] = ChanInfop.OprtDownTime_U16[CONT_CHAN_6];
   Drive_ST_p -> DriveAnalog_U16[38] = ChanInfop.OprtDownTime_U16[CONT_CHAN_7];
   Drive_ST_p -> DriveAnalog_U16[39] = ChanInfop.OprtDownTime_U16[CONT_CHAN_8];
   return CODE_OK;
}

/**********************************************************************
*Name 	  :   int8_t EventDataSave
*Function   :   Save the Event data  to event file
*Para 	  :   (FILE_FD * file_p,CHAN_DATA * ChanDatap,uint8_t EADSType,const EADS_ERROR_INFO  EADSErrInfoST,
				 const CHAN_LG_INFO LgInfoST,const CHAN_DIGITAL_INFO ChanDigitalInfo_ST,uint16_t InOpTime[])

*Return	  :   int8_t 0,success;else false.
*Version		  :   REV1.0.0		 
*Author:		  :   feng
*History:
*REV1.0.0 	feng	2020/1/29  Create
*REV1.0.1 	feng	2021/1/4   For relay test
*********************************************************************/
int8_t EventDataSaveForRelayTest(FILE_FD * file_p,CHAN_DATA * ChanDatap,uint8_t EADSType,const EADS_ERROR_INFO EADSErrInfoST,
				 const CHAN_LG_INFO LgInfoST,const CHAN_DIGITAL_INFO ChanDigitalInfo_ST,const CHAN_STATUS_INFO ChanInfop)
{
  uint8_t i;
  int16_t fwerr = 0;
  uint16_t WriteSize_U16 = 0;
  DRIVE_FILE_DATA DriveEventData_ST = {0};
  char loginfo[LOG_INFO_LENG] = {0};

  if(NULL == file_p -> EventFile_fd)
  {
	  perror("EventFile fp	is NULL");
	  snprintf(loginfo, sizeof(loginfo)-1, "EventFile fp is NULL");
	  WRITELOGFILE(LOG_ERROR_1,loginfo);
	  return CODE_ERR;
  }
  if(g_DebugType_EU == FILE_DEBUG)
  {
	  printf("EventFilefp %d \n",file_p -> EventFile_fd);

  }
  EventDataSetForRelayTest(&DriveEventData_ST,EADSType, EADSErrInfoST,
				 LgInfoST,ChanDigitalInfo_ST,ChanInfop);
  //when save digital data,should change the DriveEventData_ST[0].Analog[0]
  //fwerr = fwrite(&DriveEventData_ST.DriveAnalog_U16[0],WriteSize_U16,1,file_p -> EventFile_fd);
  fwerr = FileWriteWithTry(&DriveEventData_ST.DriveDigital_U8[0],DIGITAL_NUM_BYTE,1,file_p -> EventFile_fd);
   if(CODE_OK != fwerr)
  {

	  printf("fwrite Digital data  error");
	  snprintf(loginfo, sizeof(loginfo)-1, "fwrite	Digital data error");
	  WRITELOGFILE(LOG_ERROR_1,loginfo);
  }
 // fwerr = fwrite(&DriveEventData_ST.DriveDigital_U16[0],WriteSize_U16,1,file_p -> EventFile_fd);
  if(CTU_BOARD_ID == EADSType)
  {
	   WriteSize_U16 = (VOL_CHAN_NUM + CURR_CHAN_NUM) << 1;
  }
  else
  {
	  WriteSize_U16 = VOL_CHAN_NUM << 1;
  }
  fwerr = FileWriteWithTry(ChanDatap,WriteSize_U16,1,file_p -> EventFile_fd);
  if(CODE_OK != fwerr)
  {
	  printf("fwrite ChanData error");
	  memset(loginfo,0,LOG_INFO_LENG);
	  snprintf(loginfo, sizeof(loginfo)-1, "fwrite ChanData error");
	  WRITELOGFILE(LOG_ERROR_1,loginfo);
  }
  //save oprt time
  WriteSize_U16 = (20 << 2); //write 40 channel anoly
  //printf("LG_ZongPeiDrive %x \n ",LgInfoST.LG_ZongPeiDrive);
  fwerr = FileWriteWithTry(&DriveEventData_ST.DriveAnalog_U16[0],WriteSize_U16,1,file_p -> EventFile_fd);
  if(CODE_OK != fwerr)
  {
	  printf("fwrite  oprt time error");
	  memset(loginfo,0,LOG_INFO_LENG);
	  snprintf(loginfo, sizeof(loginfo)-1, "fwrite	oprt time error");
	  WRITELOGFILE(LOG_ERROR_1,loginfo);

  }
  fflush(file_p -> EventFile_fd);
  return fwerr;

}


/**********************************************************************
*Name       :   int8_t EventDataSave
*Function   :   Save the Event data  to event file
*Para       :   (FILE_FD * file_p,CHAN_DATA * ChanDatap,uint8_t EADSType,const EADS_ERROR_INFO  EADSErrInfoST,
                   const CHAN_LG_INFO LgInfoST,const CHAN_DIGITAL_INFO ChanDigitalInfo_ST,uint16_t InOpTime[])

*Return     :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2020/1/29  Create
*********************************************************************/
int8_t EventDataSave(FILE_FD *file_p,CHAN_DATA * ChanDatap,uint8_t EADSType,const EADS_ERROR_INFO EADSErrInfoST,\
                   const CHAN_LG_INFO LgInfoST,const CHAN_DIGITAL_INFO ChanDigitalInfo_ST,const CHAN_STATUS_INFO ChanInfop)
{
    uint8_t i;
    int16_t fwerr = 0;
    uint16_t WriteSize_U16 = 0;
    DRIVE_FILE_DATA DriveEventData_ST = {0};
    char loginfo[LOG_INFO_LENG] = {0};

    if(NULL == file_p -> EventFile_fd)
    {
        perror("EventFile fp  is NULL");
        snprintf(loginfo, sizeof(loginfo)-1, "EventFile fp is NULL");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    if(g_DebugType_EU == FILE_DEBUG)
    {
        printf("EventFilefp %d \n",file_p -> EventFile_fd);
    }
    EventDataSet(&DriveEventData_ST,EADSType, EADSErrInfoST,LgInfoST,ChanDigitalInfo_ST,ChanInfop);
    //when save digital data,should change the DriveEventData_ST[0].Analog[0]
    fwerr = FileWriteWithTry(&DriveEventData_ST.DriveDigital_U8[0],DIGITAL_NUM_BYTE,1,file_p -> EventFile_fd);
     if(CODE_OK != fwerr)
    {
        printf("fwrite Digital data  error");
        snprintf(loginfo, sizeof(loginfo)-1, "fwrite  Digital data error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }   
    if(CTU_BOARD_ID == EADSType)
    {
         WriteSize_U16 = (VOL_CHAN_NUM + CURR_CHAN_NUM) << 1;
    }
    else
    {
        WriteSize_U16 = (VOL_CHAN_NUM) << 1;
    }
    //记录通道电压、电流经过处理后的模拟量
    fwerr = FileWriteWithTry(ChanDatap,WriteSize_U16,1,file_p -> EventFile_fd);
    if(CODE_OK != fwerr)
    {
        printf("fwrite ChanData error");
        memset(loginfo,0,LOG_INFO_LENG);
        snprintf(loginfo, sizeof(loginfo)-1, "fwrite ChanData error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    //save oprt time
    WriteSize_U16 = (10 << 1);    
    fwerr = FileWriteWithTry(&DriveEventData_ST.DriveAnalog_U16[0],WriteSize_U16,1,file_p -> EventFile_fd);
    if(CODE_OK != fwerr)
    {
        printf("fwrite  oprt time error");
        memset(loginfo,0,LOG_INFO_LENG);
        snprintf(loginfo, sizeof(loginfo)-1, "fwrite  oprt time error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);

    }
    fflush(file_p -> EventFile_fd);
    return fwerr;
}
/**********************************************************************
*Name       :   int8_t EventDataSave
*Function   :   Save the Event data  to event file
*Para       :   (FILE_FD * file_p,CHAN_DATA * ChanDatap,uint8_t EADSType,const EADS_ERROR_INFO  EADSErrInfoST,
                   const CHAN_LG_INFO LgInfoST,const CHAN_DIGITAL_INFO ChanDigitalInfo_ST,uint16_t InOpTime[])

*Return     :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2020/1/29  Create
*********************************************************************/
int8_t EventRelayLifeSave(FILE_FD * file_p,uint8_t EADSType,const CHAN_STATUS_INFO ChanInfop)
{
    uint8_t i;
    int16_t fwerr = 0;
    uint16_t WriteSize_U16 = 0;
    DRIVE_FILE_DATA DriveEventData_ST = {0};
    char loginfo[LOG_INFO_LENG] = {0};

    if(NULL == file_p -> EventLifeFile_fd)
    {
        perror("EventLifeFile_fd fp  is NULL");
        snprintf(loginfo, sizeof(loginfo)-1, "EventFile fp is NULL");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    if(g_DebugType_EU == FILE_DEBUG)
    {
        printf("EventLifeFile_fd %d \n",file_p -> EventLifeFile_fd);
    }
    EventRelayLifeSet(&DriveEventData_ST,EADSType,ChanInfop);
    //when save digital data,should change the DriveEventData_ST[0].Analog[0] 
    WriteSize_U16 = VOL_CHAN_NUM << 2;
    fwerr = FileWriteWithTry(&DriveEventData_ST.DriveAnalog_U16[0],WriteSize_U16,1,file_p -> EventLifeFile_fd);
    if(CODE_OK != fwerr)
    {
        printf("fwrite relay life error");
        memset(loginfo,0,LOG_INFO_LENG);
        snprintf(loginfo, sizeof(loginfo)-1, "fwrite relay life error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    fflush(file_p -> EventLifeFile_fd);
    return fwerr;

}
/**********************************************************************
*Name       :   int8_t peripheralDataSave
*Function   :   保存数据至指定文件，只适用于外设设备。
*Para       :   FILE *device_FS,uint8_t *datebuf, BYTE_BIT Err_info
*Return     :   int8_t 0,success;else false.
*Version    :   REV1.0.0       
*Author:    :   feng
*History:
*REV1.0.0     feng    2021/9/16  Create
*********************************************************************/
int8_t peripheralDataSave(FILE *device_FP,DRIVE_FILE_DATA *databuf)
{    
    int16_t fwerr = 0,i=0;      
    char loginfo[LOG_INFO_LENG] = {0};

    if(NULL == device_FP)
    {
        perror("peripheralDevice FP is NULL!");
        snprintf(loginfo, sizeof(loginfo)-1, "peripheralDevice FP is NULL!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    if(g_DebugType_EU == FileSave_DEBUG)
    {
        printf("Sizeof Record Date:16\n");
        printf("databuf->DriveDigital_U8:\n");
        for(i=0;i<16;i++)
        {
            printf("[%02X]",databuf->DriveDigital_U8[i]);
            if(i+1 == 16)
            {
                printf("\n");
            }
        }
    }      
    //只涉及数字量存储，不涉及模拟量
    fwerr = FileWriteWithTry(&databuf->DriveDigital_U8[0],peri_WriteSize,1,device_FP);
 
    if(CODE_OK != fwerr)
    {
        printf("peripheral DataSave error!");
        snprintf(loginfo, sizeof(loginfo)-1, "peripheral DataSave error!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    fflush(device_FP);
    return fwerr;
}
/**********************************************************************
*Name     :   int8_t DriveRealFileTopSet
*Function       :   Set the Real File Top Data According Record.xml
*               :   
*Para         :   DRIVE_FILE_TOP  *DriveTop_ST_p
*
*Return       :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/1/7  Create
*
*********************************************************************/
int8_t FltRealFileTopSet(TRAIN_INFO * TranInfo_ST_p, RECORD_XML * RecXm_p,DRIVE_FILE_TOP  *DriveTop_ST_p)
{   
  
    int i;
    time_t timep_ST;
    struct tm *now_time_p_ST;
    time(&timep_ST); 
    now_time_p_ST = localtime(&timep_ST); /*change to the local time*/

    DriveTop_ST_p ->RecEnDianType_U16 = 0x01; //small End
    DriveTop_ST_p ->RecHeadSize_U16=0x82; // the top size by byte
    DriveTop_ST_p ->RecHeadVer_U16= 100; 
    DriveTop_ST_p ->RecordType_U16 = RecXm_p -> Rec_Real_ST.RecTypeIdx; //  TYPE 1=Event 2=Log 3=Fast 4=Real
    DriveTop_ST_p ->ProjectNo_U16 = 0x00;
    DriveTop_ST_p ->DCUType_U16 = 0x00;
    DriveTop_ST_p ->VehicleNo_U16 = TranInfo_ST_p -> CoachNum_U8;
    DriveTop_ST_p ->DCUNO_U16 = 0x00;
    DriveTop_ST_p ->RecordTotalChnNum_U16 = RecXm_p -> Rec_Real_ST.RecTotalChn_U8; //96 number
    DriveTop_ST_p ->RecordChnNum_U16 = RecXm_p -> Rec_Real_ST.RecChnNum_U8 ;      //Event chanal num is 8 +70=78
    DriveTop_ST_p ->RecordInterval_U16= RecXm_p -> Rec_Real_ST.RecInterval;
      
    DriveTop_ST_p ->ChnMask1_U16 = RecXm_p -> Rec_Real_ST.RecChnBitMask1; 
    DriveTop_ST_p ->ChnMask2_U16 = RecXm_p -> Rec_Real_ST.RecChnBitMask2;// Analog signal
    DriveTop_ST_p ->ChnMask3_U16 = RecXm_p -> Rec_Real_ST.RecChnBitMask3;
    DriveTop_ST_p ->ChnMask4_U16 = RecXm_p -> Rec_Real_ST.RecChnBitMask4;
    DriveTop_ST_p ->ChnMask5_U16 = RecXm_p -> Rec_Real_ST.RecChnBitMask5;
    DriveTop_ST_p ->ChnMask6_U16 = RecXm_p -> Rec_Real_ST.RecChnBitMask6;

    DriveTop_ST_p ->RecordBefore_U16= RecXm_p -> Rec_Real_ST.RecBeforeNum;  //fault-before point number
    DriveTop_ST_p ->RecordAfter_U16= RecXm_p ->  Rec_Real_ST.RecAfterNum;    //fault-after point number
    DriveTop_ST_p ->RecordTotal_U16= RecXm_p -> Rec_Real_ST.RecToTalNum;   //wave count number/ log count number /not event count number

     //uint8_T
    DriveTop_ST_p ->RecordTime_MT= (1 + now_time_p_ST->tm_mon);
    DriveTop_ST_p ->RecordTime_YY = (uint8_t)((1900 + now_time_p_ST->tm_year) - 2000);
    DriveTop_ST_p ->RecordTime_HH = (now_time_p_ST->tm_hour);
    DriveTop_ST_p ->RecordTime_DD = (now_time_p_ST->tm_mday);
    DriveTop_ST_p ->RecordTime_SS = (now_time_p_ST->tm_sec);
    DriveTop_ST_p ->RecordTime_MN = (now_time_p_ST->tm_min);
    for(i=0;i<16;i++)
    {
        DriveTop_ST_p ->EnvData_U16[i] = 0x00;
        DriveTop_ST_p ->FaultCode_U16[i] = 0x00;
    }

    DriveTop_ST_p ->HavePulse_U16=0x00;
    DriveTop_ST_p ->RecordTotalChnNumPulse_U16=0x00;
    DriveTop_ST_p ->RecordChnNumPulse_U16=0x00;;
    DriveTop_ST_p ->RecordIntervalPulse_U16=0x00;

    DriveTop_ST_p ->PulseChnMask1_U16=0x00;
    DriveTop_ST_p ->PulseChnMask2_U16=0x00;
    DriveTop_ST_p ->PulseChnMask3_U16=0x00;
    DriveTop_ST_p ->PulseChnMask4_U16=0x00;
    DriveTop_ST_p ->PulseChnMask5_U16=0xEEEE;
    DriveTop_ST_p ->PulseChnMask6_U16=0xEEEE;
    return CODE_OK;

}
/**********************************************************************
*Name     :   int8_t FltRealFileTopSave(uint8_t ChanNum)  
*Function       :   write the  Real File Top of Drive,
*               :   
*Para         :   
*Return       :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/7/7  Create
*
*********************************************************************/
int8_t FltRealFileTopSave(FILE *Fd_p, RECORD_XML * RecXm_p,TRAIN_INFO * TranInfo_ST_p)
{

    int8_t fwerr = 0;
    DRIVE_FILE_TOP DriveRealTop_ST = {0};
    char loginfo[LOG_INFO_LENG] = {0};
    FltRealFileTopSet(TranInfo_ST_p,RecXm_p,&DriveRealTop_ST);

    fwerr = FileWriteWithTry(&DriveRealTop_ST,sizeof(DRIVE_FILE_TOP),1,Fd_p);
    if(CODE_OK != fwerr)
    {
        printf("fwrite FltReal Top error");
        snprintf(loginfo, sizeof(loginfo)-1, "fwrite FltReal Top error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    return fwerr;
}
/**********************************************************************
*Name     :   int8_t FltRealFileCreat(uint8_t ChanName)
*Function       :   Creat a new .Flt real file
*Para         :   void 
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/1/29  Create
*REV1.0.1     feng    2020/5/9  Not save valut for EADS test

*********************************************************************/
int8_t FltRealFileCreat(FILE_FD * file_p,RECORD_XML * RrdXml_p,TRAIN_INFO * TranInfo_p,EADS_ERROR_INFO  *EADSErrInfo_ST)
{

    //FILE *fd;
    uint8_t File_FltName_U8[200] = {0};
    uint8_t File_Directory_U8[200] = {0};
    uint8_t TimeString_U8[40] = {0};
    uint8_t err = 0;
    uint8_t TryNum = 0;
    uint32_t TotalSizeMB_U32 = 0,FreeSizeMB_U32 = 0;
    time_t timep_ST;   
    struct tm *now_time_p_ST;
    char loginfo[LOG_INFO_LENG] = {0};
    static uint8_t s_FltPowOnDataString[10] = {0};
   
    time(&timep_ST);
    now_time_p_ST = localtime(&timep_ST); /*change to the local time*/
    sprintf(TimeString_U8,"%04d%02d%02d%02d%02d%02d",
          (1900 + now_time_p_ST->tm_year), 
          (1 + now_time_p_ST->tm_mon), now_time_p_ST->tm_mday,
          now_time_p_ST->tm_hour, now_time_p_ST->tm_min, now_time_p_ST->tm_sec); 
     // add year month folder
    if(0 == g_LinuxDebug)
    {
        sprintf(File_Directory_U8,"%s%8.8s",RrdXml_p-> Rec_Real_ST.RecPath,TimeString_U8); 
   
    }
    //for ubuntu linux test 
    else
    {
        sprintf(File_Directory_U8,"%s%8.8s","/home/feng/Desktop/xilinx_code/yaffs/REC_REALFLT/",TimeString_U8);//for ubuntu linux test  
   
    }
     /*a new day*/
    if(strncmp(s_FltPowOnDataString,TimeString_U8,8) < 0)
    {
        memcpy(s_FltPowOnDataString,TimeString_U8,8);
        /*first tar and get memory*/
        // if(0 == g_LinuxDebug)
        // {
        //     TarDir(RrdXml_p -> Rec_Real_ST.RecPath,File_Directory_U8);
        // }
        // else
        // {
        //     TarDir("/home/feng/Desktop/xilinx_code/yaffs/REC_REALFLT/",File_Directory_U8);
        // }
     //should make sure
        if(0 == g_LinuxDebug)
        {
            GetMemSize(RrdXml_p -> Rec_Real_ST.RecPath,&TotalSizeMB_U32,&FreeSizeMB_U32);
            if(FreeSizeMB_U32 < g_SpaceJudge_ST.FLT_RESER_SPACE)
            {
                printf("%s free %uMB,Rrq %uMB \n",RrdXml_p -> Rec_Real_ST.RecPath,FreeSizeMB_U32,g_SpaceJudge_ST.FLT_RESER_SPACE);
                snprintf(loginfo, sizeof(loginfo)-1, "%s free %uMB,Rrq %uMB",RrdXml_p -> Rec_Real_ST.RecPath,FreeSizeMB_U32,g_SpaceJudge_ST.FLT_RESER_SPACE);
                WRITELOGFILE(LOG_WARN_1,loginfo);
                DeleteEarliestFile(RrdXml_p -> Rec_Real_ST.RecPath,RECORD_FILE_TYPE);
               err = FileDirJudge(RrdXml_p -> Rec_Real_ST.RecPath);
		       if(REC_FILE_TYPE == err)
		       {
		           DeleteEarliestFile(RrdXml_p -> Rec_Real_ST.RecPath,RECORD_FILE_TYPE);

		       }
		       else if(REC_DIR_TYPE == err)
		       {
		           DeleteEarliestDir(RrdXml_p -> Rec_Real_ST.RecPath);

		       }
     
                //DeleteEarliestDir(RrdXml_p -> Rec_Real_ST.RecPath);
            }
        }
        //for ubuntu linux test 
        else
        {

            DeleteEarliestFile("/home/feng/Desktop/xilinx_code/yaffs/REC_REALFLT/",RECORD_FILE_TYPE);
        }
        
        err = MultiDircCreate(File_Directory_U8);
        if(CODE_CREAT == err)
        {
            memset(loginfo,0,sizeof(loginfo));
            snprintf(loginfo, sizeof(loginfo)-1, "make Dir %s success",File_Directory_U8);
            WRITELOGFILE(LOG_INFO_1,loginfo); 
        }

    }
    snprintf(File_FltName_U8,sizeof(File_FltName_U8),"%s/%s%02d_%8.8s-%s.dat",File_Directory_U8,
               RrdXml_p->Rec_Real_ST.RecFileHead,TranInfo_p -> CoachNum_U8,TimeString_U8,&TimeString_U8[8]);

    file_p -> FltRealFile_fd = fopen(File_FltName_U8, "a+");
    if(NULL == file_p ->FltRealFile_fd)
    {
        TryNum ++;
        perror(" creat FltRealfile.dat file failed");
    }
    else
    {
        EADSErrInfo_ST -> EADSErr = 0;
        printf("creat file %s success\n",File_FltName_U8);
        snprintf(loginfo, sizeof(loginfo)-1, "creat file %s",File_FltName_U8);
        WRITELOGFILE(LOG_INFO_1,loginfo);        
    } 
    while((TryNum > 0)&&(TryNum < FILETRY_NUM))
    {
        file_p -> FltRealFile_fd = fopen(File_FltName_U8, "a+"); //every time creat the file and write replace   
        if(NULL ==  file_p -> FltRealFile_fd )
        {
            TryNum ++;
            perror(" creat FltRealfile.dat File failed again");
        }
        else
        {
            TryNum = 0;/*quit the while*/
            EADSErrInfo_ST -> EADSErr = 0;
            printf("creat file %s\n",File_FltName_U8);
            snprintf(loginfo, sizeof(loginfo)-1, "creat file %s",File_FltName_U8);
            WRITELOGFILE(LOG_INFO_1,loginfo);    
        }

    }
    if(TryNum >= FILETRY_NUM)
    {
        
        EADSErrInfo_ST -> EADSErr = 1;
        snprintf(loginfo, sizeof(loginfo)-1, "creat FltRealfile failed %m");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        err  = CODE_ERR;
        return err;
    }
    err =FltRealFileTopSave(file_p ->FltRealFile_fd,RrdXml_p,TranInfo_p);
    return err;
}     
/**********************************************************************
*Name     :   int8_t RealDataSet
*Function       :   Set the menber value of  DriveAnalog_U16 
*Para         :   
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/1/29             Create
*REV1.0.1     feng    2020/5/29             not use
*********************************************************************/
int8_t RealDataSet(DRIVE_FILE_DATA *Drive_ST_p,CHAN_DATA * ChanData_ST_p,uint8_t EADSType)//uint16_t length
{
    uint8_t j= 0;
    uint8_t i = 0;
    for(j = 0;j < VOL_CHAN_NUM;j++)
    { 

        Drive_ST_p -> DriveAnalog_U16[j] = ChanData_ST_p -> VolChan_ST[j].CH_Voltage_I16 ;
        // if(g_DebugType_EU == FILE_DEBUG)
        // {        
        //     printf("Flt DriveAnalog_U16 %u : %u \n",j,Drive_ST_p -> DriveAnalog_U16[j]);   
        // }
    }
    if(EADSType == CTU_BOARD_ID)
    {
        memcpy(&Drive_ST_p -> DriveAnalog_U16[VOL_CHAN_NUM],ChanData_ST_p -> CurrChan_ST,CURR_CHAN_NUM * 2);
        for(j = 0;j < CURR_CHAN_NUM;j++)
        { 
            i =  j + VOL_CHAN_NUM;
            //if(g_DebugType_EU == FILE_DEBUG)
            //{
            //     printf("Flt DriveAnalog_U16 %u : %u \n",j + VOL_CHAN_NUM,Drive_ST_p -> DriveAnalog_U16[i]);

            //}
        }
      
    }
    return CODE_OK;
}
/**********************************************************************
*Name     :   RealDataSave
*Function       :   Save the Real data to of File  
*Para         :   uint8_t ChanName   save which Chan
*                   uint8_t ChanNumTmp the frame number of real data 
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/1/29  Create
                       2020/3/29  dir use inChanData,and not use RealDataSet
*********************************************************************/
int8_t RealDataSave(FILE * file_p,CHAN_DATA inChanData[],uint8_t ChanNumTmp,uint8_t EADSType)
{
    uint8_t i;
    int16_t fwerr = 0;
    uint16_t  WriteSize_U16 =0;
    char loginfo[LOG_INFO_LENG] = {0};

    if(NULL == file_p)
    {
        perror("FltRealFile_fd  is NULL");
        snprintf(loginfo, sizeof(loginfo)-1, "FltRealFile_fd is NULL");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }

    if(CTU_BOARD_ID == EADSType)
    {
         WriteSize_U16 = (VOL_CHAN_NUM + CURR_CHAN_NUM) << 1;
    }
    else
    {
        WriteSize_U16 = VOL_CHAN_NUM << 1;
    }
    for (i = 0; i < ChanNumTmp; i++)//每次写ProcNUM帧数据，本程序为4
    {
        // RealDataSet(&DriveRealData_ST,&inChanData[i],EADSType);
        // fwerr = fwrite(&DriveRealData_ST.DriveAnalog_U16[0],WriteSize_U16,1,file_p); 
        fwerr = FileWriteWithTry(&inChanData[i],WriteSize_U16,1,file_p); 
        if(CODE_OK != fwerr)
        {
            printf("fwrite FltReal File error");
            memset(loginfo,0,LOG_INFO_LENG);
            snprintf(loginfo, sizeof(loginfo)-1, "fwrite FltReal File error");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
        }
    }
    fflush(file_p);
    return fwerr;
}
/**********************************************************************
*Name         :   int8_t RealFileBeforWarnSave
*Function     :   Save the data before the warn happen  
*Para         :   FILE * file_p, CHAN_DATA inChanData[],uint16_t SaveNum,uint32_t NowBufNum,uint8_t EADSType
*Return       :   int8_t 0,success;else false.
*Version      :   REV1.0.0       
*Author:      :   feng
*
*History:
*REV1.0.0     feng    2020/7/29  Create
*********************************************************************/
int8_t RealFileBeforWarnSave(FILE * file_p, CHAN_DATA inChanData[],uint16_t SaveNum,uint32_t NowBufNum,uint8_t EADSType)
{
    uint16_t i;
    int16_t fwerr = 0;
    uint16_t  WriteSize_U16 =0;
    char loginfo[LOG_INFO_LENG] = {0};

    if(NULL == file_p)
    {
        perror("FltRealFile_fd  is NULL");
        snprintf(loginfo, sizeof(loginfo)-1, "FltRealFile_fd is NULL");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }

    if(CTU_BOARD_ID == EADSType)
    {
         WriteSize_U16 = (VOL_CHAN_NUM + CURR_CHAN_NUM) << 1;
    }
    else
    {
        WriteSize_U16 = VOL_CHAN_NUM << 1;
    }
    /*save the nowbufnum to savenum,such as 40~100*/
    for (i = NowBufNum; i < SaveNum; i++)
    {
        //RealDataSet(&DriveRealData_ST,&inChanData[i],EADSType);
       // printf("save CurrChan %d :%d\n",i,inChanData[i].CurrChan_ST[0].CH_Current_I16);
        fwerr = FileWriteWithTry(&inChanData[i],WriteSize_U16,1,file_p);
       // printf("loop %d,vol %d",i,inChanData[i].VolChan_ST[1].CH_Voltage_I16);
        if(CODE_OK != fwerr)
        {
            printf("fwrite Before Warn error");
            memset(loginfo,0,LOG_INFO_LENG);
            snprintf(loginfo, sizeof(loginfo)-1, "fwrite Before Warn error");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
        }
    }
    /*save the 0 to nowbufnum,such as 0~39*/
    for (i = 0; i < NowBufNum; i++)
    {
        //RealDataSet(&DriveRealData_ST,&inChanData[i],EADSType);
       // printf("save CurrChan %d :%d\n",i,inChanData[i].CurrChan_ST[0].CH_Current_I16);
        fwerr = FileWriteWithTry(&inChanData[i],WriteSize_U16,1,file_p);
       // printf("loop %d,vol %d",i,inChanData[i].VolChan_ST[1].CH_Voltage_I16);
        if(CODE_OK != fwerr)
        {
            printf("fwrite Before Warn error");
            memset(loginfo,0,LOG_INFO_LENG);
            snprintf(loginfo, sizeof(loginfo)-1, "fwrite Before Warn error");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
        }  
    }
    return fwerr;
}
#if 0
/**********************************************************************
*Name         :   int8_t FltRealFileProc(uint8_t ChanName,uint8_t ChanNumTmp)
*Function     :   the realfile save when happen warn  
*Para         :   uint8_t ChanName   save which Chan
*                   uint8_t ChanNumTmp the frame number of real data 
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/29  Create
*********************************************************************/
int8_t FltRealFileProc(FILE_FD * File_p, CHAN_STATUS_INFO * InStusInfo_p,RECORD_XML * xml_p,TRAIN_INFO * TranInfo_p,uint8_t ProcNum,uint8_t EADSType)
{ 
// channel happen warm
    int8_t err = 0;
    static uint8_t s_FltRealFileCrteFlag = 0;
    static uint32_t s_FltRealSaveNum_U32 = 0;
	char loginfo[LOG_INFO_LENG] = {0};
    uint32_t FltSaveNumMax = 0;
    uint32_t FltSaveBeforNumMax = 0;
    int8_t fd;
    FltSaveNumMax = xml_p -> Rec_Real_ST.RecToTalNum;
    if(1 == InStusInfo_p -> FltRealRecFlag_U8)
    {
            
        //have creat file
        if(1 == s_FltRealFileCrteFlag)
        {
        //reach the max Rec total number,close file and creat new
//           if(0 == (s_FltRealSaveNum_U32 % FILE_SYNC_NUM))
//           {
//			   fflush(File_p -> FltRealFile_fd);
//			   fd = fileno(File_p -> FltRealFile_fd);
//			   fsync(fd);
//
//		   }
            if(s_FltRealSaveNum_U32 >= FltSaveNumMax)
            {
                
                fflush(File_p -> FltRealFile_fd);
                fd = fileno(File_p -> FltRealFile_fd);
				snprintf(loginfo, sizeof(loginfo)-1, "flt fflush");
				WRITELOGFILE(LOG_INFO_1,loginfo);	
                fsync(fd);
				snprintf(loginfo, sizeof(loginfo)-1, "flt fsync");
				WRITELOGFILE(LOG_INFO_1,loginfo);	
                //printf("s_FltRealSaveNum_U32 %u FltSaveNumMax %u\n",s_FltRealSaveNum_U32,FltSaveNumMax);
                fclose(File_p -> FltRealFile_fd); 
                File_p -> FltRealFile_fd = NULL;
                s_FltRealSaveNum_U32 = 0; 
                s_FltRealFileCrteFlag = 0; //file close so clear
                InStusInfo_p -> FltRealRecFlag_U8 = 2; /*mean file finish save*/  
                
            }
			
            // not reach the max Rec total number
            else
            {               
                pthread_rwlock_rdlock(&g_PthreadLock_ST.ChanDatalock);
                err = RealDataSave(File_p -> FltRealFile_fd,g_ChanData_ST,ProcNum,EADSType);
                pthread_rwlock_unlock(&g_PthreadLock_ST.ChanDatalock);
                s_FltRealSaveNum_U32 += ProcNum;        
            }
        }
        //run at first time and not have creat file
        else 
        {

            err = FltRealFileCreat(File_p,xml_p,TranInfo_p,&g_EADSErrInfo_ST);//at first creat warm file,save the data before warm
            if(err != CODE_OK)
            {
                return err;
            }
            FltSaveBeforNumMax = xml_p -> Rec_Real_ST.RecBeforeNum;
            //g_file_RealSaveNum_U16[main_i]++;
            pthread_rwlock_rdlock(&g_PthreadLock_ST.RealDatalock);
            err = RealFileBeforWarnSave(File_p -> FltRealFile_fd,g_ChanRealBuf_ST,FltSaveBeforNumMax,g_RealBufNum_U32,EADSType);
            pthread_rwlock_unlock(&g_PthreadLock_ST.RealDatalock);
           // printf("g_RealBufNum_U32 %d,FltSaveBeforNumMax %d\n",g_RealBufNum_U32,FltSaveBeforNumMax);
            s_FltRealSaveNum_U32 += FltSaveBeforNumMax;
            s_FltRealFileCrteFlag = 1;//mean have creat the file ,to avoid creat repeate warn file   
  
        }
                  
    }
    //creat Real file and  InStusInfo_p -> FltRealRecFlag_U8 is  clear
    else if(1 == s_FltRealFileCrteFlag)
    {
       
    // not reach the max Rec total number
        if(s_FltRealSaveNum_U32 >= FltSaveNumMax)
        {
            //printf("falt clear s_FltRealSaveNum_U32 %u FltSaveNumMax %u\n",s_FltRealSaveNum_U32,FltSaveNumMax);
            fflush(File_p -> FltRealFile_fd);
            fd = fileno(File_p -> FltRealFile_fd);
            fsync(fd);
            fclose(File_p -> FltRealFile_fd);
            File_p -> FltRealFile_fd = NULL;
            s_FltRealFileCrteFlag = 0; //file close so clear 
            s_FltRealSaveNum_U32 = 0; 
            InStusInfo_p -> FltRealRecFlag_U8 = 2; /*mean file finish save*/       

        }
        //reach the max Rec total number,close file and creat new
        else
        {               
            pthread_rwlock_rdlock(&g_PthreadLock_ST.ChanDatalock);
            err = RealDataSave(File_p -> FltRealFile_fd,g_ChanData_ST,ProcNum,EADSType);
           // printf("falt clear s_FltRealSaveNum_U32 %u \n",s_FltRealSaveNum_U32);
            pthread_rwlock_unlock(&g_PthreadLock_ST.ChanDatalock);
            s_FltRealSaveNum_U32 += ProcNum;    
        }                    
    }
}
#endif
/**********************************************************************
*Name     :   OprtRealFileTopSet
*Function       :   Set the Oprt Real File Top Data According Record.xml
*               :   
*Para         :   DRIVE_FILE_TOP  *DriveTop_ST_p
*
*Return       :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/7/7  Create
*
*********************************************************************/
int8_t OprtRealFileTopSet(TRAIN_INFO * TranInfo_ST_p, RECORD_XML * RecXm_p,DRIVE_FILE_TOP  *DriveTop_ST_p)
{   
  
    int i;
    time_t timep_ST;
    struct tm *now_time_p_ST;
    time(&timep_ST); 
    now_time_p_ST = localtime(&timep_ST); /*change to the local time*/

    DriveTop_ST_p ->RecEnDianType_U16 = 0x01; //small End
    DriveTop_ST_p ->RecHeadSize_U16=0x82; // the top size by byte
    DriveTop_ST_p ->RecHeadVer_U16= 100; 
    DriveTop_ST_p ->RecordType_U16 = RecXm_p -> Rec_Real_ST.RecTypeIdx; //  TYPE 1=Event 2=Log 3=Fast 4=Real
    DriveTop_ST_p ->ProjectNo_U16 = 0x00;
    DriveTop_ST_p ->DCUType_U16 = 0x00;
    DriveTop_ST_p ->VehicleNo_U16 = TranInfo_ST_p -> CoachNum_U8;
    DriveTop_ST_p ->DCUNO_U16 = 0x00;
    DriveTop_ST_p ->RecordTotalChnNum_U16 = RecXm_p -> Rec_Real_ST.RecTotalChn_U8; //96 number
    DriveTop_ST_p ->RecordChnNum_U16 = RecXm_p -> Rec_Real_ST.RecChnNum_U8 ;      //Event chanal num is 8 +70=78
    DriveTop_ST_p ->RecordInterval_U16= RecXm_p -> Rec_Real_ST.RecInterval;
      
    DriveTop_ST_p ->ChnMask1_U16 = RecXm_p -> Rec_Real_ST.RecChnBitMask1; 
    DriveTop_ST_p ->ChnMask2_U16 = RecXm_p -> Rec_Real_ST.RecChnBitMask2;// Analog signal
    DriveTop_ST_p ->ChnMask3_U16 = RecXm_p -> Rec_Real_ST.RecChnBitMask3;
    DriveTop_ST_p ->ChnMask4_U16 = RecXm_p -> Rec_Real_ST.RecChnBitMask4;
    DriveTop_ST_p ->ChnMask5_U16 = RecXm_p -> Rec_Real_ST.RecChnBitMask5;
    DriveTop_ST_p ->ChnMask6_U16 = RecXm_p -> Rec_Real_ST.RecChnBitMask6;

    DriveTop_ST_p ->RecordBefore_U16= RecXm_p -> Rec_Fast_ST.RecBeforeNum;  //fault-before point number
    DriveTop_ST_p ->RecordAfter_U16= RecXm_p -> Rec_Fast_ST.RecAfterNum;    //fault-after point number
    DriveTop_ST_p ->RecordTotal_U16= RecXm_p -> Rec_Fast_ST.RecToTalNum;   //wave count number/ log count number /not event count number

     //uint8_T
    DriveTop_ST_p ->RecordTime_MT= (1 + now_time_p_ST->tm_mon);
    DriveTop_ST_p ->RecordTime_YY = (uint8_t)((1900 + now_time_p_ST->tm_year) - 2000);
    DriveTop_ST_p ->RecordTime_HH = (now_time_p_ST->tm_hour);
    DriveTop_ST_p ->RecordTime_DD = (now_time_p_ST->tm_mday);
    DriveTop_ST_p ->RecordTime_SS = (now_time_p_ST->tm_sec);
    DriveTop_ST_p ->RecordTime_MN = (now_time_p_ST->tm_min);
    for(i=0;i<16;i++)
    {
        DriveTop_ST_p ->EnvData_U16[i] = 0x00;
        DriveTop_ST_p ->FaultCode_U16[i] = 0x00;
    }

    DriveTop_ST_p ->HavePulse_U16=0x00;
    DriveTop_ST_p ->RecordTotalChnNumPulse_U16=0x00;
    DriveTop_ST_p ->RecordChnNumPulse_U16=0x00;;
    DriveTop_ST_p ->RecordIntervalPulse_U16=0x00;

    DriveTop_ST_p ->PulseChnMask1_U16=0x00;
    DriveTop_ST_p ->PulseChnMask2_U16=0x00;
    DriveTop_ST_p ->PulseChnMask3_U16=0x00;
    DriveTop_ST_p ->PulseChnMask4_U16=0x00;
    DriveTop_ST_p ->PulseChnMask5_U16=0xEEEE;
    DriveTop_ST_p ->PulseChnMask6_U16=0xEEEE;
    return CODE_OK;

}
/**********************************************************************
*Name     :   int8_t OprtRealFileTopSave
*Function       :   Set Oprt Real File Top of Drive
*               :   
*Para         :   uint8_t ChanNum: Real File of the ChanNum
*Return       :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/7/7  Create
*
*********************************************************************/
int8_t OprtRealFileTopSave(FILE *Fd_p, RECORD_XML * RecXm_p,TRAIN_INFO * TranInfo_ST_p)
{

    int8_t fwerr = 0;
    int8_t err = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    DRIVE_FILE_TOP DriveRealTop_ST = {0};
    
    OprtRealFileTopSet(TranInfo_ST_p,RecXm_p,&DriveRealTop_ST);

    fwerr = FileWriteWithTry(&DriveRealTop_ST,sizeof(DRIVE_FILE_TOP),1,Fd_p);
    if(CODE_OK != fwerr)
    {
        printf("fwrite OprtFile Top error");
        snprintf(loginfo, sizeof(loginfo)-1, "fwrite OprtFile Top error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    return fwerr;
}
/**********************************************************************
*Name     :   int8_t OprtRealFileCreat(uint8_t ChanName)
*Function       :   Creat a new .oprt real file 
*Para         :   void 
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/1/29  Create
*REV1.0.1     feng    2020/5/9  Not save valut for EADS test

*********************************************************************/
int8_t OprtRealFileCreat(FILE_FD * file_p,RECORD_XML * RrdXml_p,TRAIN_INFO * TranInfo_p,EADS_ERROR_INFO  *EADSErrInfo_ST)
{

    //FILE *fd;
    uint8_t File_FltName_U8[200] = {0};
    uint8_t File_Directory_U8[200] = {0};
    uint8_t TimeString_U8[40] = {0};
    uint32_t TotalSizeMB_U32 = 0,FreeSizeMB_U32 = 0;
    int8_t err = 0;
	uint8_t i = 0;
    uint8_t TryNum = 0;
    time_t timep_ST;   
    struct tm *now_time_p_ST;
    char loginfo[LOG_INFO_LENG] = {0};
    static uint8_t s_OprtPowOnDataString[10] = {0};
    
    time(&timep_ST);
    now_time_p_ST = localtime(&timep_ST); /*change to the local time*/
    sprintf(TimeString_U8,"%04d%02d%02d%02d%02d%02d",
          (1900 + now_time_p_ST->tm_year), 
          (1 + now_time_p_ST->tm_mon), now_time_p_ST->tm_mday,
          now_time_p_ST->tm_hour, now_time_p_ST->tm_min, now_time_p_ST->tm_sec); 
    
    if(0 == g_LinuxDebug)
    {
        sprintf(File_Directory_U8,"%s%8.8s",OPRTFILE_DIR,TimeString_U8); 
   
    }
    //for ubuntu linux test 
    else
    {
        sprintf(File_Directory_U8,"%s%8.8s","/home/feng/Desktop/xilinx_code/yaffs/REC_REALOPRT/",TimeString_U8);//for ubuntu linux test  
   
    }
    /*a new day*/
    if(strncmp(s_OprtPowOnDataString,TimeString_U8,8) < 0)
    {
        memcpy(s_OprtPowOnDataString,TimeString_U8,8);
        if(0 == g_LinuxDebug)
        {
           GetMemSize(OPRTFILE_DIR,&TotalSizeMB_U32,&FreeSizeMB_U32);
           if(FreeSizeMB_U32 < g_SpaceJudge_ST.OPRT_RESER_SPACE)
           {
                printf("%s free %uMB,Rrq %uMB\n",OPRTFILE_DIR,FreeSizeMB_U32,g_SpaceJudge_ST.OPRT_RESER_SPACE);
                snprintf(loginfo, sizeof(loginfo)-1, "%s free %uMB,Rrq %uMB",OPRTFILE_DIR,FreeSizeMB_U32,g_SpaceJudge_ST.OPRT_RESER_SPACE);
                WRITELOGFILE(LOG_WARN_1,loginfo);
                //DeleteEarliestFile(OPRTFILE_DIR,RECORD_FILE_TYPE);
			   err = FileDirJudge(OPRTFILE_DIR);
		       if(REC_FILE_TYPE == err)
		       {
		           DeleteEarliestFile(OPRTFILE_DIR,RECORD_FILE_TYPE);

		       }
		       else if(REC_DIR_TYPE == err)
		       {
		           DeleteEarliestDir(OPRTFILE_DIR);

		       }
            }
        }
        //for ubuntu linux test 
        else
        {
            
            DeleteEarliestFile("/home/feng/Desktop/xilinx_code/yaffs/REC_REALOPRT/",RECORD_FILE_TYPE);

        }
        err = MultiDircCreate(File_Directory_U8);
        if(CODE_CREAT == err)
        {
            memset(loginfo,0,sizeof(loginfo));
            snprintf(loginfo, sizeof(loginfo)-1, "make Dir %s success",File_Directory_U8);
            WRITELOGFILE(LOG_INFO_1,loginfo); 
        }

    }  
    snprintf(File_FltName_U8,sizeof(File_FltName_U8),"%s/%s%02d_%8.8s-%s.dat",File_Directory_U8,
        OPRTFILE_HEAD,TranInfo_p -> CoachNum_U8,TimeString_U8,&TimeString_U8[8]);

    file_p -> OprtRealFile_fd = fopen(File_FltName_U8, "a+");
    if(NULL == file_p ->OprtRealFile_fd)
    {
        TryNum ++;
        perror("creat OprtRealfile.dat file failed");
    }
    else
    {
        EADSErrInfo_ST -> EADSErr = 0;
 //       printf("creat file %s success\n",File_FltName_U8);
        snprintf(loginfo, sizeof(loginfo)-1, "creat file %s",File_FltName_U8);
        WRITELOGFILE(LOG_INFO_1,loginfo);        
    } 
    while((TryNum > 0)&&(TryNum < FILETRY_NUM))
    {
        file_p -> OprtRealFile_fd = fopen(File_FltName_U8, "a+"); //every time creat the file and write replace   
        if(NULL ==  file_p -> OprtRealFile_fd )
        {
            TryNum ++;
            perror("creat OprtRealfile.dat File failed again");
        }
        else
        {
            TryNum = 0;/*quit the while*/
            EADSErrInfo_ST -> EADSErr = 0;
            printf("creat file %s success\n",File_FltName_U8);
            snprintf(loginfo, sizeof(loginfo)-1, "creat file %s",File_FltName_U8);
            WRITELOGFILE(LOG_INFO_1,loginfo);    
        }

    }
    if(TryNum >= FILETRY_NUM)
    {
        
        EADSErrInfo_ST -> EADSErr = 1;
        snprintf(loginfo, sizeof(loginfo)-1, "creat OprtRealfile failed %m");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        err  = CODE_ERR;
        return err;
    }
    err = OprtRealFileTopSave(file_p -> OprtRealFile_fd,RrdXml_p,TranInfo_p);
    return err;
}     
#if 0
/**********************************************************************
*Name     :   OprtRealFileProc
*Function       :   save the Oprt real file 
*Para         :   uint8_t ChanName   save which Chan
*                   uint8_t ChanNumTmp the frame number of real data 
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/29  Create
*REV1.0.1     feng    2021/7/8   change to dircet fclose

*********************************************************************/
int8_t OprtRealFileProc(FILE_FD *File_p, CHAN_STATUS_INFO *InStusInfo_p,RECORD_XML *xml_p,TRAIN_INFO *TranInfo_p,uint8_t ProcNum,uint8_t EADSType)
{ 
// channel happen warm
    int8_t err = 0;
    static uint8_t s_OprtRealFileCrteFlag = 0;
    static uint32_t s_OprtRealSaveNum_U32 = 0;
	 char loginfo[LOG_INFO_LENG] = {0};
    uint32_t OprtSaveNumMax = 0;
    uint32_t FltSaveBeforNumMax = 0;
    int8_t fd = 0;
    /*use the Rec_Fast_ST.RecToTalNum *4,Rec_Fast_ST.RecToTalNum <10000 */
    OprtSaveNumMax = xml_p -> Rec_Fast_ST.RecToTalNum;//4400个点
    if(1== InStusInfo_p -> OprtRealRecFlag_U8)
    {

        //have creat file
        if(1 == s_OprtRealFileCrteFlag)
        {
        //reach the max Rec total number,close file and creat new
            if(s_OprtRealSaveNum_U32 >= OprtSaveNumMax)
            {
                       
				fflush(File_p -> OprtRealFile_fd);
				fd = fileno(File_p -> OprtRealFile_fd);
				snprintf(loginfo, sizeof(loginfo)-1, "oprt fflush");
				WRITELOGFILE(LOG_INFO_1,loginfo);	
				fsync(fd);
				snprintf(loginfo, sizeof(loginfo)-1, "oprt fsync");
				WRITELOGFILE(LOG_INFO_1,loginfo);
                fclose(File_p -> OprtRealFile_fd);
                File_p -> OprtRealFile_fd = NULL;
                s_OprtRealSaveNum_U32 = 0; 
                s_OprtRealFileCrteFlag = 0; //file close so clear
                InStusInfo_p -> OprtRealRecFlag_U8 = 2;      /*mean file finish save*/     
            }
            // not reach the max Rec total number
            else
            {               
                pthread_rwlock_rdlock(&g_PthreadLock_ST.ChanDatalock);
                err = RealDataSave(File_p -> OprtRealFile_fd,g_ChanData_ST,ProcNum,EADSType);
                pthread_rwlock_unlock(&g_PthreadLock_ST.ChanDatalock);
                s_OprtRealSaveNum_U32 += ProcNum;        
            }
        }
        //run at first time and not have creat file
        else 
        {
            //printf("first creal OprtFile\n");
            err = OprtRealFileCreat(File_p,xml_p,TranInfo_p,&g_EADSErrInfo_ST);//at first creat warm file,save the data before warm

            if(err != CODE_OK)
            {
                return err;
            }
			//如果还未创建记录文件，则创建后并记录创建时刻之前400个点
            FltSaveBeforNumMax = xml_p -> Rec_Fast_ST.RecBeforeNum;
            //g_file_RealSaveNum_U16[main_i]++;
            pthread_rwlock_rdlock(&g_PthreadLock_ST.RealDatalock);
            err = RealFileBeforWarnSave(File_p -> OprtRealFile_fd,g_ChanRealBuf_ST,FltSaveBeforNumMax,g_RealBufNum_U32,EADSType);
            pthread_rwlock_unlock(&g_PthreadLock_ST.RealDatalock);
           // printf("g_RealBufNum_U32 %d,FltSaveBeforNumMax %d\n",g_RealBufNum_U32,FltSaveBeforNumMax);
            s_OprtRealSaveNum_U32 += FltSaveBeforNumMax;
            s_OprtRealFileCrteFlag = 1;//mean have creat the file ,to avoid creat repeate warn file
             
        }
              
    }
    //creat Real file and  InStusInfo_p -> OprtRealRecFlag_U8 is  clear
    else if(1 == s_OprtRealFileCrteFlag)//如果记录过程中，动作标志位清除了，也会继续将数据记录完整后才关闭文件
    {
       
    // not reach the max Rec total number
        if(s_OprtRealSaveNum_U32 >= OprtSaveNumMax)
        {
            //printf("falt clear s_OprtRealSaveNum_U32 %u OprtSaveNumMax %u\n",s_OprtRealSaveNum_U32,OprtSaveNumMax);
//            fflush(File_p -> OprtRealFile_fd);
//            fd = fileno(File_p -> OprtRealFile_fd);
//            fsync(fd);
            fclose(File_p -> OprtRealFile_fd);
            File_p -> OprtRealFile_fd = NULL;
            s_OprtRealFileCrteFlag = 0; //file close so clear 
            s_OprtRealSaveNum_U32 = 0;  
            InStusInfo_p -> OprtRealRecFlag_U8 = 2;      /*mean file finish save*/        

        }
        //reach the max Rec total number,close file and creat new
        else
        {               
            pthread_rwlock_rdlock(&g_PthreadLock_ST.ChanDatalock);
            err = RealDataSave(File_p -> OprtRealFile_fd,g_ChanData_ST,ProcNum,EADSType);
           // printf("falt clear s_OprtRealSaveNum_U32 %u \n",s_OprtRealSaveNum_U32);
            pthread_rwlock_unlock(&g_PthreadLock_ST.ChanDatalock);
            s_OprtRealSaveNum_U32 += ProcNum;    
        }                    
    }
}
#endif
/**********************************************************************
*Name     :   int8_t ChanCalibFileCreat(void)
*Function       :   Creat the /tffs0/ChanCalibValue.dat for save the calib value
*Para         :   void 
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/6/29  Create
*********************************************************************/
int8_t ChanCalibFileCreat(FILE_FD *file_p,uint8_t *path,EADS_ERROR_INFO  *EADSErrInfo_ST)
{

    int8_t err = 0;
    uint8_t TryNum = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    file_p -> CalibFile_fd = fopen(path,"w+"); //every time creat the file and write replace

    if(NULL ==  file_p ->CalibFile_fd)
    {
        TryNum ++;
        perror("first creat CalibFile.dat file failed");
    }
    else
    {
        EADSErrInfo_ST -> EADSErr = 0;
        printf("creat file %s success\n",path);
        snprintf(loginfo, sizeof(loginfo)-1, "creat file %s",path);
        WRITELOGFILE(LOG_INFO_1,loginfo);        
    } 
    while((TryNum > 0)&&(TryNum < FILETRY_NUM))
    {
        file_p -> CalibFile_fd = fopen(path,"w+"); //every time creat the file and write replace   
        if(NULL ==  file_p -> CalibFile_fd )
        {
            TryNum ++;
            perror("creat CalibFile.dat File failed again");
        }
        else
        {
            TryNum = 0;/*quit the while*/
            EADSErrInfo_ST -> EADSErr = 0;
            printf("creat file %s success\n",path);
            snprintf(loginfo, sizeof(loginfo)-1, "creat file %s",path);
            WRITELOGFILE(LOG_INFO_1,loginfo);    
        }

    }
    if(TryNum >= FILETRY_NUM)
    {
        
        EADSErrInfo_ST -> EADSErr = 1;
        perror("creat CalibFile.dat File failed");
        snprintf(loginfo, sizeof(loginfo)-1, "creat CalibFile failed %m");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        err  = CODE_ERR;
    }
    return err;

}
/**********************************************************************
*Name     :   int8_t ChanCalibDataSave(CHAN_DATA *CalibDataSave_p) 
*Para         :   CHAN_DATA *CalibDataSave_p :the Calib data Save ST 
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/6/29  Create
*********************************************************************/
#if 0
int8_t ChanCalibDataSave(FILE * fd,CHAN_CALIB_DATA *CalibDataSave_p)
{
   int8_t fwerr = 0;
   int8_t  err = 0;
   uint16_t i =0;
   char loginfo[LOG_INFO_LENG] = {0};
   //ChanCalibDataSet(CalibData,CalibDataSave_p);
   if(fd == NULL)
    {
        perror("ChanCalib.dat fd is NULL");
        snprintf(loginfo, sizeof(loginfo)-1, "ChanCalib.dat fd is NULL");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    if(LINECAlib_DEBUG == g_DebugType_EU)
    {
        for(i = 0;i < VOL_CHAN_NUM;i++)
        {
            printf("save %d,%d\n", i,CalibDataSave_p -> VolChanCalib_I16[i]);
        } 
    }
   //this should note the 2 parameter
   fwerr = FileWriteWithTry(CalibDataSave_p,2,VOL_CHAN_NUM,fd); 
   if(CODE_OK != fwerr)
    {
        perror("CalibFile save failed");
        snprintf(loginfo, sizeof(loginfo)-1, "CalibFile save failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }

    return fwerr;
}

/**********************************************************************
*Name     :   int8_t OperNumFileTopSet(DRIVE_FILE_TOP  *DriveTop_ST_p) 
*Function       :   Set the Chan Oper Num File Top Data,use Fast file type 
*Para         :   DRIVE_FILE_TOP  *DriveTop_ST_p
*
*Return       :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2020/3/7  Create
*REV1.0.1     feng    2020/4/7  
*Note           :     OprtNum is 32bit type. so take 2 channel, 
*********************************************************************/
int8_t OperNumFileTopSet(const TRAIN_INFO * TranInfo_ST_p, const RECORD_XML * RecXm_p,DRIVE_FILE_TOP  *DriveTop_ST_p)
{   
    int i;
    time_t timep_ST;
    struct tm *now_time_p_ST;
    time(&timep_ST); 
    now_time_p_ST = localtime(&timep_ST); /*change to the local time*/

    DriveTop_ST_p ->RecEnDianType_U16 = 0x01; //small End
    DriveTop_ST_p ->RecHeadSize_U16 = 0x82; // the top size by byte
    DriveTop_ST_p ->RecHeadVer_U16 = 100; 
    DriveTop_ST_p ->RecordType_U16 = RecXm_p -> Rec_Fast_ST.RecTypeIdx; //  TYPE 1=Event 2=Log 3=Fast 4=Real
    DriveTop_ST_p ->ProjectNo_U16 = 0x00;
    DriveTop_ST_p ->DCUType_U16 = 0x00;
    DriveTop_ST_p ->VehicleNo_U16 = TranInfo_ST_p -> CoachNum_U8;
    DriveTop_ST_p ->DCUNO_U16 = 0x00;
    DriveTop_ST_p ->RecordTotalChnNum_U16 = RecXm_p -> Rec_Fast_ST.RecTotalChn_U8; //96 number
    DriveTop_ST_p ->RecordChnNum_U16 = RecXm_p -> Rec_Fast_ST.RecChnNum_U8 ;      //Event chanal num is 8 +70=78
    DriveTop_ST_p ->RecordInterval_U16= RecXm_p -> Rec_Fast_ST.RecInterval;
      
    DriveTop_ST_p ->ChnMask1_U16 = RecXm_p -> Rec_Fast_ST.RecChnBitMask1; 
    DriveTop_ST_p ->ChnMask2_U16 = RecXm_p -> Rec_Fast_ST.RecChnBitMask2;// Analog signal
    DriveTop_ST_p ->ChnMask3_U16 = RecXm_p -> Rec_Fast_ST.RecChnBitMask3;
    DriveTop_ST_p ->ChnMask4_U16 = RecXm_p -> Rec_Fast_ST.RecChnBitMask4;
    DriveTop_ST_p ->ChnMask5_U16 = RecXm_p -> Rec_Fast_ST.RecChnBitMask5;
    DriveTop_ST_p ->ChnMask6_U16 = RecXm_p -> Rec_Fast_ST.RecChnBitMask6;

    DriveTop_ST_p ->RecordBefore_U16= RecXm_p -> Rec_Fast_ST.RecBeforeNum;  //fault-before point number
    DriveTop_ST_p ->RecordAfter_U16= RecXm_p -> Rec_Fast_ST.RecAfterNum;    //fault-after point number
    DriveTop_ST_p ->RecordTotal_U16= RecXm_p -> Rec_Fast_ST.RecToTalNum;   //wave count number/ log count number /not event count number
     //uint8_T
    DriveTop_ST_p ->RecordTime_MT= (1 + now_time_p_ST->tm_mon);
    DriveTop_ST_p ->RecordTime_YY = (uint8_t)((1900 + now_time_p_ST->tm_year) - 2000);
    DriveTop_ST_p ->RecordTime_HH = (now_time_p_ST->tm_hour);
    DriveTop_ST_p ->RecordTime_DD = (now_time_p_ST->tm_mday);
    DriveTop_ST_p ->RecordTime_SS = (now_time_p_ST->tm_sec);
    DriveTop_ST_p ->RecordTime_MN = (now_time_p_ST->tm_min);
    for(i = 0;i < 16;i++)
    {
        DriveTop_ST_p ->EnvData_U16[i] = 0x00;
        DriveTop_ST_p ->FaultCode_U16[i] = 0x00;
     }

    DriveTop_ST_p ->HavePulse_U16=0x00;
    DriveTop_ST_p ->RecordTotalChnNumPulse_U16=0x00;
    DriveTop_ST_p ->RecordChnNumPulse_U16=0x00;;
    DriveTop_ST_p ->RecordIntervalPulse_U16=0x00;

    DriveTop_ST_p ->PulseChnMask1_U16=0x00;
    DriveTop_ST_p ->PulseChnMask2_U16=0x00;
    DriveTop_ST_p ->PulseChnMask3_U16=0x00;
    DriveTop_ST_p ->PulseChnMask4_U16=0x00;
    DriveTop_ST_p ->PulseChnMask5_U16=0xEEEE;
    DriveTop_ST_p ->PulseChnMask6_U16=0xEEEE;

    return CODE_OK;
}
/**********************************************************************
*Name     :   int8_t OperNumFileTopSave(void)
*Function       :  save the Open Num file
*               :   
*Para         :  
*Return       :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/7/7  Create
*
*********************************************************************/
int8_t OperNumFileTopSave(FILE *File_p,const RECORD_XML * inRecXm_p,const TRAIN_INFO * inTranInfo_p)
{
    int8_t fwerr = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    DRIVE_FILE_TOP OperNumTop_ST = {0};
    if(NULL == File_p)
    {
        perror("OperNum fp is NULL");
        snprintf(loginfo, sizeof(loginfo)-1, "OperNum fp is NULL");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    OperNumFileTopSet(inTranInfo_p,inRecXm_p,&OperNumTop_ST);
    fwerr = FileWriteWithTry(&OperNumTop_ST,sizeof(DRIVE_FILE_TOP),1,File_p); 
    if(CODE_OK != fwerr)
    {
        printf("fwrite OperNumFile Top error");
        memset(loginfo,0,LOG_INFO_LENG);
        snprintf(loginfo, sizeof(loginfo)-1, "fwrite OperNumFile Top error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    return fwerr;
}
/**********************************************************************
*Name     :   int8_t OperNumFileCreat(void)
*Function       :   Creat the OperaNum file
*Para         :   void 
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/7/29  Create
*********************************************************************/
int8_t OperNumFileCreat(FILE_FD * file_p,const RECORD_XML * inxmlp,const TRAIN_INFO * inTranInfo_p,EADS_ERROR_INFO  *EADSErrInfo_ST)
{

    int8_t err = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    uint8_t TryNum = 0;
    file_p-> OprtNumFile_fd = fopen(CHAN_OPER_NUM_FILE, "w+"); //every time creat the file and write replace   
    if(NULL == file_p-> OprtNumFile_fd)
    {
        TryNum ++;
        perror(" creat OperNum File File failed");
        /*try again*/
    }
    else
    {
        EADSErrInfo_ST -> EADSErr = 0;
        printf("creat file %s success\n",CHAN_OPER_NUM_FILE);
        snprintf(loginfo, sizeof(loginfo)-1, "creat file %s ",CHAN_OPER_NUM_FILE);
        WRITELOGFILE(LOG_INFO_1,loginfo);   
    }
    while((TryNum > 0)&&(TryNum < FILETRY_NUM))
    {
        file_p -> OprtNumFile_fd = fopen(CHAN_OPER_NUM_FILE, "w+"); //every time creat the file and write replace   
        if(NULL ==  file_p -> OprtNumFile_fd )
        {
            TryNum ++;
            perror("creat OperNum File File failed again");
        }
        else
        {
            TryNum = 0;/*quit the while*/
            EADSErrInfo_ST -> EADSErr = 0;
            printf("creat file %s success\n",CHAN_OPER_NUM_FILE);
            snprintf(loginfo, sizeof(loginfo)-1, "creat %s success ",CHAN_OPER_NUM_FILE);
            WRITELOGFILE(LOG_INFO_1,loginfo);    
        }
    }

    if(TryNum >= FILETRY_NUM)
    {
        
        EADSErrInfo_ST -> EADSErr = 1;
        snprintf(loginfo, sizeof(loginfo)-1, "creat OperNum File  failed %m");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        err  = CODE_ERR;
        return err;
    }

    err = OperNumFileTopSave(file_p-> OprtNumFile_fd,inxmlp,inTranInfo_p);
    if(FILE_DEBUG ==  g_DebugType_EU)
    {
        printf("ChanOpeNumFile_p %u\n", file_p-> OprtNumFile_fd);
    }
    return err;
}
/**********************************************************************
*Name     :   int8_t OperNumDataSave(uint32_t *ChanOperNum_p)
*Function       :   Save the  OperaNum  data to file of 6 Coach
*Para         :   uint32_t *ChanOperNum_p,the g_OperNum_U32
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/29  Create
*REV1.0.0     feng    2020/7/8   add fflush and fsync
*********************************************************************/
int8_t OperNumDataSave(FILE * filefd, uint32_t ChanOperNum_p[])
{
    uint8_t i = 0;
    int8_t fd_temp = 0;
    int16_t fwerr = 0;
    uint16_t WriteSize_U16 = 0;
    char loginfo[LOG_INFO_LENG] = {0};

    //OperNumDataSet(&OperNumData_ST,ChanOperNum_p);

    if(NULL == filefd)
    {
        perror("OperNum File fp is NULL");
        snprintf(loginfo, sizeof(loginfo)-1, "OperNumFile fp %m");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    //this should note the 2 parameter
    fwerr = fseek(filefd,sizeof(DRIVE_FILE_TOP),SEEK_SET);
     /*fseek is ok*/
    /*fseek is error -1*/    
    if(0 != fwerr)
    {
        perror("OperNumFile fseek error");
        snprintf(loginfo, sizeof(loginfo)-1, "OperNumFile fseek %m");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;

    }
    WriteSize_U16 = VOL_CHAN_NUM << 2;

   // fwerr = fwrite(OperNumData_ST.DriveAnalog_U32,WriteSize_U16,1,filefd); 
    fwerr = FileWriteWithTry(ChanOperNum_p,WriteSize_U16,1,filefd); 
    if(CODE_OK != fwerr)
    {
        printf("fwrite OperNumFile error");
        memset(loginfo,0,LOG_INFO_LENG);
        snprintf(loginfo, sizeof(loginfo)-1, "fwrite OperNumFile error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    fflush(filefd);
    fd_temp = fileno(filefd);
    fsync(fd_temp);
    fclose(filefd); // to make every time save the data to file
    return fwerr;
}

/**********************************************************************
*Name     :   int8_t OperNumFileSave(void)
*Function       :   Creat  the operateNum file,and save Opearate Num to file
*Para         :   
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*s
*History:
*REV1.0.0     feng    2020/2/27  Create
*********************************************************************/
int8_t OperNumFileSave(FILE_FD * File_p, CHAN_STATUS_INFO * ChanStaInfo_p,RECORD_XML * RrdXml_p,TRAIN_INFO * inTranInfo_p)
{
    int8_t err = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    err = OperNumFileCreat(File_p,RrdXml_p,inTranInfo_p,&g_EADSErrInfo_ST);
    if(err != CODE_OK)
    {
        perror("OperNumFileCreat failed\n");
        snprintf(loginfo, sizeof(loginfo)-1, "OperNumFileCreat");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    if(FILE_DEBUG ==  g_DebugType_EU)
    {
        printf("File_p-> OprtNumFile_fd  %u\n", File_p-> OprtNumFile_fd);
    }
    err = OperNumDataSave(File_p-> OprtNumFile_fd,ChanStaInfo_p -> Channel_OperaNum_U32);
    if(err != CODE_OK)
    {
        perror("OperNumFileSave  failed\n");
        snprintf(loginfo, sizeof(loginfo)-1, "OperNumFileSave failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    return CODE_OK;
}
#endif
/**********************************************************************
*Name     :   int8_t FileCreatePowOn(void)
*Function       :   when application run,creat  Event file and 
*Para         :   void 
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/6/29  Create
*********************************************************************/
int8_t FileCreatePowOn(FILE_FD *file_p,RECORD_XML *RrdXml_p,RECORD_XML *RrdLifeXml_p,TRAIN_INFO *TranInfo_p,EADS_ERROR_INFO *EADSErrInfo_p)
{
    char loginfo[LOG_INFO_LENG] = {0};
    EventFileCreateByNum(file_p,RrdXml_p,TranInfo_p,EADSErrInfo_p);
	//EventLifeFileCreateByNum(file_p,RrdLifeXml_p,TranInfo_p,EADSErrInfo_p);
    //peripheralFileCreate(Uart_Device,&g_FileFd_ST.UARTFile_fd,&g_Rec_XML_ST.Rec_Uart_ST,TranInfo_p,EADSErrInfo_p);    
    peripheralFileCreate(CAN_Device,&g_FileFd_ST.CANFile_fd,&g_Rec_XML_ST.Rec_CAN_ST,TranInfo_p,EADSErrInfo_p);
    /*for oprtNum file save*/
    if(0 == g_LinuxDebug)
    {
        MultiDircCreate(RrdXml_p -> Rec_Fast_ST.RecPath);

    }
    
    return CODE_OK;
}
/**********************************************************************
*Name     :   int8_t OprtRealFileCreat(uint8_t ChanName)
*Function       :   Creat a new .oprt real file 
*Para         :   void 
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/1/29  Create
*REV1.0.1     feng    2020/5/9  Not save valut for EADS test

*********************************************************************/
int8_t FileSpaceProc(RECORD_XML * RrdXml_p)
{

    //FILE *fd;
    uint32_t TotalSizeMB_U32 = 0,FreeSizeMB_U32 = 0;
    int8_t err = 0;
    uint8_t i = 0;
    char loginfo[LOG_INFO_LENG] = {0};
  
   GetMemSize(OPRTFILE_DIR,&TotalSizeMB_U32,&FreeSizeMB_U32);
   while((FreeSizeMB_U32 < g_SpaceJudge_ST.MIN_RESER_SPACE) && (i < 10)) //10G
   {
       i++;     
       printf("%s free %uMB,Rrq %uMB\n",OPRTFILE_DIR,FreeSizeMB_U32,g_SpaceJudge_ST.MIN_RESER_SPACE);
       snprintf(loginfo, sizeof(loginfo)-1, "%s free %uMB,Rrq %uMB",OPRTFILE_DIR,FreeSizeMB_U32,g_SpaceJudge_ST.MIN_RESER_SPACE);
       WRITELOGFILE(LOG_WARN_1,loginfo); 
       err = FileDirJudge(OPRTFILE_DIR);
       if(REC_FILE_TYPE == err)
       {
           DeleteEarliestFile(OPRTFILE_DIR,RECORD_FILE_TYPE);

       }
       else if(REC_DIR_TYPE == err)
       {
           DeleteEarliestDir(OPRTFILE_DIR);

       }
       sleep(2); //the filesystem to update the space size need time
       GetMemSize(OPRTFILE_DIR,&TotalSizeMB_U32,&FreeSizeMB_U32);
    }

    i = 0;
    GetMemSize(RrdXml_p -> Rec_Real_ST.RecPath,&TotalSizeMB_U32,&FreeSizeMB_U32);
    while((FreeSizeMB_U32 < g_SpaceJudge_ST.MIN_RESER_SPACE) && (i < 5)) //10G
    {
       i++;
       printf("%s free %uMB,Rrq %uMB\n",RrdXml_p -> Rec_Real_ST.RecPath,FreeSizeMB_U32,g_SpaceJudge_ST.MIN_RESER_SPACE);
       snprintf(loginfo, sizeof(loginfo)-1, "%s free %uMB,Rrq %uMB",RrdXml_p -> Rec_Real_ST.RecPath,FreeSizeMB_U32,g_SpaceJudge_ST.MIN_RESER_SPACE);
       WRITELOGFILE(LOG_WARN_1,loginfo); 
       
       err = FileDirJudge(RrdXml_p -> Rec_Real_ST.RecPath);
       if(REC_FILE_TYPE == err)
       {
           DeleteEarliestFile(RrdXml_p -> Rec_Real_ST.RecPath,RECORD_FILE_TYPE);

       }
       else if(REC_DIR_TYPE == err)
       {
           DeleteEarliestDir(RrdXml_p -> Rec_Real_ST.RecPath);

       }
       sleep(2); //the filesystem to update the space size need time
       GetMemSize(RrdXml_p -> Rec_Real_ST.RecPath,&TotalSizeMB_U32,&FreeSizeMB_U32);
   }   


//    i = 0;
//    GetMemSize(RrdXml_p -> Rec_Event_ST.RecPath,&TotalSizeMB_U32,&FreeSizeMB_U32);
//    while((FreeSizeMB_U32 < g_SpaceJudge_ST.EVENT_RESER_SPACE) && (i < 5)) //10G
//    {
//       i++;
//       printf("%s free %uMB,Rrq %uMB\n",RrdXml_p -> Rec_Event_ST.RecPath,FreeSizeMB_U32,g_SpaceJudge_ST.EVENT_RESER_SPACE);
//       snprintf(loginfo, sizeof(loginfo)-1, "%s free %uMB,Rrq %uMB",RrdXml_p -> Rec_Event_ST.RecPath,FreeSizeMB_U32,g_SpaceJudge_ST.EVENT_RESER_SPACE);
//       WRITELOGFILE(LOG_WARN_1,loginfo); 
//       
//       err = FileDirJudge(RrdXml_p -> Rec_Event_ST.RecPath);
//       if(REC_FILE_TYPE == err)
//       {
//           DeleteEarliestFile(RrdXml_p -> Rec_Event_ST.RecPath,RECORD_FILE_TYPE);
//
//       }
//       else if(REC_DIR_TYPE == err)
//       {
//           DeleteEarliestDir(RrdXml_p -> Rec_Event_ST.RecPath);
//
//       }
//       sleep(2); //the filesystem to update the space size need time
//       GetMemSize(RrdXml_p -> Rec_Event_ST.RecPath,&TotalSizeMB_U32,&FreeSizeMB_U32);
//   }   	
   return err;
}     


/**********************************************************************
*Name     :   int8_t DirFileTar
*Function       :   tar the Event ,RealFlt,RealOprt directory which create before today
*Para         :   void 
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/6/29  Create
*note Para  can improve with Freesize
*********************************************************************/
int8_t DirFileTar(RECORD_XML * RrdXml_p)
{ 
    //FILE *fd;
    uint8_t File_EventName_U8[200] = {0};
    uint8_t File_Directory_U8[200] = {0};
    uint8_t TimeString_U8[40] = {0};
    int8_t err = 0;
    time_t timep_ST;   
    struct tm *now_time_p_ST;
    char loginfo[LOG_INFO_LENG] = {0};

    time(&timep_ST);
    now_time_p_ST = localtime(&timep_ST); /*change to the local time*/
    sprintf(TimeString_U8,"%04d%02d%02d%02d%02d%02d",
          (1900 + now_time_p_ST->tm_year), 
          (1 + now_time_p_ST->tm_mon), now_time_p_ST->tm_mday,
          now_time_p_ST->tm_hour, now_time_p_ST->tm_min, now_time_p_ST->tm_sec); 
        // add year month folder
	
	/*firsttar Realflt dir*/
	memset(File_Directory_U8,0,sizeof(File_Directory_U8));
	if(0 == g_LinuxDebug)
	{
		sprintf(File_Directory_U8,"%s%8.8s",RrdXml_p-> Rec_Real_ST.RecPath,TimeString_U8); 
		
	}
	//for ubuntu linux test 
	else
	{
		sprintf(File_Directory_U8,"%s%8.8s","/home/feng/Desktop/xilinx_code/yaffs/REC_REALFLT/",TimeString_U8);//for ubuntu linux test	
	
	}  
		
	/*first tar Realflt file*/
	if(0 == g_LinuxDebug)
	{
		TarDir(RrdXml_p -> Rec_Real_ST.RecPath,File_Directory_U8);
	}
	else
	{
		TarDir("/home/feng/Desktop/xilinx_code/yaffs/REC_REALFLT/",File_Directory_U8);
	}

    /*senond tar event dir*/
    if(0 == g_LinuxDebug)
    {
        sprintf(File_Directory_U8,"%s%8.8s",RrdXml_p-> Rec_Event_ST.RecPath,TimeString_U8); 
   
    }
    //for ubuntu linux test 
    else
    {
        sprintf(File_Directory_U8,"%s%8.8s","/home/feng/Desktop/xilinx_code/yaffs/REC_EVTDATA/",TimeString_U8);//for ubuntu linux test  
   
    }  

     /*senond tar event file*/
    if(0 == g_LinuxDebug)
    {
        TarDir(RrdXml_p -> Rec_Event_ST.RecPath,File_Directory_U8);
    }
    else
    {
        TarDir("/home/feng/Desktop/xilinx_code/yaffs/REC_EVTDATA/",File_Directory_U8);
    }

   
    /*last RealOprt dir*/
    memset(File_Directory_U8,0,sizeof(File_Directory_U8));
     if(0 == g_LinuxDebug)
    {
        sprintf(File_Directory_U8,"%s%8.8s",OPRTFILE_DIR,TimeString_U8); 
   
    }
    //for ubuntu linux test 
    else
    {
        sprintf(File_Directory_U8,"%s%8.8s","/home/feng/Desktop/xilinx_code/yaffs/REC_REALOPRT/",TimeString_U8);//for ubuntu linux test  
   
    }
     /*last tar event file*/
    if(0 == g_LinuxDebug)
    {
        TarDir(OPRTFILE_DIR,File_Directory_U8);
    }
    else
    {
        TarDir("/home/feng/Desktop/xilinx_code/yaffs/REC_REALOPRT/",File_Directory_U8);
    }
    return err;

}

/*!
 * Function:文件记录功能
 * @param Device_Type_enum -设备类型
 * @param device_FP -文件流（二级指针）
 * @param databuf -记录的数据
 * @param datalength -数据长度
 * @param REC_TYPE -xml中的REC_TYPE
 * @param Record_Num_U32 -记录帧数
 */
void FileSaveFunc(peripheralDevice Device_Type_enum,FILE **device_FP,uint8_t *databuf,uint8_t datalength,RECORD_TYPE_CFG *REC_TYPE,\
                 uint32_t Record_Num_U32) 
{
    struct timeval A_Time_ST,A_TimeEnd_ST;
    DRIVE_FILE_DATA Record_Data={0};
    uint8_t i = 0;        
    int8_t ret_fd = 0;
    if(TIME_DEBUG  == g_DebugType_EU)
    {
        gettimeofday(&A_Time_ST,NULL);
    } 
    if(Record_Num_U32 >= REC_TYPE->RecToTalNum)
    {                
        printf("s_FrameSaveNum_U32:%d\n",Record_Num_U32);
        fflush(*device_FP);
        ret_fd = fileno(*device_FP);
        fsync(ret_fd);
        fclose(*device_FP);
        *device_FP = NULL;
        peripheralFileCreate(Device_Type_enum,device_FP,REC_TYPE,&g_TrainInfo_ST,&g_EADSErrInfo_ST);                  
    }
	else if(0 == (Record_Num_U32 % Peri_FILE_SYNC_NUM ))//每50次记录刷新缓存,理论是5s
    {
		fflush(*device_FP);
        ret_fd = fileno(*device_FP);
        fsync(ret_fd);
	}
    memcpy(&Record_Data,databuf,datalength);
    peripheralDataSave(*device_FP,&Record_Data);
       
    //保存数组至记录文件中    
    if(TIME_DEBUG  == g_DebugType_EU)
    {
            gettimeofday(&A_TimeEnd_ST,NULL);
            printf("File Save thread tim:%u \n",(uint32_t)A_TimeEnd_ST.tv_usec- (uint32_t)A_Time_ST.tv_usec); 
            printf("File Save thread usec:%u \n", (uint32_t)A_TimeEnd_ST.tv_usec);                    
    }
} 
#endif