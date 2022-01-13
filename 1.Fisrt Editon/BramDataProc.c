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
extern uint16_t g_MVB_SendFrameNum;
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
static BRAM_ADDRS s_bram_RD_TMS_SPC_Blck_ST[5] = {0};
static BRAM_ADDRS s_bram_WR_TMS_SPC_Blck_ST[5] = {0};
static int8_t s_bram_MemFd_I8 = 0;
static uint8_t *s_bram_RDFlagAddr = NULL;
static uint8_t *s_bram_WRFlagAddr = NULL;
static uint8_t *s_bram_RD_A_BLVDSBlckAddr = NULL;
static uint8_t *s_bram_RD_TMS_SPCBlckAddr = NULL;
static uint8_t *s_bram_WR_TMS_SPCBlckAddr = NULL;
static uint8_t *s_bram_RD_B_BLVDSBlckAddr = NULL;
static uint8_t *s_bram_WR_B_BLVDSBlckAddr = NULL;

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
    
    PacketLength = ((BramPacketData_ST_p -> BLVDSTOP_U32 >> 24) & 0xFF)-12;

	if(PacketLength > 0)
    {
        /*copy all data excpet TopPack and crc32*/
        memcpy(Outbuff,&BramPacketData_ST_p->BLVDSData_U32,PacketLength);          
    }
    else if (PacketLength == 0)
    {
        printf("Receive Bram PakcetLength equal to 12 , it means frame didn't include any data!\n");
    }
    else
    {
        printf("Invalid Bram PakcetLength ! PakcetLength must be greater than 12!\n");
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
    Error = ExtraBoardData(ReadDataBuf, ReadData, BramAddrs_p->ChanNum_U8);
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
    for (i=0;i<5;i++)
    {
        s_bram_RD_TMS_SPC_Blck_ST[i].MapBlckAddr_p = s_bram_RD_TMS_SPCBlckAddr;
        s_bram_RD_TMS_SPC_Blck_ST[i].MapBlckFlgAddr_p = s_bram_RDFlagAddr;
        s_bram_RD_TMS_SPC_Blck_ST[i].BramBlckAddr = BRAM_A9_RD_A_TMS_SPCBLOCK0;
        s_bram_RD_TMS_SPC_Blck_ST[i].BramBlckFlgAddr = BRAM_A9_RD_A_TMS_SPCFLAG ; 
    } 

    for (i=0;i<5;i++)
    {
        /*TMS570专用 ADDR 地址映射 A9写FPGA读*/
        s_bram_WR_TMS_SPC_Blck_ST[i].MapBlckAddr_p = s_bram_WR_TMS_SPCBlckAddr;
        s_bram_WR_TMS_SPC_Blck_ST[i].MapBlckFlgAddr_p = s_bram_WRFlagAddr;
        s_bram_WR_TMS_SPC_Blck_ST[i].BramBlckAddr = BRAM_A9_WR_A_TMS_SPCBLOCK0;
        s_bram_WR_TMS_SPC_Blck_ST[i].BramBlckFlgAddr = BRAM_A9_WR_A_TMS_SPCFLAG;
    }
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
static uint8_t Life_Judge_Fun(uint8_t life_data,uint8_t *life_lasttime,uint8_t *errnum,uint8_t *judge_val)
{
    if(life_data == *life_lasttime)
    {
        (*errnum)++;    //Please add brackets at *errnum, ++ is higher priority than *-(fetch value from addr)
        if(*errnum >= 10)
        {   
            *errnum = 0;            
            *judge_val = 1;
             return 1;
        }             
    }
    else
    {
        *errnum = 0;
        *judge_val = 0;    
        *life_lasttime = life_data;                               
    }
    return 0;
}
/**
 * @description: CAN_Life_Judge (pay attention "static")
 * @param:       void
 * @return:      void
 * @author:      zlz
 */
static void CAN_Life_Judge(struct can_frame *candata,uint8_t *judge_val,uint8_t can_devtype)
{
    uint8_t i;
    static uint8_t errnum[6]= {0};    
    static uint8_t errflag[6] = {0};
    static uint8_t life_lasttime[6] = {0};  
    char loginfo[LOG_INFO_LENG] = {0};

    if (can_devtype == CAN0_TYPE)
    {
        for (i=0;i<CAN0_READ_FRAME_NUM;i++)
        {
            switch (candata[i].can_id & 0x1FFFFFFF)
            {
                case 0x1800D0F4 :                                           
                    Life_Judge_Fun(candata[i].data[6],&life_lasttime[0],&errnum[0],&judge_val[0]);
                    if(g_DebugType_EU == CAN_RD_DEBUG)
                    {
                        printf("errnum[0]:%u\n",errnum[0]);                       
                    }
                    if(judge_val[0] ==1 && errflag[0]==0)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "BMS-CAN_ID:[0x1800D0F4] lifesignal have stopped!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[0]=1;
                    }
                    else if(judge_val[0]==0 && errflag[0]==1)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "BMS-CAN_ID:[0x1800D0F4] lifesignal have recovered!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[0]=0;
                    }                    
                    break;
                case 0x16F4C001 :                                  
                    Life_Judge_Fun(candata[i].data[7],&life_lasttime[1],&errnum[1],&judge_val[1]);
                    
                    if(g_DebugType_EU == CAN_RD_DEBUG)
                    {
                        printf("errnum[1]:%u\n",errnum[1]); 
                    }
                    if(judge_val[1]==1 && errflag[1]==0)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "DCDC-CAN_ID::[0x16F4C001] lifesignal have stopped!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[1]=1;
                    }
                    else if(judge_val[1]==0 && errflag[1]==1)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "DCDC-CAN_ID:[0x16F4C001] lifesignal have recovered!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[1]=0;
                    }
                    break;
                case 0x18FF3012 :                    
                    Life_Judge_Fun(candata[i].data[0],&life_lasttime[2],&errnum[2],&judge_val[2]);
                    if(g_DebugType_EU == CAN_RD_DEBUG)
                    {
                        printf("errnum[2]:%u\n",errnum[2]);                     
                    }
                    if(judge_val[2]==1 && errflag[2]==0)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "FC-CAN_ID:[0x18FF3012] lifesignal have stopped!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[2]=1;
                    }                    
                    else if(judge_val[2]==0 && errflag[2]==1)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "FC-CAN_ID:[0x18FF3012] lifesignal have recovered!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[2]=0;
                    }
                    break;
                case 0x18FF3013 :                    
                    Life_Judge_Fun(candata[i].data[0],&life_lasttime[3],&errnum[3],&judge_val[3]);
                    if(g_DebugType_EU == CAN_RD_DEBUG)
                    {
                        printf("errnum[3]:%u\n",errnum[3]);                     
                    }
                    if(judge_val[3]==1 && errflag[3]==0)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "FC-CAN_ID:[0x18FF3013] lifesignal have stopped!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[3]=1;
                    }                    
                    else if(judge_val[3]==0 && errflag[3]==1)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "FC-CAN_ID:[0x18FF3013] lifesignal have recovered!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[3]=0;
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
            switch (candata[i].can_id & 0x1FFFFFFF)
            {                
                case 0x15003000 :                    
                    Life_Judge_Fun(candata[i].data[0],&life_lasttime[4],&errnum[4],&judge_val[4]);
                    if(g_DebugType_EU == CAN_RD_DEBUG)
                    {
                        printf("errnum[4]:%u\n",errnum[4]);                 
                    }                   
                    if(judge_val[4]==1 && errflag[4]==0)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "VVVF-CAN_ID:[0x15003000] lifesignal have stopped!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[4]=1;
                    }
                    else if(judge_val[4]==0 && errflag[4]==1)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "VVVF-CAN_ID:[0x15003000] lifesignal have recovered!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[4]=0;
                    }
                    break;                
                case 0x19003000 :                    
                    Life_Judge_Fun(candata[i].data[0],&life_lasttime[5],&errnum[5],&judge_val[5]);
                    if(g_DebugType_EU == CAN_RD_DEBUG)
                    {
                        printf("errnum[5]:%u\n",errnum[5]);                
                    } 
                    if(judge_val[5]==1 && errflag[5]==0)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "Extension-CAN_ID:[0x19003000] lifesignal have stopped!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[5]=1;
                    }
                    else if(judge_val[5]==0 && errflag[5]==1)
                    {
                        snprintf(loginfo, sizeof(loginfo)-1, "Extension-CAN_ID:[0x19003000] lifesignal have recovered!");
                        WRITELOGFILE(LOG_ERROR_1,loginfo);
                        errflag[5]=0;
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
        candata_RD_filter[0].can_id     = 0x1800D0F4 | CAN_EFF_FLAG;
        candata_RD_filter[0].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[1].can_id     = 0x1801D0F4 | CAN_EFF_FLAG;
        candata_RD_filter[1].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[2].can_id     = 0x1802D0F4 | CAN_EFF_FLAG;
        candata_RD_filter[2].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[3].can_id     = 0x1803D0F4 | CAN_EFF_FLAG;
        candata_RD_filter[3].can_mask   = CAN_EFF_MASK;
        /*CAN0-DCDC-A9 READ*/
        candata_RD_filter[4].can_id     = 0x16F4C000 | CAN_EFF_FLAG;
        candata_RD_filter[4].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[5].can_id     = 0x16F4C001 | CAN_EFF_FLAG;
        candata_RD_filter[5].can_mask   = CAN_EFF_MASK;
        /*CAN0-FC-A9 READ*/
        candata_RD_filter[6].can_id     = 0x18FF3012 | CAN_EFF_FLAG;
        candata_RD_filter[6].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[7].can_id     = 0x18FF3112 | CAN_EFF_FLAG;
        candata_RD_filter[7].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[8].can_id     = 0x18FF3212 | CAN_EFF_FLAG;
        candata_RD_filter[8].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[9].can_id     = 0x18FF3312 | CAN_EFF_FLAG;
        candata_RD_filter[9].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[10].can_id    = 0x18FF6012 | CAN_EFF_FLAG;
        candata_RD_filter[10].can_mask  = CAN_EFF_MASK;
        candata_RD_filter[11].can_id    = 0x18FF7012 | CAN_EFF_FLAG;
        candata_RD_filter[11].can_mask  = CAN_EFF_MASK;
        candata_RD_filter[12].can_id    = 0x18FF7112 | CAN_EFF_FLAG;
        candata_RD_filter[12].can_mask  = CAN_EFF_MASK;
        candata_RD_filter[13].can_id    = 0x18FF3013 | CAN_EFF_FLAG;
        candata_RD_filter[13].can_mask  = CAN_EFF_MASK;
        candata_RD_filter[14].can_id    = 0x18FF3113 | CAN_EFF_FLAG;
        candata_RD_filter[14].can_mask  = CAN_EFF_MASK;
        candata_RD_filter[15].can_id    = 0x18FF3213 | CAN_EFF_FLAG;
        candata_RD_filter[15].can_mask  = CAN_EFF_MASK;
        candata_RD_filter[16].can_id    = 0x18FF3313 | CAN_EFF_FLAG;
        candata_RD_filter[16].can_mask  = CAN_EFF_MASK;
        candata_RD_filter[17].can_id    = 0x18FF6013 | CAN_EFF_FLAG;
        candata_RD_filter[17].can_mask  = CAN_EFF_MASK;
        candata_RD_filter[18].can_id    = 0x18FF7013 | CAN_EFF_FLAG;
        candata_RD_filter[18].can_mask  = CAN_EFF_MASK;
        candata_RD_filter[19].can_id    = 0x18FF7113 | CAN_EFF_FLAG;
        candata_RD_filter[19].can_mask  = CAN_EFF_MASK;
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
        candata_RD_filter[3].can_id    = 0x15003003 | CAN_EFF_FLAG;
        candata_RD_filter[3].can_mask   = CAN_EFF_MASK;
        /*CAN1-扩展模块-A9 READ*/ 
        candata_RD_filter[4].can_id    = 0x19003000 | CAN_EFF_FLAG;
        candata_RD_filter[4].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[5].can_id    = 0x19003001 | CAN_EFF_FLAG;
        candata_RD_filter[5].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[6].can_id    = 0x19003002 | CAN_EFF_FLAG;
        candata_RD_filter[6].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[7].can_id    = 0x19003003 | CAN_EFF_FLAG;
        candata_RD_filter[7].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[8].can_id    = 0x19003004 | CAN_EFF_FLAG;
        candata_RD_filter[8].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[9].can_id    = 0x19003005 | CAN_EFF_FLAG;
        candata_RD_filter[9].can_mask   = CAN_EFF_MASK;
        candata_RD_filter[10].can_id    = 0x19003006 | CAN_EFF_FLAG;
        candata_RD_filter[10].can_mask   = CAN_EFF_MASK;
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
int8_t CAN_Write_Option(int8_t socket_fd,struct can_frame *can_frame_data,uint8_t frames_num,uint8_t dev_type)
{    
    uint8_t i,nbytes;
    static uint8_t errnum_wr=0;
    char loginfo[LOG_INFO_LENG]={0};

    for(i=0;i<frames_num;i++)
    {
        nbytes = write(socket_fd,&can_frame_data[i], sizeof(can_frame_data[i]));
        if(nbytes != sizeof(can_frame_data[i]))
        {                
            printf("CAN%d Send frame[%u] Error!\n",dev_type,can_frame_data[i].can_id);
            errnum_wr++;
            if(errnum_wr >=10)
            {
                snprintf(loginfo, sizeof(loginfo)-1, "CAN%d Send frame[%u] Error!",dev_type,can_frame_data[i].can_id);
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
int8_t CAN_Read_Option(int8_t socket_fd,struct can_frame *can_frame_data,uint8_t frames_num,uint8_t dev_type)
{
    struct timespec begin_ts,end_ts;  
    uint8_t i,j,nbytes,ret;
    static uint8_t errnum_rd = 0;
    uint8_t errnum_timeout=0;
    char loginfo[LOG_INFO_LENG]={0};
    fd_set rfds={0};
    struct timeval tv_select={0};    

    for(i=0;i<frames_num;i++)
    {            
        //clock_gettime(CLOCK_MONOTONIC,&begin_ts);
        tv_select.tv_usec = 5000;
        FD_ZERO(&rfds);
        FD_SET(socket_fd,&rfds);
        ret = select(socket_fd+1,&rfds,NULL,NULL,&tv_select);
        if(ret>0)
        {            
            nbytes = read(socket_fd,&can_frame_data[i],sizeof(can_frame_data[i]));            
            if(nbytes != sizeof(can_frame_data[i]))
            {                
                printf("CAN%d Receive Error frame[%d]!\n",dev_type,i);
                memset(can_frame_data[i].data,0,8);
                errnum_rd++;
                if(errnum_rd >=10)
                {
                    snprintf(loginfo, sizeof(loginfo)-1, "CAN%d receive frame_ID[%u] Error!",dev_type,can_frame_data[i].can_id);
                    WRITELOGFILE(LOG_ERROR_1,loginfo);
                    errnum_rd = 0;
                }                            
            }
            else
            {
                errnum_rd = 0;
            }
        }
        else
        {
            errnum_timeout++; 
        }

        if(i == frames_num-1 && errnum_timeout>0)
        {
            printf("Frames incomplete-CAN%d A readcycle lost %d frames!\n",dev_type,errnum_timeout);
            snprintf(loginfo, sizeof(loginfo)-1, "Frames incomplete-CAN%d A readcycle lost %d frames!",dev_type,errnum_timeout);
            WRITELOGFILE(LOG_ERROR_1,loginfo);
        }

        if(g_DebugType_EU == CAN_RD_DEBUG)
        {               
            {
                printf("Read CAN%d ID:0x%x:",dev_type,can_frame_data[i].can_id & 0x1FFFFFFF);
                for (j = 0; j < 8; j++)                    
                    printf("[%x]",can_frame_data[i].data[j]);
                printf("\n");
            }                                                 
        }
        //clock_gettime(CLOCK_MONOTONIC,&end_ts);            
    }
    return CODE_OK;    
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
 * @description: extensive_board anolog data process_can_frame_ID(0x19003003)
 * @param      : struct can_frame *candata_rd 
 *                 
 * @return     : 0
 * @author:    : zlz
 */
static void Anolog_Data_calculate_1(struct can_frame *candata_rd)
{
    uint16_t    temp_value_u16;    
    float       temp_calc_value_f;

    temp_value_u16 = candata_rd->data[3] << 8 | candata_rd->data[2];
    temp_calc_value_f = temp_value_u16 * 0.001; //Number->Current(mA)
    candata_rd->data[2] = (uint8_t)((temp_calc_value_f-4)*18.75); //S=60*(I-4)/16/0.2

    temp_value_u16 = candata_rd->data[5] << 8 | candata_rd->data[4];
    temp_calc_value_f = temp_value_u16 * 0.001; //Number->Current(mA)
    temp_value_u16 = (uint16_t)((temp_calc_value_f-4)*250); //S=(I-4)*250
    candata_rd->data[3] = temp_value_u16 & 0xFF;
    candata_rd->data[4] = (temp_value_u16>>8) & 0xFF;

    temp_value_u16 = candata_rd->data[7] << 8 | candata_rd->data[6];
    temp_calc_value_f = temp_value_u16 * 0.001;  //Number->Current(mA)
    candata_rd->data[5] = (uint16_t)(temp_calc_value_f*8.75-35); //S=(I-4)*250

    memset(&candata_rd->data[6],0,2); 
}
/**
 * @description: extensive_board anolog data process_can_frame_ID(0x19003004)
 * @param      : struct can_frame *candata_rd 
 *                 
 * @return     : 0 
 * @author:    : zlz
 */
static void Anolog_Data_calculate_2(struct can_frame *candata_rd)
{
    uint16_t    temp_value_u16;    
    float       temp_calc_value_f;
    
    temp_value_u16 = candata_rd->data[1] << 8 | candata_rd->data[0];
    temp_calc_value_f = temp_value_u16 * 0.001;  //Number->Current(mA)
    candata_rd->data[0] = (uint16_t)(temp_calc_value_f*8.75-35); //S=(I-4)*250 

    temp_value_u16 = candata_rd->data[3] << 8 | candata_rd->data[2];
    temp_calc_value_f = temp_value_u16 * 0.001;  //Number->Current(mA)
    candata_rd->data[1] = (uint16_t)(temp_calc_value_f*8.75-35); //S=(I-4)*250 

    temp_value_u16 = candata_rd->data[5] << 8 | candata_rd->data[4];
    temp_calc_value_f = temp_value_u16 * 0.001;  //Number->Current(mA)
    candata_rd->data[2] = (uint16_t)(temp_calc_value_f*8.75-35); //S=(I-4)*250 

    temp_value_u16 = candata_rd->data[7] << 8 | candata_rd->data[6];
    temp_calc_value_f = temp_value_u16 * 0.001; //Number->Current(mA)
    candata_rd->data[3] = (uint8_t)((temp_calc_value_f-4)/40); //S=(I-4)/40

    memset(&candata_rd->data[4],0,4);
              
}
/**
 * @description: extensive_board anolog data process_can_frame_ID(0x19003005)
 * @param      : struct can_frame *candata_rd 
 *                 
 * @return     : 0
 * @author:    : zlz
 */
static void Anolog_Data_calculate_3(struct can_frame *candata_rd)
{
    uint16_t    temp_value_u16;    
    float       temp_calc_value_f;

    temp_value_u16 = candata_rd->data[1] << 8 | candata_rd->data[0];
    temp_calc_value_f = temp_value_u16 * 0.001; //Number->Current(mA)
    candata_rd->data[0] = (uint8_t)((temp_calc_value_f-4)/40); //S=(I-4)/40

    temp_value_u16 = candata_rd->data[3] << 8 | candata_rd->data[2];
    temp_calc_value_f = temp_value_u16 * 0.001; //Number->Current(mA)
    candata_rd->data[1] = (uint8_t)(temp_calc_value_f*12.5-50); //S=(1.25I-5)/0.1

    temp_value_u16 = candata_rd->data[5] << 8 | candata_rd->data[4];
    temp_calc_value_f = temp_value_u16 * 0.001; //Number->Current(mA)
    candata_rd->data[2] = (uint8_t)(temp_calc_value_f*12.5-50); //S=(1.25I-5)/0.1

    temp_value_u16 = candata_rd->data[7] << 8 | candata_rd->data[6];
    candata_rd->data[3]  = (uint8_t)(temp_value_u16 / 4); //extionsive_board 1->0.001 transform to 1->0.004
 
    memset(&candata_rd->data[4],0,4);

}

/**
 * @description: process CAN read data
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
    uint8_t  temp_value_u8;
    uint16_t temp_value_u16;
    float temp_calc_value_f;
    static uint8_t life_judge_val[6]={0};
    static uint16_t VVVF_Exten_life_judge_val=0;
    static uint16_t FCU_AB_Stack_life_judge_val=0;

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
                    memcpy(&bramdata_wr[1].buffer[8],&life_judge_val[0],1);
                    break;
                case 0x16F4C000 :
                    memcpy(&bramdata_wr[2].buffer[0],candata_rd[i].data,8);
                    break;
                case 0x16F4C001 :
                    memcpy(&bramdata_wr[2].buffer[2],candata_rd[i].data,8);
                    memcpy(&bramdata_wr[2].buffer[4],&life_judge_val[1],1);
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
                    memcpy(&bramdata_wr[3].buffer[14],&life_judge_val[2],1);
                    break;
                default:
                    break;              
            }
        }
    }
    else if(can_devtype == CAN1_TYPE)
    {        
        VVVF_Exten_life_judge_val = life_judge_val[4]<<8 | life_judge_val[3];
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
                case 0x15003003 :
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
                    Anolog_Data_calculate_1(&candata_rd[i]);                    
                    memcpy(&bramdata_wr[4].buffer[12],candata_rd[i].data,8);
                    break;
                case 0x19003004 :
                    Anolog_Data_calculate_2(&candata_rd[i]);
                    memcpy(&bramdata_wr[4].buffer[14],candata_rd[i].data,8);
                    break;
                case 0x19003005 :
                    Anolog_Data_calculate_3(&candata_rd[i]);
                    memcpy(&bramdata_wr[4].buffer[16],candata_rd[i].data,8);
                    break;
                case 0x19003006 :
                    memcpy(&bramdata_wr[4].buffer[18],candata_rd[i].data,8);
                    memcpy(&bramdata_wr[4].buffer[20],&VVVF_Exten_life_judge_val,2);
                    break;
                default:
                    break;    
            }
        }            
    }    
}

/**
 * @description: Init TMS570 Bram Frame Toppacket
 * @param:       void
 * @return:      void
 * @author:      zlz
 */
void TMS570_Bram_TopPack_Set(BRAM_CMD_PACKET *cmd_packet_wr,BRAM_CMD_PACKET *cmd_packet_rd,uint8_t can_devtype)
{
    //Attention:数据区长度不包含CRC32，PacktLength_u32需要在总长度上-1
    switch (can_devtype)
    {
        case CAN0_TYPE:
            /*CAN-BMS A9->570*/
            cmd_packet_wr[0].protocol_version =0x11c2;
            cmd_packet_wr[0].ChanNum_U8 = 9;
            cmd_packet_wr[0].PacktLength_U32 = 12;
            /*CAN-DCDC A9->570*/
            cmd_packet_wr[1].protocol_version =0x11c2;
            cmd_packet_wr[1].ChanNum_U8 = 10;
            cmd_packet_wr[1].PacktLength_U32 = 8;
            /*CAN-FC A9->570*/
            cmd_packet_wr[2].protocol_version =0x11c2;
            cmd_packet_wr[2].ChanNum_U8 = 11;
            cmd_packet_wr[2].PacktLength_U32 = 32;
            /*CAN-BMS 570->A9*/
            cmd_packet_rd[0].ChanNum_U8 = 9;
            cmd_packet_rd[0].PacktLength_U32 = 6;
            /*CAN-DCDC 570->A9*/
            cmd_packet_rd[1].ChanNum_U8 = 10;
            cmd_packet_rd[1].PacktLength_U32 = 6;
            /*CAN-FC 570->A9*/
            cmd_packet_rd[2].ChanNum_U8 = 11;
            cmd_packet_rd[2].PacktLength_U32 = 7;
        break;
        case CAN1_TYPE:
        /*CAN-扩展模块 A9->570*/
            cmd_packet_wr->protocol_version =0x11c2;
            cmd_packet_wr->ChanNum_U8 = 12;
            cmd_packet_wr->PacktLength_U32 = 26;
            /*CAN-扩展模块 570->A9*/
            cmd_packet_rd->ChanNum_U8 = 12;
            cmd_packet_rd->PacktLength_U32 = 9;
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
int8_t TMS570_Bram_Read_Func(BRAM_CMD_PACKET *cmd_packet_rd,TMS570_BRAM_DATA bram_data[],uint8_t frame_num,TRANS_TYPE_ENUM tans_type)
{

    int8_t ReadErr = 0,i,j;
    uint8_t offset=0;
    char loginfo[LOG_INFO_LENG] = {0};       
    uint8_t DataErrFlag = 0;
    uint8_t DataErrNum  = 0;
    if (frame_num >32)
    {
        printf("Invalid number for TMS570 Read Bram frameNum!\n");
        return -1;
    }
        switch (tans_type)
    {
        case MVB_BRAM:
             offset = 0;
            break;
        case CAN0_BRAM:
             offset = 1;
            break;
        case CAN1_BRAM:
             offset = 4;
            break;    
        default:
            break;
    }
    
    for(i=0;i<frame_num;i++)
    {       
        s_bram_RD_TMS_SPC_Blck_ST[offset+i].DataU32Length = cmd_packet_rd[i].PacktLength_U32;
        s_bram_RD_TMS_SPC_Blck_ST[offset+i].ChanNum_U8 = cmd_packet_rd[i].ChanNum_U8;
       
        ReadErr = BoardDataRead(&s_bram_RD_TMS_SPC_Blck_ST[offset+i],bram_data[i].buffer);
        
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
int8_t TMS570_Bram_Write_Func(BRAM_CMD_PACKET *cmd_packet_wr,TMS570_BRAM_DATA *bram_data,uint8_t frame_num,TRANS_TYPE_ENUM tans_type) 
{
    int8_t i,j;
    uint8_t DataErrFlag = 0;
    uint8_t DataErrNum  = 0;
    uint8_t offset = 0 ;
    static int8_t WriteErr = 0;     
    static uint16_t Life_signal = 0;  
    BRAM_PACKET_TOP TopPackST[5] = {0};
    char loginfo[LOG_INFO_LENG] = {0};

    switch (tans_type)
    {
        case MVB_BRAM:
             offset = 0;
            break;
        case CAN0_BRAM:
             offset = 1;
            break;
        case CAN1_BRAM:
             offset = 4;
            break;    
        default:
            break;
    }

    if (frame_num >32)
    {
        printf("Invalid number for TMS570 Write Bram frameNum!\n");
        return -1;
    }

    for(i=0;i<frame_num;i++)
    {
        //填充帧头
        TopPackST[i].BLVDSTOP_U32 = cmd_packet_wr[i].protocol_version;
        TopPackST[i].BLVDSReser_U32[0] = Life_signal;
        //数据长度、通道数填充
        s_bram_WR_TMS_SPC_Blck_ST[i+offset].DataU32Length = cmd_packet_wr[i].PacktLength_U32;
        s_bram_WR_TMS_SPC_Blck_ST[i+offset].ChanNum_U8 = cmd_packet_wr[i].ChanNum_U8;

        if(TMS570_BRAM_WR_DEBUG == g_DebugType_EU)
        {
            for(j=0;j<25;j++)
                printf("TMS570:Write to Bram bram_data[%d][%02d]:0x%08u\n",i,j,bram_data[i].buffer[j]);
        }    
        if(0 == g_LinuxDebug)
        {
            WriteErr = BramWriteWithChek(&s_bram_WR_TMS_SPC_Blck_ST[i+offset],bram_data[i].buffer,TopPackST[i]);
            if(WriteErr == CODE_ERR)
            {
                printf("The [%d] frame Bramdata write to TMS570 error!\n",i);					 
            }
        }        
    }
    Life_signal++;
    return WriteErr;
}

/**
 * @description: MVB初始化
 * @param:      uint8_t mvb_rd_ch_num
 *              uint8_t mvb_wr_ch_num
 * @return:     InitErr
 * @author:     zlz
 */
void  MVB_Bram_Init(BRAM_CMD_PACKET *cmd_packet_rd,BRAM_CMD_PACKET *cmd_packet_wr,\
                        BRAM_CMD_PACKET *mvb_packet_rd,BRAM_CMD_PACKET *mvb_packet_wr)
{    
    uint8_t i;
    /*MVB A9->570*/
    cmd_packet_wr->protocol_version =0x11c2;
    cmd_packet_wr->ChanNum_U8 = 8;
    cmd_packet_wr->PacktLength_U32 = 16;
    /*MVB 570->A9*/
    cmd_packet_rd->ChanNum_U8 = 8;
    cmd_packet_rd->PacktLength_U32 = 47;   

    for (i=0;i<MVB_WRITE_FRAME_NUM;i++)
    {
        mvb_packet_wr[i].protocol_version =0x11c2;
	    mvb_packet_wr[i].ChanNum_U8 = i;
        mvb_packet_wr[i].PacktLength_U32 = 11;
    }
    for (i=0;i<MVB_READ_FRAME_NUM;i++)
    {        
	    mvb_packet_rd[i].ChanNum_U8 = i;
        mvb_packet_rd[i].PacktLength_U32 = 11;
    }   
}
/**
 * @description: 从Bram指定区域读MVB数据,并发送给TMS570
 * @param:      TMS570_BRAM_DATA *bram_data_mvb_rd
 *              uint8_t frame_nums
 * @return:     ReadErr
 * @author:     zlz
 */ 
int8_t  MVB_Bram_Read_Func(BRAM_CMD_PACKET *mvb_packet_rd,TMS570_BRAM_DATA *bram_data_mvb_rd)
{
    int8_t ReadErr = 0,i,j;
    char loginfo[LOG_INFO_LENG] = {0};       
    uint8_t DataErrFlag = 0;
    uint8_t DataErrNum  = 0;   

    for(i=0;i<MVB_READ_FRAME_NUM;i++)
    {        
        /*attention:if multi_thread use the s_bram variable,please use array to isolation */
        s_bram_RD_B_BLVDSBlckAddr_ST.DataU32Length =  mvb_packet_rd[i].PacktLength_U32;
        s_bram_RD_B_BLVDSBlckAddr_ST.ChanNum_U8 =  mvb_packet_rd[i].ChanNum_U8;
           
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
 * @description:  Write MVB Bram data to FPGA
 * @param      :  TMS570_BRAM_DATA *bram_data_mvb_wr 
 *             :  uint8_t frame_nums  
 * @return     :  {uint8_t} WriteErr 
 * @author:    :  zlz
 */
int8_t 	MVB_Bram_Write_Func(BRAM_CMD_PACKET *mvb_packet_wr,TMS570_BRAM_DATA *bram_data_mvb_wr)
{
    int8_t i,j;
    static int8_t WriteErr = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    uint8_t DataErrFlag = 0;
    uint8_t DataErrNum  = 0;     
    static uint16_t Life_signal = 0;  
    BRAM_PACKET_TOP TopPackST[16] = {0};   
    
    for(i=0;i<MVB_WRITE_FRAME_NUM;i++)
    {        
        TopPackST[i].BLVDSTOP_U32 = mvb_packet_wr[i].protocol_version;
        TopPackST[i].BLVDSReser_U32[0] = Life_signal | ((16+i)<<16);        
        /*attention:if multi_thread use the s_bram variable,please use array to isolation */
        s_bram_WR_B_BLVDSBlckAddr_ST.DataU32Length = mvb_packet_wr[i].PacktLength_U32;
        s_bram_WR_B_BLVDSBlckAddr_ST.ChanNum_U8 = mvb_packet_wr[i].ChanNum_U8;
        
        if(MVB_WR_DEBUG == g_DebugType_EU)
        {
            printf("NO:%d->mvb_channel_number:%d\n",i,mvb_packet_wr[i].ChanNum_U8);
            for(j=0;j<8;j++)
                printf("MVB:Write Bram_data[%d][%d]:0x%08x\n",i,j,bram_data_mvb_wr[i].buffer[j]);
        }    
        if(0 == g_LinuxDebug)
        {            
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
 * @description: tms570 bram readdata process to mvb writedata
 * @param:       TMS570_BRAM_DATA *bram_data_mvb_wr 
 *               uint8_t frame_nums  
 * @return       {uint8_t} WriteErr  
 * @author:      zlz
 */
int8_t MVB_WR_Data_Proc(TMS570_BRAM_DATA *bram_data_mvb_wr,TMS570_BRAM_DATA *bram_data_tms570_rd)
{
    memcpy(&bram_data_tms570_rd->buffer[0], bram_data_mvb_wr[0].buffer,16);/*MVB_0X3F8*/
    memcpy(&bram_data_tms570_rd->buffer[4], bram_data_mvb_wr[1].buffer,32);/*MVB_0X3F9*/
    memcpy(&bram_data_tms570_rd->buffer[12],bram_data_mvb_wr[2].buffer,32);/*MVB_0X3FA*/
    memcpy(&bram_data_tms570_rd->buffer[20],bram_data_mvb_wr[3].buffer,32);/*MVB_0X3FB*/
    memcpy(&bram_data_tms570_rd->buffer[28],bram_data_mvb_wr[4].buffer,32);/*MVB_0X3FC*/
    memcpy(&bram_data_tms570_rd->buffer[36],bram_data_mvb_wr[5].buffer,32);/*MVB_0X3FD*/

   /*memcpy(&bram_data_mvb_wr[5].buffer[6],&bram_data_tms570_rd->buffer[42],4);
    if(g_DebugType_EU == LCU_MVB_DEBUG)
        printf("MVB->LCU:%08x\n",bram_data_mvb_wr[5].buffer[6]);*/  
}

/**
 * @description:    mvb readdata process to tms570 bram writedata
 * @param      :    TMS570_BRAM_DATA *bram_data_mvb_wr 
 *                 
 * @return     :    void
 * @author     :    zlz
 */
int8_t MVB_RD_Data_Proc(TMS570_BRAM_DATA *bram_data_mvb_rd,TMS570_BRAM_DATA *bram_data_tms570_wr)
{    
    static uint8_t errnum,judge_val;
    static uint16_t frame_life_1;
    static uint16_t frame_life_2;

    if(frame_life_1==bram_data_mvb_rd[0].buffer[0]&0xffff | frame_life_2==bram_data_mvb_rd[1].buffer[0]&0xffff)
    {
        errnum++;
        if(errnum >= 10)
        {
            errnum = 0;            
            judge_val = 1;                
        } 
    }
    else
    {
        errnum = 0;
        judge_val = 0;
    }
    
    frame_life_1=bram_data_mvb_rd[0].buffer[0]&0xffff;
    frame_life_2=bram_data_mvb_rd[1].buffer[0]&0xffff;

    memcpy(bram_data_tms570_wr->buffer,bram_data_mvb_rd[0].buffer,32);/*MVB_0XA0*/
    memcpy(&bram_data_tms570_wr->buffer[8],bram_data_mvb_rd[1].buffer,16);/*MVB_0X3F0*/
    memcpy(&bram_data_tms570_wr->buffer[12],&judge_val,1);

    /*memcpy(&bram_data_tms570_wr->buffer[6],&bram_data_mvb_rd[0].buffer[6],8);
    if(g_DebugType_EU == LCU_MVB_DEBUG)
        printf("LCU->MVB:%08x\n",(bram_data_tms570_wr->buffer[6]>>8 | bram_data_tms570_wr->buffer[7] << 24)&0xffffffff);*/                                                
}

