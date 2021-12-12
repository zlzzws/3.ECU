/**********************************************************************
*File name      :   main.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/1/29
*Description    :   the entry of the whole program
*Version        :   REV1.0.0       
*Author:        :   zuolinzheng
*
*History:
*REV1.0.0     feng    2021/12/03  Create
*
*********************************************************************/


/**********************************************************************
*
*Include file  section
*
*********************************************************************/
#include "BaseDefine.h"
#include "Socket.h"
#include "BramDataProc.h"
#include "ModbusSer.h"
#include "GPIOControl.h"
#include "FileSave.h"
#include "ModbusClientRtu.h"
#include "Rs232Fuctcl.h"
#include "xml.h"
/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/

struct sigaction Alarm_act;
BLVDS_BRD_DATA g_BrdRdBufData_ST = {0};
CHAN_DATA  g_ChanData_ST[CHAN_BUFFER_NUM] = {0};  /*304 byte*/
CHAN_DATA  g_ChanRealBuf_ST[REAL_BUFFER_NUM] = {0};  /*REAL_BUFFER_NUM = 800*/
CHAN_DATA  g_ChanSendBuf_ST[CHAN_BUFFER_NUM] = {0};  /*304 byte*/
CHAN_CALIB_DATA  g_ChanCalib110VData_ST = {0};/*read the ChanCalibData from /tffs0/ChanCalibValue.dat"*/
CHAN_CALIB_DATA  g_ChanCalib0VData_ST = {0};/*read the ChanCalibData from /tffs0/ChanCalibValue.dat"*/
CHAN_STATUS_INFO  g_ChanStatuInfo_ST = {0};/*include the real file save,Chan operate num*/
CHAN_LG_INFO g_ChanLgInfo_ST = {0};/*the chanel logic info ,for save and trdp*/
CHAN_DIGITAL_INFO g_ChanDigitalInfo_ST = {0};/*include the digital status of vol chan ,and overVOl overCurr*/
RELAY_LIFE_CFG   g_ChanRelayCfg_ST = {0};/*read the relay param config by xml */
EADS_ERROR_INFO  g_EADSErrInfo_ST  = {0};
VECH_EADS_INFO g_CCU_EADsInfo_ST = {0};//byte 1128,for even file save
TRAIN_INFO g_TrainInfo_ST = {0};
VERSION  g_Version_ST = {0};
FILE_FD   g_FileFd_ST = {0};/*the save file FP*/
RECORD_XML g_Rec_XML_ST = {0};
RECORD_XML g_LifeRec_XML_ST = {0};
SPACE_JUDGE_VALUE  g_SpaceJudge_ST = {0};
PTHREAD_INFO g_Pthread_ST = {0};
PTHREAD_LOCK g_PthreadLock_ST; 
DEBUG_TYPE_ENUM g_DebugType_EU;
uint32_t g_RealBufNum_U32 = 0;
uint16_t g_PowDebug = 0;
uint32_t g_LoopNum_U32 = 0;
uint8_t g_ProcNum_U8 = 0;
uint16_t g_ChanFiltNum_U16 = 0;
uint32_t g_ADReadSleep_U32 = 0;
uint32_t g_FltSaveSlepNum_U32 = 0;
uint32_t g_TRDPUsec_U32 = 0;
uint32_t g_LinuxDebug = 0;
uint8_t g_EADSType_U8 = 0;
int8_t g_fd_TtyPs = 0;     //文件描述符 
int8_t g_LifeFlag = 1;     //to controll the pthread
int8_t g_EADSType = 0;
uint8_t g_ChanRealWave[48] = {0}; /*for the channel real wave */
uint8_t g_ChanRealNum = 0;  /*for the channel real wave */
uint8_t g_socket_SendFlag = 0; /*for tcp modbus comunicate*/
sem_t g_ReadBrd_sem;
sem_t g_RealSend_sem;
// RTUThreadFun 
SENSOR_NUM_ENUM g_DMU_Senor_num = 1;
uint16_t g_rtu_ReadData[100] = {0};
uint16_t g_RTUBaud_U32 = 0;
uint32_t g_RTUUsec_U32 = 0;
RTU_SENSOR  SmartSensor_ST = {0};
modbus_t *g_ModbusCtx = NULL;
// UartThreadFun 
uint32_t g_UartUsec_U32 = 0;
//TRDP使用，包含ADU及语音指纹模块的故障位
BYTE_BIT g_ADU_ErrInfo = {0};
BYTE_BIT g_VoiceFinger_ErrInfo = {0};
/**********************************************************************
*
*Local Macro Define Section
*
*********************************************************************/
#define WDIOC_SETTIMEOUT _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
/**********************************************************************
*
*Local Struct Define Section
*
*********************************************************************/
static TMS570_BRAM_DATA s_tms570_bram_RD_data_st[5] = {0};
static TMS570_BRAM_DATA s_tms570_bram_WR_data_st[5] = {0};
struct can_frame s_can0_frame_RD_st[16] = {0};
struct can_frame s_can0_frame_WR_st[16] = {0};
struct can_frame s_can1_frame_RD_st[8] = {0};
struct can_frame s_can1_frame_WR_st[8] = {0};
/**********************************************************************
*
*Local Prototype Declare Section
*
*********************************************************************/
void    FuncUsage(void);
void    ArgJudge(void);
void   *RealWaveThreadFunc(void *arg);
void   *ModbusThreadFunc(void *arg);
int8_t  PowDownFun(void);
void   *FileSaveThreaFunc(void *arg);
void   *LEDWachDogPthreadFunc (void *arg);
void   *DirTarThreadFunc(void *arg);
int8_t  ThreadInit(PTHREAD_INFO * pthread_ST_p);
int8_t  ThreadOff(FILE_FD * FileFd_p,PTHREAD_INFO  * pthread_ST_p);
void    *MVBThreadFunc(void *arg);
void    *TMS570_Bram_ThreadFunc(void *arg) ; 
void    *CAN0ThreadFunc(void *arg);
void    *CAN1ThreadFunc(void *arg);


/**********************************************************************
*
*Static Variable Define Section
*
*********************************************************************/
int main(int argc, char *argv[])
{      
    struct timeval Time_ST,TimeEnd_ST;    
    int8_t res=0;
    int8_t fifofd = 0;
    uint8_t BinLife = 0;
    uint8_t FifoWrNum = 0;
    uint8_t FifoErr = 0;
    uint16_t FifoWrTime = 0;
	uint32_t EmmcTotalSizeMB_U32 = 0,EmmcFreeSizeMB_U32 = 0;  
    char ArgLogInfo[LOG_INFO_LENG] = {0};
    uint8_t BOXID =0;
    uint8_t SLOTID =0;
	char begin[LOG_INFO_LENG] = {0};
	char end[LOG_INFO_LENG] = {0};
    int SemtValue = 0;	
	struct timeval A_Time_ST,A_TimeEnd_ST;
	uint8_t i = 0;
	int SemValue = 0;
    int i2cbus_fd =0; 
	int8_t fd = 0;

	GetMemSize("/yaffs/",&EmmcTotalSizeMB_U32,&EmmcFreeSizeMB_U32);
	if(EmmcTotalSizeMB_U32 > 50000)//48G
	{ 
		g_SpaceJudge_ST.EVENT_RESER_SPACE   = 20480;//20G
		g_SpaceJudge_ST.FLT_RESER_SPACE     = 15360;//15G
		g_SpaceJudge_ST.OPRT_RESER_SPACE    = 20480;//20G
		g_SpaceJudge_ST.LOGFILE_NUM         = 100;  //100条
        g_SpaceJudge_ST.CAN_RESER_SPACE     = 3072; //3G
        g_SpaceJudge_ST.UART_RESER_SPACE    = 3072; //3G
		g_SpaceJudge_ST.MIN_RESER_SPACE     = 4096; //4G
	}	
	else
	{
		g_SpaceJudge_ST.EVENT_RESER_SPACE   = 2048;
		g_SpaceJudge_ST.FLT_RESER_SPACE     = 1536;
		g_SpaceJudge_ST.OPRT_RESER_SPACE    = 2048;
		g_SpaceJudge_ST.LOGFILE_NUM         = 30;
        g_SpaceJudge_ST.CAN_RESER_SPACE     = 300;
        g_SpaceJudge_ST.UART_RESER_SPACE    = 300;
		g_SpaceJudge_ST.MIN_RESER_SPACE     = 600;
	}
    //上电创建日志文件
    LogFileCreatePowOn();

    if (argc < 5)
    {
        FuncUsage();
        snprintf(ArgLogInfo, sizeof(ArgLogInfo)-1, "arg Num: %u,arg1 %s,arg2 %s,arg3 %s,arg4 %s,arg5 %s",
        argc,argv[1],argv[2],argv[3],argv[4],argv[5]);
        WRITELOGFILE(LOG_ERROR_1,ArgLogInfo);
        LogFileSync();
    }    
    g_LifeFlag = 1;                                                         //生命信号标志位  
    g_EADSType_U8 = 0;                                                      //暂时强制type为1，避免程序出现错误  
    g_Version_ST.EADS_RunVer_U16 = EADS_VERSION_PTU;                        //EADS软件版本，可通过PTU查看
    g_ProcNum_U8 = 4;  
    g_DebugType_EU =        (DEBUG_TYPE_ENUM)strtoul(argv[1], NULL, 10);    //Debug类型    
    g_FltSaveSlepNum_U32 =  (uint32_t)strtoul(argv[2], NULL, 10);           //文件存储延时
    g_PowDebug =            (uint16_t)strtoul(argv[3], NULL, 10);           //电源选项，涉及到掉电监测    
    g_LinuxDebug =          (uint16_t)strtoul(argv[4], NULL, 10);           //软件运行环境:0:ZYNQ 1:Ubuntu
    
    ArgJudge();//对输入参数的合法性进行判断，超过量程的数据会进行修正

    snprintf(ArgLogInfo, sizeof(ArgLogInfo)-1, "ProcNum %u,DebugType %u,EventSaveSlepNum %u,g_PowDebug %u,TRDPUsec %u",
        g_ProcNum_U8,g_DebugType_EU,g_FltSaveSlepNum_U32,g_PowDebug,g_TRDPUsec_U32);
    WRITELOGFILE(LOG_INFO_1,ArgLogInfo);	
    
    printf("g_ProcNum_U8 %u\n",g_ProcNum_U8);
    printf("g_DebugType_EU %u\n",g_DebugType_EU);    
    printf("g_FltSaveSlepNum_U32 %u\n",g_FltSaveSlepNum_U32);
    printf("g_PowDebug %u\n",g_PowDebug);
    printf("g_TRDPUsec_U32 %u\n",g_TRDPUsec_U32);    
	
    if (0 == g_LinuxDebug) 
    {
       Bram_Mapping_Init(&g_EADSErrInfo_ST,g_EADSType_U8);        
    }

    RTCTesT(); //TODO:考虑在此函数中增加上电RTC打印时间信息（是否有意义？）
    /**拨码开关-I2C功能*/
    i2cbus_fd=open(DEFAULT_I2C_BUS,O_RDWR);  
    i2c_read(i2cbus_fd,0X20,0,&BOXID,1);
    printf("BOXID is %x\n",BOXID);    
    printf("DataProcPowerOn!\n");
    //DataProcPowerOn(&g_ChanStatuInfo_ST,&g_ChanCalib0VData_ST,&g_ChanCalib110VData_ST,g_ProcNum_U8,g_EADSType_U8);    
    FileCreatePowOn(&g_FileFd_ST,&g_Rec_XML_ST,&g_LifeRec_XML_ST,&g_TrainInfo_ST,&g_EADSErrInfo_ST);       
    VersionInit(&g_Version_ST);                 //程序版本
    VersionSave(&g_Version_ST);                 //程序版本
    ThreadInit(&g_Pthread_ST);                //线程初始化

    /*****创建守护进程通讯管道*****/
    fifofd = open(FIFO_FILE,O_WRONLY|O_NONBLOCK,0); 
    if(fifofd == -1)
    {
        perror("open fifo File error ");
        memset(ArgLogInfo,0,LOG_INFO_LENG);
        snprintf(ArgLogInfo, sizeof(ArgLogInfo)-1, "open fifo File error");
        WRITELOGFILE(LOG_ERROR_1,ArgLogInfo);
        FifoErr =  1;
    }
    

    /*****监视掉电信息*****/
    if (1 == g_PowDebug)
    {
        GPIO_PowDownIoCreat();
        GPIO_PowDowOpen();    
    }
    
    while(1)
    {                         
		/*******取消此信号量*******
        res = sem_wait(&g_ReadBrd_sem);
        if (res != 0) 
        {
            printf("errno of sem_wait  %d\n",errno);  
            snprintf(ArgLogInfo, sizeof(ArgLogInfo)-1, "errno of sem_wait  %d",errno);
            WRITELOGFILE(LOG_ERROR_1,ArgLogInfo);              
        }
        if(g_DebugType_EU == SEM_DEBUG)        
        {            
            printf("g_ReadBrd_sem wait:%x\n",g_ReadBrd_sem);               
        }
        **************************/
        if(TIME_DEBUG  == g_DebugType_EU)
        {
            gettimeofday(&Time_ST,NULL);                
        }           
        
        if(1 == g_PowDebug)
        {
          PowDownFun();
        }
        /***********取消数据处理        
        EADSDataProc(g_ProcNum_U8,g_EADSType_U8,&g_EADSErrInfo_ST);               
        EADSDataBuf(g_ProcNum_U8);
        ***********************/
        //csr_driver波形实时显示线程的自旋锁++
        if(1 == g_socket_SendFlag)
        {   
            sem_post(&g_RealSend_sem);
            if(g_DebugType_EU == SEM_DEBUG)
            {               
                printf("g_RealSend_sem post:%x\n",g_RealSend_sem);               
            }
        }
        /*与守护进程管道通讯*/
        FifoWrTime++;
        if((FifoWrTime >= 20) && (FifoErr != 1)) /*20ms*/
        {  
            
            FifoWrTime = 0;
            BinLife++;

            if((FifoWrNum = write(fifofd,&BinLife,1)) == -1)
            {
                perror("FIFO has not been read yet \n");
                memset(ArgLogInfo,0,LOG_INFO_LENG);
                snprintf(ArgLogInfo, sizeof(ArgLogInfo)-1, "FIFO has not been read yet");
                WRITELOGFILE(LOG_ERROR_1,ArgLogInfo); 
            }
            else
            {
                if(DEVC_DEBUG == g_DebugType_EU)
                {
                    printf("write %d to the FIFO\n",BinLife); 
                }

            }              
        }
        /***********取消故障波形记录及动作波形记录
		pthread_rwlock_rdlock(&g_PthreadLock_ST.ChanInfolock);
        //故障波形记录
        FltRealFileProc(&g_FileFd_ST,&g_ChanStatuInfo_ST,&g_Rec_XML_ST,&g_TrainInfo_ST,g_ProcNum_U8,g_EADSType_U8);
        //动作波形记录
		OprtRealFileProc(&g_FileFd_ST,&g_ChanStatuInfo_ST,&g_Rec_XML_ST,&g_TrainInfo_ST,g_ProcNum_U8,g_EADSType_U8);        
        pthread_rwlock_unlock(&g_PthreadLock_ST.ChanInfolock);
        *****************************************/

        if(TIME_DEBUG  == g_DebugType_EU)
        {
            gettimeofday(&TimeEnd_ST,NULL);
            printf("main thread tim:%u \n",(uint32_t)TimeEnd_ST.tv_usec- (uint32_t)Time_ST.tv_usec); 
            printf("main thread tim usec:%u \n",(uint32_t)TimeEnd_ST.tv_usec);                
        }        
    }
    printf("Waiting for all threads to power off...\n");
    ThreadOff(&g_FileFd_ST,&g_Pthread_ST);
    
    if(0 == g_LinuxDebug)
    {
        BramCloseMap(); 
    }
    close(fifofd); 
    printf("Main function is exit ! All exit-operations is done\n");
    exit(EXIT_SUCCESS);
}


void FuncUsage(void)
{
    printf("For example:\n");
    printf("you can type such command: ./Demo_Run_bin  <DebugTest_EU> <g_FltSaveSlepNum_U32> <g_PowDebug> <g_TRDPUsec_U32> <g_LinuxDebug>\n");   
}

void ArgJudge(void)
{
    if((0 == g_ProcNum_U8) || ( g_ProcNum_U8 > 5))
    {
        g_ProcNum_U8 = 4;
    }
    if( g_DebugType_EU >40)
    {
        g_DebugType_EU = 0;
    }
    if((g_TRDPUsec_U32 < 10000) || ( g_TRDPUsec_U32 > 2000000))
    {
        g_TRDPUsec_U32 = 10000;
    }
    if(g_LinuxDebug > 1)
    {
        g_LinuxDebug = 0;
    }
}


/**********************************************************************
*Name           :    ThreadInit  
*Function       :    Create the all  thread
*Para           :  
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/5/6  Create
*********************************************************************/
int8_t ThreadInit(PTHREAD_INFO * pthread_ST_p)
{
    int8_t res=0;
    char loginfo[LOG_INFO_LENG] = {0};
    struct sched_param param;
    int policy;
    #if 0
    /*********取消
    res = sem_init(&g_ReadBrd_sem, 0, 0);
    if (res != 0) 
    {
        perror("ReadBrd_sem Semaphore  initialization failed");
        snprintf(loginfo, sizeof(loginfo)-1, "ReadBrd_sem init failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    ***/
    res = sem_init(&g_RealSend_sem, 0, 0);
    if (res != 0) 
    {
        perror("RealSend_sem Semaphore initialization failed");
        snprintf(loginfo, sizeof(loginfo)-1, "RealSend_sem init failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    res = pthread_rwlock_init(&g_PthreadLock_ST.BramDatalock,NULL);
    if(res != 0)
    {
        perror("BramDatalock init failed");
        snprintf(loginfo, sizeof(loginfo)-1, "BramDatalock init failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);        
    }
    
    res = pthread_rwlock_init(&g_PthreadLock_ST.ChanDatalock,NULL);
    if(res != 0)
    {
        perror("ChanDatalock init failed");
        snprintf(loginfo, sizeof(loginfo)-1, "ChanDatalock initfailed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);        
    }
    res = pthread_rwlock_init(&g_PthreadLock_ST.RealDatalock,NULL);
    if(res != 0)
    {
        perror("RealDatalock init failed");
        snprintf(loginfo, sizeof(loginfo)-1, "RealDatalock init failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);        
    }
    res = pthread_rwlock_init(&g_PthreadLock_ST.ChanInfolock,NULL);
    if(res != 0)
    {
        perror("ChanInfolock init failed");
        snprintf(loginfo, sizeof(loginfo)-1, "ChanInfolock init failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);        
    }
    
    /*Create the Pthread*/
    res = pthread_create(&pthread_ST_p -> ReadDataThread, NULL, DataReadThreadFunc, NULL);
    if (res != 0) 
    {
        perror("create ReadDataThread failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create ReadDataThread failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);     
       // exit(EXIT_FAILURE);
    }
    param.sched_priority = 10;
    policy = SCHED_FIFO;
    pthread_setschedparam(pthread_ST_p -> ReadDataThread, policy, &param);
    
    res = pthread_create(&pthread_ST_p -> RealWaveThread, NULL, RealWaveThreadFunc, NULL);
    if (res != 0) 
    {
        perror("create RealWaveThread  failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create RealWaveThread failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);    
    }
    
    res = pthread_create(&pthread_ST_p -> TRDPThread, NULL, TRDPThreadFunc, NULL);
    if (res != 0) 
    {

        perror("create TRDPThreadFunc  failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create TRDPThreadFunc failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);    
    }
    res = pthread_create(&pthread_ST_p -> FileSaveThread, NULL, FileSaveThreaFunc, NULL);
    if (res != 0) 
    { 
        perror("create FileSaveThreaFun failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create FileSaveThread failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }

    res = pthread_create(&pthread_ST_p -> ModbusThread, NULL, ModbusThreadFunc, NULL);
    if (res != 0) 
    { 
        perror("create ModbusThreadFunc failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create ModbusThreadFunc failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);   
    }

    res = pthread_create(&pthread_ST_p -> LedThread, NULL, LEDWachDogPthreadFunc, NULL);
    if (res != 0) 
    { 
        perror("create LEDWachDogPthreadFunc failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create LEDWachDogPthreadFunc failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);      
    }

    res = pthread_create(&pthread_ST_p -> DirTarThread, NULL, DirTarThreadFunc, NULL);
    if (res != 0) 
    { 
        perror("create DirTarThreaFunc failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create DirTarThreaFunc failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);      
    }
    #endif
    res = pthread_create(&pthread_ST_p -> MVBThread, NULL, MVBThreadFunc, NULL);
    if (res != 0) 
    { 
        perror("create MVBThreaFunc failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create MVBThreaFunc failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);      
    }
    
    res = pthread_create(&pthread_ST_p -> CAN0Thread, NULL,CAN0ThreadFunc, NULL);
    if (res != 0) 
    { 
        perror("create CAN0ThreadFunc failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create CAN0ThreadFunc failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);     
    }

    res = pthread_create(&pthread_ST_p -> CAN1Thread, NULL,CAN1ThreadFunc, NULL);
    if (res != 0)
    {
        perror("create CAN1ThreadFunc failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create CAN1ThreadFunc failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    
    res = pthread_create(&pthread_ST_p -> TMS570_Bram_Thread, NULL,TMS570_Bram_ThreadFunc, NULL); 
    if (res != 0) 
    { 
        perror("create TMS570_Bram_ThreadFun failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create TMS570_Bram_ThreadFun failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);      
    }
    /*pthread_detach(pthread_ST_p -> DirTarThread);
    pthread_detach(pthread_ST_p -> RealWaveThread);
    pthread_detach(pthread_ST_p -> TRDPThread);
    pthread_detach(pthread_ST_p -> ModbusThread);*/       
    pthread_detach(pthread_ST_p -> MVBThread);
    pthread_detach(pthread_ST_p -> CAN0Thread);
    pthread_detach(pthread_ST_p -> CAN1Thread);
    pthread_detach(pthread_ST_p -> TMS570_Bram_Thread);    
}

/**********************************************************************
*Name           :    ThreadOff  
*Function       :    join and recycle the resource of thread
*Para           :  
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/5/6  Create
*********************************************************************/

int8_t  ThreadOff(FILE_FD * FileFd_p,PTHREAD_INFO  * pthread_ST_p)
{
    void *thread_result;
    int8_t res=0;
    #if 0
    res = pthread_join(pthread_ST_p -> FileSaveThread, &thread_result);
    if (res == 0)
    {
        printf("thread_join FileSaveThread success\n");
    }
    else 
    {
        perror("pthread_join FileSaveThread failed\n");           
    }
     
    res = pthread_join(pthread_ST_p ->ReadDataThread, &thread_result);
    if (res == 0)
    {
        printf("thread_join ReadDataThread success\n");
    }
    else 
    {
        perror("pthread_join ReadDataThread failed\n");           
    }
    
    res = pthread_join(pthread_ST_p -> LedThread, &thread_result);
    if (res == 0)
    {
        printf("thread_join LedThread success\n");
    }
    else 
    {
        perror("pthread_join LedThread failed\n");           
    }
    sem_destroy(&g_ReadBrd_sem);
    sem_destroy(&g_RealSend_sem);
    #endif
}

/**********************************************************************
*Name           :    PowDownFun  
*Function       :    proce the Pow down IO signal,and fsync buffer page data to emmc
*Para           :  
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/5/6  Create
*********************************************************************/

int8_t PowDownFun(void)
{
    struct timeval A_Time_ST,A_TimeEnd_ST;
    uint8_t PowDowIOValue = 2;
    char LogInfo[LOG_INFO_LENG] = {0};
    int8_t fd = 0;
    uint16_t i= 0;
    static uint8_t s_PowDownNum = 0;
    static uint8_t s_ClearPowNum = 0;
    if(POWTIME_DEBUG  == g_DebugType_EU)
    {
        gettimeofday(&A_Time_ST,NULL);
                
    }
    GPIO_PowDowRead(&PowDowIOValue); //cause 13~15 us
    if(POWTIME_DEBUG  == g_DebugType_EU)
    {                  
        gettimeofday(&A_TimeEnd_ST,NULL);
        printf("Read POw Io tim:%d \n",(uint32_t)A_TimeEnd_ST.tv_usec);                                
    }
    if(0 == PowDowIOValue)
    {
        s_PowDownNum++;
        if(s_PowDownNum > POWDOW_FILT)
        {            
           if(0 == g_EADSErrInfo_ST.PowErr)
            {
                g_EADSErrInfo_ST.PowErr = 1;
                printf("PowerOff happen,start sync File \n");
				snprintf(LogInfo, sizeof(LogInfo)-1, "PowerOff happen,start sync File");
                WRITELOGFILE(LOG_INFO_1,LogInfo);
				LogFileSync();                
                OperNumFileSave(&g_FileFd_ST,&g_ChanStatuInfo_ST,&g_Rec_XML_ST,&g_TrainInfo_ST);
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

                snprintf(LogInfo, sizeof(LogInfo)-1, "PowerOff happen,sync File");
                WRITELOGFILE(LOG_INFO_1,LogInfo);
                LogFileSync();
                printf("PowerOff happen, end  sync File \n");             
            }
            s_PowDownNum = 0;          
        }
        s_ClearPowNum = 0;    
    }
    else 
    {
        /*to avoid accident power and restore the power*/
        /*almost not meet this condition*/
        s_ClearPowNum++;
        if(s_ClearPowNum > POWDOW_FILT)/*10ms*/
        {
            
            if(1 == g_EADSErrInfo_ST.PowErr)
            {
            
                g_EADSErrInfo_ST.PowErr = 0;
                printf("PowerOff restore\n");
                snprintf(LogInfo, sizeof(LogInfo)-1, "PowerOff restore");
                WRITELOGFILE(LOG_INFO_1,LogInfo);
               
            }
            s_ClearPowNum = 0;
        }
        s_PowDownNum = 0;
        
    }
}
/**********************************************************************
*Name           :    RealWaveThreadFunc  
*Function       :    communicate with CSR_drive real wave display
*Para           :  
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/1/6  Create
*********************************************************************/
void *RealWaveThreadFunc(void *arg)
{ 
    int total;
    int i,j;
    int serverfd,clientfd;
    int16_t WriteSize = 0;
    int16_t ReadSize = 0;
    uint8_t SendSize = 0;
    char readbuf[20] = {0};
    uint8_t  returnbuf[20]={0};
    int semValue = 0 ; 
    char loginfo[LOG_INFO_LENG] = {0};
    uint32_t sendNum = 0;
    
    struct sockaddr_in server,client;
    struct sigaction action;    
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;    
    // 通信断开，写缓存区出错，产生SIGPIPE信号，引导程序异常退出，所以需要设置忽略SIGPIPE。
    action.sa_handler = SIG_IGN;//set function to here
    sigaction(SIGPIPE, &action, 0);

    if(-1==( serverfd= socket(AF_INET,SOCK_STREAM,0)))
    {
        perror("socket error\n");
        snprintf(loginfo, sizeof(loginfo)-1, "socket error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        pthread_exit(NULL);
    }
    bzero(&server,sizeof(server));

    server.sin_family=AF_INET;
    server.sin_port=htons(TCP_PORT);
    server.sin_addr.s_addr=INADDR_ANY;//inet_addr(ip);
    int opt = 1;
    setsockopt(serverfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    if(-1 == bind(serverfd,(struct sockaddr *)&server,sizeof(server)))
    {
        perror("bind error\n");
        snprintf(loginfo, sizeof(loginfo)-1, "bind error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        pthread_exit(NULL);
    }

    if(-1 == listen(serverfd,10))
    {
        perror("listen error\n");
        snprintf(loginfo, sizeof(loginfo)-1, "listen error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        pthread_exit(NULL);
    }
    if(g_DebugType_EU == TCP_DEBUG)
    {
        printf("sart listen...............\n");  
    }
    socklen_t len = sizeof(client);
    bzero(&client,len); 
    while(g_LifeFlag > 0)
    {

        if(-1==(clientfd = accept(serverfd,(struct sockaddr *)&client,&len)))
        {
            perror("accept error\n");
            snprintf(loginfo, sizeof(loginfo)-1, "listen error");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            close(serverfd);
            continue;

        }
        if(DEVC_DEBUG == g_DebugType_EU)
        {
                
            printf("clientfd : %d\n",clientfd);
            printf("client port : %d \n",ntohs(client.sin_port));  
        }
        printf("TCP communicate succeeded\n");

        snprintf(loginfo, sizeof(loginfo)-1, "TCP communicate succeeded");
        WRITELOGFILE(LOG_INFO_1,loginfo);
        if(read(clientfd ,readbuf,sizeof(readbuf))>0)
        {            
            if(TCP_DEBUG == g_DebugType_EU)
            {
                for (i=0;i<20;i++)
                {
                    printf("%d,data %02x \n",i,readbuf[i]);
                } 
            }       
            //clear for next wave connect and send data.
            memset(g_ChanRealWave,0,sizeof(g_ChanRealWave));
            g_ChanRealNum = 0;
            RealTimeWaveExtr(readbuf,g_ChanRealWave,&g_ChanRealNum);
            SendSize = g_ChanRealNum << 1;           
        }
        printf("start TCP send\n");
        g_socket_SendFlag  = 1;
        while(g_socket_SendFlag)
        {
            sem_wait(&g_RealSend_sem);                
            if(g_DebugType_EU == SEM_DEBUG)
            {                
                printf("g_RealSend_sem wait %x\n",g_RealSend_sem);               
            }
            if(g_DebugType_EU ==  DEVC_DEBUG)
            {
                printf("Start  RealWaveData\n");
            }
                for(i =0;i < g_ProcNum_U8;i++)
                {
                    memset(returnbuf,0,sizeof(returnbuf));
                    pthread_rwlock_rdlock(&g_PthreadLock_ST.ChanDatalock);
                    RealWaveData(returnbuf,g_ChanRealWave,&g_ChanData_ST[i],g_ChanRealNum);                    
                    pthread_rwlock_unlock(&g_PthreadLock_ST.ChanDatalock);
                    if(TCP_DEBUG == g_DebugType_EU)
                    {
                            for(j = 0;j < SendSize;j++)
                            {                           
                                printf("%d,data %02x \n",j,returnbuf[j]);    
                            } 
                    }
                    WriteSize = write(clientfd,returnbuf,SendSize);               
                    if(WriteSize > 0)
                    {
                        //usleep(g_EventNum_U32);
                    }
                    //when Drive disconnect ,break out
                    else if(WriteSize == 0)
                    {
                         printf("client close...\n");
                         memset(loginfo,0,sizeof(loginfo));
                         snprintf(loginfo, sizeof(loginfo)-1, "TCP client close");
                         WRITELOGFILE(LOG_INFO_1,loginfo);
                         g_socket_SendFlag  = 0;
                        //close(fd);               
                        break;
                    }
                    //when Drive  disconnect ,break out,WriteSize is -1
                    else
                    {
                        printf("WriteSize %d\n",WriteSize);
                        printf("client close...\n");
                        memset(loginfo,0,sizeof(loginfo));
                        snprintf(loginfo, sizeof(loginfo)-1, "TCP client close");
                        WRITELOGFILE(LOG_INFO_1,loginfo);
                        g_socket_SendFlag  = 0;
                        break;
                    }    
                }  
                    
        }
        printf("send complete,close clientfd\n");
        close(clientfd);        
    }
    close(serverfd);
    printf("------------>server close\n");
    pthread_exit(NULL);
    return CODE_OK;        
}


/**********************************************************************
*Name           :    ModbusThreadFunc  
*Function       :    communicate with CSR_drive for channel calibrate, and show the version
*Para           :  
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/1/6  Create
*********************************************************************/
#if 0
void *ModbusThreadFunc(void *arg)
{
    
    int ModbusSocket = -1;
    int ModbusRes;
    int8_t ModCalibFlag[6] = {0};
    modbus_mapping_t *ModbusMap_p;
    modbus_t *ModbusCtx;
    uint8_t *ModbusQuery;
    int8_t Rterr = 0;
    CHAN_CALIB_DATA ChanCalibRult_ST = {0};
    CHAN_DATA ChanADCValue_ST[5] = {0};
    CHAN_DATA ChanADCFiltValue_ST = {0};
    uint8_t ADCTranType_U8 = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    ADCTranType_U8 = CALIBTRANTYPE;
    //const char *IP = "192.168.1.11";/*modbus server IP*/
    const int PORT = 502;
    /*uint16_t UT_REGISTERS_TAB[] = { 0x1234, 0x5678, 0x9ABC  };mapping data*/
    ModbusQuery = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
    ModbusCtx = modbus_new_tcp(NULL, PORT);
    printf("PORT:%d\n",PORT );
    modbus_set_debug(ModbusCtx, FALSE);
   
    /*modbus mapping init*/
    ModbusMap_p = modbus_mapping_new_start_address(UT_BITS_ADDRESS_TCP,            UT_BITS_NB_TCP,
                                                    UT_INPUT_BITS_ADDRESS_TCP,      UT_INPUT_BITS_NB_TCP,
                                                    UT_REGISTERS_ADDRESS_TCP,       UT_REGISTERS_NB_TCP,
                                                    UT_INPUT_REGISTERS_ADDRESS_TCP, UT_INPUT_REGISTERS_NB_TCP);
    if(ModbusMap_p == NULL) /*modbus mapping init failure*/
    {
        perror("Failed allocate Modbus mapping");
        modbus_free(ModbusCtx);
        snprintf(loginfo, sizeof(loginfo)-1, "Failed to allocate the mapping: %s",modbus_strerror(errno));
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        pthread_exit(NULL);
    }
    /*start to listen when the first time of click the connect button */
    ModbusSocket = modbus_tcp_listen(ModbusCtx, 1);
    /*run forever when listened until close application*/
    while(g_LifeFlag > 0) 
    {
        if(-1 == modbus_tcp_accept(ModbusCtx, &ModbusSocket))
        {
            /*accept none*/
        }
        else /*accept data*/
        {
            ModCalibFlag[0] = 1;
            ModCalibFlag[1] = 1;
            ModCalibFlag[2] = 1;
            ModCalibFlag[3] = 1;
            ModCalibFlag[4] = 1;
            ModCalibFlag[5] = 1;
            VersionSet(ModbusMap_p,&g_Version_ST);
            while(1) 
            {
                /*the event about modbus receive */
                do 
                {
                    ModbusRes = modbus_receive(ModbusCtx, ModbusQuery);
                    /* Filtered queries return CODE_OK */
                }while (ModbusRes == 0);/*require length*/
                /* The connection is not closed on errors*/
                if (ModbusRes == -1 && errno != EMBBADCRC) 
                {
                    /* Quit */
                    break;
                }
                if((ModbusMap_p -> tab_registers[2]) && (ModCalibFlag[0] == 1))  //8002                 
                {
                    //for ChanCalibFile Once accept
                    ModCalibFlag[0] = 0;
                    ChanCalibFileCreat(&g_FileFd_ST,CHAN_CALIB_0V_FILE,&g_EADSErrInfo_ST); 
                    VolChanDataTrans(&g_BrdRdBufData_ST,&ChanADCValue_ST[0],&g_ChanCalib0VData_ST,&g_ChanCalib110VData_ST,g_ProcNum_U8,CALIBTRANTYPE);
                    ChanDataFilt(&ChanADCValue_ST[0],&ChanADCFiltValue_ST,g_ProcNum_U8,g_EADSType_U8);
                    AllChanDataCalib(ADCVOLT_0V,&ChanCalibRult_ST,&ChanADCFiltValue_ST,1); //high vol calib
                    Rterr = ChanCalibDataSave(g_FileFd_ST.CalibFile_fd,&ChanCalibRult_ST);
                    fclose(g_FileFd_ST.CalibFile_fd);                 
                    //Fwrite ok,then set the Light On
                    if(Rterr  == CODE_OK)
                    {
                        printf("ChanCalibFile0V.dat File Creat\n");
                        memset(loginfo,0,sizeof(loginfo));
                        snprintf(loginfo, sizeof(loginfo)-1, "ChanCalibFile0V.dat File Creat");
                        WRITELOGFILE(LOG_INFO_1,loginfo);
                        ModbusMap_p-> tab_registers[280] = 1;//8280 
                    }
                    //else, set the Light Off
                    else
                    {
                        memset(loginfo,0,sizeof(loginfo));
                        printf("ChanCalibFile0V.dat File Failed\n");
                        snprintf(loginfo, sizeof(loginfo)-1, "ChanCalibFile0V.dat File Failed");
                        WRITELOGFILE(LOG_INFO_1,loginfo);
                        ModbusMap_p-> tab_registers[280] = 0;
                    }
                    
                }
                else if((ModbusMap_p -> tab_registers[2] == 0)&&(ModCalibFlag[0] == 0))
                {
                    ModCalibFlag[0] = 1;
                    ModbusMap_p-> tab_registers[280] = 0;
                }
                else if((ModbusMap_p -> tab_registers[24]) && (ModCalibFlag[1] == 1))  //8024                
                {
                    //for ChanCalibFile Once accept
                    ModCalibFlag[1] = 0;
                    ChanCalibFileCreat(&g_FileFd_ST,CHAN_CALIB_110V_FILE,&g_EADSErrInfo_ST); 
                    VolChanDataTrans(&g_BrdRdBufData_ST,&ChanADCValue_ST[0],&g_ChanCalib0VData_ST,&g_ChanCalib110VData_ST,g_ProcNum_U8,CALIBTRANTYPE);
                    ChanDataFilt(&ChanADCValue_ST[0],&ChanADCFiltValue_ST,g_ProcNum_U8,g_EADSType_U8);  
                    AllChanDataCalib(ADCVOLT_110V,&ChanCalibRult_ST,&ChanADCFiltValue_ST,1); //high vol calib
                    Rterr = ChanCalibDataSave(g_FileFd_ST.CalibFile_fd,&ChanCalibRult_ST);
                    fclose(g_FileFd_ST.CalibFile_fd);
                    //Fwrite ok,then set the Light On
                    if(Rterr  == CODE_OK)
                    {
                        printf("ChanCalibFile 110V.dat File Creat\n");
                        memset(loginfo,0,sizeof(loginfo));
                        snprintf(loginfo, sizeof(loginfo)-1, "ChanCalibFile 110V.dat File Creat");
                        WRITELOGFILE(LOG_INFO_1,loginfo);
                        ModbusMap_p-> tab_registers[281] = 1;//8280 
                    }
                    //else, set the Light Off
                    else
                    {
                        memset(loginfo,0,sizeof(loginfo));
                        printf("ChanCalibFile 110V.dat File Failed\n");
                        snprintf(loginfo, sizeof(loginfo)-1, "ChanCalibFile 110V.dat File Failed");
                        WRITELOGFILE(LOG_INFO_1,loginfo);
                        ModbusMap_p-> tab_registers[281] = 0;
                    }
                    
                }
                else if((ModbusMap_p -> tab_registers[24] == 0)&&(ModCalibFlag[1] == 0))
                {
                    ModCalibFlag[1] = 1;
                    ModbusMap_p-> tab_registers[281] = 0;
                }
                else if((ModbusMap_p -> tab_registers[25]) && (ModCalibFlag[2] == 1))  //8025                
                {
                    //for ChanCalibFile Once accept
                    ModCalibFlag[2] = 0;
                    printf("ChanCalibFile3Creat\n");
                    //Fwrite ok,then set the Light On
                    if(Rterr  == 0)
                    {
                        ModbusMap_p-> tab_registers[282] = 1;//8280 
                    }
                    //else, set the Light Off
                    else
                    {
                        ModbusMap_p-> tab_registers[282] = 0;
                    }
                    
                }
                else if((ModbusMap_p -> tab_registers[25] == 0)&&(ModCalibFlag[2] == 0))
                {
                    ModCalibFlag[2] = 1;
                    ModbusMap_p-> tab_registers[282] = 0;
                }
                else if((ModbusMap_p -> tab_registers[26]) && (ModCalibFlag[3] == 1))  //8026                
                {
                    //for ChanCalibFile Once accept
                    ModCalibFlag[3] = 0;
                    printf("ChanCalibFile4Creat\n");
                    //Fwrite ok,then set the Light On
                    if(Rterr  == 0)
                    {
                        ModbusMap_p-> tab_registers[283] = 1;//8280 
                    }
                    //else, set the Light Off
                    else
                    {
                        ModbusMap_p-> tab_registers[283] = 0;
                    }
                    
                }
                else if((ModbusMap_p -> tab_registers[26] == 0)&&(ModCalibFlag[3] == 0))
                {
                    ModCalibFlag[3] = 1;
                    ModbusMap_p-> tab_registers[283] = 0;
                }
                else if((ModbusMap_p -> tab_registers[27]) && (ModCalibFlag[4] == 1))  //8027                 
                {
                    //for ChanCalibFile Once accept
                    ModCalibFlag[4] = 0;
                    printf("ChanCalibFile5Creat\n");
                    //Fwrite ok,then set the Light On
                    if(Rterr  == 0)
                    {
                        ModbusMap_p-> tab_registers[284] = 1;//8280 
                    }
                    //else, set the Light Off
                    else
                    {
                        ModbusMap_p-> tab_registers[284] = 0;
                    }
                    
                }
                else if((ModbusMap_p -> tab_registers[27] == 0)&&(ModCalibFlag[4] == 0))
                {
                    ModCalibFlag[4] = 1;
                    ModbusMap_p-> tab_registers[284] = 0;
                }
                else if((ModbusMap_p -> tab_registers[28]) && (ModCalibFlag[5] == 1))  //8028                
                {
                    //for ChanCalibFile Once accept
                    ModCalibFlag[5] = 0;
                    printf("ChanCalibFile6Creat\n");
                    //Fwrite ok,then set the Light On
                    if(Rterr  == 0)
                    {
                        ModbusMap_p-> tab_registers[285] = 1;//8280 
                    }
                    //else, set the Light Off
                    else
                    {
                        ModbusMap_p-> tab_registers[285] = 0;
                    }
                    
                }
                else if((ModbusMap_p -> tab_registers[28] == 0)&&(ModCalibFlag[5] == 0))
                {
                    ModCalibFlag[5] = 1;
                    ModbusMap_p-> tab_registers[285] = 0;
                }
                else
                {

                }                               
                /*the event about modbus reply*/ 
                ModbusRes = modbus_reply(ModbusCtx, ModbusQuery, ModbusRes, ModbusMap_p);
                if(ModbusRes == -1) 
                {
                    break;
                }
            }
        }
    }
    /*close connection*/
    printf("Quit the loop: %s\n", modbus_strerror(errno));
    close(ModbusSocket);
    modbus_mapping_free(ModbusMap_p);
    free(ModbusQuery);
    pthread_exit(NULL);
    printf("Modbus close\n");
}

/**********************************************************************
*Name           :  FileSaveThreaFunc
*Function       :  the thread for Event ,RealFlt,RealOprt,OprtNum File save.

*Return         :   
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/26  Create
*REV1.0.1     feng    2020/6/29  Add Chan Filt for Eventsave 
*********************************************************************/
void *FileSaveThreaFunc(void *arg) 
{
    
    struct timeval A_Time_ST,A_TimeEnd_ST;
    uint8_t i = 0;
    CHAN_DATA FiltChanData_ST = {0};
    static uint16_t s_FltLoopNum_U16 = 0;
	static uint16_t s_LifeLoopNum_U16 = 0;
    static uint32_t s_OprtLoopNum_U32 = 0;
    static uint32_t s_EventFileSaveNum_U32 = 0;
	static uint16_t s_EventLifeFileSaveNum_U16 = 0;
     uint16_t EventLifeFileSaveInterNum_U16 = 0;
	uint32_t Delayus_U32= 0;
    int SemValue = 0;
    int8_t fd = 0;

    Delayus_U32 = g_Rec_XML_ST.Rec_Event_ST.RecInterval * 1000;//50ms
	if(Delayus_U32 > g_FltSaveSlepNum_U32)
	{
		Delayus_U32 -= g_FltSaveSlepNum_U32;
        //60000/50=1200
		EventLifeFileSaveInterNum_U16 = g_LifeRec_XML_ST.Rec_Event_ST.RecInterval / g_Rec_XML_ST.Rec_Event_ST.RecInterval;
	}
	else
	{
		EventLifeFileSaveInterNum_U16 = g_LifeRec_XML_ST.Rec_Event_ST.RecInterval;

	}
	sleep(1);
    while(g_LifeFlag > 0)
    {
        threadDelay(0,Delayus_U32);//50ms
        if(TIME_DEBUG  == g_DebugType_EU)
        {
            gettimeofday(&A_Time_ST,NULL);                    
        }
        
        if(g_DebugType_EU == DEVC_DEBUG)
        {
            printf("STart FileSave\n");       
        }
        //稳态数据
        if(s_EventFileSaveNum_U32 >= g_Rec_XML_ST.Rec_Event_ST.RecToTalNum)//60000
        {
                //close the open file and creat new event file
                printf("s_EventFileSaveNum_U32 %d\n",s_EventFileSaveNum_U32);
                fflush(g_FileFd_ST.EventFile_fd);
                fd = fileno(g_FileFd_ST.EventFile_fd);
                fsync(fd);
                fclose(g_FileFd_ST.EventFile_fd);
                g_FileFd_ST.EventFile_fd = NULL;
                EventFileCreateByNum(&g_FileFd_ST,&g_Rec_XML_ST,&g_TrainInfo_ST,&g_EADSErrInfo_ST);
                s_EventFileSaveNum_U32 = 0;
            
        }
		else if(0 == (s_EventFileSaveNum_U32 % FILE_SYNC_NUM))
        {
			fflush(g_FileFd_ST.EventFile_fd);
            fd = fileno(g_FileFd_ST.EventFile_fd);
            fsync(fd);
		}
         if(g_DebugType_EU == FILE_DEBUG)
        {
            printf(" STart EventDataSave\n");               
        }
        pthread_rwlock_rdlock(&g_PthreadLock_ST.ChanDatalock);
        //对四包数据取平均值
		ChanDataFilt(&g_ChanData_ST[0],&FiltChanData_ST,g_ProcNum_U8,g_EADSType_U8);
        //按照csr_driver规定格式保存数据
        EventDataSave(&g_FileFd_ST,&FiltChanData_ST,g_EADSType_U8, g_EADSErrInfo_ST,\
                   g_ChanLgInfo_ST,g_ChanDigitalInfo_ST,g_ChanStatuInfo_ST);           
        pthread_rwlock_unlock(&g_PthreadLock_ST.ChanDatalock);            
        s_EventFileSaveNum_U32++;
        //寿命分析 
		if(s_LifeLoopNum_U16 >= EventLifeFileSaveInterNum_U16)
        {
            s_LifeLoopNum_U16 = 0;
            if(s_EventLifeFileSaveNum_U16 >= g_LifeRec_XML_ST.Rec_Event_ST.RecToTalNum)//10000
            {
                //close the open file and creat new event file
                printf("s_EventLifeFileSaveNum_U16 %d\n",s_EventLifeFileSaveNum_U16);
                fflush(g_FileFd_ST.EventLifeFile_fd);
                fd = fileno(g_FileFd_ST.EventLifeFile_fd);
                fsync(fd);
                fclose(g_FileFd_ST.EventLifeFile_fd);
                g_FileFd_ST.EventLifeFile_fd = NULL;		
				EventLifeFileCreateByNum(&g_FileFd_ST,&g_LifeRec_XML_ST,&g_TrainInfo_ST,&g_EADSErrInfo_ST);
                s_EventLifeFileSaveNum_U16 = 0;            
            }
		    else if(0 == (s_EventLifeFileSaveNum_U16 % FILE_SYNC_NUM))
            {
			    fflush(g_FileFd_ST.EventLifeFile_fd);
                fd = fileno(g_FileFd_ST.EventLifeFile_fd);
                fsync(fd);
		    }
            if(g_DebugType_EU == FILE_DEBUG)
            {
                printf(" STart EventLifeDataSave\n");               
            } 		
			EventRelayLifeSave(&g_FileFd_ST,g_EADSType_U8,g_ChanStatuInfo_ST);
            s_EventLifeFileSaveNum_U16++;
        }
        
        if(s_OprtLoopNum_U32 >= OPERT_SAVE_TIME)
        {
            s_OprtLoopNum_U32 = 0;
            pthread_rwlock_rdlock(&g_PthreadLock_ST.ChanInfolock);
            if(g_DebugType_EU == FILE_DEBUG)
            {
                printf(" STart Opert Num Data Save\n");               
            }
            OperNumFileSave(&g_FileFd_ST,&g_ChanStatuInfo_ST,&g_Rec_XML_ST,&g_TrainInfo_ST);
            pthread_rwlock_unlock(&g_PthreadLock_ST.ChanInfolock);
        }

        s_OprtLoopNum_U32++;
		s_LifeLoopNum_U16++;
        if(TIME_DEBUG  == g_DebugType_EU)
        {
            gettimeofday(&A_TimeEnd_ST,NULL);
            printf("File Save thread tim:%u \n",(uint32_t)A_TimeEnd_ST.tv_usec- (uint32_t)A_Time_ST.tv_usec); 
            printf("File Save thread usec:%u \n", (uint32_t)A_TimeEnd_ST.tv_usec); 
                    
        }    
    }
    printf("exit File Write thread\n");
    pthread_exit(NULL);    
}
#endif
/**********************************************************************
*Name           :    DirTarThreadFunc  
*Function       :    tar the Event ,RealFlt,RealOprt directory which create before today
*Para           :  
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/7/1  Create
*********************************************************************/

void *DirTarThreadFunc(void *arg) 
{
    char loginfo[LOG_INFO_LENG] = {0};

    sleep(10);/*when pow on have many chan oprt*/
    printf("start DirTar thread\n");
    snprintf(loginfo, sizeof(loginfo)-1, "start DirTar thread");
    WRITELOGFILE(LOG_INFO_1,loginfo);

    FileSpaceProc(&g_Rec_XML_ST);
	// test
    DirFileTar(&g_Rec_XML_ST);

    printf("exit DirTar thread\n");
    memset(loginfo,0,sizeof(loginfo));
    snprintf(loginfo, sizeof(loginfo)-1, "exit DirTar thread");
    WRITELOGFILE(LOG_INFO_1,loginfo);
    pthread_exit(NULL);
}
#if 0
/**********************************************************************
*Name           :    LEDWachDogPthreadFunc  
*Function       :    Blink the led, life led and error led
*Para           :  
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/7/1  Create
*********************************************************************/
void *LEDWachDogPthreadFunc (void *arg)
{
    uint32_t led_period;
    int8_t lifeledfd;
    int8_t errledfd;
    int8_t watchdogfd;
    uint8_t errtem = 0;
    int16_t timeout;
    static uint8_t s_errledflag = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    /******读取PHY芯片寄存器所需变量*******/
    struct mii_ioctl_data *mii = NULL;
    struct ifreq ifr;
    int8_t ret,err = 0;
    int sockfd;
    uint16_t LinkStats = 0;
    uint32_t phyvalue = 0;
    uint8_t  PhyErrFlag = 0,PhyLinkErrFlag = 0;
    /************************************/

    /******读取/proc/net/dev所需变量*******/
    FILE *stream;//文件流
    char buffer[256] = {0};//缓冲区
    char *line_return = {0};//记录每次返回值（行）
    uint32_t line_count = 0;//记录行数
    char tmp_itemName[32] = {0};//临时存放文件中的每行的项目名称
    uint32_t itemReceive = 0;//存放每一个网卡的接受到的字节总数（单位：Byte）
    uint32_t itemReceivePacks = 0;//存放每一个网卡的接受到的帧总数
    uint32_t itemTransmit = 0;//存放每一个网卡的已发送的字节总数（单位：Byte）
    uint32_t itemTransmitPacks = 0;//存放每一个网卡的发送的帧总数
    uint32_t Eth1Receive = 0;//存放Eth1网卡的接受到的字节总数（单位：Byte）
    uint32_t Eth1ReceivePacks = 0;//存放Eth1网卡的接受到的帧总数
    uint32_t Eth1Transmit = 0;//存放Eth1网卡的已发送的字节总数（单位：Byte）
    uint32_t Eth1TransmitPacks = 0;//存放Eth1网卡的发送的帧总数
    uint32_t Eth1TransmitPacksLastTime = 0;
    uint32_t Eth1ReceivePacksLastTime  = 0;
    uint32_t Eth1TransmitErrTimes = 0;
    uint32_t Eth1ReceiveErrTimes  = 0;
    /************************************/

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, "eth1", IFNAMSIZ - 1);

    if(0 == g_LinuxDebug)//运行在ZYNQ平台
    {
        lifeledfd = open(LED_BRIGHTNESS, O_WRONLY);
        if (lifeledfd < 0)
        {
            printf("Cannot open Life LED\n");
            snprintf(loginfo, sizeof(loginfo)-1, "Cannot open Life LED");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            //pthread_exit("led exit");
        }
        led_period =  PERIOD_COEFF;

        errledfd = open(LEDERR_BRIGHTNESS, O_WRONLY);
        if (errledfd < 0)
        {
            printf("Cannot open Err LED\n");
            snprintf(loginfo, sizeof(loginfo)-1, "Cannot open Err LED");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            // pthread_exit("led exit");
        }

        sleep(1);
        sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
        if(sockfd < 0)
        {
            printf("Creat Sockets failed!\n");
            snprintf(loginfo, sizeof(loginfo)-1,"Creat Sockets failed!!");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            err = CODE_ERR;
        }
        ret = ioctl(sockfd, SIOCGMIIPHY, &ifr);//将Eth1加入MII总线后才能通过MIO接口进行管理
        if(ret < 0)
        {
            printf("Add Phy address to mii_bus failed!\n");
            snprintf(loginfo, sizeof(loginfo)-1,"Add Phy address to mii_bus failed!");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            err = CODE_ERR;
        }
        mii = (struct mii_ioctl_data*)&ifr.ifr_data;//将mii与ifr.ifr_date地址关联起来

        while (g_LifeFlag)
        {
            /******LED*******/
            memcpy(&errtem,&g_EADSErrInfo_ST,1);
            if((!s_errledflag) && (errtem))/* happen VechWarm*/
            {
                write(errledfd, "0", 2); //on
                s_errledflag = 1;
            }
            else if ((1 == s_errledflag) && (0 == errtem))
            {
                write(errledfd, "1", 2); //off
                s_errledflag = 0;
            }
            write(lifeledfd, "1", 2); //on
            usleep(led_period);
            write(lifeledfd, "0", 2);//off
            usleep(led_period);
            /******************/

            /******读取PHY芯片寄存器过程*******/
            mii->reg_num  = PHY_COPPER_CONTRL_REG;
            ret = ioctl(sockfd, SIOCGMIIREG, &ifr);//读REG0 控制寄存器
            if(ret < 0)
            {
                printf("Phy Read Control Reg Failed!\n");
                snprintf(loginfo, sizeof(loginfo)-1,"Phy Read Control Reg Failed!");
                WRITELOGFILE(LOG_ERROR_1,loginfo);
            }
            phyvalue = mii->val_out;
            printf("Phy Control Reg value:%x\n",phyvalue);
            if(PHY_COPPER_CONTRL_REG_OK == phyvalue)//判断是否等于0x2100
            {
                if(1 == PhyErrFlag)
                {
                    printf("Phy Control Reg return ok,value:%x\n",phyvalue);
                    snprintf(loginfo, sizeof(loginfo)-1, "Phy Control Reg return ok,value:%x",phyvalue);
                    WRITELOGFILE(LOG_ERROR_1,loginfo);
                    PhyErrFlag = 0;
                }
            }
            else
            {
                if(0 == PhyErrFlag)
                {
                    printf("Phy Control Reg Error,value:%x\n",phyvalue);
                    snprintf(loginfo, sizeof(loginfo)-1, "Phy Control Reg Error,value:%x",phyvalue);
                    WRITELOGFILE(LOG_ERROR_1,loginfo);
                    PhyErrFlag = 1;
                }
            }

            mii->reg_num  = PHY_COPPER_STATUS_REG;
            ret = ioctl(sockfd, SIOCGMIIREG, &ifr);//读取REG1 状态寄存器
            if(ret < 0)
            {
                printf("Phy Read Status Reg Failed!\n");
                snprintf(loginfo, sizeof(loginfo)-1,"Phy Read Status Reg Failed!");
                WRITELOGFILE(LOG_ERROR_1,loginfo);
            }
            phyvalue = mii->val_out;
            printf("Phy Status Reg value:%x\n",phyvalue);
            LinkStats = phyvalue & 0x4;//判断是BIT2是否等于1
            if(4 == LinkStats)
            {
                if(1 == PhyLinkErrFlag)
                {
                    printf("Phy Link Up,value:%x\n",phyvalue);
                    snprintf(loginfo, sizeof(loginfo)-1, "Phy Link Up,value:%x",phyvalue);
                    WRITELOGFILE(LOG_ERROR_1,loginfo);
                    PhyLinkErrFlag = 0;
                }
            }
            else
            {
                if(0 == PhyLinkErrFlag)
                {
                    printf( "Phy Link Down,value:%x\n",phyvalue);
                    snprintf(loginfo, sizeof(loginfo)-1, "Phy Link Failed,value:%x",phyvalue);
                    WRITELOGFILE(LOG_ERROR_1,loginfo);
                    PhyLinkErrFlag = 1;
                }
            }
            /******读取/proc/net/dev过程*******/
            stream = fopen("/proc/net/dev", "r");
            if(stream == NULL)
            {
                printf("Can't open /proc/net/dev!\n");
            }
            line_return = fgets (buffer, 256 * sizeof(char), stream);//读取第一行标题栏
            line_count++;
            line_return = fgets (buffer, 256 * sizeof(char), stream);//读取第二行标题栏
            line_count++;
            while(line_return != NULL)
            {
                line_return = fgets (buffer, 256 * sizeof(char), stream);
                line_count++;
                sscanf( buffer,"%s%d%d%d%d%d%d%d%d%d%d",
                        tmp_itemName,
                        &itemReceive,
                        &itemReceivePacks,
                        &itemTransmit,
                        &itemTransmit,
                        &itemTransmit,
                        &itemTransmit,
                        &itemTransmit,
                        &itemTransmit,
                        &itemTransmit,
                        &itemTransmitPacks);
                if(strncmp(tmp_itemName,"eth0",4) == 0)
                {
                    Eth1Receive = itemReceive;
                    Eth1Transmit = itemTransmit;
                    Eth1ReceivePacks = itemReceivePacks;
                    Eth1TransmitPacks = itemTransmitPacks;
                    break;
                }
            }
            printf("Eth1Receive:%d,Eth1ReceivePacks:%d\n", Eth1Receive,Eth1ReceivePacks);
            printf("Eth1Transmit:%d,Eth1TransmitPacks:%d\n",Eth1Transmit,Eth1TransmitPacks);
            if(Eth1ReceivePacksLastTime == Eth1ReceivePacks)
            {
                Eth1ReceiveErrTimes++;
                if(Eth1ReceiveErrTimes>=10)
                {
                    printf("MAC DIDN'T RECEIVE DATE!\n");
                }
            } else{
                Eth1ReceiveErrTimes=0;
            }
            if(Eth1TransmitPacksLastTime == Eth1TransmitPacks)
            {
                Eth1TransmitErrTimes++;
                if(Eth1TransmitErrTimes>=10)
                {
                    printf("MAC DIDN'T SEND DATE!\n");
                }
            } else{
                Eth1TransmitErrTimes=0;
            }
            Eth1ReceivePacksLastTime = Eth1ReceivePacks;
            Eth1TransmitPacksLastTime = Eth1TransmitPacks;
        }
    }
    close(errledfd);
    close(lifeledfd);
    close(watchdogfd);
    close(sockfd);
    printf("LEDWachDogPthreadFunc Quit\n");
    pthread_exit(NULL);
}
#endif
/**********************************************************************
*Name           :   CAN_DataReadThreaFunc  
*Function       :   read/write the CAN data between Bram and A9 
*Para           :  
*Version        :   REV1.0.0       
*Author:        :   zlz
*direction      :方向以TRDP为基准，接收570数据发送给TRDP认为Send，接收TRDP数据发送570认为Recv
*History:
*REV1.0.0     zlz    2021/9/6  Create
*********************************************************************/
void *TMS570_Bram_ThreadFunc(void *arg) 
{
    //#if 0
    uint8_t ret = 0;   
    static uint32_t Can_RecordNum = 0;//文件存储条数计数
    uint8_t CAN_Record_Date[16] = {0};    
    //帧文件头参数填充
    TMS570_Bram_TopPackDataSetFun();
    
    while(g_LifeFlag > 0)
    {        
        //从Bram指定地址读取数据        
        TMS570_Bram_Read_Func(s_tms570_bram_RD_data_st);        
        usleep(100000);
        //向Bram指定地址写入数据        
        TMS570_Bram_Write_Func(s_tms570_bram_WR_data_st);      
    }    
    printf("exit TMS570_Bram_Thread Function!\n");//TODO:考虑每个线程退出时记录在日志中
    pthread_exit(NULL);
    //#endif
}

/**********************************************************************
*Name           :   CAN0ThreadFunc  
*Function       :   read/write the CAN0 data
*Para           :  
*Version        :   REV1.0.0       
*Author:        :   zlz
*direction      :   从CAN0外设读取CAN数据
*History:
*REV1.0.0       :   zlz    2021/12/4  Create
*********************************************************************/
void *CAN0ThreadFunc(void *arg)
{
    uint8_t i,j;
    static errnum_wr=0,errnum_rd=0;
    int socket_can0,nbytes;
    struct sockaddr_can addr_can0;
    struct ifreq ifr_can0;
    struct timeval tv;
    char loginfo[LOG_INFO_LENG]={0};
    //Attention 帧的时序与设定的100ms不符!
    /*创建套接字并与 can0 绑定*/
    socket_can0 = socket(PF_CAN, SOCK_RAW, CAN_RAW);//创建套接字
    strcpy(ifr_can0.ifr_name, "can0" );
    ioctl(socket_can0, SIOCGIFINDEX, &ifr_can0); //指定 can0 设备
    addr_can0.can_family = AF_CAN;
    addr_can0.can_ifindex = ifr_can0.ifr_ifindex;
    bind(socket_can0, (struct sockaddr *)&addr_can0, sizeof(addr_can0));    
    /*TODO设置过滤规则*/
    ioctl(socket_can0,SIOCGSTAMP,&tv);
    //setsockopt(socket_can0, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
    /*初始化报文帧数据*/
    CAN_FrameInit(s_can0_frame_RD_st,s_can0_frame_WR_st,CAN0_TYPE);
    //TODO:针对读写错误及生命信号停止都应有一些判断过程
    while(1)
    {        
        CAN_WriteData_Pro(s_can0_frame_WR_st,s_tms570_bram_RD_data_st,CAN0_TYPE);
        for(i=0;i<CAN0_WRITE_FRAME_NUM;i++)
        {
            nbytes = write(socket_can0,&s_can0_frame_WR_st[i], sizeof(s_can0_frame_WR_st[i]));
            if(nbytes != sizeof(s_can0_frame_WR_st[i]))
            {                
                printf("CAN 0 Send frame[%u] Error!\n",s_can0_frame_WR_st[i].can_id);
                errnum_wr++;
                if(errnum_wr >=10)
                {
                    snprintf(loginfo, sizeof(loginfo)-1, "CAN 0 Send frame[%u] Error!",s_can0_frame_WR_st[i].can_id);
                    WRITELOGFILE(LOG_ERROR_1,loginfo);
                    errnum_wr = 0;
                }
                
            }
            else
            {
                errnum_wr =0;
            }
        }       
        usleep(50000);
        for(i=0;i<CAN0_READ_FRAME_NUM;i++)
        {
            nbytes = read(socket_can0,&s_can0_frame_RD_st[i],sizeof(s_can0_frame_RD_st[i]));
            if(nbytes != sizeof(s_can0_frame_RD_st[i]))
            {                
                printf("CAN0 Receive Error frame[%d]!\n",i);
                memset(s_can0_frame_RD_st[i].data,0,8);
                errnum_rd++;
                if(errnum_rd >=10)
                {
                    snprintf(loginfo, sizeof(loginfo)-1, "CAN 0 receive frame[%u] Error!",s_can0_frame_RD_st[i].can_id);
                    WRITELOGFILE(LOG_ERROR_1,loginfo);
                    errnum_rd = 0;
                }                            
            }
            else
            {
                errnum_rd = 0;
            }
            if(g_DebugType_EU == CAN_RD_DEBUG)
            {               
                printf("A9 Read CAN0 ID:0x%x:",s_can0_frame_RD_st[i].can_id & 0x1FFFFFFF);
                for (j = 0; j < 8; j++)
                    printf("[%x]",s_can0_frame_RD_st[i].data[j]);
                printf("\n");               
            }
        }               
        CAN_ReadData_Pro(s_can0_frame_RD_st,s_tms570_bram_WR_data_st,CAN0_TYPE);          
        #if 0
        if(g_DebugType_EU == CAN_RD_DEBUG)
        {
            for (i = 1; i < 4; i++)
            {
                for (j = 0; j < 25; j++)
                    printf("A9->570 BramData[%d][%d]:0x%08x\n",i,j,s_tms570_bram_WR_data_st[i].buffer[j]);
            }
        }
        #endif
        usleep(50000);
    }
    close(socket_can0);
    return 0;
}
/**********************************************************************
*Name           :   CAN1ThreadFunc  
*Function       :   read/write the CAN1 data
*Para           :  
*Version        :   REV1.0.0       
*Author:        :   zlz
*direction      :   从CAN1外设读取CAN数据
*History:
*REV1.0.0       :   zlz    2021/12/4  Create
*********************************************************************/
void *CAN1ThreadFunc(void *arg)
{
    #if 0
    uint8_t i,j;
    static uint8_t errnum_wr=0,errnum_rd=0;
    int socket_can1,nbytes;
    struct sockaddr_can addr_can1;
    struct ifreq ifr_can1;
    struct timeval tv;
    char   loginfo[LOG_INFO_LENG]={0};
    /*创建套接字并与 can1 绑定*/
    socket_can1 = socket(PF_CAN, SOCK_RAW, CAN_RAW);//创建套接字
    strcpy(ifr_can1.ifr_name, "can1" );
    ioctl(socket_can1, SIOCGIFINDEX, &ifr_can1); //指定 can0 设备
    addr_can1.can_family = AF_CAN;
    addr_can1.can_ifindex = ifr_can1.ifr_ifindex;
    bind(socket_can1, (struct sockaddr *)&addr_can1, sizeof(addr_can1));    
    /*设置过滤规则*/ 
    //TODO:修改过滤规则
    ioctl(socket_can1,SIOCGSTAMP,&tv);//打上时间戳
    //setsockopt(socket_can1, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
    /*初始化报文帧数据*/
    CAN_FrameInit(s_can1_frame_RD_st,s_can1_frame_WR_st,CAN1_TYPE);
   
    while(1)
    {
        CAN_WriteData_Pro(s_can1_frame_WR_st,&s_tms570_bram_RD_data_st[4],CAN1_TYPE);
        for(i=0;i<CAN1_WRITE_FRAME_NUM;i++)
        {
            nbytes = write(socket_can1,&s_can1_frame_WR_st[i], sizeof(s_can1_frame_WR_st[i]));
            if(nbytes != sizeof(s_can1_frame_WR_st[i]))
            {                
                printf("CAN 1 Send frame[%u] Error!\n",s_can1_frame_WR_st[i].can_id);
                errnum_wr++;
                if(errnum_wr >=10)
                {
                    snprintf(loginfo, sizeof(loginfo)-1, "CAN 1 Send frame[%u] Error!",s_can1_frame_WR_st[i].can_id);
                    WRITELOGFILE(LOG_ERROR_1,loginfo);
                    errnum_wr = 0;
                }
                
            }
            else
            {
                errnum_wr =0;
            }
        }       
        usleep(50000);
        for(i=0;i<CAN1_READ_FRAME_NUM;i++)
        {
            nbytes = read(socket_can1,&s_can1_frame_RD_st[i],sizeof(s_can1_frame_RD_st[i]));
            if(nbytes != sizeof(s_can1_frame_RD_st[i]))
            {                
                printf("CAN1 Receive Error frame[%d]!\n",i);
                memset(s_can1_frame_RD_st[i].data,0,8);
                errnum_rd++;
                if(errnum_rd >=10)
                {
                    snprintf(loginfo, sizeof(loginfo)-1, "CAN 1 receive frame[%u] Error!",s_can1_frame_RD_st[i].can_id);
                    WRITELOGFILE(LOG_ERROR_1,loginfo);
                    errnum_rd = 0;
                }                            
            }
            else
            {
                errnum_rd = 0;
            }
            if(g_DebugType_EU == CAN_RD_DEBUG)
            {               
                printf("A9 Read CAN1 ID:0x%x:",s_can1_frame_RD_st[i].can_id & 0x1FFFFFFF);
                for (j = 0; j < 8; j++)
                    printf("[%x]",s_can1_frame_RD_st[i].data[j]);
                printf("\n");               
            }
        }               
        CAN_ReadData_Pro(s_can1_frame_RD_st,&s_tms570_bram_WR_data_st[4],CAN1_TYPE);          
        #if 0
        if(g_DebugType_EU == CAN_RD_DEBUG)
        {
            for (i = 1; i < 4; i++)
            {
                for (j = 0; j < 25; j++)
                    printf("A9->570 BramData[%d][%d]:0x%08x\n",i,j,s_tms570_bram_WR_data_st[i].buffer[j]);
            }
        }
        #endif
        usleep(50000);
    }
    close(socket_can1);
    return 0;
   #endif
}
/**********************************************************************
*Name           :   MVBThreadFunc  
*Function       :   read/write the MVB data from BRAM
*Para           :  
*Version        :   REV1.0.0       
*Author:        :   zlz
*direction      :   从BRAM指定区域读写MVB数据
*History:
*REV1.0.0       :   zlz    2021/12/4  Create
*********************************************************************/
void *MVBThreadFunc(void *arg)
{
    
}