/**********************************************************************
*File name 		:   Timer.h
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/1/29
*Description    :   System timer set.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/5/16  Create
*********************************************************************/

/**********************************************************************
*
*Include file  section
*
*********************************************************************/
#include "Timer.h"

/**********************************************************************
*
*Function Define Section
*
*********************************************************************/
/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/
extern DEBUG_TYPE_ENUM g_DebugType_EU;

/***********************************************************************
*Local Macro Define Section*
*********************************************************************/
#define MONTH_PER_YEAR   12   // 一年12月
#define YEAR_MONTH_DAY   20   // 年月日缓存大小
#define HOUR_MINUTES_SEC 20   // 时分秒缓存大小


/**********************************************************************
*Name			:   void set_timer(uint8_t second,uint32_t usecond)  
*Function       :   Writ U32bit value to the phyaddr of Bram
*Para     		:   uint64_t phyaddr  the addres of Bram
*                   uint32_t val    the write value
*Return    		:   void.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*********************************************************************/
  
int8_t set_timer(uint32_t second,uint32_t usecond)  
{  
    struct itimerval itv;  
    itv.it_interval.tv_sec = second;  //设置为1秒
    itv.it_interval.tv_usec = usecond;  
    itv.it_value.tv_sec = second;  
    itv.it_value.tv_usec = usecond;  
    setitimer(ITIMER_REAL, &itv, NULL);  //此函数为linux的api,不是c的标准库函数
}  
/**********************************************************************
*Name			:   int8_t SetSystemTime(SYSTEM_TIME *DatStr)   
*Function       :   Set the Linux System Time
*Para     		:   SYSTEM_TIME *DatStr 
*Return    		:   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*********************************************************************/
int8_t SetSystemTime(SYSTEM_TIME *DatStr)  
{  
 
    uint16_t Year_U16;
    int8_t err;
    struct tm Time_ST;  
    struct timeval TimeVal_ST;  
    time_t timep;  
    char loginfo[LOG_INFO_LENG] = {0};
    Year_U16 = (DatStr -> Year_U8[0] << 8) + DatStr -> Year_U8[1];
    Time_ST.tm_year = Year_U16 - 1900; 
    Time_ST.tm_mon = DatStr -> Month_U8 - 1;
    Time_ST.tm_mday = DatStr -> Day_U8;
    Time_ST.tm_hour = DatStr -> Hour_U8;
    Time_ST.tm_min = DatStr -> Minute_U8;       
    Time_ST.tm_sec =DatStr -> Second_U8;  
  
    timep = mktime(&Time_ST);  
    TimeVal_ST.tv_sec = timep;  
    TimeVal_ST.tv_usec = 0;  
    if(settimeofday(&TimeVal_ST, (struct timezone *) 0) < 0)  
    {  	   
        err =  CODE_ERR;  
        printf("Set system time Failed\n");
        snprintf(loginfo, sizeof(loginfo)-1, "Set system time Failed");
        WRITELOGFILE(LOG_ERROR_1,loginfo); 

    }
    else
    {
    	err =  CODE_OK;
        printf("Set system time Success\n");
        snprintf(loginfo, sizeof(loginfo)-1, "Set system time Success");
        WRITELOGFILE(LOG_INFO_1,loginfo); 
    }
    if(TIME_DEBUG == g_DebugType_EU)
    { 
        printf("system time %d-%d-%d %d:%d:%d\n",Time_ST.tm_year,Time_ST.tm_mon,Time_ST.tm_mday, Time_ST.tm_hour,Time_ST.tm_min,Time_ST.tm_sec);
    }  
    return err;  
}    


/**********************************************************************
*Name           :   int8_t SetSystemTime(SYSTEM_TIME *DatStr)   
*Function       :   Set the Linux System Time
*Para           :   SYSTEM_TIME *DatStr 
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*********************************************************************/
int8_t SetRTCTime(SYSTEM_TIME *DatStr)  
{  
 
    uint16_t Year_U16;
    int8_t err;
    int8_t rtcfd;
    struct tm Time_ST;  
    char loginfo[LOG_INFO_LENG] = {0};  
    Year_U16 = (DatStr -> Year_U8[0] << 8) + DatStr -> Year_U8[1];
    Time_ST.tm_year = Year_U16 - 1900; 
    Time_ST.tm_mon = DatStr -> Month_U8 - 1;
    Time_ST.tm_mday = DatStr -> Day_U8;
    Time_ST.tm_hour = DatStr -> Hour_U8;
    Time_ST.tm_min = DatStr -> Minute_U8;       
    Time_ST.tm_sec =DatStr -> Second_U8;  
  
    if ((rtcfd = open ("/dev/rtc0", O_WRONLY )) < 0 )
    {
        perror ( "Could not access RTC" );
        snprintf(loginfo, sizeof(loginfo)-1, "Could not access /dev/rtc0");
        WRITELOGFILE(LOG_ERROR_1,loginfo); 
        return CODE_ERR;  
    } 
     
    if (ioctl( rtcfd, RTC_SET_TIME, &Time_ST ) < 0 )
    {
        perror ( "Could not set the RTC time" );
        snprintf(loginfo, sizeof(loginfo)-1, "Could not set the RTC time");
        WRITELOGFILE(LOG_ERROR_1,loginfo); 
        err = CODE_ERR;  
    }
    else
    {
        printf( "Set the RTC time success\n");
        printf("Set the RTC time: %d-%d-%d %d:%d:%d\n",Time_ST.tm_year,Time_ST.tm_mon,Time_ST.tm_mday, Time_ST.tm_hour,\
        Time_ST.tm_min,Time_ST.tm_sec);
        snprintf(loginfo, sizeof(loginfo)-1, "Set the RTC time success");
        WRITELOGFILE(LOG_INFO_1,loginfo); 
        err = CODE_OK;  
    } 
    close (rtcfd);
    return err;  
}    


/**********************************************************************
*Name           :   RTCTesT
*Function       :   
*Para           :   SYSTEM_TIME *DatStr 
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/8/26  Create
*********************************************************************/
int8_t RTCTesT(void)  
{  
 
    uint16_t Year_U16;
    int8_t err;
    int8_t rtcfd;
    struct tm Time_ST;  
    char loginfo[LOG_INFO_LENG] = {0};  
    Time_ST.tm_year = Year_U16 - 1900; 

    if ((rtcfd = open ("/dev/rtc0", O_RDONLY )) < 0 )
    {
        perror ( "Could not access RTC" );
        snprintf(loginfo, sizeof(loginfo)-1, "Could not access /dev/rtc0");
        WRITELOGFILE(LOG_ERROR_1,loginfo); 
        return CODE_ERR;  
    } 
     
    if (ioctl( rtcfd, RTC_RD_TIME, &Time_ST ) < 0 )
    {
        perror ( "Could not Read the RTC time" );
        snprintf(loginfo, sizeof(loginfo)-1, "Could not Read the RTC time");
        WRITELOGFILE(LOG_ERROR_1,loginfo); 
        err = CODE_ERR;  
    }

    Year_U16 = Time_ST.tm_year + 1900;
    if(Year_U16 > 2010)
    {
        err = CODE_OK;  
    }
    else
    {
        printf( "the RTC time Error,year is %u \n",Year_U16);
        snprintf(loginfo, sizeof(loginfo)-1, "the RTC time Error,year is %u",Year_U16);
        WRITELOGFILE(LOG_ERROR_1,loginfo); 
        err = CODE_OK;  
    } 
    close (rtcfd);
    return err;  
}    
/**********************************************************************
*Name           :   int8_t SetSystemTime(SYSTEM_TIME *DatStr)   
*Function       :   Set the Linux System Time
*Para           :   SYSTEM_TIME *DatStr 
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*********************************************************************/
int threadDelay(const long lTimeSec, const long lTimeUSec)
{
    struct timeval timeOut;
    timeOut.tv_sec = lTimeSec;
    timeOut.tv_usec = lTimeUSec;
    if (0 != select(0, NULL, NULL, NULL, &timeOut))
    {
        return 1;
    }
    return CODE_OK;
}



/**********************************************************************
*Name           :   void GetTime(uint8_t *pszTimeStr) 
*Function       :   Get the Linux System Time
*Para           :   uint8_t *pszTimeStr,the time as string
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*********************************************************************/
void GetTime(uint8_t *pszTimeStr)
{
    struct tm      tSysTime     = {0};
    struct timeval tTimeVal     = {0};
    time_t         tCurrentTime = {0};

    uint8_t  szUsec[20] = {0};    // 微秒
    uint8_t  szMsec[20] = {0};    // 毫秒

    if (pszTimeStr == NULL)
    {
        return;
    }

    tCurrentTime = time(NULL);
    localtime_r(&tCurrentTime, &tSysTime);   // localtime_r是线程安全的

    gettimeofday(&tTimeVal, NULL);    
    sprintf(szUsec, "%06d", (uint32_t)tTimeVal.tv_usec);  // 获取微秒
    strncpy(szMsec, szUsec, 3);                // 微秒的前3位为毫秒(1毫秒=1000微秒)

    sprintf(pszTimeStr, "[%04d.%02d.%02d %02d:%02d:%02d.%3.3s]", 
            tSysTime.tm_year+1900, tSysTime.tm_mon+1, tSysTime.tm_mday,
            tSysTime.tm_hour, tSysTime.tm_min, tSysTime.tm_sec, szMsec);
}
/**********************************************************************
*Name           :   i2c_write 
*Function       :   write date to i2c_slave
*Para           :   int fd, --i2c-1总线的文件描述符
                    unsigned int addr,--i2c从设备地址
                    unsigned int offset,--要写入的i2c寄存器地址
                    unsigned char *buf,--发送的数据区
                    unsigned int len --数据区长度
*Return         :  if success 0;if err -1.
*Version        :  REV1.0.0       
*Author:        :  zlz
*
*History:
*REV1.0.0     zlz    2021/10/19  Create
*********************************************************************/
int8_t i2c_write(int fd, 
              unsigned int addr,
              unsigned int offset,
              unsigned char *buf,
              unsigned int len)
{
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg i2cmsg;
	int i;
	char _buf[len+1];
 
	if(len>MAX_BYTES)
	{		
        printf("i2c datelength is invaild!\n");
        return CODE_ERR;
	}
 
	_buf[0]=offset;//要访问的寄存器地址
	for(i=0;i<len;i++)//数据区
	{
	    _buf[1+i]=buf[i];	    
	}
 
	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;//代表msg的数量，如果是n，则可以定义i2c_msg[n]数组
 
	i2cmsg.addr  = addr;//从设备地址
	i2cmsg.flags = 0;
	i2cmsg.len   = 1+len;
	i2cmsg.buf   = _buf;
 
	if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0)
    {
		perror("ioctl()");
		fprintf(stderr,"ioctl WR returned %d\n",i);
		return CODE_ERR;
	} 
	return CODE_OK;
}

/**********************************************************************
*Name           :   i2c_read 
*Function       :   read date from i2c_slave
*Para           :   int fd, --i2c-1总线的文件描述符
                    unsigned int addr,--i2c从设备地址
                    unsigned int offset,--要读取的i2c寄存器地址
                    unsigned char *buf,--发送的数据区
                    unsigned int len --数据区长度
*Return         :   if success 0;if err -1.
*Version        :   REV1.0.0       
*Author:        :   zlz
*History        :
*REV1.0.0       :   zlz    2021/10/19  Create
*********************************************************************/ 
int8_t i2c_read(int fd,
			unsigned int addr,
			unsigned int offset,
			unsigned char *buf,
			unsigned int len)
{
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg i2cmsg;
	int i;    
	if(len>MAX_BYTES)
	{
        printf("i2c datelength is invaild!\n");
		return CODE_ERR;
	}
 
	if(i2c_write(fd,addr,offset,NULL,0)<0)
	{
	    perror("ioctl()");
	    fprintf(stderr,"ioctl WR returned %d\n",i);
        return CODE_ERR;
	}	
	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;	
	i2cmsg.addr  = addr;
	i2cmsg.flags = I2C_M_RD;
	i2cmsg.len   = len;
	i2cmsg.buf   = buf; 
 
	if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0)
    {
	    perror("ioctl()");
	    fprintf(stderr,"ioctl RD returned %d\n",i);
	    return CODE_ERR;
	} 
	return CODE_OK;
}
 
 /**********************************************************************
*Name           :   GetCompileTime 
*Function       :   show compile time and author
*Para           :   void
*Return         :   void
*Version        :   REV1.0.0       
*Author:        :   zlz
*History        :
*REV1.0.0       :   zlz    2021/12/19  Create
*********************************************************************/ 
void GetCompileTime(void)
{
  uint8_t i;
  int32_t year, month, day, hour, minutes, seconds;
  const char year_month[MONTH_PER_YEAR][4] ={ "Jan", "Feb", "Mar", "Apr", "May", "Jun",\
                                              "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  char compile_date[YEAR_MONTH_DAY] = {0};
  char compile_time[HOUR_MINUTES_SEC] = {0};
  char str_month[4] = {0};
  
 
  sprintf(compile_date, "%s", __DATE__);
  sprintf(compile_time, "%s", __TIME__);
 
  sscanf(compile_date, "%s %d %d", str_month, &day, &year);
  sscanf(compile_time, "%d:%d:%d", &hour, &minutes, &seconds);
 
  for(i = 0; i < MONTH_PER_YEAR; ++i)
  {
    if(strncmp(str_month, year_month[i], 3) == 0)
    {
      month = i + 1;
      break;
    }
  } 
  printf("Compile time:%d-%d-%d %d:%d:%d,author:ZLZ\n",year,month,day,hour,minutes,seconds);
}