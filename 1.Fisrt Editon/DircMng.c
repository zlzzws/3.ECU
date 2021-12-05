/**********************************************************************
*File name    :     DircMng.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2020/1/29
*Description    :   Include Dir tar,delete,get disk size
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
#include "DircMng.h"

/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/



//extern CHAN_THREHOLD_VALUE g_channel_ThreValue_ST;
//20200328
extern DEBUG_TYPE_ENUM g_DebugType_EU;
extern SPACE_JUDGE_VALUE  g_SpaceJudge_ST;

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
*Name     :   int8_t MultiDircCreate(uint8_t *DirPath)  
*Function       :   if DirPath directory no exist creat multi directory 
*               :   if DirPath directory is exist,jump out
*Para         :   uint8_t *DirPath ./datafile/20180308
*Return       :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/3/7  Create
*
*********************************************************************/
int8_t MultiDircCreate(uint8_t *DirPath)   
{  
    uint16_t i,Len; 
    int8_t err = 0; 
    char Str[200] = {0}; 
    uint8_t  Status =0;
    char loginfo[LOG_INFO_LENG] = {0};

    Len=strlen(DirPath);       
    strncpy(Str, DirPath, Len);
    //此部分功能主要是旁段目录是否已经存在，避免创建文档报错
    //只能创建日期之前的部分，比如/yaffs/REC_CAN/20210922,此部分功能只创建/yaffs/REC_CAN/
    if(access(Str,F_OK)!= 0) //如果目录不存在
    {
        for(i = 1; i<Len; i++ )//忽略第一个'/' 
        {  
            if( Str[i] == '/' ) //找到第二个'/'
            {  
                Str[i] = '\0'; //第二个'/'替换成'\0'，代表忽略后面的字符

                if(access(Str,F_OK)!= 0) //如果目录不存在
                {   
                    umask(0);
                    Status = mkdir(Str,0777); 
                    if (Status != 0)
                    {
                        printf("Create %s failed!\n",Str);
                        perror("Create dir failed:");
                         err = CODE_ERR;      
                    }
                    else
                    {  
                        printf("Create %s done!\n",Str); 
                        err = CODE_CREAT; 
                    } 
                }
                else
                {       
                        printf("Directory %s exist!\n",Str);
                        err = CODE_EXIST;
                } 
                Str[i]='/';  //恢复第二个'/'
            }  
        } 
    }
    else
    {   
        printf("Directory %s eixst\n",Str);
        err = CODE_EXIST;       
    }
    /*创建目录的最后一级*/  
    if((Len > 0) && (access(Str,F_OK)!=0))   
    {  
        umask(0);
        mkdir(Str, 0777);
        printf("make Directory %s success\n",Str);
        err = CODE_CREAT;

    } 
    return err;  
}  
/**********************************************************************
*Name     :   int8_t DirectCreat(uint8_t *DirPath) 
*Function       :   Only creat one level directory of DirPath,
*Para         :   
*
*Return       :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/3/7  Create
*
*********************************************************************/
int8_t DirectCreat(uint8_t *DirPath)
{
    uint8_t  status =0;
    status = access(DirPath,F_OK);
    if( status != 0 )
    {
        printf("%s not exist,creat\n",DirPath);

        status = mkdir(DirPath,S_IRUSR|S_IWUSR|S_IXUSR|S_IROTH|S_IXOTH);

        if (status != 0)
        {
            printf("Create %s is failed.\n",DirPath);
            return CODE_ERR;

        }
        else
        {
            printf("Create %s done.\n",DirPath);
            return CODE_OK;
        }
    }
}
/**********************************************************************
*Name     :   unsigned long GetFileSize(char *Path)
*Function       :   Get the file size
*Para         :   char *Path
*
*Return       :   return FileSize;  
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/03/27  Create
*********************************************************************/
unsigned long GetFileSize(char *Path)  
{  
    unsigned long FileSize = 0;      
    struct stat StatBuff;  
    if(stat(Path, &StatBuff) < 0)
    {  
        FileSize = 0;  
    }
    else
    {  
        FileSize = StatBuff.st_size;  
    }  
    return FileSize;  
}  
/**********************************************************************
*Name     :   int8_t GetMemSize(uint8_t *Path,uint32_t TotalSize_MB,uint32_t FreeSize_MB)
*Function       :   Get the memory size of Path directory
*Para         :   uint8_t *Path,
*         uint32_t TotalSize_MB,
*         uint32_t FreeSize_MB
*Return       :   int8_t 1,success;0 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/3/5  Create
*REV1.0.1     feng    2020/5/11  can return the TotalSize_MB

*********************************************************************/
int8_t GetMemSize(uint8_t *Path,uint32_t *TotalSize_MB,uint32_t *FreeSize_MB)
{
    struct statfs DiskInfo;
    char loginfo[LOG_INFO_LENG] = {0}; 
    unsigned long long TotalBlocks = 0;
    unsigned long long TotalSize = 0;
    unsigned long long FreeDisk = 0;
    if(statfs(Path, &DiskInfo) == -1) 
    {
        return CODE_ERR;
    }
    TotalBlocks = DiskInfo.f_bsize; /*the blocks size,unit is Byte*/ 
    TotalSize = DiskInfo.f_blocks * TotalBlocks;/*the number of blocks*/  
    *TotalSize_MB = TotalSize >> 20;  /*the unit is MByte*/
    FreeDisk = DiskInfo.f_bfree * TotalBlocks;  /*the number of free blocks*/  
    *FreeSize_MB = FreeDisk >> 20;  /*the unit is MByte*/     
    snprintf(loginfo, sizeof(loginfo)-1, "%s total=%u MB, free=%u MB",Path,*TotalSize_MB, *FreeSize_MB);
    WRITELOGFILE(LOG_INFO_1,loginfo);
    return CODE_OK;

}

/**********************************************************************
*Name     :   int8_t RemoveDirWithFile(char *DirPath)
*Function       :   recursion delete  directory and file in the  *Path 
*Para         :   uint8_t *Path,
*         uint32_t TotalSize_MB,
*         uint32_t FreeSize_MB
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/3/5  Create
*********************************************************************/
int8_t RemoveDirWithFile(char *DirPath)
{
    char CurDir[] = ".";
    char UpDir[] = "..";
    int8_t err = 0;
    char DirName[128];
    DIR *dirp;
    DIR  *DirOpenInfo_p = NULL;
    struct dirent *DirInfo_p;
    struct stat DirStat;

    /* if DirPath no exist ,return*/
    if( 0 != access(DirPath, F_OK) ) 
    {
        perror("access stat error");
        return CODE_ERR;
    }

    /* get the infomation of DirPath errror，return */
    if( 0 > stat(DirPath, &DirStat) ) 
    {
        perror("get directory stat error");
        return CODE_ERR;
    }
     /*is the regular file,delete*/
    if( S_ISREG(DirStat.st_mode) ) 
    {  
       err = remove(DirPath);
       if(-1 == err)
       {
            printf("delete file %s failed\n", DirPath);
       }
        else
       {
            printf("delete file %s success \n", DirPath);
       }
    } 
    /*is the directory,recursion delete*/
    else if (S_ISDIR(DirStat.st_mode) ) 
    {   
        DirOpenInfo_p = opendir(DirPath);
        while ( (DirInfo_p = readdir(DirOpenInfo_p)) != NULL ) 
        {
        // skip "."" (current directory) and  ".. "(up directory)
        if ( (0 == strcmp(CurDir, DirInfo_p->d_name)) || (0 == strcmp(UpDir, DirInfo_p->d_name)) )
        {
            continue;
        }
            memset(DirName,0,sizeof(DirName));
            sprintf(DirName, "%s/%s", DirPath, DirInfo_p->d_name);            
            err = RemoveDirWithFile(DirName);   /*recursion delete*/
        }
        closedir(DirOpenInfo_p);
        rmdir(DirPath);     /*delele empty Path*/
    } 
    else 
    {
        perror("unknow file type!"); 
    }
    return err;
}

/**********************************************************************
*Name     :   int8_t FileDirJudge(uint8_t *FilePath_p)
*Function       :   Delete the earliest directory  in FilePath_p
*Para         :   uint8_t *FilePath_p,  ./datafile
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/3/9  Create
*
*********************************************************************/
int8_t FileDirJudge(uint8_t *FilePath_p)
{

    struct stat     FileStat;
    struct dirent   *DirInfo_p; 
    DIR  *DirOpenInfo_p = NULL; 
    uint8_t  FileFullName[100] = {0};
    uint8_t  EarlDirc[100] = {0};
    uint16_t  DirCompNum = 0;
	uint16_t FileCompNum = 0;
    int8_t err = 0;
	uint8_t type = 0;

    char loginfo[LOG_INFO_LENG] = {0};

    if((DirOpenInfo_p = opendir(FilePath_p)) == NULL)
    {

        printf("cannot open dir %s\n",FilePath_p);
        snprintf(loginfo, sizeof(loginfo)-1, "cannot open dir %s\n",FilePath_p);
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
   // DirOpenInfo_p = opendir(FilePath_p);/* /datfile */
    /*chdir(FilePath_p);*/ //chage the directory to FilePath,so FileFullName can not have FilePath_p
    while((DirInfo_p = readdir(DirOpenInfo_p))!=NULL) 
    { 

        /*Skip the current directory and up directory*/
        memset(FileFullName,0,sizeof(FileFullName));
        sprintf(FileFullName,"%s%s",FilePath_p,DirInfo_p->d_name);/* /datfile/20180308 */
        memset(&FileStat,0,sizeof(struct stat));
        stat(FileFullName,&FileStat);/*get file infomation*/

        /* in the directory:datafile have /datafile/. and /datafile/..*/
        if(S_ISREG(FileStat.st_mode))
        {
            if(strcmp(DirInfo_p->d_name,".")==0  || strcmp(DirInfo_p->d_name,"..")==0)
             { 
                continue;
                       
            }
            FileCompNum ++; 
        }
        else if(S_ISDIR(FileStat.st_mode))
        {
            if(strcmp(DirInfo_p->d_name,".")==0  || strcmp(DirInfo_p->d_name,"..")==0)
            { 
                continue;
                       
            }
             DirCompNum ++; 
        }  
    }
    closedir(DirOpenInfo_p);
    /*if the datafile directory has only directory, no delete
     first delete tar file*/
    if(FileCompNum > 0) 
	{
		type = REC_FILE_TYPE;
	}		
	else if(DirCompNum > 1)
    {    
		type = REC_DIR_TYPE;

    }
    return type;
}
/**********************************************************************
*Name     :   int8_t DeleteEarliestDir(uint8_t *FilePath_p)
*Function       :   Delete the earliest directory  in FilePath_p
*Para         :   uint8_t *FilePath_p,  ./datafile
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/3/9  Create
*
*********************************************************************/
int8_t DeleteEarliestDir(uint8_t *FilePath_p)
{

    struct stat     FileStat;
    struct dirent   *DirInfo_p; 
    DIR  *DirOpenInfo_p = NULL; 
    uint8_t  FileFullName[100] = {0};
    uint8_t  EarlDirc[100] = {0};
    uint16_t  DirCompNum = 0;
    int8_t err = 0;

    char loginfo[LOG_INFO_LENG] = {0};

    if((DirOpenInfo_p = opendir(FilePath_p)) == NULL)
    {

        printf("cannot open dir %s\n",FilePath_p);
        snprintf(loginfo, sizeof(loginfo)-1, "cannot open dir %s\n",FilePath_p);
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
   // DirOpenInfo_p = opendir(FilePath_p);/* /datfile */
    /*chdir(FilePath_p);*/ //chage the directory to FilePath,so FileFullName can not have FilePath_p
    while((DirInfo_p = readdir(DirOpenInfo_p))!=NULL) 
    { 

        /*Skip the current directory and up directory*/
        memset(FileFullName,0,sizeof(FileFullName));
        sprintf(FileFullName,"%s%s",FilePath_p,DirInfo_p->d_name);/* /datfile/20180308 */
        memset(&FileStat,0,sizeof(struct stat));
        stat(FileFullName,&FileStat);/*get file infomation*/

        /* in the directory:datafile have /datafile/. and /datafile/..*/
        if(S_ISDIR(FileStat.st_mode))
        {
            if(strcmp(DirInfo_p->d_name,".")==0  || strcmp(DirInfo_p->d_name,"..")==0)
            { 
                continue;
                       
            }
            /*cpy the first DirInfo_p->d_name to EarlDirc*/
            if(DirCompNum == 0) 
            {
                memset(EarlDirc,0,sizeof(EarlDirc));
                memcpy(EarlDirc,FileFullName,strlen(FileFullName));
                //printf("FileFullName %s\n",FileFullName);
                
            }   
            /*FileFullName is earlier than EarlDirc,EarlDirc first is 0*/
            if(strncmp(FileFullName,EarlDirc,strlen(FileFullName)) < 0)
            {
                memcpy(EarlDirc,FileFullName,strlen(FileFullName)); 
                //printf("FileFullName %s\n",FileFullName);
                //printf("DirCompNum %d\n",DirCompNum);
            }
                DirCompNum ++; 
        }  
    }
    closedir(DirOpenInfo_p);
    /*if the datafile directory has only directory, no delete*/
    if(DirCompNum > 1)
    {
        err = RemoveDirWithFile(EarlDirc);
        if(-1 == err)
        {
            printf("Delete Dir:%s failed\n",EarlDirc);
            snprintf(loginfo, sizeof(loginfo)-1,"Delete Dir:%s failed",EarlDirc);
            WRITELOGFILE(LOG_ERROR_1,loginfo); 
        }
        else
        {
            printf("Delete Dir:%s\n",EarlDirc);
            snprintf(loginfo, sizeof(loginfo)-1,"Delete Dir %s",EarlDirc);
            WRITELOGFILE(LOG_INFO_1,loginfo); 
        }
       

    }
    return err;
}

/**********************************************************************
*Name     :   int8_t DeleteEarliestFile(uint8_t *FilePath_p)
*Function       :   Delete the earliest File  in FilePath_p
*Para         :   uint8_t *FilePath_p,  ./datafile
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/9  Create
*
*********************************************************************/
int8_t DeleteEarliestFile(uint8_t *FilePath_p,uint8_t FileType)
{

    struct stat     FileStat;
    struct dirent   *DirInfo_p; 
    DIR  *DirOpenInfo_p = NULL; 
    uint8_t  FileFullName[100] = {0};
    uint8_t  EarlFile[100] = {0};
    uint16_t  DirCompNum = 0;
    int8_t err = 1;

    char loginfo[LOG_INFO_LENG] = {0};

    if((DirOpenInfo_p = opendir(FilePath_p)) == NULL)
    {

        printf("cannot open dir %s\n",FilePath_p);
        snprintf(loginfo, sizeof(loginfo)-1, "cannot open dir %s\n",FilePath_p);
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    DirOpenInfo_p = opendir(FilePath_p);/* /datfile */
    /*chdir(FilePath_p);*/ //chage the directory to FilePath,so FileFullName can not have FilePath_p
    while((DirInfo_p = readdir(DirOpenInfo_p))!=NULL) 
    { 

        /*Skip the current directory and up directory*/
        memset(FileFullName,0,sizeof(FileFullName));
        sprintf(FileFullName,"%s%s",FilePath_p,DirInfo_p->d_name);/* /datfile/20180308 */
        memset(&FileStat,0,sizeof(struct stat));
        stat(FileFullName,&FileStat);/*get file infomation*/

        /* in the directory:datafile have /datafile/. and /datafile/..*/
        if(S_ISREG(FileStat.st_mode))
        {
            if(strcmp(DirInfo_p->d_name,".")==0  || strcmp(DirInfo_p->d_name,"..")==0)
            { 
                continue;
                       
            }
            /*cpy the first DirInfo_p->d_name to EarlFile*/
            if(DirCompNum == 0) 
            {
                memset(EarlFile,0,sizeof(EarlFile));
                memcpy(EarlFile,FileFullName,strlen(FileFullName));
                //printf("FileFullName %s\n",FileFullName);
                
            }   
            /*FileFullName is earlier than EarlFile,EarlFile first is 0*/
            if(strncmp(FileFullName,EarlFile,strlen(FileFullName)) < 0)
            {
                memset(EarlFile,0,sizeof(EarlFile));
                memcpy(EarlFile,FileFullName,strlen(FileFullName)); 
                //printf("FileFullName %s\n",FileFullName);      
            }
            DirCompNum ++; 
            //printf("DirCompNum %d\n",DirCompNum);
            /*just for logfile delete*/                    
        }  
    }
    if(DirCompNum > 0)/*for just have 1 file*/
    {
        if(LOG_FILE_TYPE  == FileType)
        {
            if(DirCompNum > g_SpaceJudge_ST.LOGFILE_NUM)/*test is 5*/
            {
                err = remove(EarlFile);
            }
        }
        else
        {
            err = remove(EarlFile);
           
        }
        if(-1 == err)
        {
            printf("Delete File:%s failed\n",EarlFile);
            memset(loginfo,0,sizeof(loginfo));
            snprintf(loginfo, sizeof(loginfo)-1,"Delete File:%s failed",EarlFile);
            WRITELOGFILE(LOG_ERROR_1,loginfo);
        }
        else if(0 == err)
        {
            printf("Delete Early File:%s\n",EarlFile);
            memset(loginfo,0,sizeof(loginfo));
            snprintf(loginfo, sizeof(loginfo)-1,"Delete Early File:%s",EarlFile);
            WRITELOGFILE(LOG_INFO_1,loginfo);
        }
    } 
    closedir(DirOpenInfo_p);
    return err;
}

/**********************************************************************
*Name     :   int8_t TarDir(uint8_t *FilePath_p,uint8_t * NowFilePah)
*Function       :   tar the exist dir
*Para         :   uint8_t * NowFilePah :not tar the now day dir
*
*Return       :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/3/9  Create
*
*********************************************************************/
int8_t TarDir(uint8_t *FilePath_p,uint8_t * NowFilePah)
{

    struct stat     FileStat;
    struct dirent   *DirInfo_p; 
    DIR  *DirOpenInfo_p = NULL; 
    uint8_t  FileFullName[100] = {0};
    uint8_t  SystemCmd[200] = {0};
    uint8_t  DirCompNum = 0;
    int8_t err = 0;
	int16_t sysstatus = 1;
    uint8_t Len = 0;

    char loginfo[LOG_INFO_LENG] = {0};
    Len=strlen(NowFilePah);     

    // sprintf(SystemCmd,"cd %s",FilePath_p);/* /yaffs/REC_EVTDATA */
    // printf("%s\n", SystemCmd);
    // system(SystemCmd);

    if((DirOpenInfo_p = opendir(FilePath_p)) == NULL)
    {

        printf("cannot open dir %s\n",FilePath_p);
        snprintf(loginfo, sizeof(loginfo)-1, "cannot open dir %s\n",FilePath_p);
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
   // DirOpenInfo_p = opendir(FilePath_p);/* /datfile */

    /*chdir(FilePath_p);*/ //chage the directory to FilePath,so FileFullName can not have FilePath_p
    while((DirInfo_p = readdir(DirOpenInfo_p))!=NULL) 
    { 

        /*Skip the current directory and up directory*/
        memset(FileFullName,0,sizeof(FileFullName));
       // sprintf(FileFullName,"%s",FilePath_p,DirInfo_p->d_name);/*  /yaffs/REC_EVTDATA */
        sprintf(FileFullName,"%s%s",FilePath_p,DirInfo_p->d_name);/*  /yaffs/REC_EVTDATA */
        memset(&FileStat,0,sizeof(struct stat));
        stat(FileFullName,&FileStat);/*get file infomation*/

        /* in the directory:datafile have /datafile/. and /datafile/..*/
        if(S_ISDIR(FileStat.st_mode))
        {
            if((0 == strcmp(DirInfo_p->d_name,".")) || (0 == strcmp(DirInfo_p->d_name,"..")) ||  (0 == strncmp(FileFullName, NowFilePah, Len))) /*the dir is today,so not tar */
            { 
                continue;
                       
            }
            //sprintf(SystemCmd,"tar zcvf %s.tar.gz %s",DirInfo_p->d_name,DirInfo_p->d_name);/* / //20180308 */
            sprintf(SystemCmd,"tar zcvf %s.tar.gz -C %s %s",FileFullName,FilePath_p,DirInfo_p->d_name);/* / //20180308 */
            printf("%s\n", SystemCmd);
			snprintf(loginfo, sizeof(loginfo)-1,"tar dircmd  %s",SystemCmd);
            WRITELOGFILE(LOG_INFO_1,loginfo);
            sysstatus = system(SystemCmd);
			memset(loginfo,0,LOG_INFO_LENG);
            snprintf(loginfo, sizeof(loginfo)-1,"system status %d",sysstatus);
            WRITELOGFILE(LOG_INFO_1,loginfo);
			printf("system status %d\n",sysstatus);
			if(sysstatus == 0)
			{
				err = RemoveDirWithFile(FileFullName);
	            if(-1 == err)
	            {
	                printf("Delete tar Dir:%s failed\n",FileFullName);
	                snprintf(loginfo, sizeof(loginfo)-1,"Delete tar Dir:%s failed",FileFullName);
	                WRITELOGFILE(LOG_ERROR_1,loginfo); 
	            }
	            else
	            {
	                printf("Delete tar Dir:%s success\n",FileFullName);
	                snprintf(loginfo, sizeof(loginfo)-1,"Delete tar Dir %s success",FileFullName);
	                WRITELOGFILE(LOG_INFO_1,loginfo); 
	            }
			}
			else
			{
				 printf("Donot Delete tar Dir:%s for sysytem failed\n",FileFullName);
	             snprintf(loginfo, sizeof(loginfo)-1,"Donot  Delete tar Dir:%sfor sysytem  failed",FileFullName);
	             WRITELOGFILE(LOG_ERROR_1,loginfo); 
			}
           
        }  
    }
    closedir(DirOpenInfo_p);
   // system("cd /");

    return CODE_OK;
}
