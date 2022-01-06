/**********************************************************************
*File name 		:   Socket.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/5/3
*Description    :   Include Socket Communicate
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
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
#include "Socket.h"
 
/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/

extern DEBUG_TYPE_ENUM g_DebugType_EU;
/**************** ******************************************************
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
static uint8_t s_RealWaveSource = 0;

/**********************************************************************
*
*Function Define Section
*
*********************************************************************/
/**********************************************************************
*Name			:   int8_t RealTimeWaveExtr
*Function       :   Extra the info of real wav request 
*Para     		:   
*
*Return    		:   uint8_t 1,success;0 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History: 		please refer to 《Driver实时波形解析协议》
*REV1.0.0     feng    2018/5/7  Create
*********************************************************************/

int8_t RealTimeWaveExtr(uint8_t *BufferIn,uint8_t *RecorChanNum,uint8_t *ChanLenth)
{

    uint8_t RecTotalNum = 0;
    uint16_t ChanBitTem = 0;
    PTU_REAL_WAVE_SET *RealTimeWave_p;
    RealTimeWave_p = (PTU_REAL_WAVE_SET *)BufferIn;
    uint8_t i = 0,j = 0;
    //big end
	if((RealTimeWave_p ->  BufferTop[0] == 0xA5)&&(RealTimeWave_p -> BufferTop[1] == 0x5A))
	{
        if(RealTimeWave_p -> MonitSourc[1] == 0)//monitor source 1 
        {        	
        	s_RealWaveSource = 1;
        }
        else if(RealTimeWave_p -> MonitSourc[1] == 1)//monitor source 2
        {        	
        	s_RealWaveSource = 2;
        }
        else
        {
        	s_RealWaveSource = 0;
        }
        for(i=0;i<3;i++)
        {
            ChanBitTem = (RealTimeWave_p -> ChanBit[2*i]<< 8) + RealTimeWave_p -> ChanBit[2*i+1];
            for(j=0;j<16;j++)
            {               
                //C code is start from channel 0,but Drive is start from channel 1
                if((ChanBitTem >> j)&1)
                {
                	*RecorChanNum = j+i*16; 
                	RecorChanNum++;
                	RecTotalNum++;
                } 
            }
        }
        printf("RecTotalNum %u\n",RecTotalNum);        
	}
	else
	{
		printf("drive tcp error\n");
	}
	*ChanLenth = RecTotalNum;

}
/**********************************************************************
*Name			:   int8_t RealWaveData
*Function       :   set the ChanData to  RealWave  for socket send
*Para     		:   
*
*Return    		:   uint8_t 1,success;0 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/5/7  Create
*********************************************************************/
int8_t RealWaveData(uint8_t BufferIn[],uint8_t RecorChanNum[],DRIVE_FILE_DATA *ChanData_ST_p,uint8_t ChanLenth)
{     
    uint8_t ChanNameTemp = 0;
    uint8_t i = 0,j = 0;
    //send 1-40Channel
    if(s_RealWaveSource == 1) 
	{ 		
		for(i=0;i<ChanLenth;i++)
		{
		    ChanNameTemp = RecorChanNum[i];		    
		    j = i << 1;
		    if(ChanNameTemp < MAX_RealWave_Num)
		    {
		    	BufferIn[j] = ChanData_ST_p->DriveAnalog_U16[ChanNameTemp] & 0xFF;
				BufferIn[j+1] = (ChanData_ST_p->DriveAnalog_U16[ChanNameTemp]>>8) & 0xFF;			
		    }
		}  	
	}
	//send 41-80Channel
	else if(s_RealWaveSource == 2) 
	{
		for(i=0;i<ChanLenth;i++)
		{
		    ChanNameTemp = RecorChanNum[i];
		    j = i << 1;
		    if(ChanNameTemp < MAX_RealWave_Num)
		    {
		    	BufferIn[j] = ChanData_ST_p->DriveAnalog_U16[40+ChanNameTemp]&0xFF;
				BufferIn[j+1] = (ChanData_ST_p->DriveAnalog_U16[40+ChanNameTemp]>>8)&0xFF;
		    }	
		} 			
	}
   return CODE_OK;
}
