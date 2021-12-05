/**********************************************************************
*File name 		:   WriteLog
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2020/4/21
*Description    :   System timer set.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/4/21  Create
*********************************************************************/

/**********************************************************************
*
*Include file  section
*
*********************************************************************/
#include "WriteLog.h"

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
static uint32_t s_LogShowLine = 0;
static uint32_t  s_LogLevel = 0;
static FILE * s_LogFile_fd;
/**********************************************************************
*
*Function Define Section
*
*********************************************************************/


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
static int8_t LogFileCreate(uint8_t * path);
static uint8_t *LogLevel(uint32_t iLogLevel);
static void GetConfigValue(uint8_t * Logdirp);
static void GetStringContentValue(FILE *fp, uint8_t *pszSectionName, uint8_t *pszKeyName, uint8_t *pszOutput, uint32_t iOutputLen);
static void GetConfigFileStringValue(uint8_t *pszSectionName, uint8_t *pszKeyName, uint8_t *pDefaultVal, uint8_t *pszOutput, uint32_t iOutputLen, uint8_t *pszConfigFileName);
static uint32_t GetConfigFileIntValue(uint8_t *pszSectionName, uint8_t *pszKeyName, uint32_t iDefaultVal, uint8_t *pszConfigFileName);
/**********************************************************************
* 功能描述：主函数
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期        版本号       修改人            修改内容
* -------------------------------------------------------------------
* 20200421        V1.0      feng        创建
***********************************************************************/

/**********************************************************************
*Name     :   int8_t FileCreatePowOn(void)
*Function       :   when application run,creat  Event and LR file
*Para         :   void 
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/6/29  Create
*********************************************************************/
int8_t LogFileCreatePowOn(void)
{

    //creat  logfile dir;
    uint8_t File_Directory_U8[LOG_PATH_LENG] = {"/yaffs/REC_LOGGFLT/"}; //for logConFig file error
    GetConfigValue(File_Directory_U8);
    MultiDircCreate(File_Directory_U8);   
    LogFileCreate(File_Directory_U8);
    /*should create file,then delete to save,to log this infomation*/
    DeleteEarliestFile(File_Directory_U8,LOG_FILE_TYPE);  
    return CODE_OK;

}

/**********************************************************************
*Name     :   int8_t EventFileCreateByNum(void)
*Function       :   Creat Event file by Eventsave Num,For PTU analysis
*Para         :   void 
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/6/29  Create
*note Para  can improve with Freesize
*********************************************************************/
static int8_t LogFileCreate(uint8_t *path)
{ 
    //FILE *fd;
    uint8_t File_LogName_U8[200] = {0};
    uint8_t TryNum = 0;
    int8_t err = 0;
    uint8_t TimeString_U8[40] = {0};
    char loginfo[LOG_INFO_LENG] = {0};
    time_t timep_ST;   
    struct tm *now_time_p_ST; 

    time(&timep_ST);
    now_time_p_ST = localtime(&timep_ST); /*change to the local time*/
    sprintf(TimeString_U8,"%04d%02d%02d",
          (1900 + now_time_p_ST->tm_year), 
          (1 + now_time_p_ST->tm_mon), now_time_p_ST->tm_mday); 
         // 获取配置文件中各个配置项的值

    sprintf(File_LogName_U8,"%s%s_%s.txt",path,LOG_FILE_HEAD,TimeString_U8);
    if(s_LogFile_fd != NULL)
    {
        fclose(s_LogFile_fd);
        s_LogFile_fd = NULL;
    }
    s_LogFile_fd = fopen(File_LogName_U8, "at+");
    if(s_LogFile_fd == NULL)
    {
        TryNum ++;
        perror("creat log file failed");
        /*!!!!! can not write LOG,for fopen failed */
    }
    else
    {
        printf("creat log file %s success \n",File_LogName_U8);
        snprintf(loginfo, sizeof(loginfo)-1, "creat %s success ",File_LogName_U8);
        WRITELOGFILE(LOG_INFO_1,loginfo); 
        err = CODE_OK;
    }
    while((TryNum > 0)&&(TryNum < FILETRY_NUM))
    {
        s_LogFile_fd = fopen(File_LogName_U8, "at+"); //every time creat the file and write replace   
        if(NULL == s_LogFile_fd )
        {
            TryNum ++;
            perror("creat log file failed again");
        }
        else
        {
            TryNum = 0;/*quit the while*/
            printf("creat log file %s success \n",File_LogName_U8);
            snprintf(loginfo, sizeof(loginfo)-1, "creat %s success ",File_LogName_U8);
            WRITELOGFILE(LOG_INFO_1,loginfo); 
            err = CODE_OK;
 
        }

    }
    if(TryNum >= FILETRY_NUM)
    {
        
        printf("creat log file failed");
        err  = CODE_ERR;
    }
    return err;
}



int32_t VersionSave(VERSION * Ver_p)
{
    char  LogContent[LOG_INFO_LENG] = {0};
    // 先打印版本相关信息
    snprintf(LogContent,sizeof(LogContent)-1, "Total_Ver %d,Linux_Ver %d,EADS_Ver %d, Build time[%s %s].",Ver_p -> Total_Ver_U16,
    Ver_p -> Linux_Ver_U16,
    Ver_p -> EADS_RunVer_U16,
     __DATE__, __TIME__);

    WRITELOGFILE(LOG_INFO_1, LogContent);
    return CODE_OK; 
}
/**********************************************************************
*Name     :   int8_t EventFileCreateByNum(void)
*Function       :   Creat Event file by Eventsave Num,For PTU analysis
*Para         :   void 
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/6/29  Create
*note Para  can improve with Freesize
*********************************************************************/
 
/**********************************************************************
 * 功能描述： 将内容写到日志文件中
 * 输入参数： FileName-代码文件名
             pszFunctionName-代码所在函数名
             iCodeLine-代码行
             iLogLevel-日志等级
             Content-每条日志的具体内容
 * 输出参数： 无
 * 返 回 值： 无
 * 其它说明： 无
 * 修改日期        版本号         修改人          修改内容
 * -------------------------------------------------------------------
 * 20200421        V1.0       feng      创建
 ********************************************************************/    
int8_t WriteLogFile(uint8_t *FileName, const char * Function, uint32_t CodeLine, uint32_t inLogLevel, char * Content)
{
    
    char  LogContent[512] = {0};
    uint8_t  TimeStr[64]     = {0};
    uint8_t  *LogLevelInfo;
    if ((FileName == NULL) || (Content == NULL) || (s_LogFile_fd == NULL))
    {
        printf("s_LogFile_fd is NULL\n");
        return;
    }
    // 过滤日志等级
    if (inLogLevel > s_LogLevel)
    {
        return;
    }
    //printf("s_LogFile_fd %x \n",s_LogFile_fd);
    // 写入日志时间
    GetTime(TimeStr);
    fputs(TimeStr,s_LogFile_fd);
    LogLevelInfo = LogLevel(inLogLevel);

    // 写入日志内容
    if (1 == s_LogShowLine)    // 在日志信息中显示"文件名/函数名/代码行数"信息
    {
        snprintf(LogContent, sizeof(LogContent)-1, "[%-15s][%-20s][%05d][%-6s]:%s\n", FileName, Function, CodeLine,LogLevelInfo, Content);
    }
    else// 不用在日志信息中显示"文件名/代码行数"信息
    {
        snprintf(LogContent, sizeof(LogContent)-1, "[%s]%s \n",LogLevelInfo, Content);
    }
    fputs(LogContent, s_LogFile_fd);
    fflush(s_LogFile_fd);     // 刷新文件
    return CODE_OK;
}
/**********************************************************************
*Name           :   int8_t LogClose  
*Function       :   close the file to file
*Para           :  :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/7/6  Create
*REV1.0.1    feng    2020/8/31  EventFileCreateByNum first will run LogClose,but the Log not create 
so cause Segmentation fault
*********************************************************************/
int8_t LogClose(void)
{
    if(s_LogFile_fd != NULL)
    {
        fclose(s_LogFile_fd);     //close
        s_LogFile_fd = NULL;
    }
    return CODE_OK;
}

/**********************************************************************
*Name           :   int8_t LogFileSync  
*Function       :   sync the file to file
*Para           :  :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/7/6  Create
*********************************************************************/
int8_t LogFileSync(void)
{

    int8_t fd;
    fflush(s_LogFile_fd);     // 刷新文件
    fd = fileno(s_LogFile_fd);
    fsync(fd);
    printf("finish LogFileSync!\n");
}



/**********************************************************************
 * 功能描述： 获取对应的日志等级
 * 输入参数： iLogLevel-日志等级
 * 输出参数： 无
 * 返 回 值： 日志等级信息字符串
 * 其它说明： 无
 * 修改日期        版本号         修改人          修改内容
 * -------------------------------------------------------------------
 * 20200421        V1.0        feng      创建
 ********************************************************************/ 
static uint8_t *LogLevel(uint32_t iLogLevel)
{
    switch (iLogLevel)
    {
        case LOG_FATAL_1:
        {
            return "FATAL";
        }

        case LOG_ERROR_1:   
        {
            return "ERROR";
        }

        case LOG_WARN_1 :
        {
            return "WARN";
        }

        case LOG_INFO_1 :
        {
            return "INFO";
        }

        case LOG_TRACE_1:   
        {
            return "TRACE";
        }

        case LOG_DEBUG_1:   
        {
            return "DEBUG";
        }

        case LOG_ALL_1:   
        {
            return "ALL";
        }

        default: 
        {
            return "OTHER";
        }
    }
}

/**********************************************************************
 * 功能描述： 获取日志配置项的值
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 其它说明： 无
 * 修改日期        版本号         修改人         修改内容
 * -------------------------------------------------------------------
 * 20200421        V1.0       feng      创建
 ********************************************************************/ 
static void GetConfigValue(uint8_t * Logdirp)
{

    // 日志等级
    s_LogLevel = GetConfigFileIntValue("LOG", "LogLevel", 5, LOG_CFG);
    // whether write the line and function
    s_LogShowLine = GetConfigFileIntValue("LOG", "LogPosition", 1, LOG_CFG);
    // 日志文件存放目录
   // GetConfigFileStringValue("LOG", "LogDir", "", Logdirp, LOG_PATH_LENG,LOG_CFG);
}


/**********************************************************************
* 功能描述： 获取具体的字符串值
* 输入参数： fp-配置文件指针
            pszSectionName-段名, 如: GENERAL
            pszKeyName-配置项名, 如: EmployeeName
            iOutputLen-输出缓存长度
* 输出参数： pszOutput-输出缓存
* 返 回 值： 无
* 其它说明： 无
* 修改日期       版本号        修改人         修改内容
* ------------------------------------------------------------------
* 20200421       V1.0     feng        创建
********************************************************************/
static void GetStringContentValue(FILE *fp, uint8_t *pszSectionName, uint8_t *pszKeyName, uint8_t *pszOutput, uint32_t iOutputLen)
{
    uint8_t  szSectionName[100]    = {0};
    uint8_t  szKeyName[100]        = {0};
    uint8_t  szContentLine[256]    = {0};
    uint8_t  szContentLineBak[256] = {0};
    uint32_t iContentLineLen       = 0;
    uint32_t iPositionFlag         = 0;

    // 先对输入参数进行异常判断
    if (fp == NULL || pszSectionName == NULL || pszKeyName == NULL || pszOutput == NULL)
    {
        printf("GetStringContentValue: input parameter(s) is NULL!\n");
        return;
    }

    sprintf(szSectionName, "[%s]", pszSectionName);
    strcpy(szKeyName, pszKeyName);

    while (feof(fp) == 0)
    {
        memset(szContentLine, 0x00, sizeof(szContentLine));
        fgets(szContentLine, sizeof(szContentLine), fp);      // 获取段名

        // 判断是否是注释行(以;开头的行就是注释行)或以其他特殊字符开头的行
        if (szContentLine[0] == ';' || szContentLine[0] == '\r' || szContentLine[0] == '\n' || szContentLine[0] == '\0')
        {
            continue;
        }

        // 匹配段名
        if (strncasecmp(szSectionName, szContentLine, strlen(szSectionName)) == 0)     
        {
            while (feof(fp) == 0)
            {
                memset(szContentLine,    0x00, sizeof(szContentLine));
                memset(szContentLineBak, 0x00, sizeof(szContentLineBak));
                fgets(szContentLine, sizeof(szContentLine), fp);     // 获取字段值

                // 判断是否是注释行(以;开头的行就是注释行)
                if (szContentLine[0] == ';')
                {
                    continue;
                }

                memcpy(szContentLineBak, szContentLine, strlen(szContentLine));

                // 匹配配置项名
                if (strncasecmp(szKeyName, szContentLineBak, strlen(szKeyName)) == 0)     
                {
                    iContentLineLen = strlen(szContentLine);
                    for (iPositionFlag = strlen(szKeyName); iPositionFlag <= iContentLineLen; iPositionFlag ++)
                    {
                        if (szContentLine[iPositionFlag] == ' ')
                        {
                            continue;
                        }
                        if (szContentLine[iPositionFlag] == '=')
                        {
                            break;
                        }

                        iPositionFlag = iContentLineLen + 1;
                        break;
                    }

                    iPositionFlag = iPositionFlag + 1;    // 跳过=的位置

                    if (iPositionFlag > iContentLineLen)
                    {
                        continue;
                    }

                    memset(szContentLine, 0x00, sizeof(szContentLine));
                    strcpy(szContentLine, szContentLineBak + iPositionFlag);

                    // 去掉内容中的无关字符
                    for (iPositionFlag = 0; iPositionFlag < strlen(szContentLine); iPositionFlag ++)
                    {
                        if (szContentLine[iPositionFlag] == '\r' || szContentLine[iPositionFlag] == '\n' || szContentLine[iPositionFlag] == '\0')
                        {
                            szContentLine[iPositionFlag] = '\0';
                            break;
                        }
                    }

                    // 将配置项内容拷贝到输出缓存中
                    strncpy(pszOutput, szContentLine, iOutputLen-1);
                    break;
                }
                else if (szContentLine[0] == '[')
                {
                    break;
                }
            }
            break;
        }
    }
}


/**********************************************************************
* 功能描述： 从配置文件中获取字符串
* 输入参数： pszSectionName-段名, 如: GENERAL
            pszKeyName-配置项名, 如: EmployeeName
            pDefaultVal-默认值
            iOutputLen-输出缓存长度
            pszConfigFileName-配置文件名
* 输出参数： pszOutput-输出缓存
* 返 回 值： 无
* 其它说明： 无
* 修改日期       版本号        修改人         修改内容
* ------------------------------------------------------------------
* 20200421       V1.0     feng       创建
********************************************************************/  
static void GetConfigFileStringValue(uint8_t *pszSectionName, uint8_t *pszKeyName, uint8_t *pDefaultVal, uint8_t *pszOutput, uint32_t iOutputLen, uint8_t *pszConfigFileName)
{
    FILE  *fp   = NULL;
    // 先对输入参数进行异常判断
    if (pszSectionName == NULL || pszKeyName == NULL || pszOutput == NULL || pszConfigFileName == NULL)
    {
        printf("GetConfigFileStringValue: input parameter(s) is NULL!\n");
        return;
    }
    if (pDefaultVal == NULL)
    {
        strcpy(pszOutput, "");
    }
    else
    {
        strcpy(pszOutput, pDefaultVal);
    }

    fp = fopen(pszConfigFileName, "r");
    if (fp == NULL)
    {
        printf("GetConfigFileStringValue: open %s failed!\n", pszConfigFileName);
        return;
    }
    // 调用函数用于获取具体配置项的值
    GetStringContentValue(fp, pszSectionName, pszKeyName, pszOutput, iOutputLen);
    // 关闭文件
    fclose(fp);
    fp = NULL;
}


/**********************************************************************
* 功能描述： 从配置文件中获取整型变量
* 输入参数： pszSectionName-段名, 如: GENERAL
            pszKeyName-配置项名, 如: EmployeeName
            iDefaultVal-默认值
            pszConfigFileName-配置文件名
* 输出参数： 无
* 返 回 值： iGetValue-获取到的整数值   -1-获取失败
* 其它说明： 无
* 修改日期       版本号       修改人         修改内容
* ------------------------------------------------------------------
* 20200421       V1.0     feng      创建
********************************************************************/  
static uint32_t GetConfigFileIntValue(uint8_t *pszSectionName, uint8_t *pszKeyName, uint32_t iDefaultVal, uint8_t *pszConfigFileName)
{
    uint8_t  szGetValue[512] = {0};
    int32_t  iGetValue       = 0;

    // 先对输入参数进行异常判断
    if (pszSectionName == NULL || pszKeyName == NULL || pszConfigFileName == NULL)
    {
        printf("GetConfigFileIntValue: input parameter(s) is NULL!\n");
        return CODE_ERR;
    }

    GetConfigFileStringValue(pszSectionName, pszKeyName, NULL, szGetValue, 512-1, pszConfigFileName);    // 先将获取的值存放在字符型缓存中

    if (szGetValue[0] == '\0' || szGetValue[0] == ';')    // 如果是结束符或分号, 则使用默认值
    {
        iGetValue = iDefaultVal;
    }
    else
    {
        iGetValue = atoi(szGetValue);
    }

    return iGetValue;
}

