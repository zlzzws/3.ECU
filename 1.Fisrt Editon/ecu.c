#include "ecu.h"


#if 1

/**
 * @description:    H2_SOC_Calculate-Calculate H2 SOC
 * @param:          float pressure,float *temperature
 * @return:         float soc_ret
 * @author:         zlz
 */
static float H2_SOC_Calculate(float pressure,float *temperature)
{
    uint8_t i,j;
    float soc_ret;
    float z_factor[4][6]=Z_COMPRESSION_FACTOR;
    float z_num=0;
    float temperature_max;
    float temperature_k;

    for(i=0;i<11;i++)
    {
        temperature_max = temperature_max >= temperature[i] ? temperature_max : temperature[i];
    }

    temperature_k = 100 / ( temperature_max + 273.15 );

    for(j=0;j<6;j++)
    {
        for(i=0;i<4;i++)
        {
            z_num += z_factor[i][j] * pow(pressure,j) * pow(temperature_k,i);
        }
    }

    return z_num;
}

/**
 * @description:    H2_Fault_Judge
 * @param:          judge the h2 concentration,temperature,pressure
 * @return:         uint8_t life stop ->1 life normal ->0
 * @author:         zlz
 */
static int8_t H2_Fault_Judge(TMS570_BRAM_DATA *bram_tms_data,uint8_t *faultbuffer,ECU_ERROR_INFO *err_info)
{
    uint8_t tempbuffer[30]={0};
    uint8_t i,j,k;
    int8_t err_con=0,err_temper=0,err_temper_count=0,err_communication=0;
    int8_t err_high_pre=0,err_low_pre=0,err_sensor=0,ret=0;
    float h2_concentration[11] = {0};
    float high_pressure;
    uint16_t low_pressure;
    int8_t h2_temperature[6] = {0};
    static uint32_t count = 0 ;
    float h2_soc; 
    uint8_t h2_soc_uint8;   

    memcpy(tempbuffer,&bram_tms_data->buffer[10],21);//储氢系统相关的模拟量数据
    memcpy(&tempbuffer[21],&bram_tms_data->buffer[8],8);//传感器故障数据
    
    /*氢瓶内高压判断*/
    high_pressure = tempbuffer[17]*0.2 ;
    if(high_pressure <= 2)              //瓶内高压低于2Mpa,认为2级故障,需要降载并关闭燃料电池
    {
        err_high_pre = 2;
        faultbuffer[62] = 0x10;       
    }
    else if(high_pressure > 40)
    {
        err_high_pre = 1;
        faultbuffer[0] = 0x1;
    }    
    /*中压管低压判断*/
    low_pressure  = tempbuffer[18] | tempbuffer[19] << 8; 
    if(low_pressure > 2800)             //中压管低压高于2800kpa,认为3级故障，立即切断储氢设备（FCU关闭，进入EV模式）
    {
        err_low_pre = 3;
        faultbuffer[1] = 0x4;
    }   

    /*氢瓶温度判断*/
    for (i=0;i<6;i++)
    {
        h2_temperature[i] = tempbuffer[i+11]-40;
        if (h2_temperature[i] <= -40)    //氢瓶温度超过2个低于-40℃，则认为2级故障
        {
            err_temper = 2;
            err_temper_count += 1;
            (faultbuffer[14+i] >> 4) = 0x1;
        }
        else if((h2_temperature[i] > -40 && h2_temperature[i] <= -30) || h2_temperature[i] >= 85)
        {        
            err_temper = 1;
            faultbuffer[14+i] = 0x1;
        }                
    }
    /*氢泄露浓度 传感器故障 通讯故障判断*/
    for(i=0;i<11;i++)
    {
        /*传感器判断*/
        if(tempbuffer[24] || tempbuffer[25] )
        {
            err_sensor = 1;
            faultbuffer[20] = tempbuffer[24] ;                  //请浓度传感器1-8故障
            faultbuffer[21] = tempbuffer[25] & 0x7;             //氢浓度传感器9-11故障            
        }
        /*储氢/扩展单元通讯判断*/
        if(err_info->commu_err[6])
        {
            err_communication = 3 ;
            (faultbuffer[21] >>3) = err_info->commu_err[6]; //通讯故障
        }
        /*氢泄露浓度判断*/
        h2_concentration[i] = tempbuffer[i]*0.0002;
        if ((h2_concentration[i] >= 0.004 && h2_concentration[i] < 0.01) && count >=300) //持续30s,假定1个循环是100ms,输出尽快停车指令
        {
            err_con = 1;
            j = i % 2;
            k = i / 2;
            if(j == 0 )
            {
                faultbuffer[22+i+k] = 0x1;
            }
            else if (j == 1)
            {
                faultbuffer[22+i+k] >> 3 = 0x1;
            }
        }        
        else if(h2_concentration[i] > 0.01 && h2_concentration[i] < 0.02) //氢浓度超过1%则认为2级故障，关闭燃料电池
        {
            err_con = 2;
            j = i % 2;
            k = i / 2;
            if(j == 0 )
            {
                faultbuffer[22+i+k] = 0x2;
            }
            else if (j == 1)
            {
                faultbuffer[22+i+k] >> 3 = 0x2;
            }            
        }
        else if(h2_concentration[i] > 0.02) //氢浓度超过2%则认为3级故障，立即切断储氢设备（FCU关闭，进入EV模式）
        {
            err_con = 3;
            j = i % 2;
            k = i / 2;
            if(j == 0 )
            {
                faultbuffer[22+i+k] = 0x4;
            }
            else if (j == 1)
            {
                faultbuffer[22+i+k] >> 3 = 0x4;
            }
        }            
    }
    /*氢气剩余量计算*/
    h2_soc =  H2_SOC_Calculate(high_pressure,h2_concentration);
    h2_soc_uint8 = (uint8_t)(h2_soc / 0.004);
    faultbuffer[29] = h2_soc_uint8;

    /*三级故障*/
    if(err_con == 3 || err_low_pre == 3)
    {
        err_info->h2_store_err_level[2] = 1 ;
    }
    else
    {
        err_info->h2_store_err_level[2] = 0;
    }
    /*二级故障*/
    if(err_con == 2 || err_temper ==2 || err_high_pre == 2)
    {
        err_info->h2_store_err_level[1] = 1;
    }
    else
    {
        err_info->h2_store_err_level[1] = 0;
    }
    /*一级故障*/
    if (err_con == 1 || err_sensor == 1 || err_temper == 1 || err_high_pre == 1)
    {
        err_info->h2_store_err_level[0] = 1;
    }
    else
    {
        err_info->h2_store_err_level[0] = 0;
    }    
    
    count++;
    /*返回是否存在二级、三级故障*/
    ret = err_info->h2_store_err_level[1] || err_info->h2_store_err_level[2] ;
    return ret;    
}

/**
 * @description: FCU_Fault_Judge-judge the FCU fault
 * @param:       TMS570_BRAM_DATA *bram_tms_data,
 * @param:       uint8_t *faultbuffer.
 * @param:       ECU_ERROR_INFO *err_info
 * @return:      err
 * @author:      zlz
 */
static int8_t FCU_falut_judge(TMS570_BRAM_DATA *blvds_rd_data,TMS570_BRAM_DATA *bram_tms_data,uint8_t *faultbuffer,ECU_ERROR_INFO *err_info)
{
    /*TODO 增加硬线信号对于故障的判断*/
    /*A/B堆出现3级故障，但是需要细分 A B堆分别的故障状态*/
    err_info->fcu_err_level[2] =  ((bram_tms_data[2].buffer[0]>>8 & 0xff == 0x04) ? 1 : 0)  ||   ((bram_tms_data[2].buffer[14]>>8 & 0xff == 0x04) ? 1 : 0);
    err_info->fcu_err_level[1] =  ((bram_tms_data[2].buffer[0]>>8 & 0xff == 0x02) ? 1 : 0)  ||   ((bram_tms_data[2].buffer[14]>>8 & 0xff == 0x02) ? 1 : 0);
    err_info->fcu_err_level[0] =  ((bram_tms_data[2].buffer[0]>>8 & 0xff == 0x01) ? 1 : 0)  ||   ((bram_tms_data[2].buffer[14]>>8 & 0xff == 0x01) ? 1 : 0);
    return err_info->fcu_err_level[2];
}

/**
 * @description:    DCDC_Fault_Judge-judge the DCDC fault
 * @param:          TMS570_BRAM_DATA *bram_tms_data,
 * @param:          uint8_t *faultbuffer.
 * @param:          ECU_ERROR_INFO *err_info
 * @return:         err
 * @author:         zlz
 */
static int8_t DCDC_falut_judge(TMS570_BRAM_DATA *bram_tms_data,uint8_t *faultbuffer,ECU_ERROR_INFO *err_info)
{
    uint8_t i;
    static uint8_t count=0;
    float output_voltage;
    float input_voltage;
    float temperature;
    float input_current;
    float output_current;    
    uint8_t input_voltage_judge;
    uint8_t output_voltage_judge;
    uint8_t temperature_judge;
    uint8_t module_judge;
    uint8_t internal_judge;
    uint16_t tempbuffer[5] = {0};
    uint8_t  dcdc_err_status;

    memcpy(tempbuffer,&bram_tms_data[1].buffer[0],10);
    for (i=0;i<5;i++)
    {
       tempbuffer[i] = Litte2BigEnd(tempbuffer[i]);
    }
    dcdc_err_status = (bram_tms_data[1].buffer[2] >>24) & 0xff;

    output_voltage = tempbuffer[0]*0.1;
    input_voltage = tempbuffer[1]*0.1;
    tempbuffer = tempbuffer[2]*0.1-40;
    input_current = tempbuffer[3]*0.1;
    output_current = tempbuffer[4]*0.1;

    if(input_voltage >= 700 && input_voltage < 730)
    {
        input_voltage_judge = 1;
    }
    else if(input_voltage >= 730 && input_voltage < 750)
    {   
        input_voltage_judge  = 2;
    }
    else if(input_voltage >= 750)
    {
        input_voltage_judge  = 3;
    }
    else if(input_voltage <= 335)
    {
        input_voltage_judge  = 3;
    }
    else if(input_voltage > 335 && input_voltage <= 340)
    {
        input_voltage_judge  = 2;
    }
    else if (input_voltage > 340 && input_voltage <= 350)
    {
        count++;
        if(count >= 20)
        {
            input_voltage_judge = 1;
            count=0;
        }
        else
        {
            count = 0;
        }        
    }

    if(output_voltage >= 830 && output_voltage < 840)
    {
        output_voltage_judge = 1;
    }
    else if(output_voltage >= 840 && output_voltage < 850)
    {
        output_voltage_judge = 2;
    }
    else if(output_voltage >= 850)
    {
        output_voltage_judge = 3;
    }

    if(temperature >= 70 && temperature <80)
    {
        temperature_judge = 1;
    }
    else if (temperature >= 80 && temperature <90)
    {
        temperature_judge = 2;
    }
    else if (temperature >= 90)
    {
        temperature_judge = 3;
    }

    module_judge = (dcdc_err_status >> 4) == 0 ? 0 : 1;

    internal_judge = (dcdc_err_status >>1 & 0x1) == 0 ? 0 : 1;

    /*TODO 缺少输入输出电流过流的判断*/


    if(input_voltage_judge == 1 || output_voltage_judge == 1 || temperature_judge == 1 )
    {
        err_info->dc_dc_err_level[0] = 1;
    } 
    else
    {
        err_info->dc_dc_err_level[0] = 0;
    }

    if(output_voltage_judge == 2 || input_voltage_judge == 2 || temperature_judge ==2)
    {
        err_info->dc_dc_err_level[1] = 1;
    }
    else
    {
        err_info->dc_dc_err_level[1] = 0;
    }

    if (output_voltage_judge == 3 || input_voltage_judge == 3 || temperature_judge == 3 \
        || module_judge || internal_judge)
    {
        err_info->dc_dc_err_level[2] = 1;
    }
    else
    {
        err_info->dc_dc_err_level[2] = 0;
    }
    
    return err_info->dc_dc_err_level[2];    
}

/**
 * @description:    BMS_Fault_Judge-judge the BMS fault
 * @param:          TMS570_BRAM_DATA *bram_tms_data,
 * @param:          uint8_t *faultbuffer.
 * @param:          ECU_ERROR_INFO *err_info
 * @return:         err
 * @author:         zlz
 */
static int8_t BMS_falut_judge(TMS570_BRAM_DATA *blvds_rd_data,TMS570_BRAM_DATA *bram_tms_data,uint8_t *faultbuffer,ECU_ERROR_INFO *err_info)
{
   /*TODO考虑动力电池硬线信号对故障的判断*/
    float soc_bat1;
    float soc_bat2;
    float soc_bat;
    uint8_t soc_lower_err_1,soc_lower_err_2,soc_lower_err_3;
    uint8_t bat1_level1_err,bat2_level1_err;
    uint8_t bat1_level2_err,bat2_level2_err;
    uint8_t bat1_level3_err,bat2_level3_err;
    uint8_t battery_status_err;
    uint8_t temperbuffer[32]={0};
    float output_voltage;
    uint8_t over_voltage_err_1,over_voltage_err_2,over_voltage_err_3;
    uint8_t under_voltage_err_1,under_voltage_err_2,under_voltage_err_3;
    uint8_t hardwire_bms_err;

    memcpy(temperbuffer,&bram_tms_data->buffer[0],32);

    battery_status_err = (temperbuffer[0] & 0xf) == 0x0011 ? 1 : 0 ; //暂时不知道用处是什么

    bat1_level1_err = temperbuffer[31] & 0x1 == 0x1 ? 1 : 0;
    bat1_level2_err = temperbuffer[31] & 0x2 == 0x2 ? 1 : 0;
    bat1_level3_err = temperbuffer[31] & 0x3 == 0x3 ? 1 : 0;

    bat2_level1_err = temperbuffer[31] & 0x4 == 0x4 ? 1 : 0;
    bat2_level2_err = temperbuffer[31] & 0x8 == 0x8 ? 1 : 0;
    bat2_level3_err = temperbuffer[31] & 0xc == 0xc ? 1 : 0;
    /*关于电池的输入过压、输入欠压,ECU(系统)的判断条件会更严苛*/
    output_voltage = (temperbuffer[2] << 8 | temperbuffer[3])*0.1;

    if(output_voltage <580)
    {
        under_voltage_err_3 = 1;
    }
    else if (output_voltage >= 580 && output_voltage <610)
    {
        under_voltage_err_2 = 1;
    }
    else if (output_voltage >= 610 && output_voltage <640)
    {
        under_voltage_err_1 = 1;
    }
    else if(output_voltage >= 835 && output_voltage <845)
    {
        over_voltage_err_1 = 1;
    }
    else if (output_voltage >= 845 && output_voltage < 860)
    {
        over_voltage_err_2 = 1;
    }
    else if (output_voltage >= 860)
    {
        over_voltage_err_3 = 1;
    }
    else
    {
        over_voltage_err_1=0;
        over_voltage_err_2=0;
        over_voltage_err_3=0;

        under_voltage_err_1=0;
        under_voltage_err_2=0;
        under_voltage_err_3=0;
    }
    /*关于电池剩余电量判断*/
    soc_bat1 = temperbuffer[1] *0.004;
    soc_bat2 = temperbuffer[24] *0.004;
    soc_bat = soc_bat1 >= soc_bat2 ? soc_bat2 : soc_bat1;

    if(soc_bat >= 0.2 && soc_bat <= 0.3)
    {
        soc_lower_err_1 = 1;
    }
    else if (soc_bat >= 0.05 && soc_bat < 0.2)
    {
        soc_lower_err_2 = 1;
    }
    else if (soc_bat < 0.05)
    {
        soc_lower_err_3 = 1;
    }
    else
    {
        soc_lower_err_1 = 0;
        soc_lower_err_2 = 0;
        soc_lower_err_3 = 0;
    }
    /*总体故障判断*/
    if(under_voltage_err_3 || over_voltage_err_3 || bat1_level3_err || bat2_level3_err || soc_lower_err_3)
    {
        err_info->bms_err_level[2] = 1;
    }
    else
    {
        err_info->bms_err_level[2] = 0;
    }

    if (under_voltage_err_2 || over_voltage_err_2 || bat1_level2_err || bat2_level2_err || soc_lower_err_2)
    {
        err_info->bms_err_level[1] = 1;
    }
    else
    {
        err_info->bms_err_level[1] = 0;
    }

    if (under_voltage_err_1 || over_voltage_err_1 || bat1_level1_err || bat2_level1_err || soc_lower_err_1)
    {
        err_info->bms_err_level[0] = 1;
    }
    else
    {
        err_info->bms_err_level[1] = 0;
    }
    return CODE_OK;
}

/**
 * @description:    commuication_fault_judge - can mvb lifesignal stop
 * @param:          TMS570_BRAM_DATA *bram_tms_data,
 * @param:          uint8_t *faultbuffer.
 * @param:          ECU_ERROR_INFO *err_info
 * @return:         err
 * @author:         zlz
 */
static int8_t commuication_fault_judge(uint8_t *faultbuffer,ECU_ERROR_INFO *err_info)
{
    uint8_t i;
    uint8_t commu_value = 0;
    /*CAN、MVB故障判断*/
    for(i=1;i<7;i++)
    {
        commu_value = commu_value || err_info->commu_err[i] ;
    }
    /*TODO  通讯故障在faultbuffer内进行填充*/
    return commu_value;
}

/**
 * @description:  Cooling_system_fault_judge-judge the inverter fault cooling system fault
 * @param:       TMS570_BRAM_DATA *bram_tms_data,
 * @param:       uint8_t *faultbuffer.
 * @param:       ECU_ERROR_INFO *err_info
 * @return:      err
 * @author:      zlz
 */
static int8_t Cooling_system_fault_judge(TMS570_BRAM_DATA *bram_tms_data,uint8_t *faultbuffer,ECU_ERROR_INFO *err_info)
{
    //需要在特定工况下，风扇才会开启
    //
}


/**
 * @description: FCU_Start_Stage
 * @param:      TMS570_BRAM_DATA *bram_tms_wr_data,
 * @param:      TMS570_BRAM_DATA *bram_tms_rd_data,
 * @param:      TMS570_BRAM_DATA *bram_rd_data,
 * @param:      TMS570_BRAM_DATA *bram_wr_data
 * @param:      ECU_ERROR_INFO *err_info
 * @return:     err
 * @author:     zlz
 */
int8_t FCU_Start_Stage(TMS570_BRAM_DATA *tms_bram_wr_data,TMS570_BRAM_DATA *tms_bram_rd_data,TMS570_BRAM_DATA *blvds_bram_rd_data,
                        TMS570_BRAM_DATA *blvds_bram_wr_data,ECU_ERROR_INFO *err_info)
{
    int8_t err;
    uint8_t hardwire_fcu_use ;
    uint8_t bms_km1_km2_use ;
    uint8_t h2_spuly_request;
    uint8_t customization_key;
    uint8_t ssc_in_sig,enable_sig;
    uint8_t start_sig,dc_dc_start_sig;
    /*FIXME:是否考虑A/B同时故障才不允许Start？*/
    /*TODO 增加对低温保护的判断*/
    /*条件是收到燃料电池启动指令且A/B堆无三级故障 且动力电池KM1 KM2投入 且FCU不存在二级或三级故障*/
    err_info->fcu_err_level[2] = (s_tms570_bram_WR_data_ch9_11_st[2].buffer[0] & 0x400) || (s_tms570_bram_WR_data_ch9_11_st[2].buffer[14] & 0x400);
    hardwire_fcu_use = (blvds_bram_rd_data->buffer[1] & 0x4) && !(blvds_bram_rd_data->buffer[1] & 0x40) ;
    bms_km1_km2_use = ((blvds_bram_rd_data->buffer[1]>>16) & 0x0440) == 0x0440 ? 1 : 0;
    if(hardwire_fcu_use && bms_km1_km2_use && !err_info->fcu_err_level[2]) 
    {
        customization_key = 0x1;       
        if(g_tms570_errflag == 1)
        {
            memcpy(&tms_bram_rd_data[2].buffer[0],&customization_key,1);       
        }
        ssc_in_sig = 0x9;
        enable_sig = 0x9;
        /*输出SSC_IN ENABLE硬线信号*/
        blvds_bram_wr_data->buffer[0] = ssc_in_sig <<16 | enable_sig <<8;
        while (1)
        {
            //阻塞过程中也要一直更新相关状态
            err_info->fcu_err_level[2] = (s_tms570_bram_WR_data_ch9_11_st[2].buffer[0] & 0x400) || (s_tms570_bram_WR_data_ch9_11_st[2].buffer[14] & 0x400);
            hardwire_fcu_use = (blvds_bram_rd_data->buffer[1] & 0x4) && !(blvds_bram_rd_data->buffer[1] & 0x40) ;
            bms_km1_km2_use = ((blvds_bram_rd_data->buffer[1]>>16) & 0x0440) == 0x0440 ? 1 : 0;
            h2_spuly_request = (tms_bram_wr_data[2].buffer[1]>>24) & 0x3;
            /*收到FCU发送的h2 supply open request Can信号*/
            if( h2_spuly_request && hardwire_fcu_use && bms_km1_km2_use && !err_info->fcu_err_level[2])
            {
                /*开启电磁阀、瓶口阀0.5s*/
                blvds_bram_wr_data->buffer[0] = 0x99;
                usleep(500000);
                /*输出start硬线信号*/
                start_sig = 0x9;
                blvds_bram_wr_data->buffer[0] = enable_sig <<8 | start_sig <<12 | ssc_in_sig <<16 ;
                /*FCU CAN协议 operate status == 100*/
                usleep(500000);
                /*TODO 启动阶段需要加上各种判断的时间延时*/
                if(((tms_bram_wr_data[2].buffer[3] & 0xff) == 100))
                {
                    dc_dc_start_sig = 0x1;
                    if(g_tms570_errflag == 1)
                    {
                        memcpy(&tms_bram_rd_data[1].buffer[0],&dc_dc_start_sig,1);
                    }
                }
                return CODE_OK;
            }
        }       
    }
    return CODE_ERR;
}

/**
 * @description: ECU_Diagnose
 * @param:      uint8_t *buffer,
 * @param:      TMS570_BRAM_DATA *blvds_bram_rd_data,
 * @param:      TMS570_BRAM_DATA *blvds_bram_wr_data,
 * @param:      TMS570_BRAM_DATA *tms_wr_data_9_11,
 * @param:      TMS570_BRAM_DATA *tms_wr_data_12 
 * @param:      ECU_ERROR_INFO *err_info
 * @param:      uint8_t diagnose_type
 * @return:     err
 * @author:     zlz
 */
int8_t ECU_Diagnose(uint8_t *fault_buffer,TMS570_BRAM_DATA *blvds_bram_rd_data,TMS570_BRAM_DATA *blvds_bram_wr_data,\
                    TMS570_BRAM_DATA *tms_wr_data_9_11,TMS570_BRAM_DATA *tms_wr_data_12,ECU_ERROR_INFO *err_info,uint8_t diagnose_type)
{
    uint8_t i;
    int8_t err=0;
    int8_t comm_ret=0;
    int8_t h2_ret,fcu_ret,bms_ret;
    int8_t dcdc_ret,cooling_system_ret;
    
    switch (diagnose_type)
    {
        case DIAGNOSE_STANDBY:
            blvds_bram_wr_data->buffer[0] = 0x99;/*电磁阀，瓶口阀输出*/
            usleep(2500000);/*延时2.5s*/
            /*储氢系统故障判断*/
            h2_ret = H2_Fault_Judge(tms_wr_data_12,fault_buffer,err_info);
            if(h2_ret)
            {
                printf("On stanby_diagnose stage,there are h2 store error in system!\n");
                blvds_bram_wr_data->buffer[0] = 0xaa;/*储氢系统故障，关闭电磁阀，瓶口阀输出*/                
            }
            comm_ret = commuication_fault_judge(fault_buffer,err_info);
            if(comm_ret)
            {
                printf("On stanby_diagnose stage,there are communication error in system!\n");                
            }
            err = h2_ret || comm_ret;
            break;
        case DIAGNOSE_HYBRID:
            h2_ret = H2_Fault_Judge(tms_wr_data_12,fault_buffer,err_info);
            cooling_system_ret = Cooling_system_fault_judge(tms_wr_data_12,fault_buffer,err_info);
            fcu_ret = FCU_falut_judge(blvds_bram_rd_data,tms_wr_data_9_11,fault_buffer,err_info);
            bms_ret = BMS_falut_judge(blvds_bram_rd_data,tms_wr_data_9_11,fault_buffer,err_info);
            dcdc_ret = DCDC_falut_judge(tms_wr_data_9_11,fault_buffer,err_info);
            comm_ret = commuication_fault_judge(fault_buffer,err_info);
            /*系统存在三级故障*/
            err = h2_ret || cooling_system_ret || fcu_ret || bms_ret || dcdc_ret || comm_ret;
            break;
        case DIAGNOSE_EV:
            //在EV阶段 不要去诊断FCU,FCU故障状态保持在关机前状态，否则关机后，各种异常可能会清零导致误判（但通讯肯定会在15min后关闭）
            break;
        default:
            break;
    }
    return err;
}

/**
 * @description: power_control_fun
 * @param:      uint8_t *buffer,
 * @param:      TMS570_BRAM_DATA *tms_wr_data_9_11,
 * @param:      TMS570_BRAM_DATA *tms_wr_data_12,
 * @param:      ECU_ERROR_INFO *err_info
 * @return:     err
 * @author:     zlz
 */
int8_t Hybrid_Power_ctrl(int8_t *power_buffer,TMS570_BRAM_DATA *tms_wr_data_8,TMS570_BRAM_DATA *tms_wr_data_9_11,ECU_ERROR_INFO *err_info)
{
    static struct timespec begin_ts,last_ts;    //计时间
    static uint8_t i=0;
    uint8_t level2_err;
    uint8_t train_status;
    uint8_t temperbuffer[100]={0};      //临时数据
    float demand_power;                 //车辆运行需求功率=牵引功率+附件功率
    int16_t train_middle_voltage;       //中间电压
    int16_t train_middle_current;       //总中间电流
    float aux_power;                    //附件功率
    float traction_power;               //牵引功率=中间电压*总中间电流
    float battery_soc1;
    float battery_soc2;
    float battery_soc;
    float bms_voltage;
    float accelerate_bms_current_limit;
    uint16_t accelerate_bms_power_limit;
    float brake_bms_current_limit;
    uint16_t brake_bms_power_limit;
    float temperature_dcdc;
    float output_voltage_dcdc;
    float max_sigle_voltage_bms;   
    static uint8_t power_160kw_limit_flag =0,power_120kw_limit_flag =0,power_80kw_limit_flag =0;
    uint8_t fcu_max_power;
    uint8_t fcu_power_target;
    uint8_t fcu_step_increase;
    uint8_t fcu_set_power;
    uint8_t bms_set_power;
    static uint8_t life_signal=0;
    static uint8_t last_second_power=0;//上一秒周期的功率需求
    uint16_t fcu_stack_voltage;
    uint16_t fcu_stack_current;
    uint16_t fcu_allow_current;
    uint16_t dcdc_set_current;

    level2_err = err_info->bms_err_level[1] || err_info->h2_store_err_level[1] || err_info->heat_dissipation_err_level[1] \
                    || err_info->dc_dc_err_level[1] || err_info->fcu_err_level[1];
    
    if(level2_err) //降低功率运行
    {
        
    }
    else //正常运行
    {
    
        memcpy(temperbuffer,&tms_wr_data_8->buffer[8],13);              /*TCMS*/
        memcpy(temperbuffer[13],&tms_wr_data_9_11[0].buffer[0],22);     /*BMS*/
        memcpy(temperbuffer[35],&tms_wr_data_9_11[0].buffer[6],1);      /*BMS*/
        memcpy(temperbuffer[36],&tms_wr_data_9_11[1].buffer[0],10);     /*DCDC*/
        memcpy(temperbuffer[46],&tms_wr_data_9_11[2].buffer[0],14);     /*FCU*/

        /*DC-DC温度判断*/
        temperature_dcdc = (temperbuffer[40]<<8 | temperbuffer[41])*0.1-40;
        /*DC-DC输出电压*/
        output_voltage_dcdc = (temperbuffer[36]<<8 | temperbuffer[37])*0.1;
        /*动力电池最高单体电压*/
        max_sigle_voltage_bms = (temperbuffer[29]<<8 | temperbuffer[30])*0.001;
        //TODO 需要考虑A/B堆分开功率输出工控，特别是故障时如何控制,目前整个逻辑此部分都未考虑
        /*功率限制判断*/
        if(!power_160kw_limit_flag && !power_120kw_limit_flag && !power_80kw_limit_flag &&\
             temperature_dcdc <= 70 && output_voltage_dcdc <= 820 && max_sigle_voltage_bms <= 2.55)
        {
            fcu_max_power = 200;
        }
        else if(!power_160kw_limit_flag && ((temperature_dcdc > 70 && temperature_dcdc <=74) || (output_voltage_dcdc > 820 && output_voltage_dcdc <= 830)\
                || (max_sigle_voltage_bms > 2.55 && max_sigle_voltage_bms <= 2.65)))
        {
            fcu_max_power = 160;
            power_160kw_limit_flag =1;
        }
        else if (!power_120kw_limit_flag && ((temperature_dcdc > 74 && temperature_dcdc <= 78) || (output_voltage_dcdc > 830 && output_voltage_dcdc <= 835)\
                || (max_sigle_voltage_bms > 2.65 && max_sigle_voltage_bms <= 2.75)))
        {
            fcu_max_power = 120;
            power_120kw_limit_flag =1;
        }
        else if (!power_80kw_limit_flag && ((temperature_dcdc > 78 && temperature_dcdc <= 80) || (output_voltage_dcdc > 835 && output_voltage_dcdc <= 840)\
                || (max_sigle_voltage_bms > 2.75 && max_sigle_voltage_bms <= 2.9)))
        {
            fcu_max_power = 80;
            power_80kw_limit_flag =1;
        }
        else if(power_160kw_limit_flag && temperature_dcdc < 65 && output_voltage_dcdc <780 && max_sigle_voltage_bms <2.4)
        {
            fcu_max_power = 200;
            power_160kw_limit_flag = 0 ;
        }
        else if(power_120kw_limit_flag && temperature_dcdc < 70 && output_voltage_dcdc <790 && max_sigle_voltage_bms <2.5)
        {
            fcu_max_power = 200;
            power_120kw_limit_flag = 0 ;
        }
        else if(power_80kw_limit_flag && temperature_dcdc < 74 && output_voltage_dcdc <800 && max_sigle_voltage_bms <2.6)
        {
            fcu_max_power = 200;
            power_80kw_limit_flag = 0 ;
        }

        /*车辆功率需求*/
        train_middle_voltage = temperbuffer[4] <<8 | temperbuffer[5] ;
        train_middle_current = temperbuffer[6] <<8 | temperbuffer[7] ;
        //aux_power = (temperbuffer[14]<<8 | temperbuffer[15])*0.1;
        aux_power = 20;/*目前固定给附件功率20KW*/
        traction_power = (train_middle_voltage * train_middle_current) / 1000;
        demand_power = aux_power + traction_power;
        train_status = temperbuffer[3] >> 4 | 0x3;/*0x2 制动 0x1 牵引*/
        /*BMS 电池SOC判断*/
        battery_soc1 = temperbuffer[14]*0.004;
        battery_soc2 = temperbuffer[27]*0.004;
        battery_soc = battery_soc1 >= battery_soc2 ? battery_soc2:battery_soc1;
        bms_voltage = (temperbuffer[15] >> 8 | temperbuffer[16])*0.1;/*动力电池电压*/
        accelerate_bms_current_limit = temperbuffer[21]>>8 | temperbuffer[22];/*加速工况下最大放电电流*/
        brake_bms_current_limit = temperbuffer[23]>>8 | temperbuffer[24];/*制动工况下最大充电电流*/       
        accelerate_bms_power_limit = (uint16_t)((bms_voltage * accelerate_bms_current_limit) / 1000);/*加速工况下最大放电功率*/ 
        brake_bms_power_limit = (uint16_t)((bms_voltage * brake_bms_current_limit) / 1000); /*制动工况下最大充电功率*/
        /*燃料电池输出电压*/
        fcu_allow_current = (uint16_t)((temperbuffer[48] | temperbuffer[49] << 8) * 0.1);
        fcu_stack_voltage = (temperbuffer[54] | temperbuffer[55] << 8) - 500 ;
        fcu_stack_current = ((uint16_t)((temperbuffer[56] | temperbuffer[57] << 8) * 0.1)) -500 ;

        /*DCDC电流电压*/
        
        /*车辆工控判断*/
        if(train_status == 0x1)
        {            
            if( demand_power >= fcu_max_power+accelerate_bms_power_limit)
            {
                printf("In rapidly accelerate stage,system can't apply the demand_power!\n");
                fcu_power_target = fcu_max_power;
                bms_set_power = accelerate_bms_power_limit;
            }
            else if(demand_power >= fcu_max_power)
            {
                fcu_power_target = fcu_max_power;
                bms_set_power = demand_power - fcu_set_power;
            }
            else if(demand_power <= 200 && demand_power > 20)
            {
                if(battery_soc <0.4)
                {
                    if((fcu_max_power - demand_power) <= brake_bms_power_limit)
                    {
                        fcu_power_target = fcu_max_power;
                    }
                    else
                    {
                        fcu_power_target = demand_power + brake_bms_power_limit;
                    }                   
                }
                else if(battery_soc >= 0.45 && battery_soc <= 0.55)
                {
                    fcu_power_target = fcu_max_power >= 150 ? 150 : fcu_max_power;
                    bms_set_power = demand_power - fcu_set_power > 0 ? demand_power -fcu_set_power : 0;
                }
                else if(battery_soc > 0.55 && battery_soc <= 0.65)
                {
                    fcu_power_target = fcu_max_power >= 120 ? 120 : fcu_max_power;
                    bms_set_power = demand_power - fcu_set_power > 0 ? demand_power -fcu_set_power : 0;
                }
                else if(battery_soc > 0.65)
                {
                    fcu_power_target = fcu_max_power >= 20 ? 20 : fcu_max_power;
                    bms_set_power = demand_power - fcu_set_power > 0 ? demand_power -fcu_set_power : 0;
                }
            }
            
            /*控制燃料电池在急加速过程中的电流上升斜率，0~400A以内，30A/1s，>400A,20A/1s*/
            clock_gettime(CLOCK_MONOTONIC,&begin_ts);
            
            if(begin_ts.tv_sec-last_ts.tv_sec >= 1)
            {
                if(fcu_stack_current >0 && fcu_stack_current <=400 )
                {
                    fcu_step_increase = (uint8_t)(fcu_stack_voltage*30/1000);
                    fcu_set_power =  fcu_power_target - last_second_power > fcu_step_increase ? last_second_power + fcu_step_increase : fcu_power_target;
                }
                else if(fcu_stack_current > 400)
                {
                    fcu_step_increase = (uint8_t)(fcu_stack_voltage*20/1000);
                    fcu_set_power =  fcu_power_target - last_second_power > fcu_step_increase ? last_second_power + fcu_step_increase : fcu_power_target;
                }
            } 
            last_ts = begin_ts; 
            last_second_power = fcu_set_power;          
        }
        else if(train_status == 0x2)
        {
            fcu_power_target = 25;
        }
        else if((temperbuffer[8] << 8 | temperbuffer[9]) == 0)//车辆静止
        {
            if(battery_soc <0.4)
            {
                fcu_power_target = fcu_max_power >= 180 ? 180 : fcu_max_power;
                /*TODO监测是否超出最大充电功率*/
                if((fcu_set_power-20) <= brake_bms_power_limit)
                {
                    
                }
                else
                {
                    fcu_power_target = 20 + brake_bms_power_limit;
                } 
            }
            else if(battery_soc >= 0.45 && battery_soc <= 0.55)
            {
                fcu_power_target = fcu_max_power >= 150 ? 150 : fcu_max_power;
                
            }
            else if(battery_soc > 0.55 && battery_soc <= 0.65)
            {
                fcu_power_target = fcu_max_power >= 120 ? 120 : fcu_max_power;
            }
            else if(battery_soc > 0.65)
            {
                fcu_power_target = fcu_max_power >= 20 ? 20 : fcu_max_power;
            }
        }
        /****************BMS CAN帧数据填充********************/        
        power_buffer[0] = 0xc;/*供电+非严重故障+保持*/
        power_buffer[1] = 0x42;/*本车已激活+运行*/
        power_buffer[2] = (uint8_t)((temperbuffer[8] << 8 | temperbuffer[9])*0.1);/*车辆速度*/
        power_buffer[3] = (temperbuffer[10] <<8 | temperbuffer[11])*0.0001;/*FIXME  如何将TCMS司控器级位与这个字节对应*/
        power_buffer[4] = (((uint16_t) (bms_set_power / 100)) >> 8) & 0xff;
        power_buffer[5] = ((uint16_t) (bms_set_power / 100)) & 0xff;
        power_buffer[6] =   life_signal;
        
        /****************DC-DC CAN帧数据填充********************/
        dcdc_set_current = (uint16_t)(fcu_set_power * 1000 / fcu_stack_voltage);
        dcdc_set_current = dcdc_set_current > fcu_allow_current ? fcu_allow_current : dcdc_set_current;

        power_buffer[8] = 0x1;/*DC_DC启动*/
        power_buffer[10] = (dcdc_set_current >> 8 ) & 0xff;/*DCDC电流设定高字节*/
        power_buffer[11] = dcdc_set_current & 0xff;/*DCDC电流设定低字节*/
        power_buffer[12] = (fcu_stack_voltage >> 8) & 0xff;/*DCDC电压设定高字节*/   
        power_buffer[13] = fcu_stack_voltage & 0xff;/*DCDC电压设定低字节*/ 
        power_buffer[15] =  life_signal;

        /****************FCU CAN帧数据填充********************/ 
        power_buffer[16] =  139;
        power_buffer[17] = power_buffer[11];
        power_buffer[18] = (uint8_t)((temperbuffer[8] << 8 | temperbuffer[9])*0.1);
        power_buffer[19] = 0x1;/*module run on*/
        power_buffer[23] = 0;
        
        life_signal++; 
    }   

}


/**
 * @description: Send_Data_binding
 * @param:      uint8_t *fault_buffer,
 * @param:      uint8_t *power_buffer
 * @param:      TMS570_BRAM_DATA *tms_rd_data_8,
 * @param:      TMS570_BRAM_DATA *tms_rd_data_9_11,
 * @param:      TMS570_BRAM_DATA *tms_rd_data_12,
 * @return:     err
 * @author:     zlz
 */
int8_t Send_Data_binding(uint8_t *fault_buffer,uint8_t *power_buffer,TMS570_BRAM_DATA *tms_rd_data_8,TMS570_BRAM_DATA *tms_rd_data_9_11,TMS570_BRAM_DATA *tms_rd_data_12)
{

}

int8_t FCU_DCDC_Shut_down()
{

}


int8_t EV_Shut_down()
{

}


#endif