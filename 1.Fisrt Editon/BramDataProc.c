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
*REV1.0.0     feng    2018/05/16  Create
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
#include "BramDataProc.h"
/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/
extern DEBUG_TYPE_ENUM g_DebugType_EU;
extern BLVDS_BRD_DATA g_BrdRdBufData_ST;
extern BLVDS_BRD_DATA g_BrdBufData_ST;
extern uint32_t g_LinuxDebug;
extern uint32_t g_CAN_RDDate[64];
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
//20200325
extern EADS_ERROR_INFO  g_EADSErrInfo_ST;

/**********************************************************************
*
*Static Variable Define Section
*
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
    uint16_t PacketLength,CpLength;
    int8_t ErrorCode = 0;
    static uint32_t s_BramLife_U32[BRAM_BOARD_NUM] = {0};//Attention:此处我将BRAM_BOARD_NUM从5改为10，评估影响  
    BRAM_PACKET_DATA *BramPacketData_ST_p;
    BramPacketData_ST_p = (BRAM_PACKET_DATA *)Inbuff;
    PacketLength = (BramPacketData_ST_p -> BLVDSTOP_U32 >> 24) & 0xFF;    

	if(PacketLength != 0)
    {
        memcpy(Outbuff,&BramPacketData_ST_p -> BLVDSData_U32,PacketLength);           
    }
	//以CRC校验值作为生命信号，判断生命信号是否跳变
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
*Name           :  BramRDMAX10ADCData(uint32_t BlockAddr,uint32_t BlockFlagAddr,
*                   uint8_t ChanNum,uint32_t *ReadDataBuf)
*Function       :   Read the MAX ADC data
*Para           :   uint32_t BlockAddre , BRAM_A9_RD_A_BLVDS_BLOCK0
*               :   uint32_t BlockFlagAddr BRAM_A9_RD_A_BLVDS_FlagBLOCK0
*               :   uint8_t ChanNum  :the Block number
*               :   uint8_t U32Length : LCU_BOARD_LENGTH
*               :   uint32_t *ReadDataBuf,
*Return         :   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author         :   feng
*History:
*REV1.0.0     feng    2018/05/07  Create
*********************************************************************/
int8_t BoardDataRead(BRAM_ADDRS *BramAddrs_p,uint32_t *ReadData,uint8_t ChanNum)
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
    Error = ExtraBoardData(ReadDataBuf,ReadData,ChanNum);   
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

int8_t BramOpenMap(EADS_ERROR_INFO * EADSErrInfop)
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
int8_t Bram_Mapping_Init(EADS_ERROR_INFO *EADSErrInfop,uint8_t EADSType)  
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
        ReadErr = BoardDataRead(&s_Bram_A_BLVDSBlckAddr_ST,&g_BrdRdBufData_ST.Board0_Data_U32[ReadNum_U8][0],ADU_BOARD_ID);
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

/**
 * @description: 初始化TMS570交互的Bram通道包头数据(包含CAN及MVB数据)
 * @param:       void
 * @return:      void
 * @author:      zlz
 */
void TMS570_Bram_TopPackDataSetFun(void)
{
    /*MVB A9->570*/
    CmdPact_WR_ST[0].protocol_version =0x11c2;
	CmdPact_WR_ST[0].ChanNum_U8 = 8;
    CmdPact_WR_ST[0].PacktLength_U32 = 17;
    /*CAN-BMS A9->570*/
    CmdPact_WR_ST[1].protocol_version =0x11c2;
	CmdPact_WR_ST[1].ChanNum_U8 = 9;
    CmdPact_WR_ST[1].PacktLength_U32 = 15;
    /*CAN-DCDC A9->570*/
    CmdPact_WR_ST[2].protocol_version =0x11c2;
	CmdPact_WR_ST[2].ChanNum_U8 = 10;
    CmdPact_WR_ST[2].PacktLength_U32 = 8;
    /*CAN-FC A9->570*/
    CmdPact_WR_ST[3].protocol_version =0x11c2;
	CmdPact_WR_ST[3].ChanNum_U8 = 11;
    CmdPact_WR_ST[3].PacktLength_U32 = 18;
    /*CAN-扩展模块 A9->570*/
    CmdPact_WR_ST[4].protocol_version =0x11c2;
	CmdPact_WR_ST[4].ChanNum_U8 = 12;
    CmdPact_WR_ST[4].PacktLength_U32 = 8;
    /*MVB 570->A9*/
    CmdPact_RD_ST[0].ChanNum_U8 = 8;
    CmdPact_RD_ST[0].PacktLength_U32 = 48;
    /*CAN-BMS 570->A9*/
    CmdPact_RD_ST[1].ChanNum_U8 = 9;
    CmdPact_RD_ST[1].PacktLength_U32 = 7;
    /*CAN-DCDC 570->A9*/
    CmdPact_RD_ST[2].ChanNum_U8 = 10;
    CmdPact_RD_ST[2].PacktLength_U32 = 7;
    /*CAN-FC 570->A9*/
    CmdPact_RD_ST[3].ChanNum_U8 = 10;
    CmdPact_RD_ST[3].PacktLength_U32 = 8;
    /*CAN-扩展模块 570->A9*/
    CmdPact_RD_ST[4].ChanNum_U8 = 10;
    CmdPact_RD_ST[4].PacktLength_U32 = 8;
}

/**
 * @description: 从Bram指定区域读TMS570数据
 * @param {uint8_t} ReadNum_U8 
 * @return {*}
 * @author: zlz
 */
int8_t TMS570_Bram_ReadThreadFunc(CAN_FingerPrint_TPYE *CANReadDate) 
{

    int8_t ReadErr = 0,i;
    char loginfo[LOG_INFO_LENG] = {0};
    uint32_t ReadDate[CANDATE_LENGTH_U32-BRAM_PCKT_TOP_LNGTH_U32] = {0};
    uint8_t CANDataErrFlag = 0;
    uint8_t CANDataErrNum  = 0;
    
    //指除CRC外其他数据长度，按照32位计算
    s_bram_RD_TMS_SPC_Blck_ST.DataU32Length = CANDATE_LENGTH_U32;
    s_bram_RD_TMS_SPC_Blck_ST.ChanNum_U8 = CAN_CHAN;
    if(BRAM_DEBUG == g_DebugType_EU)
    {
        printf("Read CAN Bram:\n");

    }
    if(0 == g_LinuxDebug)
    {
        ReadErr = BoardDataRead(&s_bram_RD_TMS_SPC_Blck_ST,ReadDate,s_bram_RD_TMS_SPC_Blck_ST.ChanNum_U8);
        if(CODE_ERR == ReadErr) 
        {
            CANDataErrNum++;  
            if(CANDataErrNum > BRAMERR_NUM) 
            {
                if(0 == CANDataErrFlag)
                {                    
                    CANDataErrFlag = 1;
                    printf("CAN data read from TMS570 is Err.\n");
                    snprintf(loginfo, sizeof(loginfo)-1, "CAN data read from TMS570 is Err.");
                    WRITELOGFILE(LOG_ERROR_1,loginfo);
                }
                CANDataErrNum = 0;
            }
        }
        else if(CODE_OK == ReadErr) 
        {
            CANDataErrNum = 0;
            if(1 == CANDataErrFlag)
            {
                CANDataErrFlag = 0;
                printf("CAN data read from TMS570 return to normal.\n");
                snprintf(loginfo, sizeof(loginfo)-1, "CAN data read from TMS570 rerurn to normal.");
                WRITELOGFILE(LOG_ERROR_1,loginfo);   
            }                 
            CANReadDate ->Life_fingerprint_U8 = (uint8_t)((ReadDate[1]>>16) & 0XFF);
            CANReadDate ->CanDate_U8[0] = (uint8_t)(ReadDate[0] & 0XFF);
            CANReadDate ->CanDate_U8[1] = (uint8_t)((ReadDate[0] >>8) & 0XFF);
            CANReadDate ->CanDate_U8[2] = (uint8_t)((ReadDate[0] >>16) & 0XFF);
            CANReadDate ->CanDate_U8[3] = (uint8_t)((ReadDate[0] >>24) & 0XFF);
        }
        if(g_DebugType_EU == CAN_RD_DEBUG)
        {
            
            printf("CANReadDate ->Life_fingerprint_U8:%x\n", CANReadDate ->Life_fingerprint_U8);
            for(i=0;i<4;i++)
            {
                printf("CANReadDate ->CanDate_U8[%d]:%x\n", i,CANReadDate ->CanDate_U8[i]);
            }
        } 
    }
    return ReadErr;
}

/**
 * @description: 向Bram指定区域写CAN数据
 * @param        {uint8_t} ReadNum_U8  写数据区数量
 * @return       {uint8_t} WriteErr   写数据返回值
 * @author: zlz
 */
int8_t CANDataWriteThreadFunc(CAN_FingerPrint_TPYE *CANRecvDate,BRAM_CMD_PACKET *CmdPact_p) 
{

    int8_t WriteErr = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    uint8_t CANDataErrFlag = 0;
    uint8_t CANDataErrNum  = 0;
    uint32_t WriteDate[CANDATE_LENGTH_U32-BRAM_PCKT_TOP_LNGTH_U32] = {0} ;  
    static uint16_t CAN_Life = 0;  
    BRAM_PACKET_TOP TopPackST = {0};
    if(CAN_WR_DEBUG == g_DebugType_EU)
    {
        printf("Write CAN from Bram\n");

    }
    //填充帧头
    TopPackST.BLVDSTOP_U32 = CmdPact_p->protocol_version;
    TopPackST.BLVDSReser_U32[1] = CAN_Life;
    //数据长度、通道数填充
    s_bram_WR_TMS_SPC_Blck_ST.DataU32Length = CmdPact_p-> PacktLength_U32;
    s_bram_WR_TMS_SPC_Blck_ST.ChanNum_U8 = CmdPact_p -> ChanNum_U8 ;
    //CAN数据区填充          
    WriteDate[0] = ((CANRecvDate ->CanDate_U8[0] | (CANRecvDate ->CanDate_U8[1] <<8) | (CANRecvDate ->CanDate_U8[2] <<16)) &0XFFFFFF) ;
    WriteDate[1] = (CANRecvDate -> Life_fingerprint_U8 << 16);
    if(CAN_WR_DEBUG == g_DebugType_EU)
    {
        printf("WriteDate[0]:%08x\n",WriteDate[0]);
        printf("WriteDate[1]:%08x\n",WriteDate[1]);
    }    
    if(0 == g_LinuxDebug)
    {
       WriteErr = BramWriteWithChek(&s_bram_WR_TMS_SPC_Blck_ST,WriteDate,TopPackST);
	    if(WriteErr == CODE_ERR)
		 {
			 printf("BramWriteWithChek error\n");					 
		 }
    }
    CAN_Life++;
    return WriteErr;

}

