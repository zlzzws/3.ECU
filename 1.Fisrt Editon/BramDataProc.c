/**********************************************************************
*File name      :   BramDataProc.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/5/16
*Description    :   Include the Write data to 570,Read data from 570,
*                   Write data to MAX10,Read data from MAX10
*                   Read 570 RTC information and set system time

*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/05/16  Create*
*********************************************************************/

/***********************************************************************
*Debug switch  section*
*********************************************************************/


/***********************************************************************
*Include file  section*
*********************************************************************/
#include "BramDataProc.h"
/***********************************************************************
*Global Variable Declare Section*
*********************************************************************/
extern DEBUG_TYPE_ENUM g_DebugType_EU;
extern BLVDS_BRD_DATA g_BrdRdBufData_ST;
extern BLVDS_BRD_DATA g_BrdBufData_ST;
extern uint32_t g_LinuxDebug;
extern uint32_t g_CAN_RDDate[64];
extern uint16_t g_MVB_SendFrameNum; //TODO Just for test please verify
/***********************************************************************
*Local Macro Define Section*
*********************************************************************/

/***********************************************************************
*Local Struct Define Section*
*********************************************************************/

/***********************************************************************
*Local Prototype Declare Section*
*********************************************************************/
//20200325
extern EADS_ERROR_INFO  g_EADSErrInfo_ST;

/***********************************************************************
*Static Variable Define Section*
*********************************************************************/
static uint8_t s_bram_WRRDErrNum_U32 = 0;
static BRAM_ADDRS s_Bram_A_BLVDSBlckAddr_ST = {0};
static BRAM_ADDRS s_bram_RD_B_BLVDSBlckAddr_ST = {0};
static BRAM_ADDRS s_bram_WR_B_BLVDSBlckAddr_ST = {0};
static BRAM_ADDRS s_bram_RD_TMS_SPC_Blck_ST = {0};
static BRAM_ADDRS s_bram_WR_TMS_SPC_Blck_ST = {0};
static int8_t s_bram_MemFd_I8 = 0;
static uint8_t *s_bram_RDFlagAddr = NULL;
static uint8_t *s_bram_WRFlagAddr = NULL;
static uint8_t *s_bram_RD_A_BLVDSBlckAddr = NULL;
static uint8_t *s_bram_RD_TMS_SPCBlckAddr = NULL;
static uint8_t *s_bram_WR_TMS_SPCBlckAddr = NULL;
static uint8_t *s_bram_RD_B_BLVDSBlckAddr = NULL;
static uint8_t *s_bram_WR_B_BLVDSBlckAddr = NULL;
static BRAM_CMD_PACKET CmdPact_RD_ST[5] = {0};
static BRAM_CMD_PACKET CmdPact_WR_ST[5] = {0};
static BRAM_CMD_PACKET MVB_CmdPact_RD_ST[32] = {0};
static BRAM_CMD_PACKET MVB_CmdPact_WR_ST[32] = {0}; //TODO just for test！please verify to 6！
/**********************************************************************
*Name           :   BRAM_RETN_ENUM BramReadDataExtraWiOutLife(uint32_t *Inbuff,uint32_t *Outbuff)
*Function       :   Extract the data of ReadData,without the CurrePackNum judge 提取数据
*Para           :   uint32_t *Inbuff 
*                   uint32_t *Outbuff
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/3/29  Create
*REV1.0.1     feng    2020/8/26 note the diffrent board
*REV1.0.2     feng    2021/2/26  when life lost,also extact data
*********************************************************************/
int8_t ExtraBoardData(uint32_t *Inbuff,uint32_t *Outbuff,uint8_t ChanNum)
{
    uint16_t PacketLength;
    int8_t ErrorCode = 0;
    static uint32_t s_BramLife_U32[BRAM_BOARD_NUM] = {0};
    BRAM_PACKET_DATA *BramPacketData_ST_p;
    BramPacketData_ST_p = (BRAM_PACKET_DATA *)Inbuff;
    PacketLength = (BramPacketData_ST_p -> BLVDSTOP_U32 >> 24) & 0xFF;
	if(PacketLength != 0)
    {
        memcpy(Outbuff,&BramPacketData_ST_p -> BLVDSData_U32,PacketLength);           
    }	
    if(BramPacketData_ST_p -> BLVDSReser_U32[0] == s_BramLife_U32[ChanNum])
    {
        ErrorCode = CODE_ERR; 
    }
    else 
    {            
        ErrorCode = CODE_OK;
    }       
    s_BramLife_U32[ChanNum] = BramPacketData_ST_p -> BLVDSReser_U32[0];
    /*when power on,the read length is 0,should be wait bram fresh by FPGA*/
    return ErrorCode;
}
/**********************************************************************
*Name           :   BoardDataRead(BRAM_ADDRS *BramAddrs_p,uint32_t *ReadData)
*Function       :   Read the Bram data from the specified addr
*Para           :   BRAM_ADDRS *BramAddrs_p
*                   uint32_t *ReadData
*Return         :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author         :   feng
*History:
*REV1.0.0     feng    2018/05/07  Create
*********************************************************************/
int8_t BoardDataRead(BRAM_ADDRS *BramAddrs_p,uint32_t *ReadData)
{
    uint32_t ReadDataBuf[64] = {0};    
    int8_t Error = 0;
    char loginfo[LOG_INFO_LENG] = {0};    
    
    Error = BramReadWithChek(BramAddrs_p,ReadDataBuf);
    if(Error == CODE_ERR)
    {
        printf("BramReadWithChek error\n");
        snprintf(loginfo, sizeof(loginfo)-1,"BramReadWithChek error");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
    }
    Error = ExtraBoardData(ReadDataBuf,ReadData,BramAddrs_p->ChanNum_U8);   
    return Error;
}

/**********************************************************************
*Name           :   BRAM_RETN_ENUM BramReadDataExtraCMD(uint32_t *Inbuff,uint16_t CurrPackNum)
*Function       :   Extract the CMD data of ReadData,If the CurrPackNum is equal to the CurrPackNumTemp
*                   meaning 570 has receiver success,otherwise is failure.
*Para           :   uint32_t *Inbuff 
*                   uint16_t TotalPackNum   The  total write packet number
*                   uint16_t CurrPackNum    The current write packet number
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/3/29  Create
*********************************************************************/
BRAM_RETN_ENUM BramReadDataExtraCMD(uint32_t *Inbuff,uint16_t CurrPackNum)
{
    
    uint16_t TotalPackNumTemp = 0,CurrPackNumTemp = 0;
    uint8_t UpDateTypeTemp = 0,UpDateStatTemp = 0,UpDateStat = 0;
    BRAM_RETN_ENUM ErrorCode  = 0;
    BRAM_PACKET_DATA *BramPacketData_ST_p;
    BramPacketData_ST_p = (BRAM_PACKET_DATA *)Inbuff;

    CurrPackNumTemp = (BramPacketData_ST_p -> BLVDSReser_U32[1] & 0xFFFF);
    UpDateStatTemp =  (BramPacketData_ST_p -> BLVDSReser_U32[1] >> 16) & 0xFF;
    //UpDateTypeTemp = (BramPacketData_ST_p -> BLVDSReser_U32[1] >> 24) & 0xFF;

    UpDateStat =  (CMD_STATUS_ENUM)UpDateStatTemp;

    /*check program update whether right*/
    if(CurrPackNumTemp == CurrPackNum)
    {
        if(CMD_OK == UpDateStat)
        {              
            ErrorCode = RETURN_OK;
            s_bram_WRRDErrNum_U32 = 0;
        }
        /*CRC error ,repeat write*/
        else if(CMD_CRC_ERROR == UpDateStat)
        {
            ErrorCode = RETURN_CRC_ERROR;
            s_bram_WRRDErrNum_U32 ++;
        }
        /*another error ,repeat write*/
        else if(CMD_ERROR == UpDateStat) 
        {
            ErrorCode = RETURN_ERROR;
            s_bram_WRRDErrNum_U32 ++;
        }
        else if (CMD_BUSY == UpDateStat)/*for max 10*/
        {
            ErrorCode = RETURN_BUSY;
            s_bram_WRRDErrNum_U32 = 0; 
        }
        /*for reset the max10 program*/

    }
    else
    {
        ErrorCode = RETURN_ELSE_ERROR;
        s_bram_WRRDErrNum_U32 ++;
    }

    return ErrorCode;
}
/**********************************************************************
*Name           :   int8_t BramWriteAssigVal(uint16_t Length,uint32_t *Inbuf,
*                   uint8_t ChanNum,uint16_t CurrPackNum,uint8_t CurrCMD,COMMU_MCU_ENUM TargeMCU)
*                   
*Function       :   AssigVal to Bram data
*Para           :   uint16_t Length: data length no include frame top
*               :   uint32_t *Inbuf: the data 
*               :   uint8_t ChanNum: Write which Chan 
*                   uint16_t CurrPackNum:
*                   uint8_t CurrCMD :
*                   COMMU_MCU_ENUM TargeMCU :MAX and TMS570
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/05/07  Create
*********************************************************************/
int8_t BramWriteAssigVal(BRAM_CMD_PACKET *CmdPact_p,uint32_t *Outbuf,uint32_t *Inbuf)
{

    uint16_t i = 0;
    uint32_t TempCrcValue_U32;
    uint32_t Framelen = 0;
    uint32_t Temp32Value = 0;
    //uint8_t TempBlockID_U8 = 0;
    BRAM_PACKET_DATA *BramPacketData_ST_p;
    BramPacketData_ST_p = (BRAM_PACKET_DATA *)Outbuf;
    
    /*top */
    if(CmdPact_p -> TargeMCU == TMS570_MCU)
    {
        Temp32Value = 0x11C2;
    }
    else if(CmdPact_p -> TargeMCU == MAX10_MCU)
    {
        Temp32Value = 0x1144;
    }
    else
    {
        Temp32Value = 0x0;
    }
    /*ChanNum is occupy 6 bit */
    Temp32Value += ((CmdPact_p -> ChanNum_U8) << 18);
    Framelen = ((CmdPact_p -> PacktLength_U32) + BRAM_PCKT_TOP_LNGTH_U32) * 4;
    Temp32Value += (Framelen << 24);
    BramPacketData_ST_p -> BLVDSTOP_U32 = Temp32Value;

    BramPacketData_ST_p -> BLVDSReser_U32[0] = 0;
    Temp32Value =  CmdPact_p -> BlockNum_U16;
    Temp32Value += ((CmdPact_p -> PacktCMD_U8) << 24);
    BramPacketData_ST_p -> BLVDSReser_U32[1] = Temp32Value;
    /*data, memcp uinit is byte,so the lenth is Length*4*/
    memcpy(&BramPacketData_ST_p -> BLVDSData_U32,Inbuf,(CmdPact_p -> PacktLength_U32) * 4); 
    TempCrcValue_U32 = Crc32CalU32Bit(Outbuf,(CmdPact_p -> PacktLength_U32) + BRAM_PCKT_TOP_LNGTH_U32);
    /*small end*/
    /*crc*/
    BramPacketData_ST_p -> BLVDSData_U32[CmdPact_p -> PacktLength_U32]= TempCrcValue_U32;
    //printf("Write CrcValue_U32 %x\n",TempCrcValue_U32); 
    return CODE_OK;
}

/**
 * @description: Bram内存映射
 * @param {EADS_ERROR_INFO *EADSErrInfop 错误信息指针}
 * @return {*err}
 * @author: feng
 */

int8_t BramOpenMap(EADS_ERROR_INFO *EADSErrInfop)
{
    int8_t err = 0;
    uint8_t ErrNum = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    s_bram_MemFd_I8 = BramOpen();
    if(-1 == s_bram_MemFd_I8)
    {
        ErrNum++;
        while((ErrNum < MAPERR_NUM) && (CODE_ERR == s_bram_MemFd_I8)) /*try 3 times*/
        {
            s_bram_MemFd_I8 = BramOpen();
            ErrNum++;
        }
        if(ErrNum >= MAPERR_NUM)
        {
            EADSErrInfop -> EADSErr = 1;
            snprintf(loginfo, sizeof(loginfo)-1, "BramOpen failed %m");
            WRITELOGFILE(LOG_ERROR_1,loginfo);        
            err = CODE_ERR;
            return err;
        }
        else
        {
            err = CODE_OK;
        }

    }
    //内存映射：本系BLVDS通道（4K大小）
    s_bram_RD_A_BLVDSBlckAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_RD_A_BLVDS_BLOCK0,BRAM_4BLCK_LNGTH_U8);
    ErrNum = 0;
    if(MAP_FAILED == s_bram_RD_A_BLVDSBlckAddr)
    {
        ErrNum++;
        while((ErrNum < MAPERR_NUM) && (MAP_FAILED == s_bram_RD_A_BLVDSBlckAddr)) /*try 3 times*/
        {
            s_bram_RD_A_BLVDSBlckAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_RD_A_BLVDS_BLOCK0,BRAM_4BLCK_LNGTH_U8);
            ErrNum++;
        }
        if(ErrNum >= MAPERR_NUM)
        {
            EADSErrInfop -> EADSErr = 1;
            snprintf(loginfo, sizeof(loginfo)-1, "BramMap BRAM_A9_RD_A_BLVDS_BLOCK0 failed %m");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            err = CODE_ERR;
        }
        else
        {
            err = CODE_OK;
        }
    }
    //内存映射：他系BLVDS专用通道 A9读 FPGA写（4K大小）
    s_bram_RD_B_BLVDSBlckAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_RD_B_BLVDS_BLOCK0,BRAM_4BLCK_LNGTH_U8);
    ErrNum = 0;
    if(MAP_FAILED == s_bram_RD_B_BLVDSBlckAddr)
    {
        ErrNum++;
        while((ErrNum < MAPERR_NUM) && (MAP_FAILED == s_bram_RD_B_BLVDSBlckAddr)) /*try 3 times*/
        {
            s_bram_RD_B_BLVDSBlckAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_RD_B_BLVDS_BLOCK0,BRAM_4BLCK_LNGTH_U8);
            ErrNum++;
        }
        if(ErrNum >= MAPERR_NUM)
        {
            EADSErrInfop -> EADSErr = 1;
            snprintf(loginfo, sizeof(loginfo)-1, "BramMap BRAM_A9_RD_B_BLVDS_BLOCK0 failed %m");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            err = CODE_ERR;
        }
        else
        {
            err = CODE_OK;
        }
    }
    //内存映射：他系BLVDS专用通道 A9写 FPGA读（4K大小）
    s_bram_WR_B_BLVDSBlckAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_WR_B_BLVDS_BLOCK0 ,BRAM_4BLCK_LNGTH_U8);
    ErrNum = 0;
    if(MAP_FAILED == s_bram_WR_B_BLVDSBlckAddr)
    {
        ErrNum++;
        while((ErrNum < MAPERR_NUM) && (MAP_FAILED == s_bram_WR_B_BLVDSBlckAddr)) /*try 3 times*/
        {
            s_bram_WR_B_BLVDSBlckAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_WR_B_BLVDS_BLOCK0 ,BRAM_4BLCK_LNGTH_U8);
            ErrNum++;
        }
        if(ErrNum >= MAPERR_NUM)
        {
            EADSErrInfop -> EADSErr = 1;
            snprintf(loginfo, sizeof(loginfo)-1, "BramMap BRAM_A9_WR_B_BLVDS_BLOCK0  failed %m");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            err = CODE_ERR;
        }
        else
        {
            err = CODE_OK;
        }
    }
    //内存映射：570专用通道 A9读 FPGA写（4K大小）
    s_bram_RD_TMS_SPCBlckAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_RD_A_TMS_SPCBLOCK0,BRAM_4BLCK_LNGTH_U8);
    ErrNum = 0;
    if(MAP_FAILED == s_bram_RD_TMS_SPCBlckAddr)
    {
        ErrNum++;
        while((ErrNum < MAPERR_NUM) && (MAP_FAILED == s_bram_RD_TMS_SPCBlckAddr)) /*try 3 times*/
        {
            s_bram_RD_TMS_SPCBlckAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_RD_A_TMS_SPCBLOCK0,BRAM_4BLCK_LNGTH_U8);
            ErrNum++;
        }
        if(ErrNum >= MAPERR_NUM)
        {
            EADSErrInfop -> EADSErr = 1;
            snprintf(loginfo, sizeof(loginfo)-1, "BramMap BRAM_A9_RD_A_TMS_SPCBLOCK0 failed %m");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            err = CODE_ERR;
        }
        else
        {
            err = CODE_OK;
        }

    }
    //内存映射：570专用通道 A9写 FPGA读（4K大小）
    s_bram_WR_TMS_SPCBlckAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_WR_A_TMS_SPCBLOCK0,BRAM_4BLCK_LNGTH_U8);
    ErrNum = 0;
    if(MAP_FAILED == s_bram_WR_TMS_SPCBlckAddr)
    {
        ErrNum++;
        while((ErrNum < MAPERR_NUM) && (MAP_FAILED == s_bram_WR_TMS_SPCBlckAddr)) /*try 3 times*/
        {
            s_bram_WR_TMS_SPCBlckAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_WR_A_TMS_SPCBLOCK0,BRAM_4BLCK_LNGTH_U8);
            ErrNum++;
        }
        if(ErrNum >= MAPERR_NUM)
        {
            EADSErrInfop -> EADSErr = 1;
            snprintf(loginfo, sizeof(loginfo)-1, "BramMap BRAM_A9_WR_A_TMS_SPCBLOCK0 failed %m");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            err = CODE_ERR;
        }
        else
        {
            err = CODE_OK;
        }

    }
    //内存映射：读标志位（4K大小）
    s_bram_RDFlagAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_RD_A_TMS_FLAG,300);//both TMS and blvsd  use this addres        
    ErrNum = 0;
    if(MAP_FAILED == s_bram_RDFlagAddr)
    {
        ErrNum++;
        while((ErrNum < MAPERR_NUM) && (MAP_FAILED == s_bram_RDFlagAddr)) /*try 3 times*/
        {
            s_bram_RDFlagAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_RD_A_TMS_FLAG,300);//both TMS and blvsd  use this addres     
            ErrNum++;
        }
        if(ErrNum >= MAPERR_NUM)
        {
            EADSErrInfop -> EADSErr = 1;
            snprintf(loginfo, sizeof(loginfo)-1, "BramMap BRAM_A9_RD_A_TMS_FLAG failed %m");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            err = CODE_ERR;
        }
        else
        {
            err = CODE_OK;
        }

    }
    //内存映射：写标志位（4K大小）
    s_bram_WRFlagAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_WR_A_TMS_FLAG,300);//when run mamap,to avoid mamap many times
    ErrNum = 0;
    if(MAP_FAILED == s_bram_WRFlagAddr)
    {
        ErrNum++;
        while((ErrNum < MAPERR_NUM) && (MAP_FAILED == s_bram_WRFlagAddr)) /*try 3 times*/
        {
            s_bram_WRFlagAddr = BramMap(s_bram_MemFd_I8,BRAM_A9_WR_A_TMS_FLAG,300);;//both TMS and blvsd  use this addres     
            ErrNum++;
        }
        if(ErrNum >= MAPERR_NUM)
        {
            EADSErrInfop -> EADSErr = 1;
            snprintf(loginfo, sizeof(loginfo)-1, "BramMap BRAM_A9_WR_A_TMS_FLAG failed %m");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            err = CODE_ERR;
        }
        else
        {
            err = CODE_OK;
        }

    }
    return err;
}

int8_t BramCloseMap(void)
{
    CloseBram((void *)s_bram_RD_A_BLVDSBlckAddr,BRAM_4BLCK_LNGTH_U8);
    CloseBram((void *)s_bram_RD_B_BLVDSBlckAddr,BRAM_4BLCK_LNGTH_U8);
    CloseBram((void *)s_bram_WR_B_BLVDSBlckAddr,BRAM_4BLCK_LNGTH_U8);
    CloseBram((void *)s_bram_RD_TMS_SPCBlckAddr,BRAM_4BLCK_LNGTH_U8);
    CloseBram((void *)s_bram_WR_TMS_SPCBlckAddr,BRAM_4BLCK_LNGTH_U8);
    CloseBram((void *)s_bram_RDFlagAddr,300); 
    CloseBram((void *)s_bram_WRFlagAddr,300);
    close(s_bram_MemFd_I8);
    return CODE_OK;
}

/**
 * @description: Bram物理地址映射为内存地址
 * @param {EADS_ERROR_INFO *} EADSErrInfop 错误信息
 * @param {uint8_t} EADSType EADS类型
 * @return {int8_t} ReadErr
 * @author: zlz
 */
int8_t Bram_Mapping_Init(EADS_ERROR_INFO *EADSErrInfop)  
{  
    uint16_t i = 0;
    int8_t ReadErr = 0;
    ReadErr = BramOpenMap(EADSErrInfop);
    if(CODE_ERR == ReadErr)
    {
        perror("BramOpenMap faled");
    }
    /*BLVDS ADDR 地址映射 A9读FPGA写*/
    s_Bram_A_BLVDSBlckAddr_ST.MapBlckAddr_p = s_bram_RD_A_BLVDSBlckAddr;
    s_Bram_A_BLVDSBlckAddr_ST.MapBlckFlgAddr_p = s_bram_RDFlagAddr;
    s_Bram_A_BLVDSBlckAddr_ST.BramBlckAddr = BRAM_A9_RD_A_BLVDS_BLOCK0;
    s_Bram_A_BLVDSBlckAddr_ST.BramBlckFlgAddr = BRAM_A9_RD_A_TMS_FLAG;
    
    /*他系BLVDS通道 ADDR 地址映射 A9读FPGA写*/
    s_bram_RD_B_BLVDSBlckAddr_ST.MapBlckAddr_p = s_bram_RD_B_BLVDSBlckAddr;
    s_bram_RD_B_BLVDSBlckAddr_ST.MapBlckFlgAddr_p = s_bram_RDFlagAddr;
    s_bram_RD_B_BLVDSBlckAddr_ST.BramBlckAddr = BRAM_A9_RD_B_BLVDS_BLOCK0;
    s_bram_RD_B_BLVDSBlckAddr_ST.BramBlckFlgAddr = BRAM_A9_RD_B_BLVDS_FLAG ; 
    
    /*他系BLVDS通道 ADDR 地址映射 A9写FPGA读*/
    s_bram_WR_B_BLVDSBlckAddr_ST.MapBlckAddr_p = s_bram_WR_B_BLVDSBlckAddr;
    s_bram_WR_B_BLVDSBlckAddr_ST.MapBlckFlgAddr_p = s_bram_WRFlagAddr;
    s_bram_WR_B_BLVDSBlckAddr_ST.BramBlckAddr = BRAM_A9_WR_B_BLVDS_BLOCK0;
    s_bram_WR_B_BLVDSBlckAddr_ST.BramBlckFlgAddr = BRAM_A9_WR_B_BLVDS_FLAG;

    /*TMS570专用 ADDR 地址映射 A9读FPGA写*/
    s_bram_RD_TMS_SPC_Blck_ST.MapBlckAddr_p = s_bram_RD_TMS_SPCBlckAddr;
    s_bram_RD_TMS_SPC_Blck_ST.MapBlckFlgAddr_p = s_bram_RDFlagAddr;
    s_bram_RD_TMS_SPC_Blck_ST.BramBlckAddr = BRAM_A9_RD_A_TMS_SPCBLOCK0;
    s_bram_RD_TMS_SPC_Blck_ST.BramBlckFlgAddr = BRAM_A9_RD_A_TMS_SPCFLAG ; 
    
    /*TMS570专用 ADDR 地址映射 A9写FPGA读*/
    s_bram_WR_TMS_SPC_Blck_ST.MapBlckAddr_p = s_bram_WR_TMS_SPCBlckAddr;
    s_bram_WR_TMS_SPC_Blck_ST.MapBlckFlgAddr_p = s_bram_WRFlagAddr;
    s_bram_WR_TMS_SPC_Blck_ST.BramBlckAddr = BRAM_A9_WR_A_TMS_SPCBLOCK0;
    s_bram_WR_TMS_SPC_Blck_ST.BramBlckFlgAddr = BRAM_A9_WR_A_TMS_SPCFLAG;

    return ReadErr;
}
#if 0
/**
 * @description: BLVDS数据读取线程功能
 * @param {uint8_t} ReadNum_U8
 * @param {uint8_t} EADSType
 * @param {EADS_ERROR_INFO *} EADSErrInfop
 * @return {*}
 * @author: zlz
 */
int8_t BLVDSDataReadThreadFunc(uint8_t ReadNum_U8,uint8_t EADSType,EADS_ERROR_INFO * EADSErrInfop) 
{

    int8_t ReadErr = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    static uint8_t s_ADUDataErrFlag = 0;
    static uint8_t s_CTUDataErrFlag = 0;
    static uint8_t s_ADUDataErrNum  = 0;
    static uint8_t s_CTUDataErrNum  = 0;
    if(BRAM_DEBUG == g_DebugType_EU)
    {
        printf("Read ADU_BOARD BLVDS[%d]:\n",ReadNum_U8);
    }
    s_Bram_A_BLVDSBlckAddr_ST.DataU32Length = BRAM_LENGTH_U32 ;
    s_Bram_A_BLVDSBlckAddr_ST.ChanNum_U8 = ADU_BOARD_ID;
    if(0 == g_LinuxDebug)//for zynq linux Running
    {
        ReadErr = BoardDataRead(&s_Bram_A_BLVDSBlckAddr_ST,&g_BrdRdBufData_ST.Board0_Data_U32[ReadNum_U8][0]);
        if(CODE_ERR == ReadErr) 
        {
            s_ADUDataErrNum++;  
            if(s_ADUDataErrNum > BRAMERR_NUM) 
            {
                if(0 == s_ADUDataErrFlag)
                {
                    EADSErrInfop -> BLVDSErr = 1; 
                    s_ADUDataErrFlag = 1;
                    printf("ADU_BOARD Bram data read err!\n");
                    snprintf(loginfo, sizeof(loginfo)-1, "ADU_BOARD Bram data read err!");
                    WRITELOGFILE(LOG_ERROR_1,loginfo);
                }
                s_ADUDataErrNum = 0;
            }
        }
        else if(CODE_OK == ReadErr) 
        {
            s_ADUDataErrNum = 0;
            if(1 == s_ADUDataErrFlag)
            {
                EADSErrInfop -> BLVDSErr = 0;
                s_ADUDataErrFlag = 0;
                printf("ADU_BOARD Bram data return to normal!\n");
                snprintf(loginfo, sizeof(loginfo)-1, "ADU_BOARD Bram data return to normal!");
                WRITELOGFILE(LOG_ERROR_1,loginfo);   
            }
        }
    }     
    return ReadErr;
}
#endif
/**
 * @description: just for this page can program use!(pay attention "static")
 * @param:       void life_data,void *life_lasttime,uint8_t *errnum
 * @return:      uint8_t life stop ->1 life normal ->0
 * @author:      zlz
 */
static uint8_t Life_Judge_Fun(uint8_t life_data,uint8_t *life_lasttime,uint8_t *errnum)
{
    if(life_data == *life_lasttime)
    {
        *errnum++;
        if(*errnum >= 10)
        {   
            *errnum = 0;            
            return 1;
        }             
    }
    else
    {
        *errnum = 0;
        *life_lasttime = life_data;
        return 0;       
    }

}
/**
 * @description: CAN 设备生命信号进行判断处理 (pay attention "static")
 * @param:       void
 * @return:      void
 * @author:      zlz
 */
static void CAN_Life_Judge(struct can_frame *candata,uint32_t *judge_val,uint8_t can_devtype)
{
    uint8_t i;
    static uint8_t errnum[5]= {0};    
    static uint8_t errflag[5] = {0};
    static uint8_t life_lasttime[5] = {0};    
    char loginfo[LOG_INFO_LENG] = {0};

    if (can_devtype == CAN0_TYPE)
    {
        for (i=0;i<CAN0_READ_FRAME_NUM;i++)
        {
            switch (candata[i].can_id)
            {
                case 0x1800D0F4 :
                    judge_val[0] = Life_Judge_Fun(candata[i].data[6],&life_lasttime[0],&errnum[0]);
                    if(judge_val[0])
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "CAN Receiv frame from COMID:[0x1800D0F4] life have stopped!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[0]=1;
                    }
                    else if(judge_val[0]==0 && errflag[0]==1)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "CAN Receiv frame from COMID:[0x1800D0F4] life have recovered!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[0]=0;
                    }                    
                    break;
                case 0x16F4C001 :    
                    judge_val[1] = Life_Judge_Fun(candata[i].data[7],&life_lasttime[1],&errnum[1]);
                    if(judge_val[1])
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "CAN Receiv frame from COMID:[0x16F4C001] life have stopped!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[1]=1;
                    }
                    else if(judge_val[1]==0 && errflag[1]==1)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "CAN Receiv frame from COMID:[0x16F4C001] life have recovered!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[1]=0;
                    }
                    break;
                case 0x18FF3012 :    
                    judge_val[2] = Life_Judge_Fun(candata[i].data[0],&life_lasttime[2],&errnum[2]);
                    if(judge_val[2])
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "CAN Receiv frame from COMID:[0x18FF3012] life have stopped!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[2]=1;
                    }
                    else if(judge_val[2]==0 && errflag[2]==1)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "CAN Receiv frame from COMID:[0x18FF3012] life have recovered!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[2]=0;
                    }
                    break;
                default:
                    break;
            }
        }  
    }
    else if (can_devtype == CAN1_TYPE)
    {
        for (i=0;i<CAN1_READ_FRAME_NUM;i++)
        {
            switch (candata[i].can_id)
            {
                #if 0 //FIXME 变频器暂无生命信号，后期增加后请修改此处
                case 0x15003000 :
                    judgeret[3] = Life_Judge_Fun(candata[i].data[0],&life_lasttime[4],&errnum[4]);
                    if(judgeret[3])
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "CAN Receiv frame from COMID:[0x15003000] life have stopped!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[3]=1;
                    }
                    else if(judgeret[3]==0 && errflag[3]==1)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "CAN Receiv frame from COMID:[0x15003000] life have recovered!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[3]=0;
                    }
                    break;
                #endif
                case 0x19003000 :
                    judge_val[0] = Life_Judge_Fun(candata[i].data[0],&life_lasttime[4],&errnum[4]);
                    if(judge_val[0])
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "CAN Receiv frame from COMID:[0x19003000] life have stopped!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[4]=1;
                    }
                    else if(judge_val[0]==0 && errflag[4]==1)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "CAN Receiv frame from COMID:[0x19003000] life have recovered!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[4]=0;
                    }                   
                    break;                
                default:
                    break;
            }
        }
    }   
}
/**
 * @description: 初始化CAN0数据帧ID及帧长度信息
 * @param:       void
 * @return:      void
 * @author:      zlz
 */
void CAN_FrameInit(struct can_filter *candata_RD_filter,struct can_frame *candata_WR,uint8_t can_devtype)
{
    switch (can_devtype)
    {
        case CAN0_TYPE: 
        /*CAN0-BMS-A9 READ*/
        candata_RD_filter[0].can_id    = 0x1800D0F4 | CAN_EFF_FLAG;
        candata_RD_filter[0].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[1].can_id    = 0x1801D0F4 | CAN_EFF_FLAG;
        candata_RD_filter[1].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[2].can_id    = 0x1802D0F4 | CAN_EFF_FLAG;
        candata_RD_filter[2].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[3].can_id    = 0x1803D0F4 | CAN_EFF_FLAG;
        candata_RD_filter[3].can_mask   = CAN_EFF_MASK;
        /*CAN0-DCDC-A9 READ*/
        candata_RD_filter[4].can_id    = 0x16F4C000 | CAN_EFF_FLAG;
        candata_RD_filter[4].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[5].can_id    = 0x16F4C001 | CAN_EFF_FLAG;
        candata_RD_filter[5].can_mask   = CAN_EFF_MASK;
        /*CAN0-FC-A9 READ*/
        candata_RD_filter[6].can_id    = 0x18FF3012 | CAN_EFF_FLAG;
        candata_RD_filter[6].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[7].can_id    = 0x18FF3112 | CAN_EFF_FLAG;
        candata_RD_filter[7].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[8].can_id    = 0x18FF3212 | CAN_EFF_FLAG;
        candata_RD_filter[8].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[9].can_id    = 0x18FF3312 | CAN_EFF_FLAG;
        candata_RD_filter[9].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[10].can_id   = 0x18FF6012 | CAN_EFF_FLAG;
        candata_RD_filter[10].can_mask  = CAN_EFF_MASK;
        candata_RD_filter[11].can_id   = 0x18FF7012 | CAN_EFF_FLAG;
        candata_RD_filter[11].can_mask  = CAN_EFF_MASK;
        candata_RD_filter[12].can_id   = 0x18FF7112 | CAN_EFF_FLAG;
        candata_RD_filter[12].can_mask  = CAN_EFF_MASK;
        /*CAN0-BMS-A9 WRITE*/
        candata_WR[0].can_id    = 0x1800F4D0 | CAN_EFF_FLAG;
        candata_WR[0].can_dlc   = 8;
        /*CAN0-DCDC-A9 WRITE*/
        candata_WR[1].can_id    = 0x16F4C002 | CAN_EFF_FLAG;
        candata_WR[1].can_dlc   = 8;
        /*CAN0-FC-A9 WRITE*/
        candata_WR[2].can_id    = 0x18FF0B27 | CAN_EFF_FLAG;
        candata_WR[2].can_dlc   = 8;
        candata_WR[3].can_id    = 0x18FF0A27 | CAN_EFF_FLAG;
        candata_WR[3].can_dlc   = 8;
        break;
        case CAN1_TYPE:
        /*CAN1-变频器-A9 READ*/        
        candata_RD_filter[0].can_id    = 0x15003000 | CAN_EFF_FLAG;
        candata_RD_filter[0].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[1].can_id    = 0x15003001 | CAN_EFF_FLAG;
        candata_RD_filter[1].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[2].can_id    = 0x15003002 | CAN_EFF_FLAG;
        candata_RD_filter[2].can_mask   = CAN_EFF_MASK;
        /*CAN1-扩展模块-A9 READ*/ 
        candata_RD_filter[3].can_id    = 0x19003000 | CAN_EFF_FLAG;
        candata_RD_filter[3].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[4].can_id    = 0x19003001 | CAN_EFF_FLAG;
        candata_RD_filter[4].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[5].can_id    = 0x19003002 | CAN_EFF_FLAG;
        candata_RD_filter[5].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[6].can_id    = 0x19003003 | CAN_EFF_FLAG;
        candata_RD_filter[6].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[7].can_id    = 0x19003004 | CAN_EFF_FLAG;
        candata_RD_filter[7].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[8].can_id    = 0x19003005 | CAN_EFF_FLAG;
        candata_RD_filter[8].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[9].can_id    = 0x19003006 | CAN_EFF_FLAG;
        candata_RD_filter[9].can_mask   = CAN_EFF_MASK;
        /*CAN1-A9-变频器 Write*/
        candata_WR[0].can_id    = 0x15004000 | CAN_EFF_FLAG;
        candata_WR[0].can_dlc   = 8;
        candata_WR[1].can_id    = 0x15004001 | CAN_EFF_FLAG;
        candata_WR[1].can_dlc   = 8;
        /*CAN1-A9-扩展单元 Write*/
        candata_WR[2].can_id    = 0x19004000 | CAN_EFF_FLAG;
        candata_WR[2].can_dlc   = 8;
        break;
        default:
            printf("invalid can_devtype!\n");
            break;
    }
}

/**
 * @description: CAN写操作
 * @param:       struct can_frame *candata_wr --要发送的CAN数据
 *               TMS570_BRAM_DATA *bramdata_rd  --从570读取的Bram数据 
 *               uint8_t can_devtype            -- CAN0 or CAN1
 * @return:      void
 * @author:      zlz
 */
int8_t CAN_Write_Option(int8_t socket_fd,struct can_frame *can_frame_data,uint8_t frames_num)
{    
    uint8_t i,nbytes;
    static uint8_t errnum_wr=0;
    char loginfo[LOG_INFO_LENG]={0};

    for(i=0;i<frames_num;i++)
    {
        nbytes = write(socket_fd,&can_frame_data[i], sizeof(can_frame_data[i]));
        if(nbytes != sizeof(can_frame_data[i]))
        {                
            printf("CAN Send frame[%u] Error!\n",can_frame_data[i].can_id);
            errnum_wr++;
            if(errnum_wr >=10)
            {
                snprintf(loginfo, sizeof(loginfo)-1, "CAN Send frame[%u] Error!",can_frame_data[i].can_id);
                WRITELOGFILE(LOG_ERROR_1,loginfo);
                errnum_wr = 0;
                return -1;
            }                
        }
        else
        {
            errnum_wr =0;
        }        
    }
    return 0;      
}
/*
 * @description: CAN读操作
 * @param:       struct can_frame *candata_wr --要发送的CAN数据
 *               TMS570_BRAM_DATA *bramdata_rd  --从570读取的Bram数据 
 *               uint8_t can_devtype            -- CAN0 or CAN1
 * @return:      void
 * @author:      zlz
 */
int8_t CAN_Read_Option(int8_t socket_fd,struct can_frame *can_frame_data,uint8_t frames_num)
{
    uint8_t i,j,nbytes;
    static uint8_t errnum_rd=0,errnum_timeout=0;
    char loginfo[LOG_INFO_LENG]={0};
    fd_set rdfs={0};
    struct timeval tv_select={0,1000};

    for(i=0;i<frames_num;i++)
    {            
        //FD_ZERO(&rfds);
        //FD_SET(socket_fd,&rfds);
        //ret = select(socket_fd+1,&rfds,NULL,NULL,&tv_select);
        //if(ret>0)
        {
            errnum_timeout=0;
            nbytes = read(socket_fd,&can_frame_data[i],sizeof(can_frame_data[i]));            
            if(nbytes != sizeof(can_frame_data[i]))
            {                
                printf("CAN0 Receive Error frame[%d]!\n",i);
                memset(can_frame_data[i].data,0,8);
                errnum_rd++;
                if(errnum_rd >=10)
                {
                    snprintf(loginfo, sizeof(loginfo)-1, "CAN0 receive frame[%u] Error!",can_frame_data[i].can_id);
                    WRITELOGFILE(LOG_ERROR_1,loginfo);
                    errnum_rd = 0;
                }                            
            }
            else
            {
                errnum_rd = 0;
            }
        }
        /*else
        {
            errnum_timeout++;
            if(errnum_timeout==1)
                printf("can0 read time out ,already receive %d frames!\n",i);
            printf("can0 read time out %d times!\n",errnum_timeout);
            if(errnum_timeout >=13)
            {
                snprintf(loginfo, sizeof(loginfo)-1, "CAN 0 receive frame time out!");
                WRITELOGFILE(LOG_ERROR_1,loginfo);
                errnum_timeout = 0;
            } 
        }*/            
        if(g_DebugType_EU == CAN_RD_DEBUG)
        {               
            {
                    printf("Read CAN0 ID:0x%x:",can_frame_data[i].can_id & 0x1FFFFFFF);
                    for (j = 0; j < 8; j++)                    
                        printf("[%x]",can_frame_data[i].data[j]);
                    printf("\n");
            }                                                 
        }
    }    
}
/**
 * @description: CAN发送数据前,将TMS570通过Bram反馈的数据进行处理
 * @param:       struct can_frame *candata_wr --要发送的CAN数据
 *               TMS570_BRAM_DATA *bramdata_rd  --从570读取的Bram数据 
 *               uint8_t can_devtype            -- CAN0 or CAN1
 * @return:      void
 * @author:      zlz
 */
void CAN_WriteData_Pro(struct can_frame *candata_wr,TMS570_BRAM_DATA *bramdata_rd,uint8_t can_devtype)
{
    uint8_t i,j;    
    /*uint8_t testbuff[32]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,\
                        0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x00,\
                        0x12,0x34,0x56,0x78,0x9a,0xbc,0xde,0xf0,\
                        0x78,0x65,0x32,0x10,0x54,0x23,0x99,0xaa};*/

    switch (can_devtype)
    {
        case CAN0_TYPE:
            memcpy(candata_wr[0].data,bramdata_rd[1].buffer,8);
            memcpy(candata_wr[1].data,bramdata_rd[2].buffer,8);
            memcpy(candata_wr[2].data,bramdata_rd[3].buffer,8);
            memcpy(candata_wr[3].data,&bramdata_rd[3].buffer[2],8);
            if(g_DebugType_EU == CAN_WR_DEBUG)
            {               
                for(j=0;j<4;j++)
                {
                    printf("candata_wr[%d]:0X",j);
                    for (i=0;i<8;i++)
                        printf("[%x]",candata_wr[j].data[i]);
                    printf("\n");                   
                }                               
            }
            break;
        case CAN1_TYPE:                        
            for(j=0;j<3;j++)
            {               
                memcpy(candata_wr[j].data,&bramdata_rd[4].buffer[j*2],8);                               
            }
            if(g_DebugType_EU == CAN_WR_DEBUG)
            {               
                for(j=0;j<3;j++)
                {
                    printf("candata_wr[%d]:0X",j);
                    for (i = 0; i < 8; i++)
                        printf("[%x]",candata_wr[j].data[i]);
                    printf("\n");
                }                                              
            }
            break;
        default:
            break;
    }
}
/**
 * @description: 接收CAN数据后进行数据处理
 * @param:       struct can_frame *candata_rd   --从CAN口获取的CAN数据
 *               TMS570_BRAM_DATA *bramdata_wr  --A9发送给570的BRAM数据 
 *               uint8_t can_devtype   --CAN0 or CAN1
 *               uint8_t can_frame_num --CAN帧数量
 * @return:      void
 * @author:      zlz
 */
void CAN_ReadData_Pro(struct can_frame *candata_rd,TMS570_BRAM_DATA *bramdata_wr,uint8_t can_devtype)
{
    uint8_t i,j;
    uint8_t tempdata[240]={0};
    uint32_t life_judge_val[3]={0};

    CAN_Life_Judge(candata_rd,life_judge_val,can_devtype);

    if(can_devtype == CAN0_TYPE)
    {    
        for(i=0;i<CAN0_READ_FRAME_NUM;i++)
        {
            switch (candata_rd[i].can_id & 0x1FFFFFFF)
            {
                case 0x1800D0F4 :
                    memcpy(&bramdata_wr[1].buffer[0],candata_rd[i].data,8);
                    break;
                case 0x1801D0F4 :
                    memcpy(&bramdata_wr[1].buffer[2],candata_rd[i].data,8);
                    break;
                case 0x1802D0F4 :
                    memcpy(&bramdata_wr[1].buffer[4],candata_rd[i].data,8);
                    break;                    
                case 0x1803D0F4 :
                    memcpy(&bramdata_wr[1].buffer[6],candata_rd[i].data,8);
                    memcpy(&bramdata_wr[1].buffer[8],&life_judge_val[0],4);
                    break;
                case 0x16F4C000 :
                    memcpy(&bramdata_wr[2].buffer[0],candata_rd[i].data,8);
                    break;
                case 0x16F4C001 :
                    memcpy(&bramdata_wr[2].buffer[2],candata_rd[i].data,8);
                    memcpy(&bramdata_wr[2].buffer[4],&life_judge_val[1],4);
                    break;
                case 0x18FF3012 :
                    memcpy(&bramdata_wr[3].buffer[0],candata_rd[i].data,8);
                    break;
                case 0x18FF3112 :
                    memcpy(&bramdata_wr[3].buffer[2],candata_rd[i].data,8);
                    break;
                case 0x18FF3212 :
                    memcpy(&bramdata_wr[3].buffer[4],candata_rd[i].data,8);
                    break;
                case 0x18FF3312 :
                    memcpy(&bramdata_wr[3].buffer[6],candata_rd[i].data,8);
                    break;
                case 0x18FF6012 :
                    memcpy(&bramdata_wr[3].buffer[8],candata_rd[i].data,8);
                    break;
                case 0x18FF7012 :
                    memcpy(&bramdata_wr[3].buffer[10],candata_rd[i].data,8);
                    break;
                case 0x18FF7112 :
                    memcpy(&bramdata_wr[3].buffer[12],candata_rd[i].data,8);
                    memcpy(&bramdata_wr[3].buffer[14],&life_judge_val[2],4);
                    break;
                default:
                    break;              
            }
        }
    }
    else if(can_devtype == CAN1_TYPE)
    {        
        for(i=0;i<CAN1_READ_FRAME_NUM;i++)
        {
            switch (candata_rd[i].can_id & 0x1FFFFFFF)
            {   
                case 0x15003000 :
                    memcpy(&bramdata_wr[4].buffer[0],candata_rd[i].data,8);
                    break;
                case 0x15003001 :
                    memcpy(&bramdata_wr[4].buffer[2],candata_rd[i].data,8);
                    break;
                case 0x15003002 :
                    memcpy(&bramdata_wr[4].buffer[4],candata_rd[i].data,8);
                    break;
                case 0x19003000 :
                    memcpy(&bramdata_wr[4].buffer[6],candata_rd[i].data,8);
                    break;
                case 0x19003001 :
                    memcpy(&bramdata_wr[4].buffer[8],candata_rd[i].data,8);
                    break;
                case 0x19003002 :
                    memcpy(&bramdata_wr[4].buffer[10],candata_rd[i].data,8);
                    break;
                case 0x19003003 :
                    memcpy(&bramdata_wr[4].buffer[12],candata_rd[i].data,8);
                    break;
                case 0x19003004 :
                    memcpy(&bramdata_wr[4].buffer[14],candata_rd[i].data,8);
                    break;
                case 0x19003005 :
                    memcpy(&bramdata_wr[4].buffer[16],candata_rd[i].data,8);
                    break;
                case 0x19003006 :
                    memcpy(&bramdata_wr[4].buffer[18],candata_rd[i].data,8);
                    memcpy(&bramdata_wr[4].buffer[20],&life_judge_val[0],4);
                    break;
                default:
                    break;    
            }
        }            
    }    
}
/**
 * @description: 初始化TMS570交互的Bram通道包头数据(包含CAN及MVB数据)
 * @param:       void
 * @return:      void
 * @author:      zlz
 */
void TMS570_Bram_TopPackDataSetFun(uint8_t can_devtype)
{
    //Attention:数据区长度不包含CRC32，PacktLength需要在总长度上-1
    switch (can_devtype)
    {
        case CAN0_TYPE:
            /*CAN-BMS A9->570*/
            CmdPact_WR_ST[1].protocol_version =0x11c2;
            CmdPact_WR_ST[1].ChanNum_U8 = 9;
            CmdPact_WR_ST[1].PacktLength_U32 = 12;
            /*CAN-DCDC A9->570*/
            CmdPact_WR_ST[2].protocol_version =0x11c2;
            CmdPact_WR_ST[2].ChanNum_U8 = 10;
            CmdPact_WR_ST[2].PacktLength_U32 = 8;
            /*CAN-FC A9->570*/
            CmdPact_WR_ST[3].protocol_version =0x11c2;
            CmdPact_WR_ST[3].ChanNum_U8 = 11;
            CmdPact_WR_ST[3].PacktLength_U32 = 18;
            /*CAN-BMS 570->A9*/
            CmdPact_RD_ST[1].ChanNum_U8 = 9;
            CmdPact_RD_ST[1].PacktLength_U32 = 6;
            /*CAN-DCDC 570->A9*/
            CmdPact_RD_ST[2].ChanNum_U8 = 10;
            CmdPact_RD_ST[2].PacktLength_U32 = 6;
            /*CAN-FC 570->A9*/
            CmdPact_RD_ST[3].ChanNum_U8 = 11;
            CmdPact_RD_ST[3].PacktLength_U32 = 7;
        break;
        case CAN1_TYPE:
        /*CAN-扩展模块 A9->570*/
            CmdPact_WR_ST[4].protocol_version =0x11c2;
            CmdPact_WR_ST[4].ChanNum_U8 = 12;
            CmdPact_WR_ST[4].PacktLength_U32 = 24;
            /*CAN-扩展模块 570->A9*/
            CmdPact_RD_ST[4].ChanNum_U8 = 12;
            CmdPact_RD_ST[4].PacktLength_U32 = 9;
        default:
            break;
    }
}

/**
 * @description: 从Bram指定区域读TMS570数据
 * @param:      TMS570_BRAM_DATA *bram_data
 * @return:     ReadErr
 * @author:     zlz
 */ 
int8_t TMS570_Bram_Read_Func(TMS570_BRAM_DATA bram_data[],uint8_t begin_index,uint8_t end_index)
{

    int8_t ReadErr = 0,i,j;
    char loginfo[LOG_INFO_LENG] = {0};
    uint32_t tempdatabuffer[64] = {64};    
    uint8_t DataErrFlag = 0;
    uint8_t DataErrNum  = 0;
    if (begin_index>4 || end_index>4)
    {
        printf("Invalid number for TMS570 Read BramCycle!!\n");
        return -1;
    }
    
    for(i=begin_index;i<end_index+1;i++)
    {       
        s_bram_RD_TMS_SPC_Blck_ST.DataU32Length = CmdPact_RD_ST[i].PacktLength_U32;
        s_bram_RD_TMS_SPC_Blck_ST.ChanNum_U8 = CmdPact_RD_ST[i].ChanNum_U8;
        memset(tempdatabuffer,0,256);
        ReadErr = BoardDataRead(&s_bram_RD_TMS_SPC_Blck_ST,tempdatabuffer);
        memcpy(bram_data[i].buffer,&tempdatabuffer[12],240);
        if(CODE_ERR == ReadErr) 
        {
            DataErrNum++;  
            if(DataErrNum > BRAMERR_NUM) 
            {
                if(0 == DataErrFlag)
                {                    
                    DataErrFlag = 1;
                    printf("The [%d] frame Bramdata read from TMS570 is Err.\n",i);
                    snprintf(loginfo, sizeof(loginfo)-1, "The [%d] frame Bramdata read from TMS570 is Err.",i);
                    WRITELOGFILE(LOG_ERROR_1,loginfo);                        
                }
                DataErrNum = 0;
            }
        }
        if(TMS570_BRAM_RD_DEBUG == g_DebugType_EU)
        {
            for(j=0;j<25;j++)
                printf("TMS570:Read from Bram bram_data[%d][%02d]:0x%08u\n",i,j,bram_data[i].buffer[j]);              
        } 
        
    }
    return ReadErr;
}

/**
 * @description: 向Bram指定区域写TMS570数据
 * @param:       TMS570_BRAM_DATA *bram_data
 * @return       {uint8_t} WriteErr   写数据返回值
 * @author:      zlz
 */
int8_t TMS570_Bram_Write_Func(TMS570_BRAM_DATA *bram_data,uint8_t begin_index,uint8_t end_index) 
{
    int8_t i,j;
    static int8_t WriteErr = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    uint8_t DataErrFlag = 0;
    uint8_t DataErrNum  = 0;     
    static uint16_t Life_signal = 0;  
    BRAM_PACKET_TOP TopPackST[5] = {0};

    if (begin_index>4 || end_index>4)
    {
        printf("Invalid number for TMS570 Write BramCycle!\n");
        return -1;
    }

    for(i=begin_index;i<end_index+1;i++)
    {
        //填充帧头
        TopPackST[i].BLVDSTOP_U32 = CmdPact_WR_ST[i].protocol_version;
        TopPackST[i].BLVDSReser_U32[0] = Life_signal;
        //数据长度、通道数填充
        s_bram_WR_TMS_SPC_Blck_ST.DataU32Length = CmdPact_WR_ST[i].PacktLength_U32;
        s_bram_WR_TMS_SPC_Blck_ST.ChanNum_U8 = CmdPact_WR_ST[i].ChanNum_U8;

        if(TMS570_BRAM_WR_DEBUG == g_DebugType_EU)
        {
            for(j=0;j<25;j++)
                printf("TMS570:Write to Bram bram_data[%d][%02d]:0x%08u\n",i,j,bram_data[i].buffer[j]);
        }    
        if(0 == g_LinuxDebug)
        {
            WriteErr = BramWriteWithChek(&s_bram_WR_TMS_SPC_Blck_ST,bram_data[i].buffer,TopPackST[i]);
            if(WriteErr == CODE_ERR)
            {
                printf("The [%d] frame Bramdata write to TMS570 error!\n",i);					 
            }
        }        
    }
    Life_signal++;
    if(TMS570_BRAM_WR_DEBUG == g_DebugType_EU)
    {
        printf("TMS570_WR_BramLife signal:%04u\n",Life_signal);
    }
    return WriteErr;
}

/**
 * @description: MVB初始化
 * @param:      uint8_t mvb_rd_ch_num
 *              uint8_t mvb_wr_ch_num
 * @return:     InitErr
 * @author:     zlz
 */
int8_t  MVB_Bram_Init(uint8_t mvb_rd_ch_num,uint8_t mvb_wr_ch_num)
{    
    uint8_t i;
    uint32_t Addr_wrch=0X8000E01C,Addr_rdch=0X8000E018;
    uint32_t fdback_Addr_wrch=0X4000E01C,fdback_Addr_rdch=0X4000E018;        
    uint8_t fdback_wrch,fdback_rdch;    
    char loginfo[LOG_INFO_LENG]={0}; 
    /*MVB A9->570*/
    CmdPact_WR_ST[0].protocol_version =0x11c2;
    CmdPact_WR_ST[0].ChanNum_U8 = 8;
    CmdPact_WR_ST[0].PacktLength_U32 = 16;
    /*MVB 570->A9*/
    CmdPact_RD_ST[0].ChanNum_U8 = 8;
    CmdPact_RD_ST[0].PacktLength_U32 = 47;   

    for (i=0;i<32;i++)//TODO JUST FOR TEST ! Please verify to 6 and 2 !
    {
        MVB_CmdPact_WR_ST[i].protocol_version =0x11c2;
	    MVB_CmdPact_WR_ST[i].ChanNum_U8 = i;
        MVB_CmdPact_WR_ST[i].PacktLength_U32 = 11;
    }
    for (i=0;i<32;i++)//TODO JUST FOR TEST  PLEASE VERIFY
    {        
	    MVB_CmdPact_RD_ST[i].ChanNum_U8 = i;
        MVB_CmdPact_RD_ST[i].PacktLength_U32 = 11;
    }         
    /*
    BramWriteU8(Addr_wrch,s_bram_WRFlagAddr,mvb_wr_ch_num);
    BramWriteU8(Addr_rdch,s_bram_WRFlagAddr,mvb_rd_ch_num);
    usleep(100);
    fdback_wrch=BramReadU8(fdback_Addr_wrch,s_bram_RDFlagAddr);
    fdback_rdch=BramReadU8(fdback_Addr_rdch,s_bram_RDFlagAddr);
    if ((fdback_wrch==mvb_wr_ch_num) && (fdback_rdch==mvb_rd_ch_num))
    {
        return  0;
    }
    else
    {
        for (i=0;i<5;i++)
        {
            BramWriteU8(Addr_wrch,s_bram_WRFlagAddr,mvb_wr_ch_num);
            BramWriteU8(Addr_rdch,s_bram_WRFlagAddr,mvb_rd_ch_num);
            usleep(100);
            fdback_wrch=BramReadU8(fdback_Addr_wrch,s_bram_RDFlagAddr);
            fdback_rdch=BramReadU8(fdback_Addr_rdch,s_bram_RDFlagAddr);
            if ((fdback_wrch==mvb_wr_ch_num) && (fdback_rdch==mvb_rd_ch_num))
            {                
                return 0;
            }
        }       
        printf("Correct Configures mvb_read:%d or mvb_write:%d , FPGA feedback wrong configures mvb_read:%d or mvb_write:%d\n",\
                mvb_rd_ch_num,mvb_wr_ch_num,fdback_rdch,fdback_wrch);
        snprintf(loginfo, sizeof(loginfo)-1, "FPGA feedback wrong mvb read or write channel configures!");
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return -1;
    } */   
}
/**
 * @description: 从Bram指定区域读MVB数据,并发送给TMS570
 * @param:      TMS570_BRAM_DATA *bram_data_mvb_rd
 *              uint8_t frame_nums
 * @return:     ReadErr
 * @author:     zlz
 */ 
int8_t  MVB_Bram_Read_Func(TMS570_BRAM_DATA *bram_data_mvb_rd)
{
    int8_t ReadErr = 0,i,j;
    char loginfo[LOG_INFO_LENG] = {0};       
    uint8_t DataErrFlag = 0;
    uint8_t DataErrNum  = 0;
    for(i=0;i<16;i++)//TODO JUST FOR TEST PLEASE VERIFY
    {        
        s_bram_RD_B_BLVDSBlckAddr_ST.DataU32Length =  MVB_CmdPact_RD_ST[i].PacktLength_U32;
        s_bram_RD_B_BLVDSBlckAddr_ST.ChanNum_U8 =  MVB_CmdPact_RD_ST[i].ChanNum_U8;
           
        ReadErr = BoardDataRead(&s_bram_RD_B_BLVDSBlckAddr_ST,bram_data_mvb_rd[i].buffer);
                    
        if(CODE_ERR == ReadErr)
        {
            DataErrNum++;  
            if(DataErrNum > BRAMERR_NUM) 
            {
                if(0 == DataErrFlag)
                {                    
                    DataErrFlag = 1;
                    printf("The [%d] frame Bramdata read from MVB is Err.\n",i);
                    snprintf(loginfo, sizeof(loginfo)-1, "The [%d] frame Bramdata read from MVB is Err.",i);
                    WRITELOGFILE(LOG_ERROR_1,loginfo);                        
                }
                DataErrNum = 0;
            }
        }
        if(MVB_RD_DEBUG == g_DebugType_EU)
        {
            for(j=0;j<10;j++)
                printf("MVB:Read Bram_data[%d][%02d]:0x%08X\n",i,j,bram_data_mvb_rd[i].buffer[j]);            
        }        
    }    
    return ReadErr;
}

/**
 * @description: 从TMS570读取数据，向Bram指定区域写MVB数据
 * @param        TMS570_BRAM_DATA *bram_data_mvb_wr 
 *               uint8_t frame_nums  
 * @return       {uint8_t} WriteErr   写数据返回值
 * @author:      zlz
 */
int8_t 	MVB_Bram_Write_Func(TMS570_BRAM_DATA *bram_data_mvb_wr)
{
    int8_t i,j;
    static int8_t WriteErr = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    uint8_t DataErrFlag = 0;
    uint8_t DataErrNum  = 0;     
    static uint16_t Life_signal = 0;  
    BRAM_PACKET_TOP TopPackST[32] = {0};    //TODO JUST FOR TEST PLEASE VERIFY!
    
    for(i=0;i<g_MVB_SendFrameNum;i++)       //TODO Just for test ! Please verify to MVB_WRITE_FRAME_NUM
    {        
        TopPackST[i].BLVDSTOP_U32 = MVB_CmdPact_WR_ST[i].protocol_version;
        TopPackST[i].BLVDSReser_U32[0] = Life_signal | ((16+i)<<16);
        
        s_bram_WR_B_BLVDSBlckAddr_ST.DataU32Length = MVB_CmdPact_WR_ST[i].PacktLength_U32;
        s_bram_WR_B_BLVDSBlckAddr_ST.ChanNum_U8 = MVB_CmdPact_WR_ST[i].ChanNum_U8;
        
        if(MVB_WR_DEBUG == g_DebugType_EU)
        {
            printf("NO:%d->mvb_channel_number:%d\n",i,MVB_CmdPact_WR_ST[i].ChanNum_U8);
            for(j=0;j<8;j++)
                printf("MVB:Write Bram_data[%d][%d]:0x%08x\n",i,j,bram_data_mvb_wr[i].buffer[j]);
        }    
        if(0 == g_LinuxDebug)
        {
            bram_data_mvb_wr[i].buffer[0]= (Life_signal+100)<<16;
            WriteErr = BramWriteWithChek(&s_bram_WR_B_BLVDSBlckAddr_ST,bram_data_mvb_wr[i].buffer,TopPackST[i]);
            if(WriteErr == CODE_ERR)
            {
                printf("MVB-[%d] frame write to Bramdata error!\n",i);
                snprintf(loginfo, sizeof(loginfo)-1, "MVB-[%d] frame write to Bramdata error!",i);
                WRITELOGFILE(LOG_ERROR_1,loginfo); 				 
            }
        }        
    }    
    Life_signal++;
    return WriteErr;
}
/**
 * @description: 
 * @param        TMS570_BRAM_DATA *bram_data_mvb_wr 
 *               uint8_t frame_nums  
 * @return       {uint8_t} WriteErr   写数据返回值
 * @author:      zlz
 */
int8_t MVB_WR_Data_Proc(TMS570_BRAM_DATA *bram_data_mvb_wr,TMS570_BRAM_DATA *bram_data_tms570_rd)
{

}
/**
 * @description: 
 * @param        TMS570_BRAM_DATA *bram_data_mvb_wr 
 *               uint8_t frame_nums  
 * @return       {uint8_t} WriteErr   写数据返回值
 * @author:      zlz
 */
int8_t MVB_RD_Data_Proc(TMS570_BRAM_DATA *bram_data_mvb_rd,TMS570_BRAM_DATA *bram_data_tms570_wr)
{
    
}