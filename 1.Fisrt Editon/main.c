/**********************************************************************
*File name      :   main.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2021/12/03
*Description    :   the entry of the whole program
*Version        :   REV1.0.0       
*Author:        :   zuolinzheng
*
*History:
*REV1.0.0     feng    2021/12/03  Create*
*********************************************************************/


/***********************************************************************
*Include file  section*
*********************************************************************/
#include "Socket.h"
#include "BramDataProc.h"
#include "ModbusSer.h"
#include "GPIOControl.h"
#include "FileSave.h"
#include "xml.h"
/***********************************************************************
*Global Variable Declare Section*
*********************************************************************/
sem_t               g_RealSend_sem;
DEBUG_TYPE_ENUM     g_DebugType_EU;
EADS_ERROR_INFO     g_EADSErrInfo_ST = {0};
TRAIN_INFO          g_TrainInfo_ST = {0,0};
VERSION             g_Version_ST = {0};
RECORD_XML          g_Rec_XML_ST = {0};
SPACE_JUDGE_VALUE   g_SpaceJudge_ST = {0};
PTHREAD_INFO        g_Pthread_ST = {0};
PTHREAD_LOCK        g_PthreadLock_ST= {0};
uint16_t            g_PowDebug = 0;
uint32_t            g_LinuxDebug = 0; 
int8_t              g_LifeFlag = 1;                             //to controll the pthread
uint8_t             g_ChanRealWave[48] = {0};                   /*for the channel real wave */
uint8_t             g_ChanRealNum = 0;                          /*for the channel real wave */
uint8_t             g_socket_SendFlag = 0;                      /*for tcp modbus comunicate*/
FILE_FD             g_FileFd_ST = {0};                          /*the save file FP*/
CHAN_STATUS_INFO    g_ChanStatuInfo_ST = {0};                   /*include the real file save,Chan operate num*/

/***********************************************************************
*Local Macro Define Section*
*********************************************************************/
#define WDIOC_SETTIMEOUT _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
/***********************************************************************
*Local Struct Define Section*
*********************************************************************/
static DRIVE_FILE_DATA s_save_to_csr_driver={0};
/***********************************************************************
*Local Prototype Declare Section*
*********************************************************************/
void    FuncUsage(void);
void    ArgJudge(void);
int8_t  ThreadInit(PTHREAD_INFO *pthread_ST_p);
int8_t  ThreadOff(FILE_FD *FileFd_p,PTHREAD_INFO *pthread_ST_p);
int8_t  PowDownFun(void);
void    *RealWaveThreadFunc(void *arg);
void    *Udp_Intool_ThreadFunc(void *arg);
void    *ModbusThreadFunc(void *arg);
void    *FileSaveThreaFunc(void *arg);
void    *DirTarThreadFunc(void *arg);
void    *LEDPthreadFunc (void *arg);
void    *CAN0ThreadFunc(void *arg);
void    *CAN1ThreadFunc(void *arg);
void    *MVBThreadFunc(void *arg);

/***********************************************************************
*Static Variable Define Section*
*********************************************************************/

/*The entry of the whole program*/
int main(int argc, char *argv[])
{ 
    uint8_t i = 0;         
    int8_t  fifofd = 0;
    int i2cbus_fd =0;    
    uint8_t BOXID =0;     
    uint8_t BinLife = 0;
    uint8_t FifoWrNum = 0;
    uint8_t FifoErr = 0;
    uint16_t FifoWrTime = 0;
	uint32_t EmmcTotalSizeMB_U32 = 0,EmmcFreeSizeMB_U32 = 0;     
    char    ArgLogInfo[LOG_INFO_LENG] = {0};    

    LogFileCreatePowOn();
    
    GetCompileTime();

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
    /*if logfileNum more than g_SpaceJudge_ST.LOGFILE_NUM,delete the earliest logfile*/
    DeleteEarliestFile("/yaffs/REC_LOGGFLT/",LOG_FILE_TYPE);
    /*judge and process the main_function arguments*/
    if (argc < 2)
    {
        FuncUsage();
        snprintf(ArgLogInfo, sizeof(ArgLogInfo)-1, "User have typed invalid arguments:(argc<2)!");
        WRITELOGFILE(LOG_ERROR_1,ArgLogInfo);
        LogFileSync();
    }

    g_LifeFlag      =   1;
    g_LinuxDebug    =   0;                                                  //软件运行环境:0:ZYNQ 1:Ubuntu       
    g_DebugType_EU  =   (DEBUG_TYPE_ENUM)strtoul(argv[1],NULL,10);                    
    g_PowDebug      =   (uint16_t)strtoul(argv[2],NULL,10);                 //电源选项，1-使能掉电监测 , 0-不使能                                                                   
    g_Version_ST.ECU_RunVer_U16 = ECU_VERSION_PTU;

    ArgJudge();
    snprintf(ArgLogInfo,sizeof(ArgLogInfo)-1,"g_DebugType_EU:%u,g_PowDebug:%u,g_LinuxDebug:%u",g_DebugType_EU,g_PowDebug,g_LinuxDebug);
    WRITELOGFILE(LOG_INFO_1,ArgLogInfo); 

	/*Bram Init*/
    if (0 == g_LinuxDebug)
    {
       Bram_Mapping_Init(&g_EADSErrInfo_ST);     
    }

    /*RTC Test*/
    RTCTesT();//FIXME Please add timeshow in logfile and printf

    /*I2C*/
    i2cbus_fd=open(DEFAULT_I2C_BUS,O_RDWR);  
    i2c_read(i2cbus_fd,0X20,0,&BOXID,1);
    printf("BOXID is %x\n",BOXID);

    /*Event File xml_config_file*/
    SysXmlParInit(ECU_CONFIG,&g_Rec_XML_ST,&g_Version_ST);
    XmlParJudge(&g_Rec_XML_ST);   
    EventFileCreateByNum(&g_FileFd_ST,&g_Rec_XML_ST,&g_TrainInfo_ST,&g_EADSErrInfo_ST);
    VersionInit(&g_Version_ST);                
    VersionSave(&g_Version_ST);               
    ThreadInit(&g_Pthread_ST);

    /*****open fifo tunnel*****/
    fifofd = open(FIFO_FILE,O_WRONLY|O_NONBLOCK,0); 
    if(fifofd == -1)
    {
        perror("open fifo File error ");
        memset(ArgLogInfo,0,LOG_INFO_LENG);
        snprintf(ArgLogInfo, sizeof(ArgLogInfo)-1, "open fifo File error");
        WRITELOGFILE(LOG_ERROR_1,ArgLogInfo);
        FifoErr =  1;
    }
        
    if (1 == g_PowDebug)
    {
        GPIO_PowDownIoCreat();
        GPIO_PowDowOpen();   
    }
    
    while(1)
    {      
        if(1 == g_PowDebug)
        {
          PowDownFun();
        }
        /*fifo tunnel*/
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
              
    }

    memset(ArgLogInfo,0,LOG_INFO_LENG);
    snprintf(ArgLogInfo, sizeof(ArgLogInfo)-1, "Main thread exit ! Ready to close all thread and BramMap !");
    WRITELOGFILE(LOG_ERROR_1,ArgLogInfo);    
    ThreadOff(&g_FileFd_ST,&g_Pthread_ST);    
    if(0 == g_LinuxDebug)
    {
        BramCloseMap(); 
    }
    close(fifofd);    
    exit(EXIT_SUCCESS);
}

/**
 * @description : show how to type a right command
 * @param       : void  
 * @return      : void 
 * @author:     : feng
 */
void FuncUsage(void)
{    
    printf("Usage introduce:\nCommand consist of : /tffs0/Demo_Run_bin <DebugTest_EU> <g_PowDebug>\n");
    printf("you can type such command: :/tffs0/Demo_Run_bin 0 1\n");
}

/**
 * @description : argument judge and process
 * @param       : void  
 * @return      : void 
 * @author:     : feng
 */
void ArgJudge(void)
{
    if( g_DebugType_EU >40)
    {
        g_DebugType_EU = 0;
    }
    if(g_PowDebug > 1)
    {
        g_PowDebug = 0;
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
    int policy;
    struct sched_param param;
    char loginfo[LOG_INFO_LENG] = {0};      
     
   /* res = sem_init(&g_RealSend_sem, 0, 0);
    if (res != 0) 
    {
        perror("RealSend_sem Semaphore initialization failed");
        snprintf(loginfo, sizeof(loginfo)-1, "RealSend_sem init failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }*/

    res = pthread_rwlock_init(&g_PthreadLock_ST.BramDatalock,NULL);
    if(res != 0)
    {
        perror("BramDatalock init failed");
        snprintf(loginfo, sizeof(loginfo)-1, "BramDatalock init failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);        
    }
       
    /*Create the Pthread*/   
    res = pthread_create(&pthread_ST_p->RealWaveThread,NULL,RealWaveThreadFunc,NULL);
    if (res != 0) 
    {
        perror("create RealWaveThread  failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create RealWaveThread failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);    
    }
    res = pthread_create(&pthread_ST_p->UdpThread,NULL,Udp_Intool_ThreadFunc,NULL);
    if (res != 0) 
    {
        perror("create Udp_Intool_Thread failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create Udp_Intool_Thread failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);    
    }
    res = pthread_create(&pthread_ST_p->FileSaveThread,NULL,FileSaveThreaFunc,NULL);
    if (res != 0) 
    { 
        perror("create FileSaveThreaFun failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create FileSaveThread failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    
    res = pthread_create(&pthread_ST_p->ModbusThread,NULL,ModbusThreadFunc,NULL);
    if (res != 0) 
    { 
        perror("create ModbusThreadFunc failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create ModbusThreadFunc failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);   
    }
    
    res = pthread_create(&pthread_ST_p->LedThread,NULL,LEDPthreadFunc,NULL);
    if (res != 0) 
    { 
        perror("create LEDPthreadFunc failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create LEDPthreadFunc failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);      
    }
    
    res = pthread_create(&pthread_ST_p->DirTarThread,NULL,DirTarThreadFunc,NULL);
    if (res != 0) 
    { 
        perror("create DirTarThreaFunc failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create DirTarThreaFunc failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);      
    }
    
    res = pthread_create(&pthread_ST_p->MVBThread,NULL,MVBThreadFunc,NULL);
    if (res != 0) 
    { 
        perror("create MVBThreaFunc failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create MVBThreaFunc failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);      
    }
    
    res = pthread_create(&pthread_ST_p->CAN0Thread,NULL,CAN0ThreadFunc,NULL);
    if (res != 0) 
    { 
        perror("create CAN0ThreadFunc failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create CAN0ThreadFunc failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);     
    }

    res = pthread_create(&pthread_ST_p->CAN1Thread,NULL,CAN1ThreadFunc,NULL);
    if (res != 0)
    {
        perror("create CAN1ThreadFunc failed");
        snprintf(loginfo, sizeof(loginfo)-1, "create CAN1ThreadFunc failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }    
 
    pthread_detach(pthread_ST_p -> DirTarThread);
    pthread_detach(pthread_ST_p -> RealWaveThread);
    pthread_detach(pthread_ST_p -> UdpThread);   
    pthread_detach(pthread_ST_p -> ModbusThread);       
    pthread_detach(pthread_ST_p -> MVBThread);
    pthread_detach(pthread_ST_p -> CAN0Thread);
    pthread_detach(pthread_ST_p -> CAN1Thread);
    pthread_detach(pthread_ST_p -> LedThread);       
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
    
    res = pthread_join(pthread_ST_p -> FileSaveThread, &thread_result);
    if (res == 0)
    {
        printf("thread_join FileSaveThread success\n");
    }
    else 
    {
        perror("pthread_join FileSaveThread failed\n");           
    }
    sem_destroy(&g_RealSend_sem);    
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

    GPIO_PowDowRead(&PowDowIOValue);

    if(0 == PowDowIOValue)
    {
        s_PowDownNum++;
        //if(s_PowDownNum > POWDOW_FILT)
        {            
           //if(0 == g_EADSErrInfo_ST.PowErr)
            {
                //g_EADSErrInfo_ST.PowErr = 1;
                printf("PowerOff happen,start sync File\n");
				snprintf(LogInfo, sizeof(LogInfo)-1, "PowerOff happen , start sync File");
                WRITELOGFILE(LOG_INFO_1,LogInfo);
				LogFileSync();                

                if(NULL != g_FileFd_ST.EventFile_fd)
                {
                    fflush(g_FileFd_ST.EventFile_fd);
                    fd = fileno(g_FileFd_ST.EventFile_fd);
                    fsync(fd);
                    printf("finish evetfile sync\n");
                }

                if(NULL != g_FileFd_ST.EventBLVDS_fd)
                {
                    fflush(g_FileFd_ST.EventBLVDS_fd);
                    fd = fileno(g_FileFd_ST.EventBLVDS_fd);
                    fsync(fd);
                    printf("finish Blvds_evetfile sync\n");
                }
                 
                snprintf(LogInfo, sizeof(LogInfo)-1,"PowerOff happen,sync File");
                WRITELOGFILE(LOG_INFO_1,LogInfo);
                LogFileSync();
                printf("PowerOff happen,sync File finish\n");          
            }
            s_PowDownNum = 0;          
        }
        s_ClearPowNum = 0;    
    }
    else 
    {
        /*to avoid accident poweroff and  power recover */
        /*almost not meet this condition*/
        s_ClearPowNum++;
        if(s_ClearPowNum > POWDOW_FILT)/*10ms*/
        {            
            if(1 == g_EADSErrInfo_ST.PowErr)
            {            
                g_EADSErrInfo_ST.PowErr = 0;
                printf("PowerOff recover\n");
                snprintf(LogInfo, sizeof(LogInfo)-1, "PowerOff recover");
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
    char loginfo[LOG_INFO_LENG] = {0};
    uint32_t sendNum = 0;
    
    struct sockaddr_in server,client;
    struct sigaction action;    
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;    
    // 通信断开产生SIGPIPE信号，引导程序异常退出，需要设置忽略SIGPIPE
    action.sa_handler = SIG_IGN;//set function to here
    sigaction(SIGPIPE, &action, 0);
    /*creat server socket*/
    if(-1==( serverfd= socket(AF_INET,SOCK_STREAM,0)))
    {
        perror("RealWaveThreadFunc socket error\n");
        snprintf(loginfo, sizeof(loginfo)-1, "RealWaveThreadFunc socket error!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        pthread_exit(NULL);
    }
    /*set server socket option*/
    bzero(&server,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(TCP_PORT);
    server.sin_addr.s_addr=INADDR_ANY;//inet_addr(ip);
    int opt = 1;
    setsockopt(serverfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    /*bind socket and ipaddr*/
    if(-1 == bind(serverfd,(struct sockaddr *)&server,sizeof(server)))
    {
        perror("bind error\n");
        snprintf(loginfo, sizeof(loginfo)-1, "RealWaveThreadFunc bind error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        pthread_exit(NULL);
    }
    /*enter listen mode*/
    if(-1 == listen(serverfd,10))
    {
        perror("listen error\n");
        snprintf(loginfo, sizeof(loginfo)-1, "RealWaveThreadFunc listen error");
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
        if(TCP_DEBUG == g_DebugType_EU)
        {                
            printf("clientfd : %d\n",clientfd);
            printf("client port : %d \n",ntohs(client.sin_port));            
        }

        printf("TCP communicate succeeded!\n");
        snprintf(loginfo, sizeof(loginfo)-1, "TCP communicate succeeded!");
        WRITELOGFILE(LOG_INFO_1,loginfo);
        printf("start TCP read:\n");
        if(read(clientfd ,readbuf,sizeof(readbuf))>0)
        {            
            if(TCP_DEBUG == g_DebugType_EU)
            {
                for (i=0;i<20;i++)
                {
                    printf("Readbuf[%02d]-data:0x%02x\n",i,readbuf[i]);
                } 
            }       
            //clear for next wave connect and send data.
            memset(g_ChanRealWave,0,sizeof(g_ChanRealWave));
            g_ChanRealNum = 0;
            //process the data for csr_driver to indicate which channel should be used
            RealTimeWaveExtr(readbuf,g_ChanRealWave,&g_ChanRealNum);
            SendSize = g_ChanRealNum << 1;            
        }
        printf("start TCP send:\n");
        g_socket_SendFlag  = 1;
        while(g_socket_SendFlag)
        {
            pthread_rwlock_rdlock(&g_PthreadLock_ST.BramDatalock);
            memset(returnbuf,0,sizeof(returnbuf));
            RealWaveData(returnbuf,g_ChanRealWave,&s_save_to_csr_driver,g_ChanRealNum);
            pthread_rwlock_unlock(&g_PthreadLock_ST.BramDatalock);

            if(TCP_DEBUG == g_DebugType_EU)
            {
                for(j = 0;j<SendSize;j+=2)
                {                           
                    printf("SendBuf[%02d:%02d]-data:0x%04x\n",j+1,j,returnbuf[j+1]<<8 | returnbuf[j]);   
                } 
            }

            WriteSize = write(clientfd,returnbuf,SendSize);               
            if(WriteSize > 0)
            {
                
            }
            //when Drive disconnect,return writesize 0,break out
            else if(WriteSize == 0)
            {
                printf("client close...\n");
                memset(loginfo,0,sizeof(loginfo));
                snprintf(loginfo, sizeof(loginfo)-1, "TCP client close");
                WRITELOGFILE(LOG_INFO_1,loginfo);
                g_socket_SendFlag  = 0;                               
                break;
            }
            //when Drive turn a wrong WriteSize is -1,break out
            else
            {
                printf("WriteSize error:%d\n",WriteSize);
                printf("client close...\n");
                memset(loginfo,0,sizeof(loginfo));
                snprintf(loginfo, sizeof(loginfo)-1, "TCP client close");
                WRITELOGFILE(LOG_INFO_1,loginfo);
                g_socket_SendFlag  = 0;
                break;
            }    
                                
        }
        printf("send complete,close clientfd\n");
        close(clientfd);        
    }
    close(serverfd);
    printf("------------>TCP server close\n");    
    pthread_exit(NULL);
    printf("Exit RealWaveThreadFunc\n");
    return CODE_OK;              
}

/**********************************************************************
*Name           :    Udp_Intool_ThreadFunc 
*Function       :    Udp communicate with Intool for Digtal Realwave Display
*Para           :  
*Version        :    REV1.0.0       
*Author:        :    zlz
*
*History:
*REV1.0.0       :   zlz    2022/1/6  Create
*********************************************************************/
void *Udp_Intool_ThreadFunc(void *arg)
{    
    int i;
    int serverfd;    
    uint16_t ReadSize = 0;
    uint16_t SendSize = 0;
    uint8_t szUsec[20] = {0};    // 微秒
    uint8_t szMsec[20] = {0};    // 毫秒 
    uint8_t readbuf[8] = {0};
    uint8_t sendbuf[264] = {0};  
    time_t  tCurrentTime = {0};
    uint8_t socket_SendFlag = 0;    
    struct tm tSysTime = {0};
    struct timespec timespec_st={0};
    static uint16_t UDP_LIFE = 0;
    struct sockaddr_in server,client;
    char loginfo[LOG_INFO_LENG]={0};    
   
    if(-1==(serverfd= socket(AF_INET,SOCK_DGRAM,0)))/*SOCK_DGRAM-使用UDP协议*/ 
    {
        printf("Udp Server socket creat error\n");
        memset(loginfo,0,sizeof(loginfo));
        snprintf(loginfo, sizeof(loginfo)-1, "Udp Server socket creat error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }

    bzero(&server,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(1610);//本机端口
    server.sin_addr.s_addr=inet_addr("192.168.3.11");//本机IP
    
    if(-1 == bind(serverfd,(struct sockaddr *)&server,sizeof(server)))//绑定服务器socket地址<ZYNQ相当于服务端>
    {
        printf("Udp Server bind error\n");
        memset(loginfo,0,sizeof(loginfo));
        snprintf(loginfo, sizeof(loginfo)-1, "Udp Server bind error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
        
    int length = sizeof(client);
    bzero(&client,length);
    client.sin_family=AF_INET;
    client.sin_port=htons(1600);//目标端口
    client.sin_addr.s_addr=inet_addr("192.168.3.100");//目标IP

    while(1)
    {
        /* if(UDP_DEBUG == g_DebugType_EU)
        {
            printf("UDP_Receive----->\n");
        }*/
        ReadSize = recvfrom(serverfd,readbuf,8,0,(struct sockaddr*)&client,&length);       
        if(ReadSize>0)//获取上位机发来的请求信息并进行解析
        {           
           /* if(UDP_DEBUG == g_DebugType_EU)
            {
                for (i=0;i<8;i++)
                {
                    printf("readbuf[%d]:%02x\n",i,readbuf[i]);
                }
                printf("Value:%d\n",(readbuf[2]>>1)&0x1);
            }*/ 
            if((readbuf[2]>>1)&0x1)
            {
                socket_SendFlag =1;
                /* if(UDP_DEBUG == g_DebugType_EU)
                {               
                    printf("UDP_Send<-----\n");
                }*/                
            } 
            else
            {
                printf("ECU have receive command from Intool,but SendFlag bit is not 1!\n");
            }               
        }
        else
        {            
            socket_SendFlag ==0;
        }

        if(socket_SendFlag)
        {            
            usleep(100000);
            tCurrentTime = time(NULL);
            localtime_r(&tCurrentTime, &tSysTime);
            clock_gettime(CLOCK_MONOTONIC,&timespec_st);  
            sprintf(szUsec,"%09d",(uint32_t)timespec_st.tv_nsec);  // get (ns) the right 9 number,the 9 number stands (s) transform to (ns)
            strncpy(szMsec,szUsec,3);// get the left 3 numbe , the 3 number mean (ms)

            sendbuf[0] = 0XA5 ;
            sendbuf[1] = 0X5A ;
            sendbuf[2] = 0X2  ;
            sendbuf[4] = 0X1  ;
            sendbuf[5] = 0X2  ;
            sendbuf[6] = UDP_LIFE >>8;
            sendbuf[7] = UDP_LIFE & 0XFF;
            sendbuf[8] = tSysTime.tm_year-100;
            sendbuf[9] = tSysTime.tm_mon+1;
            sendbuf[10] = tSysTime.tm_mday;
            sendbuf[11] = tSysTime.tm_hour;
            sendbuf[12] = tSysTime.tm_min;
            sendbuf[13] = tSysTime.tm_sec;
            sendbuf[14] = szMsec[0];
            sendbuf[15] = szMsec[1];
            sendbuf[62] = UDP_LIFE >>8;
            sendbuf[63] = UDP_LIFE & 0XFF;
            if(UDP_DEBUG == g_DebugType_EU)
            {
                printf("EADS_UDP_LIFE:0X%X\n",UDP_LIFE);
                printf("UDPTIME 20%d-%d-%d %d:%d:%d:%d\n",sendbuf[8],sendbuf[9],sendbuf[10],sendbuf[11],sendbuf[12],sendbuf[13],sendbuf[14]*10+sendbuf[15]);
            }
            pthread_rwlock_rdlock(&g_PthreadLock_ST.BramDatalock);
            SendSize = sendto(serverfd,sendbuf,sizeof(sendbuf),0,(struct sockaddr*)&client,length); //TODO 数字量实时波形显示的数据是否已经处理            
            pthread_rwlock_unlock(&g_PthreadLock_ST.BramDatalock);
            if(SendSize == sizeof(sendbuf))
            {
         
            }
            else
            {
                perror("Udp SendSize error!");                
            }
            UDP_LIFE++;    
        }                     
    }
    close(serverfd);
    printf("------------>Upd server close\n");    
    pthread_exit(NULL);
    printf("Exit Upd_Intool_ThreadFunc\n");
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
void *ModbusThreadFunc(void *arg)
{   
    int ModbusSocket = -1;
    int ModbusRes;
    modbus_mapping_t *ModbusMap_p;
    modbus_t *ModbusCtx;
    uint8_t *ModbusQuery;
    int8_t Rterr = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    const int PORT = 502;

    ModbusQuery = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
    ModbusCtx = modbus_new_tcp(NULL, PORT);
    printf("PORT:%d\n",PORT );
    modbus_set_debug(ModbusCtx, FALSE);
   
    /*modbus mapping init*/
    ModbusMap_p = modbus_mapping_new_start_address(UT_BITS_ADDRESS_TCP,            UT_BITS_NB_TCP,
                                                    UT_INPUT_BITS_ADDRESS_TCP,      UT_INPUT_BITS_NB_TCP,
                                                    UT_REGISTERS_ADDRESS_TCP,       UT_REGISTERS_NB_TCP,
                                                    UT_INPUT_REGISTERS_ADDRESS_TCP, UT_INPUT_REGISTERS_NB_TCP);
    /*modbus mapping init failure*/
    if(ModbusMap_p == NULL) 
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
            VersionSet(ModbusMap_p,&g_Version_ST);
            while(1) 
            {                
                /*the modbus receive */
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
                /*the modbus reply*/
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
    uint8_t i = 0;
    int8_t event_fd_1;
    int8_t event_fd_2;
    uint32_t Delayus_U32= 0;
    TMS570_BRAM_DATA Bram_Blvds_Read_Data={0};
        
    static uint32_t s_EventFileSaveNum_U32 = 0;   
    struct timeval A_Time_ST,A_TimeEnd_ST;
    printf("EventFile Delay Time(ms):%u\n",g_Rec_XML_ST.Rec_Event_ST.RecInterval);
    Delayus_U32 = g_Rec_XML_ST.Rec_Event_ST.RecInterval * 1000;//100ms
	sleep(1);

    while(g_LifeFlag > 0)
    {
        threadDelay(0,Delayus_U32);   
        
        if(s_EventFileSaveNum_U32 >= g_Rec_XML_ST.Rec_Event_ST.RecToTalNum)//60000
        {                
                printf("EventFile-Frames Number Reach:%d\n",s_EventFileSaveNum_U32);
                fflush(g_FileFd_ST.EventFile_fd);
                event_fd_1 = fileno(g_FileFd_ST.EventFile_fd);
                fsync(event_fd_1);
                fclose(g_FileFd_ST.EventFile_fd);
                g_FileFd_ST.EventFile_fd = NULL;
                EventFileCreateByNum(&g_FileFd_ST,&g_Rec_XML_ST,&g_TrainInfo_ST,&g_EADSErrInfo_ST);
                s_EventFileSaveNum_U32 = 0;            
        }
		else if(0 == (s_EventFileSaveNum_U32 % FILE_SYNC_NUM)) //5min
        {
			fflush(g_FileFd_ST.EventFile_fd);
            event_fd_1 = fileno(g_FileFd_ST.EventFile_fd);
            fsync(event_fd_1);
		}
        
        if(s_EventFileSaveNum_U32 >= g_Rec_XML_ST.Rec_Event_ST.RecToTalNum)//60000
        {                
                printf("EventFile-Frames Number Reach:%d\n",s_EventFileSaveNum_U32);
                fflush(g_FileFd_ST.EventFile_fd);
                event_fd_2 = fileno(g_FileFd_ST.EventFile_fd);
                fsync(event_fd_2);
                fclose(g_FileFd_ST.EventFile_fd);
                g_FileFd_ST.EventFile_fd = NULL;
                EventFileCreateByNum(&g_FileFd_ST,&g_Rec_XML_ST,&g_TrainInfo_ST,&g_EADSErrInfo_ST);
                s_EventFileSaveNum_U32 = 0;            
        }
		else if(0 == (s_EventFileSaveNum_U32 % FILE_SYNC_NUM)) //5min
        {
			fflush(g_FileFd_ST.EventFile_fd);
            event_fd_2 = fileno(g_FileFd_ST.EventFile_fd);
            fsync(event_fd_2);
		}

        pthread_rwlock_rdlock(&g_PthreadLock_ST.BramDatalock);	        
        ECU_EventDataSave(&g_FileFd_ST,&s_save_to_csr_driver);
        pthread_rwlock_unlock(&g_PthreadLock_ST.BramDatalock);
        s_EventFileSaveNum_U32++;

       BLVDSDataReadFunc(&Bram_Blvds_Read_Data,&g_EADSErrInfo_ST);
    }
    printf("exit FileSave thread\n");
    pthread_exit(NULL);      
}

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
    DirFileTar(&g_Rec_XML_ST);

    printf("exit DirTar thread\n");
    memset(loginfo,0,sizeof(loginfo));
    snprintf(loginfo, sizeof(loginfo)-1, "exit DirTar thread");
    WRITELOGFILE(LOG_INFO_1,loginfo);
    pthread_exit(NULL);
}

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
void *LEDPthreadFunc (void *arg)
{
    uint32_t led_period;
    int8_t lifeledfd;
    int8_t errledfd;
    
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
        lifeledfd = open(LEDLIFE_BRIGHTNESS, O_WRONLY);
        if (lifeledfd < 0)
        {
            printf("Cannot open Life LED\n");
            snprintf(loginfo, sizeof(loginfo)-1, "Cannot open Life LED");
            WRITELOGFILE(LOG_ERROR_1,loginfo);            
        }
        led_period =  PERIOD_COEFF;

        errledfd = open(LEDERR_BRIGHTNESS, O_WRONLY);
        if (errledfd < 0)
        {
            printf("Cannot open Err LED\n");
            snprintf(loginfo, sizeof(loginfo)-1, "Cannot open Err LED");
            WRITELOGFILE(LOG_ERROR_1,loginfo);           
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

        while (g_LifeFlag>0)
        {
            /******LED*******/
            #if 0
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
            #endif
            write(lifeledfd,"0",2); //on
            write(errledfd,"0",2); //on
            usleep(led_period);
            write(lifeledfd,"1",2);//off
            write(errledfd,"1",2);//off
            usleep(led_period);
            /******************/
            #if 0
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
            #endif
        }
    }
    close(errledfd);
    close(lifeledfd);    
    close(sockfd);
    printf("LEDPthreadFunc Quit\n");
    pthread_exit(NULL);
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
    #if 0
    /*time test*/
    struct timespec begin_ts,end_ts;    
    /*time test*/
    int8_t i,j,ret;
    static errnum_wr=0,errnum_rd=0,errnum_timeout=0;
    int socket_can0,nbytes;
    struct sockaddr_can addr_can0;
    struct ifreq ifr_can0;
    fd_set rfds ={0};
    struct timeval tv={0},tv_select={0};    
    char loginfo[LOG_INFO_LENG]={0};
    BRAM_CMD_PACKET CmdPact_RD_ST[3] = {0};
    BRAM_CMD_PACKET CmdPact_WR_ST[3] = {0};
    struct can_filter recv_filter[CAN0_READ_FRAME_NUM];
    struct can_frame s_can0_frame_RD_st[CAN0_READ_FRAME_NUM]    = {0};
    struct can_frame s_can0_frame_WR_st[CAN0_WRITE_FRAME_NUM]   = {0};
    TMS570_BRAM_DATA s_tms570_bram_RD_data_ch9_11_st[3] = {0};  //Read  bram data from  tms570
    TMS570_BRAM_DATA s_tms570_bram_WR_data_ch9_11_st[3] = {0};  //Write bram data to    tms570
    
    
    socket_can0 = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    strcpy(ifr_can0.ifr_name, "can0" );
    ioctl(socket_can0, SIOCGIFINDEX, &ifr_can0);
    addr_can0.can_family = AF_CAN;
    addr_can0.can_ifindex = ifr_can0.ifr_ifindex;
    bind(socket_can0, (struct sockaddr *)&addr_can0, sizeof(addr_can0));/*bind*/   
    ioctl(socket_can0,SIOCGSTAMP,&tv);/*add time stamp*/   
    setsockopt(socket_can0,SOL_CAN_RAW,CAN_RAW_FILTER,recv_filter,sizeof(recv_filter));/*Filter*/
    
    CAN_FrameInit(recv_filter,s_can0_frame_WR_st,CAN0_TYPE);/*CAN_ID Init*/
    TMS570_Bram_TopPack_Set(CmdPact_WR_ST,CmdPact_RD_ST,CAN0_TYPE); /*TMS570 Bram TopPack Init*/

    uint8_t testbuff[32]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,\
                    0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x00,\
                    0x12,0x34,0x56,0x78,0x9a,0xbc,0xde,0xf0,\
                    0x78,0x65,0x32,0x10,0x54,0x23,0x99,0xaa};
    memcpy(&s_tms570_bram_WR_data_ch9_11_st[0].buffer[0],testbuff,32);
    memcpy(&s_tms570_bram_WR_data_ch9_11_st[1].buffer[0],testbuff,32);
    memcpy(&s_tms570_bram_WR_data_ch9_11_st[2].buffer[0],testbuff,32);

    while(g_LifeFlag>0)
    {        
        clock_gettime(CLOCK_MONOTONIC,&begin_ts);
        tv_select.tv_usec = 100000;
        FD_ZERO(&rfds);
        FD_SET(socket_can0,&rfds);
        ret = select(socket_can0+1,&rfds,NULL,NULL,&tv_select);
        if(ret == 0)
        {         
            errnum_timeout++;
            printf("can0 read time out %d times!\n",errnum_timeout);
            if(errnum_timeout >=5)
            {
                snprintf(loginfo, sizeof(loginfo)-1, "CAN0 receive frame time out!");
                WRITELOGFILE(LOG_ERROR_1,loginfo);
                errnum_timeout = 0;
            }
            continue; 
        }
        else if(ret>0)
        {
            errnum_timeout=0;
            CAN_Read_Option(socket_can0,s_can0_frame_RD_st,CAN0_READ_FRAME_NUM,CAN0_TYPE);               
            CAN_ReadData_Pro(s_can0_frame_RD_st,s_tms570_bram_WR_data_ch9_11_st,CAN0_TYPE);            
            TMS570_Bram_Write_Func(CmdPact_WR_ST,s_tms570_bram_WR_data_ch9_11_st,3,CAN0_BRAM);
        }        
        else if(ret == -1)
        {
            printf("CAN0-select Fun return -1!\n");
            continue;
        }

        TMS570_Bram_Read_Func(CmdPact_RD_ST,s_tms570_bram_RD_data_ch9_11_st,3,CAN0_BRAM);
        CAN_WriteData_Pro(s_can0_frame_WR_st,s_tms570_bram_RD_data_ch9_11_st,CAN0_TYPE);
        CAN_Write_Option(socket_can0,s_can0_frame_WR_st,CAN0_WRITE_FRAME_NUM,CAN0_TYPE);
        /*time test*/
        clock_gettime(CLOCK_MONOTONIC,&end_ts);
        if(g_DebugType_EU == TIME_DEBUG)
        {
            printf("SigleCycle-can1 cost time:%ld(us)\n",1000000*(end_ts.tv_sec-begin_ts.tv_sec)+(end_ts.tv_nsec-begin_ts.tv_nsec)/1000);
        }       

        usleep(100000);
    }
    close(socket_can0);
    return 0;
    #endif   
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
    /*time test*/
    struct timespec begin_ts,end_ts;    
    /*time test*/
    uint8_t i,j,ret;    
    static int errnum_timeout=0;
    int    socket_can1;
    struct sockaddr_can addr_can1;
    struct ifreq ifr_can1;
    fd_set rfds;
    char loginfo[LOG_INFO_LENG]={0}; 
    struct timeval tv={0},tv_select={0};
    struct can_filter recv_filter[CAN1_READ_FRAME_NUM];
    BRAM_CMD_PACKET CmdPact_RD_ST = {0};
    BRAM_CMD_PACKET CmdPact_WR_ST = {0};
    struct can_frame s_can1_frame_RD_st[16] = {0};
    struct can_frame s_can1_frame_WR_st[8] = {0};
    TMS570_BRAM_DATA s_tms570_bram_RD_data_ch12_st = {0};  //Read  bram data from  tms570
    TMS570_BRAM_DATA s_tms570_bram_WR_data_ch12_st = {0};  //Write bram data to    tms570      
    
    socket_can1 = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    strcpy(ifr_can1.ifr_name, "can1" );
    ioctl(socket_can1, SIOCGIFINDEX, &ifr_can1);
    addr_can1.can_family = AF_CAN;
    addr_can1.can_ifindex = ifr_can1.ifr_ifindex;
    bind(socket_can1, (struct sockaddr *)&addr_can1, sizeof(addr_can1));/*bind*/    
    ioctl(socket_can1,SIOCGSTAMP,&tv); // add time stamp   
    setsockopt(socket_can1,SOL_CAN_RAW,CAN_RAW_FILTER,recv_filter,sizeof(recv_filter));/*Filter*/
    
    CAN_FrameInit(recv_filter,s_can1_frame_WR_st,CAN1_TYPE);/*CAN_ID Init*/
    TMS570_Bram_TopPack_Set(&CmdPact_WR_ST,&CmdPact_RD_ST,CAN1_TYPE); /*TMS570 Bram TopPack Init*/    
    
    uint8_t testbuff[32]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,\
                    0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x00,\
                    0x12,0x34,0x56,0x78,0x9a,0xbc,0xde,0xf0,\
                    0x78,0x65,0x32,0x10,0x54,0x23,0x99,0xaa};
    /*memcpy(&s_tms570_bram_WR_data_st[4].buffer[0],testbuff,32);*/
    for (i=0;i<3;i++)
    {
        memcpy(&s_can1_frame_WR_st[i].data[0],testbuff,8);
    }    
    memcpy(&s_tms570_bram_WR_data_ch12_st.buffer[0],testbuff,32);
    while(g_LifeFlag>0)
    {
        //clock_gettime(CLOCK_MONOTONIC,&begin_ts);
        tv_select.tv_usec = 100000;
        FD_ZERO(&rfds);
        FD_SET(socket_can1,&rfds);
        ret = select(socket_can1+1,&rfds,NULL,NULL,&tv_select);
        
        if(ret == 0)
        {
            errnum_timeout++;
            printf("can1 read time out %d times!\n",errnum_timeout);
            if(errnum_timeout >=13)
            {
                snprintf(loginfo, sizeof(loginfo)-1, "CAN1 receive frame time out!");
                WRITELOGFILE(LOG_ERROR_1,loginfo);
                errnum_timeout = 0;
            }
            continue; 
        }
        else if(ret>0)
        {            
            errnum_timeout=0;
            CAN_Read_Option(socket_can1,s_can1_frame_RD_st,CAN1_READ_FRAME_NUM,CAN1_TYPE);                       
            CAN_ReadData_Pro(s_can1_frame_RD_st,&s_tms570_bram_WR_data_ch12_st,CAN1_TYPE);
            TMS570_Bram_Write_Func(&CmdPact_WR_ST,&s_tms570_bram_WR_data_ch12_st,1,CAN1_BRAM);
        }
        else if(ret == -1)
        {
            printf("CAN1-select Fun return -1!\n");
            continue;
        }

        TMS570_Bram_Read_Func(&CmdPact_RD_ST,&s_tms570_bram_RD_data_ch12_st,1,CAN1_BRAM);
        CAN_WriteData_Pro(s_can1_frame_WR_st,&s_tms570_bram_RD_data_ch12_st,CAN1_TYPE);
        CAN_Write_Option(socket_can1,s_can1_frame_WR_st,CAN1_WRITE_FRAME_NUM,CAN1_TYPE);
        /*time test*/
        //clock_gettime(CLOCK_MONOTONIC,&end_ts);
        if(g_DebugType_EU == TIME_DEBUG)
        {
            printf("SigleCycle-can1 cost time:%ld(us)\n",1000000*(end_ts.tv_sec-begin_ts.tv_sec)+(end_ts.tv_nsec-begin_ts.tv_nsec)/1000);  
        }
        
        usleep(100000);
    }
    close(socket_can1);
    return 0;
    #endif      
}

/**********************************************************************
*Name           :   MVBThreadFunc  
*Function       :   read/write the MVB data from BRAM and read/write the MVB date to TMS570
*Para           :  
*Version        :   REV1.0.0       
*Author:        :   zlz
*direction      :   从BRAM指定区域读写MVB数据
*History:
*REV1.0.0       :   zlz    2021/12/4  Create
*********************************************************************/
void *MVBThreadFunc(void *arg)
{
    #if 0
    int8_t  i,j;    
    uint8_t testbuff_32[32]={0};    
    BRAM_CMD_PACKET CmdPact_RD_ST ={0};
    BRAM_CMD_PACKET CmdPact_WR_ST ={0};  
    BRAM_CMD_PACKET MVB_CmdPact_RD_ST[16] = {0};
    BRAM_CMD_PACKET MVB_CmdPact_WR_ST[16] = {0};
    TMS570_BRAM_DATA s_mvb_bram_RD_data_st[16] = {0};
    TMS570_BRAM_DATA s_mvb_bram_WR_data_st[16] = {0};    
    TMS570_BRAM_DATA s_tms570_bram_RD_data_ch8_st = {0};  //Read  bram data from  tms570
    TMS570_BRAM_DATA s_tms570_bram_WR_data_ch8_st = {0};  //Write bram data  to   tms570        
    
    /*Just for test*/
    for(j=0;j<6;j++)
    {
        for (i=0;i<32;i++)
        {
            testbuff_32[i] = i+j;
        }
        memcpy(s_mvb_bram_WR_data_st[j].buffer,testbuff_32,32);
    }
    /*Just for test*/

    MVB_Bram_Init(&CmdPact_RD_ST,&CmdPact_WR_ST,MVB_CmdPact_RD_ST,MVB_CmdPact_WR_ST);
    while(1)
    {       
        pthread_rwlock_wrlock(&g_PthreadLock_ST.BramDatalock);
        MVB_Bram_Read_Func(MVB_CmdPact_RD_ST,s_mvb_bram_RD_data_st);
        MVB_RD_Data_Proc(s_mvb_bram_RD_data_st,&s_tms570_bram_WR_data_ch8_st);       
        TMS570_Bram_Write_Func(&CmdPact_WR_ST,&s_tms570_bram_WR_data_ch8_st,1,MVB_BRAM);  
        
        TMS570_Bram_Read_Func(&CmdPact_RD_ST,&s_tms570_bram_RD_data_ch8_st,1,MVB_BRAM);
        MVB_WR_Data_Proc(s_mvb_bram_WR_data_st,&s_tms570_bram_RD_data_ch8_st);
        MVB_Bram_Write_Func(MVB_CmdPact_WR_ST,s_mvb_bram_WR_data_st);
        ECU_Record_Data_Pro_Fun(&s_save_to_csr_driver,&s_tms570_bram_RD_data_ch8_st,&s_tms570_bram_WR_data_ch8_st,g_EADSErrInfo_ST);
        pthread_rwlock_unlock(&g_PthreadLock_ST.BramDatalock);        
        usleep(64000);       
    }
    printf("exit MVBThreadFunc Function!\n");
    pthread_exit(NULL);
    #endif      
}