/**********************************************************************
*File name      :   ModifConfig.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/02/25
*Description    :   Read and modify content value of config.ini file
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/02/25  Create
*
*
*
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
#include "ModifConfig.h"
 
/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/
extern RECORD_XML g_Rec_XML_ST;
extern uint16_t g_Ver_SysPrmFile_U16;
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
*Name           :   int8_t GetChanCalibData(CHAN_CALIB_DATA *ChanCalibDat_ST_p)
*Function       :   Read the Chan Calibrate Data from /tffs0/ChanCalibValue.dat"
*Para           :   CHAN_CALIB_DATA *ChanCalibDat_ST_p: the Struct of ChanCalib
*Return         :   0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/7/27  Create
*
*********************************************************************/
int8_t GetChanCalibData(uint8_t *Path,CHAN_CALIB_DATA *ChanCalibDat_ST_p)
{
    FILE *fd_CalibName;
    fd_CalibName = fopen(Path, "r+"); //every time creat the file and write replace
    char loginfo[LOG_INFO_LENG] = {0};
    if(fd_CalibName == NULL)
    {
        printf("open %s failed\n",Path);
        snprintf(loginfo, sizeof(loginfo)-1, "open %s file failed  %m",Path);
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;//must add reture to avoid run the next fread() to cause error

    }
    fread(ChanCalibDat_ST_p,2,VOL_CHAN_NUM,fd_CalibName);
    fclose(fd_CalibName);
    return CODE_OK;

}
/**********************************************************************
*Name           :   int8_t GetChanOperateNum(uint32_t *Channel_OperNum_U32)

*Function       :   Read the Chan Operatre Num from "/yaffs/ChanOperaNum.dat"
*Para           :   
*Return         :   uint8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/7/27  Create
*REV1.0.0     feng    2018/8/17  change for 6che
*
*********************************************************************/
int8_t GetChanOperateNum(uint32_t Channel_OperNum_U32[])
{
    FILE *fd_OperNumName;
    int8_t error =0;
    uint32_t freadNum = 0;
    uint32_t OpeNumTem[50] = {0};
    uint8_t i =0;
    char loginfo[LOG_INFO_LENG] = {0};
    fd_OperNumName = fopen(CHAN_OPER_NUM_FILE, "r+"); //the fisrt time have no file ,so return 
    if(fd_OperNumName == NULL)
    {
        perror("open ChanOperaNum.dat file failed");
        snprintf(loginfo, sizeof(loginfo)-1, "open ChanOperaNum.dat %m");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }
    error = fseek(fd_OperNumName,130,SEEK_SET);//skip drive top
    if(error  == -1)
    {
        perror("Fseek ChanOperaNum.dat  error");

    }
    freadNum = fread(OpeNumTem,OPRTNUM_LENGTH,g_Rec_XML_ST.Rec_Fast_ST.RecChnNum_U8,fd_OperNumName);

    //the 0-39  V channel
    for(i = 0;i < VOL_CHAN_NUM;i++)
    {
        Channel_OperNum_U32[i] = OpeNumTem[i];
    
    }
    fclose(fd_OperNumName);
    return CODE_OK;

}
/**********************************************************************
*Name           :   int8_t GetPTUChanThresholdValue(uint8_t *ConfPath_p,CHAN_THREHOLD_VALUE *OutputValue_p)
*Function       :   Read the 75 Chan ThresholdValue from  "/tffs0/UsrSYSPrmTable.dat"   
*                   every channel has it own ThreholdValue.The ChaneConfig.ini file has 75 channel data         
*Para           :   uint8_t *ConfPath:  the file path
*                   CHAN_THREHOLD_VALUE *OutputValue_p:  save the operate Number 
*Return         :   0 success,-1,false;
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/07/27  Create
*
*********************************************************************/
int8_t GetPTUChanThresholdValue(uint8_t *ConfPath_p,CHAN_THREHOLD_VALUE *OutputValue_p,VERSION *Version_p)
{
    uint8_t  ChanValue[PARMTABLE_LENGTH]  = {0};
    uint16_t ValueTemp = 0;
    uint8_t j = 0,i = 0; 
    char loginfo[LOG_INFO_LENG] = {0};
    FILE *fp = fopen(ConfPath_p, "r+");
    if(fp == NULL)
    {
        perror("open UsrSYSPrmTable.dat file failed");
        snprintf(loginfo, sizeof(loginfo)-1, "Open UsrSYSPrmTable.dat  %m");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
	    OutputValue_p -> LowVol_H =  330;
	    OutputValue_p -> HighVol_L =  450;
	    OutputValue_p -> HighVol_H =  1375;
        OutputValue_p -> SlugVol =  50;
	    for(j = 0; j < CURR_CHAN_NUM;j++)
	    {
	        OutputValue_p-> CurrChan_ThreValue_ST[j].HighCurr = 3000;
	 
	    }
        return CODE_ERR;
    }
    fread(ChanValue, 1,PARMTABLE_LENGTH,fp);
   

    
    ValueTemp =  (ChanValue[4] << 8) + ChanValue[5];
    Version_p -> SysPrmFile_Ver_U16 = ValueTemp;

    //g_Ver_SysPrmFile_U16 = ValueTemp;
    ValueTemp =  (ChanValue[8] << 8) + ChanValue[9];
    OutputValue_p -> LowVol_H =  ValueTemp;
    ValueTemp =  (ChanValue[10] << 8) + ChanValue[11];
    OutputValue_p -> HighVol_L =  ValueTemp;
    ValueTemp =  (ChanValue[12] << 8) + ChanValue[13];
    OutputValue_p -> HighVol_H =  ValueTemp;
    ValueTemp =  (ChanValue[14] << 8) + ChanValue[15];
    OutputValue_p -> SlugVol =  ValueTemp;
	
    for(j = 0; j < CURR_CHAN_NUM;j++)
    {
        i = j * 2 + 16;
        ValueTemp = (ChanValue[i] << 8) + ChanValue[i+1];
        OutputValue_p-> CurrChan_ThreValue_ST[j].HighCurr =  ValueTemp;
 
    }

	//need change the sysCfgFile
    ValueTemp = (ChanValue[44] << 8) + ChanValue[101];
    OutputValue_p -> SlugCurr =  ValueTemp;
    fclose(fp); 
    return CODE_OK;

}
/**********************************************************************
*Name           :   int8_t GetPTUChanThresholdValue(uint8_t *ConfPath_p,CHAN_THREHOLD_VALUE *OutputValue_p)
*Function       :   Read the 75 Chan ThresholdValue from  "/tffs0/UsrSYSPrmTable.dat"   
*                   every channel has it own ThreholdValue.The ChaneConfig.ini file has 75 channel data         
*Para           :   uint8_t *ConfPath:  the file path
*                   CHAN_THREHOLD_VALUE *OutputValue_p:  save the operate Number 
*Return         :   0 success,-1,false;
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/07/27  Create
*
*********************************************************************/


int8_t ChanDataPrint(CHAN_DATA *ChanData_ST_p,uint8_t EADSType)//uint16_t length
{
        
    uint8_t j= 0;
    for(j = 0;j < VOL_CHAN_NUM;j++)
    { 
 
       printf("VChan %u :%d\n",j,ChanData_ST_p -> VolChan_ST[j].CH_Voltage_I16); 
    
    }
    if(EADSType == CTU_BOARD_ID)
    {
        for(j = 0;j < CURR_CHAN_NUM;j++)
        { 
 
            printf("CurrChan %u :%d\n",j,ChanData_ST_p -> CurrChan_ST[j].CH_Current_I16);
    
        }
        
    }
    return CODE_OK;
}