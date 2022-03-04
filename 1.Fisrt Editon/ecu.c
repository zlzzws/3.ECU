#include "ecu.h"

/**
 * @description: ECU_Diagnose_function
 * @param:       TMS570_BRAM_DATA *bram_data,DRIVE_FILE_DATA *Drive_ST_p
 * @return:      uint8_t life stop ->1 life normal ->0
 * @author:      zlz
 */
static int8_t H2_Fault_Judge(uint8_t *buffer)
{
    uint8_t i;
    int8_t err_con=0,err_temper=0,err_high_pre=0,err_low_pre=0,ret=0;
    float h2_concentration[11] = {0};
    float high_pressure;
    uint16_t low_pressure;
    int8_t h2_temperature[6] = {0};
    for(i=0;i<11;i++)
    {
        h2_concentration[i] = buffer[i]*0.0002;
        if (h2_concentration[i] > 0.01) //氢浓度超过1%则认为2级故障
        {
            err_con = 1;
        }        
    }
    for (i=0;i<6;i++)
    {
        h2_temperature[i] = buffer[i+11]-40;
        if (h2_temperature[i] < -40)//氢瓶温度超过2个低于-40℃，则认为2级故障
        {
            err_temper += 1;
        }        
    }

    high_pressure = buffer[17]*0.2 ;
    if(high_pressure < 2)           //瓶内高压低于2Mpa,认为2级故障
    {
        err_high_pre = 1;
    }
    low_pressure  = buffer[18] | buffer[19] << 8; 
    if(low_pressure > 2800)         //中压管低压高于2800kpa,认为2级故障
    {
        err_low_pre = 1;
    }

    ret = err_con || (err_temper>=2) || err_high_pre || err_low_pre ;
    
}


static int8_t FCU_falut_judge(uint8_t *buffer,TMS570_BRAM_DATA *bram_data)
{
    /*TODO:FCU已经判断好了，需要分析CAN协议帧内的故障编码*/    
}

static int8_t DCDC_falut_judge(uint8_t *buffer,TMS570_BRAM_DATA *bram_data)
{

}

static int8_t BMS_falut_judge(uint8_t *buffer,TMS570_BRAM_DATA *bram_data)
{

}


static int8_t inverter_fault_judge(uint8_t *buffer,TMS570_BRAM_DATA *bram_data)
{

}

/**
 * @description: ECU_Diagnose_standby
 * @param:       TMS570_BRAM_DATA *blvds_bram_rd_data,
 * @param:       TMS570_BRAM_DATA *blvds_bram_wr_data,
 * @param:       TMS570_BRAM_DATA *bram_tms570_9_11_data,
 * @param:       TMS570_BRAM_DATA *bram_tms570_12_data, *    
 * @param:       ECU_ERROR_INFO *err_info
 * @return:      err
 * @author:      zlz
 */
int8_t ECU_Diagnose_Standby(TMS570_BRAM_DATA *blvds_bram_wr_data,TMS570_BRAM_DATA *bram_tms570_12_data,ECU_ERROR_INFO *err_info)
{
    uint8_t i;
    int8_t h2_ret,comm_ret=0;
    uint8_t tempbuff[24] ={0};

    blvds_bram_wr_data->buffer[0] = 0x99;/*电磁阀，瓶口阀输出*/
    usleep(2500000);/*延时2.5s*/
    //需要考虑与BLVDS线程同步
    memcpy(tempbuff,&bram_tms570_12_data->buffer[10],21);

    h2_ret = H2_Fault_Judge(tempbuff);

    if(h2_ret == 1)
    {
        printf("On stanby_diagnose stage,there are h2 store error in system!\n");
        bram_wr_data->buffer[0] = 0xaa;/*储氢系统故障，关闭电磁阀，瓶口阀输出*/
        return CODE_ERR;
    }
    for(i=1;i<7;i++)
    {
       comm_ret = comm_ret || err_info->commu_err[i] ;
    }
    if(comm_ret)              /*CAN或MVB存在故障*/
    {
        printf("On stanby_diagnose stage,there are communication error in system!\n");
        return CODE_ERR;
    }

    return CODE_OK;
}




/**
 * @description: FCU_Start_Stage
 * @param:      TMS570_BRAM_DATA *bram_tms_wr_data,
 * @param:      TMS570_BRAM_DATA *bram_tms_rd_data,
 * @param:      TMS570_BRAM_DATA *bram_rd_data,
 * @param:      TMS570_BRAM_DATA *bram_wr_data
 * @return:     err
 * @author:     zlz
 */
int8_t FCU_Start_Stage(TMS570_BRAM_DATA *tms_bram_wr_data,TMS570_BRAM_DATA *tms_bram_rd_data,TMS570_BRAM_DATA *blvds_bram_rd_data,TMS570_BRAM_DATA *blvds_bram_wr_data)
{
    int8_t err;
    uint8_t customization_key,ssc_in_sig,enable_sig,start_sig,dc_dc_start_sig;
    /*FIXME:是否考虑A/B同时故障才不允许Start？*/
    /*条件是收到燃料电池启动指令且A/B堆无三级故障 且动力电池KM1 KM2投入 且FCU不存在二级或三级故障*/
    if((blvds_bram_rd_data->buffer[1] & 0x4) && !(blvds_bram_rd_data->buffer[1] & 0x40) && (((blvds_bram_rd_data->buffer[1]>>16) & 0x0440) == 0x0440) \
        && !(tms_bram_wr_data[2].buffer[8]&0xffff) && !(tms_bram_wr_data[2].buffer[22]&0xffff)) 
    {
        customization_key = 0x1;
        /*FIXME 尽量不要extern 这个全局变量*/
        if(g_tms570_errflag == 1)
        {
            memcpy(&tms_bram_rd_data[2].buffer[0],&customization_key,1);       
        }
        ssc_in_sig = 0x9;
        enable_sig = 0x9;
        /*输出SSC_IN ENABLE硬线信号*/
        blvds_bram_wr_data->buffer[0] = ssc_in_sig <<16 | enable_sig <<8;
        /*收到FCU发送的h2 supply open request Can信号*/
        if(((tms_bram_wr_data[2].buffer[1]>>24) & 0x3) == 0x1)
        {
            /*开启电磁阀、瓶口阀0.5s*/
            blvds_bram_wr_data->buffer[0] = 0x99;
            usleep(500000);
            /*输出start硬线信号*/
            start_sig = 0x9;
            blvds_bram_wr_data->buffer[0] = enable_sig <<8 | start_sig <<12 | ssc_in_sig <<16 ;
            /*FCU CAN协议 operate status == 100*/
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
    return CODE_ERR;
}



int8_t ECU_Diagnose_EV(TMS570_BRAM_DATA *bram_tms_wr_9_11_data,ECU_ERROR_INFO *err_info)
{
   if((bram_tms_wr_9_11_data[2].buffer[0] & 0xff00) == 0x4) 
   {
       err_info->fcu_err_level = 3;
   }
   if ((bram_tms_wr_9_11_data[0].buffer[0] & 0xf) == 0x3 || ((bram_tms_wr_9_11_data[0].buffer[7] >>24) & 0x3) == 0x3\
    || ((bram_tms_wr_9_11_data[0].buffer[7] >>24) & 0xc) == 0xc)
   {
       err_info->bms_err_level = 3;
   }
   //FIXME 还是需要使用故障填充函数
    FCU_falut_judge(buffer,tms_wr_data_9_11);
    DCDC_falut_judge(buffer,tms_wr_data_9_11);
    BMS_falut_judge(buffer,tms_wr_data_9_11);
    H2_Fault_Judge(buffer,tms_wr_data_12);
    inverter_fault_judge(buffer,tms_wr_data_12);
   return 0;
}

/**
 * @description: fault_judgment_fun
 * @param:      uint8_t *buffer,
 * @param:      TMS570_BRAM_DATA *tms_wr_data_9_11,
 * @param:      TMS570_BRAM_DATA *tms_wr_data_12,
 * @param:      ECU_ERROR_INFO *err_info
 * @return:     err
 * @author:     zlz
 */
int8_t ECU_Diagnose_Hybrid(uint8_t *buffer,TMS570_BRAM_DATA *tms_wr_data_9_11,TMS570_BRAM_DATA *tms_wr_data_12,ECU_ERROR_INFO *err_info)
{
    FCU_falut_judge(buffer,tms_wr_data_9_11);
    DCDC_falut_judge(buffer,tms_wr_data_9_11);
    BMS_falut_judge(buffer,tms_wr_data_9_11);
    H2_Fault_Judge(buffer,tms_wr_data_12);
    inverter_fault_judge(buffer,tms_wr_data_12);
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
int8_t Hybrid_Power_ctrl(int8_t *buffer,TMS570_BRAM_DATA *tms_wr_data_8,ECU_ERROR_INFO *err_info)
{
    static uint8_t i=0;
    if (i <= 5)
    {
        printf("Entry Hybrid mode!\n");
        i++;
    }
    
    

}

int8_t EV_Power_ctrl(int8_t *buffer,TMS570_BRAM_DATA *tms_wr_data_8,ECU_ERROR_INFO *err_info)
{
    if (i <= 5)
    {
        printf("Entry EV mode!\n");
        i++;
    }

}

/**
 * @description: run_on_process
 * @param:      uint8_t *fault_buffer,
 * @param:      uint8_t *power_buffer
 * @param:      TMS570_BRAM_DATA *tms_rd_data_8,
 * @param:      TMS570_BRAM_DATA *tms_rd_data_9_11,
 * @param:      TMS570_BRAM_DATA *tms_rd_data_12,
 * @return:     err
 * @author:     zlz
 */
int8_t Communicate_Data_process(uint8_t *fault_buffer,uint8_t *power_buffer,TMS570_BRAM_DATA *tms_rd_data_8,TMS570_BRAM_DATA *tms_rd_data_9_11,TMS570_BRAM_DATA *tms_rd_data_12)
{

}

int8_t ECU_Shut_down()
{

}