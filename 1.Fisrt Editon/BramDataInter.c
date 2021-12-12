/**********************************************************************
*File name 		:   BramDataInter.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/3/29
*Description    :   Include Read Bram data,Write Bram Data

*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/03/29  Create
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
*Include file  sectiorn
*
*********************************************************************/
#include "BramDataInter.h"
 
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
/*contain the num when FPGA calculate CRC Err*/
static uint8_t s_bram_WriteCRCErrNum_U8 = 0;
static uint8_t s_bram_ReadCRCErrNum_U8 = 0;
/**********************************************************************
*
*Function Define Section
*
*********************************************************************/
/**********************************************************************
*Name           :   int8_t BramReadDataWithCRC(uint32_t *Inbuff)
*Function       :   CRC the read data from BRAM
*Para           :   uint32_t *Inbuff  
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/1/29  Create
*REV1.0.1     feng    2020/7/31 use crc as life
*REV1.0.2     feng    2020/8/26   only the  length is > 12, can read the PackeCrcValue_U32
*********************************************************************/
int8_t BramReadDataWithCRC(uint32_t * Inbuff)
{
    uint16_t PacketLength = 0;
    int8_t err = 0;
    uint8_t i =0;
    uint32_t TempCrcValue_U32 = 0,PackeCrcValue_U32 = 0,LifeNum_U32 = 0;
    
    BRAM_PACKET_DATA *BramPacketData_ST_p;
    BramPacketData_ST_p = (BRAM_PACKET_DATA *)Inbuff;
    PacketLength = (BramPacketData_ST_p -> BLVDSTOP_U32 >> 24) & 0xFF;//获取帧数据长度
    /*change to 32bit length*/
    PacketLength = PacketLength >> 2;
   //如果数据长度大于包头12字节长度
    if(PacketLength >= BRAM_PCKT_TOP_LNGTH_U32)
    {
        TempCrcValue_U32 = Crc32CalU32Bit(Inbuff,PacketLength);
        PackeCrcValue_U32 = (BramPacketData_ST_p -> BLVDSData_U32[PacketLength - BRAM_PCKT_TOP_LNGTH_U32]);
         /*use crc as life extradate时会判断*/
        BramPacketData_ST_p -> BLVDSReser_U32[0] = PackeCrcValue_U32;
        if(TempCrcValue_U32 == PackeCrcValue_U32)
        {
            err = CODE_OK;
        }
        else
        {
            if(BRAM_DEBUG == g_DebugType_EU)
            {
                printf("Bram read Crc failed\n");
                printf("TempCrcValue_U32 %08x,PackeCrcValue_U32 %08x \n",TempCrcValue_U32,PackeCrcValue_U32);    
            }
            err =  CODE_ERR;       
        }
    }
    else
    {        
        err =  CODE_WARN;       
    }
    if(BRAM_DEBUG == g_DebugType_EU)
    {
        printf("BramReadDataWithCRC:\n");
        if(PacketLength != 0)
        {
            for(i = 0; i < (PacketLength+1); i++)
            {
                printf("Bram Byte %u data %08x\n",i, *Inbuff); 
                Inbuff++; 
            }
        }
    }
    return err;
    
}
/**********************************************************************
*Name           :   int8_t BramBlockFlagWait
*Function       :   Wait the FPGA write Flag is clear.Then set the A9 read Block Flag
*Para           :   uint32_t mapbase        The mmap  Block Addres,
*               :   uint32_t BlockFlagAddre The Block Flag Addres,
*               :   uint16_t ChanNum        THe Channel number of Bram Block
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author         :   feng
*History:
*REV1.0.0     feng    2018/03/12  Create
*REV1.0.1     feng    2020/12/23  change i from uint8 to uint16_t
*********************************************************************/
int8_t BramBlockRDFlagWait(BRAM_ADDRS *BramAddrs_p)
{
    uint32_t ChanAddress = 0,ChanFlagAddr = 0,ChanFedFlagAddr = 0;
    uint8_t RDFlagInt = 0,RDFlagBit = 0;
    uint8_t RDFlagTemp = 0,RDFlagValue = 0;
    uint16_t WaitFlagClerNum = 0,i;
    struct timeval TimeStar,TimeEnd;
    uint8_t ChanNum = 0;
    ChanNum = BramAddrs_p -> ChanNum_U8;

    RDFlagInt = ChanNum / 8;
    RDFlagBit = ChanNum % 8;
    
    ChanFlagAddr = BramAddrs_p -> BramBlckFlgAddr + RDFlagInt;
    ChanFedFlagAddr = ChanFlagAddr + BRAM_PCKT_LNGTH_U8;

     /*Wait the 570 write flag clear*/ 
    WaitFlagClerNum = 500;

    for(i = 0;i < WaitFlagClerNum;i++)
    {           
        RDFlagValue = BramReadU8(ChanFlagAddr,BramAddrs_p -> MapBlckFlgAddr_p);
        RDFlagTemp = RDFlagValue >> RDFlagBit;
        RDFlagTemp = RDFlagTemp & 1; 
  
        if(RDFlagTemp == 0)
        {
            break;
        }
        if(i == 450)
        {
            return CODE_ERR;             
        }
      
    }
    /*Set the ReadFlag bit*/
    RDFlagTemp = (1 << RDFlagBit); 
    BramWriteU8(ChanFedFlagAddr,BramAddrs_p -> MapBlckFlgAddr_p,RDFlagTemp);
    return CODE_OK;
}
/**********************************************************************
*Name           :   int8_t BramBlockFlagClear
*Function       :   Clear the A9 Read flag
*Para           :   uint32_t mapbase    The mmap Block Addres,
*               :   uint32_t BlockFlagAddre The Block Flag Addres,
*               :   uint16_t ChanNum        THe Channel number of Bram Block
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author         :   feng
*History:
*REV1.0.0     feng    2018/03/12  Create
*********************************************************************/
int8_t BramBlockFlagClear(BRAM_ADDRS *BramAddrs_p)
{
    uint32_t ChanFlagAddr = 0,ChanFedFlagAddr = 0;
    uint8_t WRFlagInt = 0,WRFlagBit = 0;
    uint8_t WRFlagTemp = 0,i;
    struct timeval TimeStar,TimeEnd;
    uint8_t ChanNum = 0;

    ChanNum = BramAddrs_p -> ChanNum_U8;
    WRFlagInt = ChanNum / 8;
    WRFlagBit = ChanNum % 8;
    ChanFlagAddr = BramAddrs_p -> BramBlckFlgAddr + WRFlagInt;
    ChanFedFlagAddr = ChanFlagAddr + BRAM_PCKT_LNGTH_U8;   
    /*Clear the ReadFlag bit*/
    WRFlagTemp = 0 ;  
    BramWriteU8(ChanFedFlagAddr,BramAddrs_p -> MapBlckFlgAddr_p,WRFlagTemp);//1us 
    return CODE_OK;
}
/**********************************************************************
*Name           :   int8_t BramBlockReadOpti
*Function       :   Read one  Block data from Bram, The channel address is BlockAddre + 256 * ChanNum
*Para           :   uint32_t BaseAddr     The mmap Block Addres,
*               :   uint32_t BlockAddre   The Block Addres,
*               :   uint16_t ChanNum      THe Channel number of Bram Block
*               :   uint32_t *Outbuf      Point to data save array 
*               :   uint16_t Length       lenth of the read data,
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author         :   feng
*History:
*REV1.0.0     feng    2018/03/12  Create
*********************************************************************/
int8_t BramBlockReadOpti(BRAM_ADDRS *BramAddrs_p,uint32_t *Outbuf)
{
    uint32_t ChanAddress = 0;
    uint8_t i;
    int8_t CrcErr = 0;
    struct timeval TimeStar,TimeEnd;
    uint16_t RdLength = 0;
    uint8_t BlockNum = 0;
    uint8_t *BlockMapAddr_p;
    uint8_t ChanNum = 0;
    ChanNum = BramAddrs_p -> ChanNum_U8;   
    ChanAddress = BramAddrs_p -> BramBlckAddr + ChanNum * BRAM_PCKT_LNGTH_U8;
    
    if(BRAM_DEBUG == g_DebugType_EU)
    {
        printf("BramBlockReadOpti ChanAddress %x BaseAddr %x\n",(uint32_t)ChanAddress,(uint32_t)BramAddrs_p -> MapBlckAddr_p);
    } 

	RdLength = BramAddrs_p -> DataU32Length + BRAM_PCKT_CRC_LNGTH_U32;    
    BramPackReadU32(ChanAddress,BramAddrs_p -> MapBlckAddr_p,Outbuf,RdLength);       
    if(BRAM_DEBUG == g_DebugType_EU)
    {            
        printf("BramBlockReadOpti:\n");
        for( i = 0; i < RdLength; i++)
        {
            printf("Read Byte[%u]  data:%08x\n",i, Outbuf[i]);   
        }  

    } 
    /*Clear the ReadFlag bit*/
    CrcErr = BramReadDataWithCRC(Outbuf);
    /*CrcErr = 2,mean Crc error*/
    if(CODE_ERR == CrcErr)
    {
        s_bram_ReadCRCErrNum_U8++;          
    }
    else
    {
        s_bram_ReadCRCErrNum_U8 = 0;
    }
 
    return CrcErr;
}
/**********************************************************************
*Name           :   int8_t BramBlockWRFlagWait
*Function       :   Wait  the A9 Write Flag  is clear
*Para           :   uint32_t mapbase     The  mmap Block Addres,
*               :   uint32_t BlockFlagAddre The Block Flag Addres,
*               :   uint16_t ChanNum        THe Channel number of Bram Block
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author         :   feng
*History:
*REV1.0.0     feng    2018/03/12  Create
*REV1.0.1     feng    2020/12/23  change i from uint8 to uint16_t

*********************************************************************/
int8_t BramBlockWRFlagWait(BRAM_ADDRS *BramAddrs_p)
{
    uint32_t ChanAddress = 0,ChanFlagAddr = 0,ChanFedFlagAddr = 0;
    uint8_t WRFlagInt = 0,WRFlagBit = 0;
    uint8_t WRFlagTemp = 0,WRFlagValue = 0;
    uint16_t WaitFlagClerNum = 0,i;
    struct timeval TimeStar,TimeEnd;
    uint8_t ChanNum = 0;
    ChanNum = BramAddrs_p -> ChanNum_U8;
    WRFlagInt = ChanNum / 8;//字节
    WRFlagBit = ChanNum % 8;//位
    ChanFlagAddr = BramAddrs_p -> BramBlckFlgAddr + WRFlagInt;//会进行对齐4K取余数，作为页偏移附加到虚拟映射地址
     /*Wait the 570 write flag clear*/ 
    WaitFlagClerNum = 500;
    for(i = 0;i < WaitFlagClerNum;i++)
    {           
        WRFlagValue = BramReadU8(ChanFlagAddr,BramAddrs_p -> MapBlckFlgAddr_p);
        WRFlagTemp = WRFlagValue >> WRFlagBit;
        WRFlagTemp = WRFlagTemp & 1; 
        if(BRAM_DEBUG  == g_DebugType_EU)
        {
           printf("NO:%u-Read WRFlag:%02x\n",i,WRFlagValue);   
        }   
        if(WRFlagTemp == 0)//代表570 write flag clear，A9可以发送数据
        {
            break;
        }
        if(i == 450)//超过450次，返回错误
        {
            return CODE_ERR;             
        }      
    }
    return CODE_OK;
}
/**********************************************************************
*Name           :   int8_t BramBlockFlagCrc
*Function       :   1,Set the A9 write Flag
*               :   2,wait A9 write flag clear
*               :   3.Read the FPGA Crc Flag 
*Para           :   uint32_t mapbase     The mmap Block Addres,
*               :   uint32_t BlockFlagAddre The Block Flag Addres,
*               :   uint16_t ChanNum        THe Channel number of Bram Block
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author         :   feng
*History:
*REV1.0.0     feng    2018/03/12  Create
*REV1.0.1     feng    2020/12/23  change i from uint8 to uint16_t

*********************************************************************/
int8_t BramBlockFlagCrc(BRAM_ADDRS *BramAddrs_p)
{
    uint32_t ChanFlagAddr = 0,ChanFedFlagAddr = 0;
    uint8_t WRFlagInt = 0,WRFlagBit = 0;
    uint8_t WRFlagTemp = 0,WRFlagValue;
    struct timeval TimeStar,TimeEnd;
    uint16_t WaitFlagClerNum = 0,i;
    uint8_t FPGACrcErr = 0,FPGACrcErTemp = 0;
    int8_t ErrorCode = 0;
    uint8_t ChanNum = 0;
    ChanNum = BramAddrs_p -> ChanNum_U8;

    WRFlagInt = ChanNum / 8;
    WRFlagBit = ChanNum % 8;
    ChanFlagAddr = BramAddrs_p -> BramBlckFlgAddr + WRFlagInt;
    ChanFedFlagAddr = ChanFlagAddr + BRAM_PCKT_LNGTH_U8;
     /*Set the A9 write flag */ 
    WRFlagTemp = (1 << WRFlagBit);
    BramWriteU8(ChanFlagAddr,BramAddrs_p -> MapBlckFlgAddr_p,WRFlagTemp);
    if(BRAM_DEBUG == g_DebugType_EU)
    {
        printf("Write ChanFlagddress %x BaseAddr %x\n",(uint32_t)ChanFlagAddr,(uint32_t)BramAddrs_p -> MapBlckFlgAddr_p);
    }
    /*Wait FPGA clear the A9 write flag*/ 
    WaitFlagClerNum = 500;
    for(i = 0;i < WaitFlagClerNum;i++)
    {           
        WRFlagValue = BramReadU8(ChanFlagAddr,BramAddrs_p -> MapBlckFlgAddr_p);
        WRFlagTemp = WRFlagValue >> WRFlagBit;
        WRFlagTemp = WRFlagTemp & 1;   
        if(WRFlagTemp == 0)
        { 
            break;
        }
        if(i == 450)
        {
            return CODE_ERR;             
        }
      
    }

     /*Read the  FPGA crc error */ 
    FPGACrcErr = BramReadU8(ChanFedFlagAddr,BramAddrs_p -> MapBlckFlgAddr_p);
    if(BRAM_DEBUG == g_DebugType_EU)
    {
        printf("the FPGACrcErr 0x%02x\n",FPGACrcErr);
    }
    
    FPGACrcErTemp = FPGACrcErr >> WRFlagBit;
    FPGACrcErTemp = FPGACrcErTemp & 1; 
    /*CRC error*/
    if(FPGACrcErTemp != 0)
    {
        s_bram_WriteCRCErrNum_U8 ++;
        ErrorCode = -1;
    }
    /*CRC Right*/
    else
    {
        s_bram_WriteCRCErrNum_U8 = 0;
        ErrorCode = 0;
    }   
    return ErrorCode;
}
/**********************************************************************
*Name           :   int8_t BramBlockWriteOpti
*Function       :   Write one  Channel  data to Bram, The channel address is BlockAddre + 256 * ChanNum
*Para           :   uint32_t BaseAddr     The mmap Block Addres,
*               :   uint32_t BlockAddre   The Block Addres,
*               :   uint16_t ChanNum      THe Channel number of Bram Block
*               :   uint32_t *Outbuf      Point to write data
*               :   uint16_t Length       lenth of the read data,
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author         :   feng
*History:
*REV1.0.0     feng    2018/03/12  Create
*********************************************************************/
int8_t BramBlockWriteOpti(BRAM_ADDRS *BramAddrs_p,uint32_t *Inbuf)
{
    uint32_t ChanAddress = 0;
    uint8_t i;
    int8_t CrcErr = 0;
    uint16_t WrLength;
    struct timeval TimeStar,TimeEnd;
    uint32_t RdBuf[BRAM_PCKT_LNGTH_U32] = {0};
    uint8_t ChanNum = 0;

    ChanNum = BramAddrs_p -> ChanNum_U8;
    ChanAddress = BramAddrs_p -> BramBlckAddr + ChanNum * BRAM_PCKT_LNGTH_U8;
    if(BRAM_DEBUG == g_DebugType_EU)
    {
        printf("Write ChanAddress %x BaseAddr %x\n",ChanAddress,(uint32_t)BramAddrs_p -> MapBlckAddr_p);
    }
    // /*read the 570 feed back data 16Byte*/ 
	WrLength = BramAddrs_p -> DataU32Length + BRAM_PCKT_CRC_LNGTH_U32;
    BramPackWriteU32(ChanAddress,BramAddrs_p -> MapBlckAddr_p,Inbuf,WrLength);
    if(BRAM_DEBUG == g_DebugType_EU)
    {    
        BramPackReadU32(ChanAddress,BramAddrs_p -> MapBlckAddr_p,RdBuf,WrLength);
        printf("write bram and readback\n");
        for( i = 0; i < WrLength; i++)
        {
            printf("Write Byte %u data %08x\n",i, RdBuf[i]);   
        }  

    }  
    return CODE_OK;
}

/**********************************************************************
*Name           :   int8_t BramBlockWrite
*Function       :   Write one  Channel  data to Bram, The channel address is BlockAddre + 256 * ChanNum
*Para           :   uint32_t BlockAddre     The Block Addres,
*               :   uint32_t BlockFlagAddre The Block Flag Addres,
*               :   uint16_t ChanNum        THe Channel number of Bram Block
*               :   uint32_t *Outbuf        Point to data save array 
*               :   uint16_t Length         lenth of the read data,
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author         :   feng
*History:
*REV1.0.0     feng    2018/03/12  Create
*********************************************************************/
int8_t BramBlockWrite(BRAM_ADDRS *BramAddrs_p,uint32_t *Inbuf)
{
    int8_t ErrorRet = 0;  
    ErrorRet = BramBlockWRFlagWait(BramAddrs_p);
    if( ErrorRet == -1)
    {
        //FIXME:后续这句打印应该恢复
        //printf("waiting WRflag clear error\n");
        return ErrorRet; 
    }

    BramBlockWriteOpti(BramAddrs_p,Inbuf);

    BramBlockFlagCrc(BramAddrs_p);
    return CODE_OK;
}

/**********************************************************************
*Name           :   BramBlockRead
*Function       :   Block data Read
*Para           :   (BRAM_ADDRS *BramAddrs_p,uint32_t *Outbuf)
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author         :   feng
*History:
*REV1.0.0     feng    2018/03/12  Create
*********************************************************************/
int8_t BramBlockRead(BRAM_ADDRS *BramAddrs_p,uint32_t *Outbuf)
{
    int8_t ErrorRet = 0; 
    if(BRAM_DEBUG == g_DebugType_EU)
    {
        printf("BramBLVdsRead MapBlckFlagAddre %x MapBlockAddr %x\n",(uint32_t)BramAddrs_p -> MapBlckFlgAddr_p,(uint32_t)BramAddrs_p -> MapBlckAddr_p);
    } 

	//1st,wait the FPGA write flag clear
	//2nd,set the A9 read flag
    ErrorRet = BramBlockRDFlagWait(BramAddrs_p);
    if(CODE_ERR ==  ErrorRet)
    {
        printf("wait Flag time out error\n");
        
    }
	//3rd,read the data
    ErrorRet = BramBlockReadOpti(BramAddrs_p,Outbuf);
    //4th,clear the A9 read flag
    BramBlockFlagClear(BramAddrs_p);

    return ErrorRet;
}
/**********************************************************************
*Name           :   int8_t BramReadWithChek
*Function       :   Read  Bram data with proce the read data crc error.When crc errror,it will repeat read 3 times
*Para           :   uint32_t BlockAddre     The Block Addres,
*               :   uint32_t BlockFlagAddre The Block Flag Addres,
*               :   uint16_t ChanNum        THe Channel number of Bram Block
*               :   uint32_t *Outbuf        Point to data save array 
*               :   uint16_t Length         lenth of the read data,
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*History:
*REV1.0.0     feng    2018/4/1  Create
*********************************************************************/
int8_t BramReadWithChek(BRAM_ADDRS *BramAddrs_p,uint32_t *Outbuf)
{
    
    int8_t ErrorCode = 0;
    char loginfo[LOG_INFO_LENG] = {0};
    ErrorCode = BramBlockRead(BramAddrs_p,Outbuf);
    if(s_bram_ReadCRCErrNum_U8 > 0)
    {
        /*FPGA Crc error reach 3times,return*/
        /*if crc right ,will clear in BramBlockReadOpti*/
        while((s_bram_ReadCRCErrNum_U8 > 0) && (s_bram_ReadCRCErrNum_U8 < RD_CRCERR_NUM))
        {
            /*Repeat Write Data to Bram*/
            /*Now often happen crc err ,and repeate 1st read is OK.should know the detail of mmap*/
            if(DEVC_DEBUG == g_DebugType_EU)
            {
               printf("repeate read %u for crc error\n",s_bram_ReadCRCErrNum_U8);                
            }
            ErrorCode = BramBlockRead(BramAddrs_p,Outbuf);

        }  
        if(s_bram_ReadCRCErrNum_U8 >= RD_CRCERR_NUM)/*here ==,s_bram_ReadCRCErrNum_U8 > RD_CRCERR_NUM*/
        {
            s_bram_ReadCRCErrNum_U8 = 0;
            printf("Bram Read crc error\n");
            snprintf(loginfo, sizeof(loginfo)-1, "Bram Read crc error");
            WRITELOGFILE(LOG_ERROR_1,loginfo);
            ErrorCode = CODE_ERR;
        }
    }
    return ErrorCode;
}
/**********************************************************************
*Name           : BramWrDataSet
*                  uint16_t ChanNum,uint32_t *Outbuf ,uint16_t Length)
*Function       :   Write a packet data to Bram,if The FPGA feed back the crc is error,it will repeat write 3 times
*Para           :   uint32_t BlockAddre     The Block Addres,
*               :   uint32_t BlockFlagAddre The Block Flag Addres,
*               :   uint16_t ChanNum        THe Channel number of Bram Block
*               :   uint32_t *Outbuf       Point to data save array 
*               :   uint16_t Length         lenth of the read data,
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/3/29  Create
*
*********************************************************************/
int8_t BramWrDataSet(BRAM_ADDRS *BramAddrs_p,uint32_t Inbuf[],BRAM_PACKET_TOP TopPackST,uint32_t Outbuf[])
{
    BRAM_PACKET_DATA *BramPacketData_ST_p = {0};
	BramPacketData_ST_p = (BRAM_PACKET_DATA *)Outbuf;
	
	uint8_t Framelen = 0;
	uint8_t DataLen = 0;
	uint32_t Temp32Value = 0;
	uint8_t ChanNum = 0;
	uint32_t TempCrcValue_U32;
	uint8_t i = 0;

	/*设置包头*/
	ChanNum = BramAddrs_p -> ChanNum_U8;            //占用的Bram通道
	Temp32Value = TopPackST.BLVDSTOP_U32;           //已包含协议版本
	Temp32Value += (ChanNum << 18);                 //将通道写入Byte[1]-bit[7:2]位置
	Framelen = BramAddrs_p -> DataU32Length << 2;   //8位数据长度 == 32位数据长度*4
	Temp32Value += (Framelen << 24);                //将数据长度写入Byte[3]-bit[7:0]位置
	BramPacketData_ST_p -> BLVDSTOP_U32 = Temp32Value;//将包头Byte[3]-Byte[0]写入BramPacketData_ST_p -> BLVDSTOP_U32
	BramPacketData_ST_p -> BLVDSReser_U32[0] = TopPackST.BLVDSReser_U32[0];//已包含生命信号-Byte[4]-Byte[5]
    BramPacketData_ST_p -> BLVDSReser_U32[1] = TopPackST.BLVDSReser_U32[1];
	//DataU32Length*4包含了包头12字节，cpy数据时需要去掉包头长度
	DataLen = Framelen - BRAM_PACKET_TOP_LENGTH_U8; 
	memcpy(&BramPacketData_ST_p -> BLVDSData_U32,Inbuf,DataLen); 
	Framelen = BramAddrs_p -> DataU32Length; //U32 Lenth
	TempCrcValue_U32 = Crc32CalU32Bit(Outbuf,Framelen);//计算CRC32值
	//将CRC32校验值放置BramPacketData_ST_p -> BLVDSData_U32[DataLen]中
	DataLen = BramAddrs_p -> DataU32Length - BRAM_PCKT_TOP_LNGTH_U32;
	BramPacketData_ST_p -> BLVDSData_U32[DataLen]= TempCrcValue_U32;
 
	if(BRAM_DEBUG == g_DebugType_EU)
	{
		for( i = 0; i < Framelen; i++)
		{
			printf("Bram_DataWrite_U32 %d data %08x\n",i,Outbuf[i]);   
		} 

	}
	return 0;
}

/**********************************************************************
*Name           :  int8_t BramWriteWithChek(uint32_t BlockAddre,uint32_t BlockFlagAddre,
*                  uint16_t ChanNum,uint32_t *Outbuf ,uint16_t Length)
*Function       :   Write a packet data to Bram,if The FPGA feed back the crc is error,it will repeat write 3 times
*Para           :   uint32_t BlockAddre     The Block Addres,
*               :   uint32_t BlockFlagAddre The Block Flag Addres,
*               :   uint16_t ChanNum        THe Channel number of Bram Block
*               :   uint32_t *Outbuf       Point to data save array 
*               :   uint16_t Length         lenth of the read data,
*Return         :   int8_t 0,success;-1 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/3/29  Create
*
*********************************************************************/
int8_t BramWriteWithChek(BRAM_ADDRS *BramAddrs_p,uint32_t Inbuf[],BRAM_PACKET_TOP TopPackST)
{

	uint32_t WriteDataBuf[64] = {0};
	int8_t ErrorCode = 0;

	BramWrDataSet(BramAddrs_p,Inbuf,TopPackST,WriteDataBuf);    
    BramBlockWrite(BramAddrs_p,WriteDataBuf);
    if(s_bram_WriteCRCErrNum_U8 > 0)
    {
        /*FPGA Crc error reach 3times,return error*/
        /*if write success ,the s_bram_WriteCRCErrNum_U8 will be clear*/
        while((0 < s_bram_WriteCRCErrNum_U8) && (s_bram_WriteCRCErrNum_U8 < WR_CRCERR_NUM))
        {
            /*Repeat Write Data to Bram*/
            printf("The %u times Repeat Write Bram\n",s_bram_WriteCRCErrNum_U8);
            BramBlockWrite(BramAddrs_p,WriteDataBuf);
        }
        /*write failure*/
        if(WR_CRCERR_NUM == s_bram_WriteCRCErrNum_U8)
        {
            s_bram_WriteCRCErrNum_U8 = 0;
            printf("Write Bram Failure\n");
            ErrorCode = -1;
        }
    }
    else
    {
        //printf("Write Bram Right");
        ErrorCode = 0;
    }
    return ErrorCode;
}


