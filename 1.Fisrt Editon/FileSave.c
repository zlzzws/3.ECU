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
extern DEBUG_TYPE_ENUM      g_DebugType_EU;
extern FILE_FD              g_FileFd_ST;
extern RECORD_XML           g_Rec_XML_ST;
extern TRAIN_INFO           g_TrainInfo_ST;
extern PTHREAD_LOCK         g_PthreadLock_ST;
extern SPACE_JUDGE_VALUE    g_SpaceJudge_ST;
extern ECU_ERROR_INFO       g_ECUErrInfo_ST;
extern uint32_t             g_LinuxDebug;

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
int8_t DirFileTar(RECORD_XML *RrdXml_p)
{      
    time_t timep_ST;   
    struct tm *now_time_p_ST;
    uint8_t TimeString_U8[40] = {0}; 
    char loginfo[LOG_INFO_LENG] = {0};
    uint8_t File_Directory_U8[200] = {0};
    uint8_t File_BLVDS_Directory_U8[200] = {0};

    time(&timep_ST);
    now_time_p_ST = localtime(&timep_ST); /*change to the local time*/
    sprintf(TimeString_U8,"%04d%02d%02d%02d%02d%02d",
          (1900 + now_time_p_ST->tm_year), 
          (1 + now_time_p_ST->tm_mon), now_time_p_ST->tm_mday,
          now_time_p_ST->tm_hour, now_time_p_ST->tm_min, now_time_p_ST->tm_sec);   

    if(0 == g_LinuxDebug)
    {
        sprintf(File_Directory_U8,"%s%8.8s",RrdXml_p->Rec_Event_ST.RecPath,TimeString_U8);
        sprintf(File_BLVDS_Directory_U8,"%s%8.8s",BLVDS_EVENT_PATH,TimeString_U8);
        TarDir(RrdXml_p->Rec_Event_ST.RecPath,File_Directory_U8);
        TarDir(BLVDS_EVENT_PATH,File_BLVDS_Directory_U8);
    }
    return CODE_OK;
}
/**********************************************************************
*Name     	  :   FileWriteWithTry
*Function     :   write with try 3 time
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

    DriveTop_ST_p ->RecEnDianType_U16 = 0x01;//small End
    DriveTop_ST_p ->RecHeadSize_U16=0x82; // 0x82->130
    DriveTop_ST_p ->RecHeadVer_U16=100; 
    DriveTop_ST_p ->RecordType_U16 = RecXm_p -> Rec_Event_ST.RecTypeIdx; //TYPE 1=Event 2=Log 3=Fast 4=Real
    DriveTop_ST_p ->ProjectNo_U16 = 0x00;
    DriveTop_ST_p ->DCUType_U16 = 0x00;
    DriveTop_ST_p ->VehicleNo_U16 = TranInfo_ST_p -> CoachNum_U8;
    DriveTop_ST_p ->DCUNO_U16 = 0x00;
    DriveTop_ST_p ->RecordTotalChnNum_U16 = RecXm_p -> Rec_Event_ST.RecTotalChn_U8;
    DriveTop_ST_p ->RecordChnNum_U16 = RecXm_p -> Rec_Event_ST.RecChnNum_U8 ;
    DriveTop_ST_p ->RecordInterval_U16= RecXm_p -> Rec_Event_ST.RecInterval;
      
    DriveTop_ST_p ->ChnMask1_U16 = RecXm_p -> Rec_Event_ST.RecChnBitMask1; 
    DriveTop_ST_p ->ChnMask2_U16 = RecXm_p -> Rec_Event_ST.RecChnBitMask2;
    DriveTop_ST_p ->ChnMask3_U16 = RecXm_p -> Rec_Event_ST.RecChnBitMask3;
    DriveTop_ST_p ->ChnMask4_U16 = RecXm_p -> Rec_Event_ST.RecChnBitMask4;
    DriveTop_ST_p ->ChnMask5_U16 = RecXm_p -> Rec_Event_ST.RecChnBitMask5;
    DriveTop_ST_p ->ChnMask6_U16 = RecXm_p -> Rec_Event_ST.RecChnBitMask6;

    DriveTop_ST_p ->RecordBefore_U16= RecXm_p -> Rec_Event_ST.RecBeforeNum;  
    DriveTop_ST_p ->RecordAfter_U16= RecXm_p ->  Rec_Event_ST.RecAfterNum;  
    DriveTop_ST_p ->RecordTotal_U16= RecXm_p -> Rec_Event_ST.RecToTalNum; 
    
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
int8_t EventFileTopSave(FILE *Fd_p, RECORD_XML *RecXm_p,TRAIN_INFO *TranInfo_ST_p)
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
int8_t EventFileCreateByNum(FILE_FD *file_p,RECORD_XML *RrdXml_p,TRAIN_INFO *TranInfo_p,ECU_ERROR_INFO  *ECUErrInfo_ST)
{   
    uint8_t TryNum = 0;
    int8_t  err,err_1,err_2,err_3,err_4;
    time_t  timep_ST;   
    struct tm *now_time_p_ST;
    char    loginfo[LOG_INFO_LENG] = {0};
    uint8_t TimeString_U8[40] = {0};
    uint8_t File_EventName_U8[200] = {0};
    uint8_t File_BLVDS_EventName_U8[200] = {0};
    uint8_t File_Directory_U8[200] = {0};
    uint8_t File_BLVDS_Directory_U8[200] = {0};   
    uint32_t TotalSizeMB_U32_1 = 0,FreeSizeMB_U32_1 = 0;
    uint32_t TotalSizeMB_U32_2 = 0,FreeSizeMB_U32_2 = 0;    
    static uint8_t s_EventPowOnDataString[10] = {0};

    time(&timep_ST);
    now_time_p_ST = localtime(&timep_ST); /*change to the local time*/
    sprintf(TimeString_U8,"%04d%02d%02d%02d%02d%02d",(1900 + now_time_p_ST->tm_year),\
          (1 + now_time_p_ST->tm_mon), now_time_p_ST->tm_mday,\
          now_time_p_ST->tm_hour, now_time_p_ST->tm_min, now_time_p_ST->tm_sec); 
        
    if(0 == g_LinuxDebug)
    {
        sprintf(File_Directory_U8,"%s%8.8s",RrdXml_p-> Rec_Event_ST.RecPath,TimeString_U8);
        sprintf(File_BLVDS_Directory_U8,"%s%8.8s",BLVDS_EVENT_PATH,TimeString_U8); 
    } 
    /*machine power on or run into a new day*/
    if(strncmp(s_EventPowOnDataString,TimeString_U8,8) < 0)
    {
        // if run into a new day,do the option below
        if(strlen(s_EventPowOnDataString) != 0) 
        {
			FileSpaceProc(&g_Rec_XML_ST);
			LogFileCreatePowOn();
            printf("Go to a new day:Creat a new logfile!\n");
        }
        memcpy(s_EventPowOnDataString,TimeString_U8,8);
        if(0 == g_LinuxDebug)
        {
            GetMemSize(RrdXml_p->Rec_Event_ST.RecPath,&TotalSizeMB_U32_1,&FreeSizeMB_U32_1);            
            if(FreeSizeMB_U32_1 < g_SpaceJudge_ST.EVENT_RESER_SPACE)
            {
               printf("%s free %uMB,Rrq %uMB \n",RrdXml_p->Rec_Event_ST.RecPath,FreeSizeMB_U32_1,g_SpaceJudge_ST.EVENT_RESER_SPACE);
               snprintf(loginfo, sizeof(loginfo)-1, "%s free %uMB,Rrq %uMB",RrdXml_p->Rec_Event_ST.RecPath,FreeSizeMB_U32_1,g_SpaceJudge_ST.EVENT_RESER_SPACE);
               WRITELOGFILE(LOG_WARN_1,loginfo);
                /*Judge the path is dir or file , 2 == file , 1 == dir*/
			   err = FileDirJudge(RrdXml_p->Rec_Event_ST.RecPath);
		       if(REC_FILE_TYPE == err)
		       {
		           DeleteEarliestFile(RrdXml_p -> Rec_Event_ST.RecPath,RECORD_FILE_TYPE);
		       }
		       else if(REC_DIR_TYPE == err)
		       {
		           DeleteEarliestDir(RrdXml_p -> Rec_Event_ST.RecPath);
		       }
            }
            GetMemSize(BLVDS_EVENT_PATH,&TotalSizeMB_U32_2,&FreeSizeMB_U32_2);
            if(FreeSizeMB_U32_2 < g_SpaceJudge_ST.EVENT_RESER_SPACE )            
            {               	
                printf("%s free %uMB,Rrq %uMB \n",BLVDS_EVENT_PATH,FreeSizeMB_U32_2,g_SpaceJudge_ST.EVENT_RESER_SPACE);
                snprintf(loginfo, sizeof(loginfo)-1, "%s free %uMB,Rrq %uMB",BLVDS_EVENT_PATH,FreeSizeMB_U32_2,g_SpaceJudge_ST.EVENT_RESER_SPACE);
                WRITELOGFILE(LOG_WARN_1,loginfo);   
                
                err = FileDirJudge(BLVDS_EVENT_PATH);
		        if(REC_FILE_TYPE == err )
		        {
		           DeleteEarliestFile(BLVDS_EVENT_PATH,RECORD_FILE_TYPE);
		        }
		        else if(REC_DIR_TYPE == err)
		        {
		           DeleteEarliestDir(BLVDS_EVENT_PATH);
		        }
            }
        }
        err = MultiDircCreate(File_Directory_U8);
        if(CODE_CREAT == err)
        {
            memset(loginfo,0,sizeof(loginfo));
            snprintf(loginfo, sizeof(loginfo)-1, "make Dir %s success",File_Directory_U8);
            WRITELOGFILE(LOG_INFO_1,loginfo); 
        }
        err = MultiDircCreate(File_BLVDS_Directory_U8);
        if(CODE_CREAT == err)
        {
            memset(loginfo,0,sizeof(loginfo));
            snprintf(loginfo, sizeof(loginfo)-1, "make Dir %s success",File_BLVDS_Directory_U8);
            WRITELOGFILE(LOG_INFO_1,loginfo); 
        }
    }

    snprintf(File_EventName_U8,sizeof(File_EventName_U8),"%s/%s%02d_%8.8s-%s.dat",File_Directory_U8,\
        RrdXml_p->Rec_Event_ST.RecFileHead,TranInfo_p->CoachNum_U8,TimeString_U8,&TimeString_U8[8]);
    snprintf(File_BLVDS_EventName_U8,sizeof(File_EventName_U8),"%s/%s%02d_%8.8s-%s.dat",File_BLVDS_Directory_U8,\
        BLVDS_EVENT_FileName_head,TranInfo_p->CoachNum_U8,TimeString_U8,&TimeString_U8[8]);

    file_p->EventFile_fd = fopen(File_EventName_U8,"a+");
    file_p->EventBLVDS_fd = fopen(File_BLVDS_EventName_U8,"a+");
    /*EVENT*/
    if(NULL == file_p->EventFile_fd)
    {
        TryNum ++;
        perror("creat Eventfile file failed");
        while(TryNum < FILETRY_NUM)
        {
            file_p->EventFile_fd = fopen(File_EventName_U8, "a+");   
            if(NULL ==  file_p->EventFile_fd )
            {
                TryNum ++;
                perror("creat Eventfile File failed again");
            }
            else
            {
                /*quit the cycle*/
                TryNum = 0;
                ECUErrInfo_ST->ecu_app_err.filesave_EVENT_err = 0;
                printf("creat Eventfile %s success\n",File_EventName_U8);
                snprintf(loginfo, sizeof(loginfo)-1, "creat Eventfile %s success!",File_EventName_U8);
                WRITELOGFILE(LOG_INFO_1,loginfo);
                err_1 = CODE_OK; 
                break; 
            }
        }
        if(TryNum >= FILETRY_NUM)
        {        
            ECUErrInfo_ST->ecu_app_err.filesave_EVENT_err = 1;
            snprintf(loginfo, sizeof(loginfo)-1, "creat Eventfile failed!");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            err_1 = CODE_ERR;            
        }
    }
    else
    {
        ECUErrInfo_ST->ecu_app_err.filesave_EVENT_err = 0;
        printf("creat Eventfile %s success!\n",File_EventName_U8);
        snprintf(loginfo, sizeof(loginfo)-1, "creat Eventfile %s success!",File_EventName_U8);
        WRITELOGFILE(LOG_INFO_1,loginfo); 
        err_1 = CODE_OK;       
    }
    /*BLVDS_EVENT*/
    if(NULL == file_p->EventBLVDS_fd)
    {
        TryNum ++;
        perror("creat BLVDS_Eventfile failed");
        while(TryNum < FILETRY_NUM)
        {
            file_p->EventBLVDS_fd = fopen(File_BLVDS_EventName_U8,"a+");   
            if(NULL == file_p->EventBLVDS_fd)
            {
                TryNum ++;
                perror("creat BLVDS_Eventfile failed again");
            }
            else
            {
                /*quit the cycle*/
                TryNum = 0;
                ECUErrInfo_ST->ecu_app_err.filesave_BLVDS_err = 0;
                printf("creat BLVDS_Eventfile %s success\n",File_BLVDS_EventName_U8);
                snprintf(loginfo, sizeof(loginfo)-1, "creat BLVDS_Eventfile %s success!",File_BLVDS_EventName_U8);
                WRITELOGFILE(LOG_INFO_1,loginfo);
                err_2 = CODE_OK; 
                break; 
            }
        }
        if(TryNum >= FILETRY_NUM)
        {        
            ECUErrInfo_ST->ecu_app_err.filesave_BLVDS_err  = 1;
            snprintf(loginfo, sizeof(loginfo)-1, "creat BLVDS_Eventfile failed!");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            err_2 = CODE_ERR;            
        }
    }
    else
    {
        ECUErrInfo_ST->ecu_app_err.filesave_BLVDS_err = 0;
        printf("creat BLVDS_Eventfile %s success!\n",File_BLVDS_EventName_U8);
        snprintf(loginfo, sizeof(loginfo)-1, "creat BLVDS_Eventfile %s success!",File_BLVDS_EventName_U8);
        WRITELOGFILE(LOG_INFO_1,loginfo);
        err_2 = CODE_OK;        
    }
    if(err_1 == 0)
    {
        err_3 = EventFileTopSave(file_p->EventFile_fd,RrdXml_p,TranInfo_p);
    }
    if(err_2 == 0)
    {
        err_4 = EventFileTopSave(file_p->EventBLVDS_fd,RrdXml_p,TranInfo_p);
    }
    
    return err_1 || err_2 || err_3 || err_4 ;
}

/**********************************************************************
*Name       :   int8_t ECU_Record_Data_Pro_Fun
*Function   :   process the data which will be stored in ecu
*Para       :   DRIVE_FILE_DATA  *Drive_ST_p    the address of Save DriveEventST
*               TMS570_BRAM_DATA *bram_data_rd  the address of Save ChanData_ST
                TMS570_BRAM_DATA *bram_data_wr  the address of Save ChanData_ST
                EADS_ERROR_INFO EADSErrInfoST the error info of device
*Return     :   int8_t 0,success;else false
*Version    :   REV1.0.0       
*Author:    :   zlz
*History:
*REV1.0.0     zlz    2021/12/29  Create                                                              
*********************************************************************/
int8_t ECU_Record_Data_Pro_Fun(DRIVE_FILE_DATA *Drive_ST_p,TMS570_BRAM_DATA *bram_data_rd,TMS570_BRAM_DATA *bram_data_wr,ECU_ERROR_INFO ECUErrInfoST)
{    
    uint8_t j= 0;
    uint8_t i = 0;
    uint8_t temp_rd_buffer[240]={0};
    uint8_t temp_wr_buffer[240]={0};
    memcpy(temp_rd_buffer,bram_data_rd->buffer,240);
    memcpy(temp_wr_buffer,bram_data_wr->buffer,240);
    /*数字量处理*/	
    Drive_ST_p -> DriveDigital_U8[0] = (temp_wr_buffer[2]&0x3) | ((temp_wr_buffer[3]<<2)&0xc) | (temp_wr_buffer[35]&0x70) | ((temp_wr_buffer[48] << 7)&0x80);
    Drive_ST_p -> DriveDigital_U8[1] = temp_rd_buffer[2];
    Drive_ST_p -> DriveDigital_U8[2] = temp_rd_buffer[3];
    memcpy(&Drive_ST_p -> DriveDigital_U8[3],&temp_rd_buffer[50],11);
    memcpy(&Drive_ST_p -> DriveDigital_U8[14],&temp_rd_buffer[62],18);
    Drive_ST_p -> DriveDigital_U8[28] = Drive_ST_p -> DriveDigital_U8[28] | ECUErrInfoST.ecu_app_err.bram_blvds_rd_err << 6 | ECUErrInfoST.ecu_app_err.bram_blvds_wr_err <<7;
    Drive_ST_p -> DriveDigital_U8[29] = Drive_ST_p -> DriveDigital_U8[29] | ECUErrInfoST.ecu_app_err.bram_mvb_rd_err <<3 | ECUErrInfoST.ecu_app_err.bram_mvb_wr_err << 4 \
                                        | ECUErrInfoST.ecu_app_err.filesave_EVENT_err << 5 | ECUErrInfoST.ecu_app_err.filesave_BLVDS_err << 6 | ECUErrInfoST.ecu_app_err.bram_init_err << 7 ;
    Drive_ST_p -> DriveDigital_U8[31] = Drive_ST_p -> DriveDigital_U8[31] | ECUErrInfoST.ecu_app_err.tcp_err << 1 | ECUErrInfoST.ecu_app_err.modbus_err << 2 | ECUErrInfoST.ecu_app_err.udp_err << 3 \
                                        | ECUErrInfoST.ecu_app_err.phy_link_err << 4 | ECUErrInfoST.ecu_app_err.power_err << 5 | ECUErrInfoST.ecu_app_err.max10_life_err << 6\
                                        | ECUErrInfoST.ecu_app_err.tms570_life_err << 7;
    /*模拟量处理*/
    memcpy(&Drive_ST_p -> DriveAnalog_U16[0],&temp_wr_buffer[36],6);
    Drive_ST_p -> DriveAnalog_U16[3] = temp_wr_buffer[42] << 8;
    Drive_ST_p -> DriveAnalog_U16[4] = temp_wr_buffer[43] << 8;
    memcpy(&Drive_ST_p -> DriveAnalog_U16[5],&temp_wr_buffer[44],4);
    memcpy(&Drive_ST_p -> DriveAnalog_U16[7],&temp_rd_buffer[4],12);
    for(i=0;i<12;i++)
    {
        Drive_ST_p -> DriveAnalog_U16[13+i] = temp_rd_buffer[18+i] << 8;
    }
    memcpy(&Drive_ST_p -> DriveAnalog_U16[25],&temp_rd_buffer[30],2);
    Drive_ST_p -> DriveAnalog_U16[26] = temp_rd_buffer[32] << 8;
    Drive_ST_p -> DriveAnalog_U16[27] = temp_rd_buffer[33] << 8;
    memcpy(&Drive_ST_p -> DriveAnalog_U16[28],&temp_rd_buffer[34],6);
    for(i=0;i<7;i++)
    {
        Drive_ST_p -> DriveAnalog_U16[31+i] = temp_rd_buffer[41+i] << 8;
    }
    memcpy(&Drive_ST_p -> DriveAnalog_U16[38],&temp_rd_buffer[82],16);
    memcpy(&Drive_ST_p -> DriveAnalog_U16[46],&temp_rd_buffer[100],4);
    for(i=0;i<8;i++)
    {
        Drive_ST_p -> DriveAnalog_U16[48+i] = temp_rd_buffer[104+i] << 8;
    }
    for(i=0;i<13;i++)
    {
        Drive_ST_p -> DriveAnalog_U16[56+i] = temp_rd_buffer[130+i] << 8;
    }
    Drive_ST_p -> DriveAnalog_U16[69] = temp_rd_buffer[146] << 8;
    Drive_ST_p -> DriveAnalog_U16[70] = temp_rd_buffer[147] << 8;
    memcpy(&Drive_ST_p -> DriveAnalog_U16[71],&temp_rd_buffer[148],18);
    /*big to small endian*/
    for (i=0;i<80;i++)
    {
        Drive_ST_p -> DriveAnalog_U16[i]=BigU8ToLitteU16(Drive_ST_p -> DriveAnalog_U16[i]&0xFF,\
                                                            Drive_ST_p -> DriveAnalog_U16[i]>>8);
    }
    /*CSR_Driver无法补足电流-500A的偏移量，由应用程序补足*/
    Drive_ST_p -> DriveAnalog_U16[72]  =  Drive_ST_p -> DriveAnalog_U16[72]-5000;//电堆A输出电流
    Drive_ST_p -> DriveAnalog_U16[74]  =  Drive_ST_p -> DriveAnalog_U16[74]-5000;//电堆B输出电流
    if(g_DebugType_EU == FileSave_DEBUG)
    {
        printf("File_save_Pro-Digital channel data:\n");
        for (i=0;i<32;i++)
        {
            printf("Digital-Channel[%02d]:0x[%02x]\n",i,Drive_ST_p -> DriveDigital_U8[i]);
        }
        printf("\nFile_save_Pro-Anolog channel data:\n");
        for (i=0; i<80;i++)
        {
            printf("Anolog-Channel[%02d]:0x[%04x]\n",i,Drive_ST_p -> DriveAnalog_U16[i]);
        }        
    }   
    return CODE_OK;    
}

/**********************************************************************
*Name       :   int8_t ECU_EventDataSave
*Function   :   Save the Event data to event file
*Para       :   FILE_FD * file_p
                DRIVE_FILE_DATA *Drive_ST_p
*Return     :   0->success;-1->false
*Version    :   REV1.0.0       
*Author:    :   zlz
*History:   :
*REV1.0.0   :  zlz   2021/12/29  Create
*********************************************************************/
int8_t ECU_EventDataSave(FILE_FD *file_p,DRIVE_FILE_DATA *Drive_ST_p)
{    
    int16_t fwerr = 0;   
    char loginfo[LOG_INFO_LENG] = {0};

    if(NULL == file_p -> EventFile_fd)
    {
        perror("EventFile_fd is NULL!\n");
        snprintf(loginfo, sizeof(loginfo)-1, "EventFile_fd is NULL!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    if(g_DebugType_EU == FILE_DEBUG)
    {
        printf("EventFile_fd %d \n",file_p->EventFile_fd);
    }    
    
    fwerr = FileWriteWithTry(&Drive_ST_p->DriveDigital_U8[0],DIGITAL_NUM_BYTE,1,file_p->EventFile_fd);
     if(CODE_OK != fwerr)
    {
        printf("EventFile_fd fwrite Digital data  error!\n");
        snprintf(loginfo, sizeof(loginfo)-1, "EventFile_fd fwrite Digital data  error!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }      
    
    fwerr = FileWriteWithTry(&Drive_ST_p->DriveAnalog_U16[0],ANOLOG_NUM_BYTE,1,file_p->EventFile_fd);
    if(CODE_OK != fwerr)
    {
        printf("EventFile_fd fwrite Analog Data error!\n");
        memset(loginfo,0,LOG_INFO_LENG);
        snprintf(loginfo, sizeof(loginfo)-1, "EventFile_fd fwrite Analog Data error!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    fflush(file_p -> EventFile_fd);
    return fwerr;    
}

/**
 * @description: Bram MAX10 Read Data Process
 * @param:       TMS570_BRAM_DATA *bram_data,DRIVE_FILE_DATA *Drive_ST_p
 * @return:      uint8_t life stop ->1 life normal ->0
 * @author:      zlz
 */
int8_t MAX10_RD_DataProc(TMS570_BRAM_DATA *bram_data,DRIVE_FILE_DATA *Drive_ST_p)
{
    uint8_t i=0,j=0;
    uint8_t temparray_u8[24]={0};
    uint8_t digital_tempdata[8] = {0};
    BYTE_BIT digtal_byte_bit[5] = {0};
    uint16_t temparray_u16[10]={0};
    float temparray_f[10]={0};
    /*MAX10-DI info*/    
    memcpy(Drive_ST_p->DriveDigital_U8,bram_data->buffer,10);
    /*MAX10-DO info*/ 
    memcpy(digital_tempdata,&bram_data->buffer[2],8);
    /*
    for(i=2;i<7;i++)
    {   
        printf("digital_tempdata[%d]:%02x\n",i,digital_tempdata[i]);
    } */    
    
    for(i=0;i<5;i++)
    {
        switch(digital_tempdata[2+i] & 0xF)
        {
            case(0x9):
                digtal_byte_bit[i].Bit0 = 1;/*开通*/
            break;
            case(0xa):
                digtal_byte_bit[i].Bit1 = 1;/*关断*/
            break;
            case(0x6):
                digtal_byte_bit[i].Bit2 = 1;/*过流*/
            break;
            case(0x5):
                digtal_byte_bit[i].Bit3 = 1;/*反馈异常*/
            break;
            default:
            break;            
        }
        switch((digital_tempdata[2+i]>>4) & 0xF)
        {
            case(0x9):
                digtal_byte_bit[i].Bit4 = 1;/*开通*/
            break;
            case(0xa):
                digtal_byte_bit[i].Bit5 = 1;/*关断*/
            break;
            case(0x6):
                digtal_byte_bit[i].Bit6 = 1;/*过流*/
            break;
            case(0x5):
                digtal_byte_bit[i].Bit7 = 1;/*反馈异常*/
            break;
            default:
            break;            
        }
    } 

    /*for(i=0;i<5;i++)
    {
        printf("digtal_byte_bit[%d]:0x%02x\n",i,digtal_byte_bit[i]);
    } */  

    memcpy(&Drive_ST_p->DriveDigital_U8[10],digtal_byte_bit,5);
    /*MAX10_Anolog current calculate*/
    memcpy(temparray_u8,&bram_data->buffer[3],24);    
    for(i=0;i<10;i++)
    {        
        temparray_u16[i] =  temparray_u8[3+j] | temparray_u8[4+j] << 8 ;
        //printf("temparray_u16[%d]:0x%04d\n",i,temparray_u16[i]);             
        temparray_f[i] =  temparray_u16[i]*1.5;          
        temparray_u16[i] = (uint16_t)(temparray_f[i] - 3103.03);
        j=j+2;       
    }
    memcpy(Drive_ST_p->DriveAnalog_U16,temparray_u16,20);
    /*MAX10 main mos info*/
    memcpy(&Drive_ST_p->DriveDigital_U8[15],&temparray_u8[23],1);
    return CODE_OK;
}

/**
 * @description: Bram MAX10 Write Data Process
 * @param:       TMS570_BRAM_DATA *bram_data
 * @return:      uint8_t life stop ->1 life normal ->0
 * @author:      zlz
 */
int8_t MAX10_WR_DataProc(TMS570_BRAM_DATA *bram_data)
{

}
/**********************************************************************
*Name       :   int8_t MAX10_EventDataSave
*Function   :   Save the Event data to event file
*Para       :   FILE_FD * file_p
                DRIVE_FILE_DATA *Drive_ST_p
*Return     :   0->success;-1->false
*Version    :   REV1.0.0       
*Author:    :   zlz
*History:   :
*REV1.0.0   :  zlz   2021/12/29  Create
*********************************************************************/
int8_t MAX10_EventDataSave(FILE_FD *file_p,DRIVE_FILE_DATA *Drive_ST_p)
{    
    int16_t fwerr = 0;   
    char loginfo[LOG_INFO_LENG] = {0};

    if(NULL == file_p->EventBLVDS_fd)
    {
        perror("EventBLVDS_fd is NULL!\n");
        snprintf(loginfo, sizeof(loginfo)-1, "EventBLVDS_fd is NULL!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    if(g_DebugType_EU == FILE_DEBUG)
    {
        printf("EventBLVDS_fd %d\n",file_p->EventBLVDS_fd);
    }    
    
    fwerr = FileWriteWithTry(&Drive_ST_p->DriveDigital_U8[0],DIGITAL_NUM_BYTE,1,file_p->EventBLVDS_fd);
    if(CODE_OK != fwerr)
    {
        printf("EventBLVDS_fd fwrite Digital data error!\n");
        snprintf(loginfo, sizeof(loginfo)-1, "EventBLVDS_fd fwrite Digital data error!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }      
    
    fwerr = FileWriteWithTry(&Drive_ST_p->DriveAnalog_U16[0],ANOLOG_NUM_BYTE,1,file_p->EventBLVDS_fd);
    if(CODE_OK != fwerr)
    {
        printf("EventBLVDS_fd fwrite Analog Data error!\n");
        memset(loginfo,0,LOG_INFO_LENG);
        snprintf(loginfo, sizeof(loginfo)-1,"EventBLVDS_fd fwrite Analog Data error!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    fflush(file_p->EventBLVDS_fd);
    return fwerr;    
}