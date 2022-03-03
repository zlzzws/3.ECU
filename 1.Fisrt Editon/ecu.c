#include "ecu.h"

/**
 * @description: ECU_Diagnose_function
 * @param:       TMS570_BRAM_DATA *bram_data,DRIVE_FILE_DATA *Drive_ST_p
 * @return:      uint8_t life stop ->1 life normal ->0
 * @author:      zlz
 */
int8_t H2_Fault_Judge(uint8_t *buffer)
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

/**
 * @description: ECU_Diagnose_function
 * @param:       TMS570_BRAM_DATA *bram_data,DRIVE_FILE_DATA *Drive_ST_p
 * @return:      uint8_t life stop ->1 life normal ->0
 * @author:      zlz
 */
int8_t ECU_Diagnose_function(TMS570_BRAM_DATA *bram_rd_data,TMS570_BRAM_DATA *bram_wr_data,TMS570_BRAM_DATA *bram_tms570_data,ECU_ERROR_INFO err_info)
{
    int8_t ret;
    uint8_t tempbuff[24] ={0};

    bram_wr_data->buffer[0] = 0x99;/*电磁阀，瓶口阀输出*/
    usleep(2500000);/*延时2.5s*/

    memcpy(tempbuff,&bram_tms570_data->buffer[10],21);

    ret = H2_Fault_Judge(tempbuff);

    if(ret == 1)
    {
        bram_wr_data->buffer[0] = 0xaa;/*储氢系统故障，关闭电磁阀，瓶口阀输出*/
        return CODE_ERR;
    }
    if(err_info.commu_err)              /*CAN或MVB存在故障*/
    {
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
    uint8_t customization_key;
    /*FIXME:是否考虑A/B同时故障才不允许Start？*/
    /*条件是收到燃料电池启动指令且A/B堆无三级故障 且动力电池KM1 KM2投入 且FCU不存在二级或三级故障*/
    if((blvds_bram_rd_data->buffer[1] & 0x4) && !(blvds_bram_rd_data->buffer[1] & 0x40) && (((blvds_bram_rd_data->buffer[1]>>16) & 0x0440) == 0x0440) \
        && !(tms_bram_wr_data->buffer[8]&0xffff) && !(tms_bram_wr_data->buffer[22]&0xffff)) 
    {
        customization_key = 0x1;
        if(g_tms570_errflag == 1)
        {
            memcpy(tms_bram_wr_data->buffer[0],&customization_key,1);
        
        }

    }
    else
    {
        return CODE_ERR;
    }
}