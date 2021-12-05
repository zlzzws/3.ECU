/**********************************************************************
*File name 		:   BramDataProc.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/5/16
*Description    :   Include process the ADC data of ADE7913
                    1.Trans the ADE7913 binary data to Voltage and Current
                    2.Process the Voltage using chan calibrate data

*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/05/16  Create
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
#include "ADCDataProc.h"
 
/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/

extern DEBUG_TYPE_ENUM g_DebugType_EU;

extern CHAN_CALIB_DATA  g_ChanCalib110VData_ST;/*read the ChanCalibData from /tffs0/ChanCalib10VValue.dat"*/

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
*Name           :   int8_t ADC7913IDataTranInt
*Function       :   change the Current channel Binary data of Bram to Decinal data
*Para           :    uint32_t *BramInBuf,  the Binary data
*                   CHAN_DATA *g_ChanData_ST_p            the  Decinal data
*                   uint8_t TranType (not use): 0 is Tran with Calib Value;1 is Tran without Calib Value for Calib Data Save
*                   
*Return         :   uint8_t 1,success;0 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2020/1/29  Create
*REV1.0.1     feng    2020/6/19  Add the offset value
*REV1.0.2     zlz     2021/9/19  硬件变为AD7616
*********************************************************************/
int8_t ADC7913IDataTranInt(uint32_t *BramInBuf,CHAN_DATA *OutBuf,uint8_t TranLeng,uint8_t TranType)
{
    uint16_t i=0,j=0;
    int16_t VoltageTemBin_H = 0,VoltageTemBin_L = 0;
	int16_t CurrTemBin_H = 0,CurrTemBin_L = 0; //the AD data with 16bit   
    float   CurrTem_L = 0,CurrTem_H = 0;//the AD data After Tran
    int16_t CurrIntTem_H = 0,CurrIntTem_L = 0;   
    uint32_t Inbuf;
    uint8_t VolIntFlag = 0;

    for(i = 0;i <TranLeng/2;i++)
    {
		j=15-i;
		Inbuf = *BramInBuf;
        
        CurrTemBin_L = (int16_t)(Inbuf & 0xFFFF);
        CurrTemBin_H = (int16_t)(Inbuf >> 16) & 0xFFFF;
        
        /*Accord 7616 */
        if(ADC_IDATE == g_DebugType_EU)
        {
            printf("CurrCH:%u CurrTemBin_L:%x\n",i, CurrTemBin_L);
			printf("CurrCH:%u CurrTemBin_H:%x\n",j, CurrTemBin_H);
        }
        //单位是uV，以2.5V为基准
        CurrTem_L = (float)CurrTemBin_L * CURR_HALL_BINTAN_FACTOR -2500000 ; 
		CurrTem_H = (float)CurrTemBin_H * CURR_HALL_BINTAN_FACTOR -2500000 ;
        //0.1V = 1A
        CurrTem_H *= CURR_REALTAN_FACTOR;
	    CurrTem_L *= CURR_REALTAN_FACTOR; 
        //uA 转为 mA
        CurrIntTem_L =(int16_t)(CurrTem_L /= 1000);
		CurrIntTem_H =(int16_t)(CurrTem_H /= 1000);
        //for the voltage in circuit is  opposite the measure value
        OutBuf -> CurrChan_ST[i].CH_Current_I16 = CurrIntTem_L;
        if(j<14)
        {
            OutBuf -> CurrChan_ST[j].CH_Current_I16 = CurrIntTem_H;
        }
        else//丢弃OutBuf -> CurrChan_ST[15]-[16]电流数据
        {

        }	
       if(ADC_IDATE == g_DebugType_EU)
        {
            printf("CurrCH:%u OutBuf:%dmA\n",i, OutBuf -> CurrChan_ST[i].CH_Current_I16);
            if(j<14)
            {
                printf("CurrCH:%u OutBuf:%dmA\n",j, OutBuf -> CurrChan_ST[j].CH_Current_I16);
            }			
        }
        BramInBuf++;
    }
    return CODE_OK;
}
/**********************************************************************
*Name           :   int8_t ADC7913VDataTranInt
*Function       :   change the Voltage Channel Binary data of Bram to Decinal data 将Bram的数字量转换为实际电压
*Para           :   uint32_t *BramInBuf  
*                   CHAN_DATA *OutBuf  
                    CHAN_CALIB_DATA * ChanCalib_ST_p, the Calib file data
*                   uint16_t TranLeng         
*                   uint8_t TranType: 0 is Tran with Calib Value;1 is Tran without Calib Value for Calib Data Save
*                                     
*Return         :   uint8_t 1,success;0 false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/1/29  Create
*REV1.0.1     feng    2020/7/31  add  0V,filt
*********************************************************************/
int8_t ADC7913VDataTranInt(uint32_t BramInBuf[],CHAN_DATA *OutBuf,CHAN_CALIB_DATA *ChanLowCalib_ST_p,CHAN_CALIB_DATA *ChanHighCalib_ST_p,uint16_t TranLeng,uint8_t TranType)
{
    uint16_t i = 0;
    uint8_t j = 0;
    uint8_t k = 0;    
	uint8_t num = 8;
    int16_t VoltageTemBin = 0,HVoltageTemBin = 0; //the AD data with 16bit
    uint16_t VoltageBinAbs = 0,HVoltageBinAbs = 0;
    float  VoltageTem = 0,HVoltageTem = 0;//the AD data After Tran
    int16_t VolIntTem = 0,HVolIntTem = 0;
    float VolDenoValue = 0;
    int16_t VolCorrtValue = 0;
    uint32_t Inbuf = 0;
    int16_t Calib110VDevt = 0;

    //使用飞鸿FPGA双口RAM协议
    for(num=8;num<= TranLeng;num+=16)
    {
    	for(i=num-8;i<num;i++)
    	{
            j =(num*2-1)-i;//i,j来控制输出给存储文件或者PTU的通道，i代表1个通道，j代表另一个通道。
            Inbuf = BramInBuf[k];//读取的电压、电路值缓存，通过k来控制变量通道
            VoltageTemBin =  (int16_t)(Inbuf & 0xFFFF); 
            HVoltageTemBin =  (int16_t)((Inbuf >> 16) & 0xFFFF); 
            /*AD7616 */
            if(ADC_VDATE == g_DebugType_EU)
            {
                printf("VolCH%u VoltageTemBin:%x\n",i,VoltageTemBin);
                printf("VolCH%u HVoltageTemBin:%x\n",j,HVoltageTemBin);
            }
        
            VoltageTem = (float)VoltageTemBin * VOL_BINTAN_FACTOR; //单位数字量代表305.1851uV        
            HVoltageTem = (float)HVoltageTemBin * VOL_BINTAN_FACTOR; //单位数字量代表305.1851uV
            VoltageTem  *= VOL_REALTAN_FACTOR;// 分压比例*44.75
            HVoltageTem *= VOL_REALTAN_FACTOR;// 分压比例*44.75        
            VolIntTem = (int16_t)(VoltageTem / 100000); // uV Trans to 100mV
            HVolIntTem = (int16_t)(HVoltageTem / 100000); // uV Trans to 100mV   
            OutBuf -> VolChan_ST[i].CH_Voltage_I16 = VolIntTem;
            if(j<40)
            {
                OutBuf -> VolChan_ST[j].CH_Voltage_I16 = HVolIntTem;
            }
            //丢弃VolChan_ST[40]-VolChan_ST[47]电压值
            else
            {
                
            }            
            if(ADC_VDATE == g_DebugType_EU)
            {
                printf("VolCH%u OutBuf:%d(100mV)\n",i, OutBuf -> VolChan_ST[i].CH_Voltage_I16);
                if(j<40)
                {
                  printf("VolCH%u OutBuf:%d(100mV)\n",j, OutBuf -> VolChan_ST[j].CH_Voltage_I16);
                }                
            }
            /*110 * VOL /(110+$v110-$v0) - $v0*/
            if(NORMTRANTYPE == TranType)
            {        
                /*$v110-$v0*/
                Calib110VDevt = ChanHighCalib_ST_p-> VolChanCalib_I16[i] - ChanLowCalib_ST_p-> VolChanCalib_I16[i];
                /*110 * Voltage*/
                VolDenoValue = (float)OutBuf -> VolChan_ST[i].CH_Voltage_I16 * ADCVOLT_110V;
                /*110 * VOL /(110+$v110-$v0)*/
                if(Calib110VDevt > -100)
                {
                    VolCorrtValue = (int16_t)(VolDenoValue / (ADCVOLT_110V + Calib110VDevt));   
                }
                else
                {
                    printf("L_CH:%u Calib110VDevt:%d,out range!\n",i,Calib110VDevt);
                }      
                /*110 * VOL /(110+$v110-$v0)  - $v0 */
                OutBuf -> VolChan_ST[i].CH_Voltage_I16  = VolCorrtValue - ChanLowCalib_ST_p-> VolChanCalib_I16[i] ;
                if(LINECAlib_DEBUG == g_DebugType_EU)
                {
                    printf("L_CH:%u Low_VolChanCalib:%d,High_VolChanCalib:%d\n ",i,ChanLowCalib_ST_p-> VolChanCalib_I16[i],ChanHighCalib_ST_p-> VolChanCalib_I16[i]);
                    printf("L_CH:%u Calib110VDevt:%d\n",i, Calib110VDevt);
                    printf("L_CH:%u VolDenoValue:%8.4f\n ",i, VolDenoValue);
                    printf("L_CH:%u MesurVolCorrtValue:%d\n",i,VolCorrtValue);    
                    printf("L_CH:%u After calib:%d\n",i, OutBuf -> VolChan_ST[i].CH_Voltage_I16);
                }
                /*for high chanel*/

                /*$v110-$v0*/
			    if(j<40)//第41-48通道未使用
                {
                    Calib110VDevt = ChanHighCalib_ST_p-> VolChanCalib_I16[j] - ChanLowCalib_ST_p-> VolChanCalib_I16[j];
                    /*110 * Voltage*/
                    VolDenoValue = (float)OutBuf -> VolChan_ST[j].CH_Voltage_I16 * ADCVOLT_110V;
                    /*110 * VOL /(110+$v110-$v0)*/
                    if(Calib110VDevt > -100)
                    {
                        VolCorrtValue = (int16_t)(VolDenoValue / (ADCVOLT_110V + Calib110VDevt));   
                    }
                    else
                    {
                        printf("H_CH:%u Calib110VDevt:%d,out range!\n",j,Calib110VDevt);
                    }      
                    /*110 * VOL /(110+$v110-$v0)  - $v0 */
                    OutBuf -> VolChan_ST[j].CH_Voltage_I16  = VolCorrtValue - ChanLowCalib_ST_p-> VolChanCalib_I16[j] ;
                    if(LINECAlib_DEBUG == g_DebugType_EU)
                    {
                        printf("H_CH:%u Low_VolChanCalib_I16:%d,High_VolChanCalib_I16:%d\n",j,ChanLowCalib_ST_p-> VolChanCalib_I16[j],ChanHighCalib_ST_p-> VolChanCalib_I16[j]);
                        printf("H_CH:%u Calib110VDevt:%d\n",j,Calib110VDevt);
                        printf("H_CH:%u VolDenoValue:%8.4f\n ",j,VolDenoValue);
                        printf("H_CH:%u MesurVolCorrtValue:%d\n",j,VolCorrtValue);
                        printf("H_CH:%u After calib:%d\n",j, OutBuf -> VolChan_ST[j].CH_Voltage_I16);
                    }
                }
            }
            k++;
    	}
    }
    return CODE_OK;
}

